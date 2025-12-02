/*
 * comuser.c
 *
 *  Created on: 2023
 *      Author: Standa
 */


#include "stm32g4xx_hal.h"
#include "error.h"
#include <stdint.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "commands.h"
#include "usbd_cdc_if.h"
#include "comuser.h"
#include "main.h"


#define COMU_RXBUFSIZE				1024								//Must be power of two because of some math magic
#define COMU_RXBUFTAILMASK			( COMU_RXBUFSIZE - 1 )
#define COMU_ENDOFCMDCHAR			'\n'

#define COMU_TXBUFSIZE				4096								//Must be power of two because of some math magic
#define COMU_TXBUFHEADMASK			( COMU_TXBUFSIZE - 1 )

#define COMU_SIG_CMDRECEIVED		0x01								//Complete command (\n) received via UART/USB
#define COMU_SIG_TRANSMIT 			0x02								//Data to transmit available in UART/USB bufer
#define COMU_SIG_CANLASTERROR		0x04								//CAN error interrupt occured - forward via UART/USB
#define COMU_SIG_CANMSGRECEIVED		0x08								//CAN msg received - forward it via UART/USB


static uint32_t StatusRegister = 0;
//static char EvtStr[256];

static uint8_t	comu_RxBuffer[COMU_RXBUFSIZE];
static uint32_t comu_RxHead = 0;
static uint32_t comu_RxTail = 0;
static int32_t comu_RxCmdCnt = 0;
static int32_t comu_HandledRxCmdCnt = 0;
static uint8_t comu_Cmd[COMU_RXBUFSIZE/2];

static uint8_t comu_TxBuffer[COMU_TXBUFSIZE];
static uint32_t comu_TxHead = 0;
static uint32_t comu_TxTail = 0;
static char comu_TmpStr[COMU_TXBUFSIZE/2];


/*
 * Initialize communication module - virtual serial port via USBC to communicate with user.
 */
void comu_Init(void){
	HAL_GPIO_WritePin(USB_DIS_GPIO_Port, USB_DIS_Pin, GPIO_PIN_SET);		// Disconnect the USBC pullup to reset connection
	HAL_Delay(5);															// Wait for short time so master can deinitialize the connection
	HAL_GPIO_WritePin(USB_DIS_GPIO_Port, USB_DIS_Pin, GPIO_PIN_RESET);		// Reconnect the USBC pullup to reinitialize the USBC connection
}


/*
 * Send data to user PC via virtual serial port.
 */
err_Td comu_SendF(char *format, ...){
	va_list va;
	int32_t i;

	va_start(va, format);													//Start reading of parameters
	vsnprintf(comu_TmpStr, sizeof(comu_TmpStr), format, va);				//Format new string and save its length
	va_end(va);																//End of reading parameters

	if( strlen(comu_TmpStr) < sizeof(comu_TmpStr) ){						//Check if formated string including terminating zero fits into buffer
		for( i=0; i<strlen(comu_TmpStr); i++ ){								//Copy byte by byte into tx buffer
			comu_TxBuffer[ comu_TxHead++ ] = comu_TmpStr[i];
			comu_TxHead &= COMU_TXBUFHEADMASK;
		}
		StatusRegister |= COMU_SIG_TRANSMIT;
		return err_Td_Ok;
	}
	else{
		return err_Td_Overflow;
	}
}


/*
 * Copy data from communication peripheral buffer into this module circular buffer where they can be parsed.
 * User must make sure only one way to access receive buffer is used (so only USART OR USB access in this case)
 */
err_Td comu_Receive(uint8_t* buf, uint32_t len){
	int32_t i;
	if( buf && len <= COMU_RXBUFSIZE ){
		for( i=0; i<len; i++ ){
			comu_RxBuffer[comu_RxHead++] = buf[i];
			comu_RxHead &= COMU_RXBUFTAILMASK;
			if( buf[i] == COMU_ENDOFCMDCHAR ){
				comu_RxCmdCnt++;											//Increment counter of received commands in buffer
				StatusRegister |= COMU_SIG_CMDRECEIVED;						//Set signal to receiving task
			}
		}
		return err_Td_Ok;
	}
	else if(len > COMU_RXBUFSIZE){
		return err_Td_Overflow;
	}
	else{
		return err_Td_Null;
	}
}


/*
 * Parsing of incoming messages.
 * When command is founf in buffer it is thrown into commands.c module where it is decoded
 * and if found also executed.
 */
static void HandleRecCmd(void){
	uint8_t tmp;
	uint32_t i = 0;

	while( ( ( tmp = comu_RxBuffer[ (comu_RxTail++) & COMU_RXBUFTAILMASK ] ) != COMU_ENDOFCMDCHAR ) ){
		if( i < ( sizeof(comu_Cmd) - 2 ) ){								//Take into account terminating zero
			comu_Cmd[i++] = tmp;										//Copy byte by byte to command buffer until end of command byte is found or command buffer is full
		}
		else{
			break;
		}
	}

	comu_Cmd[i++] = COMU_ENDOFCMDCHAR;
	comu_Cmd[i] = 0;
	cmd_Handle((char*)comu_Cmd);
}


/*
 * Transmits data from Tx circular buffer either in one part if data is linear or in two parts if data is broken by circular buffers.
 */
static uint32_t TxCnt;
static void TransmitTxBuffer(void){

	if( comu_TxHead != comu_TxTail ){												//If there is data to transfer
		if( comu_TxHead > comu_TxTail ){											//If all data in circular buffer is linear
			TxCnt = comu_TxHead - comu_TxTail;

			if( CDC_Transmit_FS(comu_TxBuffer + comu_TxTail, TxCnt ) != HAL_OK ){	//Transmit and if it failed
				StatusRegister |= COMU_SIG_TRANSMIT;								//Request repeated transmission
				return;
			}
			comu_TxTail = comu_TxHead;												//Move tail to head position (errors are ignored)
		}
		else if( comu_TxHead < comu_TxTail ){										//If data is separated by circular buffer end
			TxCnt = COMU_TXBUFSIZE - comu_TxTail;
			if( CDC_Transmit_FS(comu_TxBuffer + comu_TxTail, TxCnt ) != HAL_OK ){	//Transmit and if it failed
				StatusRegister |= COMU_SIG_TRANSMIT;								//Request repeated transmission
				return;
			}
			comu_TxTail = 0;														//Transmit first end of buffer data part and then linear part from beginning of circular buffer
			StatusRegister |= COMU_SIG_TRANSMIT;									//Request repeated transmission
		}
	}
}


/*
 * Handles all the task required by this module (UART and CAN transmission and reception).
 */
void comu_HandleCommunication(void){
	//uint32_t CalCRC, i;

	if( StatusRegister & COMU_SIG_TRANSMIT ){
		StatusRegister &= ~COMU_SIG_TRANSMIT;
		TransmitTxBuffer();
	}

	if( StatusRegister & COMU_SIG_CMDRECEIVED ){
		StatusRegister &= ~COMU_SIG_CMDRECEIVED;
		while( comu_RxCmdCnt > comu_HandledRxCmdCnt ){
			HandleRecCmd();
			comu_HandledRxCmdCnt++;
		}
	}


/* TODO toto se pak predela na odesilani zprav s informacemi z EMC labiny
	if( StatusRegister & COM_SIG_CANMSGRECEIVED ){
		StatusRegister &= ~COM_SIG_CANMSGRECEIVED;

		//[CRC] evt canrx [TimeStamp] [StdId_UINT12_HEX] [ExtId_UINT12_HEX] [DLC_UINT4_HEX] [Byte7_HEX] [Byte6_HEX] [Byte5_HEX] [Byte4_HEX] [Byte3_HEX] [Byte2_HEX] [Byte1_HEX] [Byte0_HEX]\r\n

		if(com_GetByteOrder()){		//If byte order mode is inverted
			i = snprintf(EvtStr, sizeof(EvtStr), "evt canrx\t%u\t%03x\t%08x\t%01x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t\r\n",
					(unsigned int)HAL_GetTick(),
					//(unsigned int)((IsExtIdUsed) ? (CANRxHeader.ExtId) : (CANRxHeader.StdId)), (unsigned int)(CANRxHeader.DLC),
					(unsigned int)(CANRxHeader.StdId), (unsigned int)(CANRxHeader.ExtId), (unsigned int)(CANRxHeader.DLC),
					(unsigned int)(CANRxData[7]), (unsigned int)(CANRxData[6]), (unsigned int)(CANRxData[5]), (unsigned int)(CANRxData[4]), (unsigned int)(CANRxData[3]), (unsigned int)(CANRxData[2]), (unsigned int)(CANRxData[1]), (unsigned int)(CANRxData[0]));
		}
		else{
			i = snprintf(EvtStr, sizeof(EvtStr), "evt canrx\t%u\t%03x\t%08x\t%01x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t%02x\t\r\n",
					(unsigned int)HAL_GetTick(),
					//(unsigned int)((IsExtIdUsed) ? (CANRxHeader.ExtId) : (CANRxHeader.StdId)), (unsigned int)(CANRxHeader.DLC),
					(unsigned int)(CANRxHeader.StdId), (unsigned int)(CANRxHeader.ExtId), (unsigned int)(CANRxHeader.DLC),
					(unsigned int)(CANRxData[0]), (unsigned int)(CANRxData[1]), (unsigned int)(CANRxData[2]), (unsigned int)(CANRxData[3]), (unsigned int)(CANRxData[4]), (unsigned int)(CANRxData[5]), (unsigned int)(CANRxData[6]), (unsigned int)(CANRxData[7]));
		}


		CalCRC = HAL_CRC_Calculate(&hcrc, (uint32_t*)(EvtStr), i);
		CalCRC = ~CalCRC;
		com_SendF("%05d %s", CalCRC & 0xFFFF, EvtStr);

	}
	*/
}

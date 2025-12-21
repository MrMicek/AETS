/*
 * commands.c
 *
 *  Created on: 8 Nov 2023
 *      Author: Standa
 */

#include "commands.h"
#include "comuser.h"
#include "error.h"
#include "utility.h"
#include "stdio.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "crc.h"
#include "appinfo.h"
#include "kvstore.h"
#include "eeprom.h"
#include "app_sm.h"
#include "io_control.h"
#include "relay_counter.h"
#include "relay.h"
#include "mosfet.h"
#include "mux.h"
#include "current.h"




#define HELP_LINE_1		"> CanToUsb help.\r\n"
#define HELP_LINE_2		"> Space or tab interchangeably used as separator for commands, responses and events. Variables are closed in [] brackets.\r\n"
#define HELP_LINE_3		"> Generic command syntax is: [CRC] [Cmd] [CmdId] [Param1] [Param2] ... [ParamN].\r\n"
#define HELP_LINE_3_1	">    Example: tx OFF 0A 8 0B 02 00 00 00 00 00 00. Please note that CRC might be optionally omitted.\r\n"
#define HELP_LINE_3_2	">    When text is used instead of CmdId, it is evaluated as zero. Helpful to tag commands.\r\n"
#define HELP_LINE_3_3	">    Some commands are followed by synchronous response: [CRC] cmd [Cmd] [CmdId] [Param1] [Param2] ... [ParamN].\r\n"
#define HELP_LINE_4		">    Each command is acknowledged: [CRC] ack [Cmd] [CmdId] [ErrorCode] [TimeStamp_ms].\r\n"
#define HELP_LINE_5		"> Events are asynchronous to command reception and acknowledge and may be received any time without any request.\r\n"
#define HELP_LINE_6		"> Transmit CAN message: [CRC] tx [CmdId] [MsgId_hex] [ByteCnt_hex] [B0_hex] [B1_hex] [B2_hex] [B3_hex] [B4_hex] [B5_hex] [B6_hex] [B7_hex].\r\n"
#define HELP_LINE_7		"> Received CAN message event: [CRC] evt canrx [TimeStamp_ms] [StdId_hex] [ExtId_hex] [ByteCnt_hex] [B0_hex] [B1_hex] [B2_hex] [B3_hex] [B4_hex] [B5_hex] [B6_hex] [B7_hex].\r\n"
#define HELP_LINE_7_1	"> Received CAN bus error event: [CRC] evt err [TimeStamp] [REC_hex] [TEC_hex] [LEC_hex] [BOFF_hex] [EPVF_hex] [EWGF_hex].\r\n"
#define HELP_LINE_7_2	">    REC, TEC: receive, transmit error counters. BOFF: bus off flag, EPVF: error passive flag, EWGF: error warning flag.\r\n"
#define HELP_LINE_7_3	">    LEC: last error code. 0 = no, 1 = stuff, 2 = form, 3 = ack, 4 = bit recessive, 5 = bit dominant, 6 = CRC, 7 = SW.\r\n"
#define HELP_LINE_8		"> Byte order might be changed for both tx command and canrx event: [CRC] so [CmdId] [Order: 0 = normal, 1 = inverted].\r\n"
#define HELP_LINE_9		">    Currently set byte order might be read: [CRC] go [CmdId]. Response: [CRC] go [CmdId] [Order].\r\n"
#define HELP_LINE_10	"> Baud rate configuration command: [CRC] sr [CmdId] [BR_kbps: 250, 500, 1000].\r\n"
#define HELP_LINE_11	">    Get current baud rate command: [CRC] gr [CmdId]. Response: [CRC] gr [CmdId] [BR_kbps].\r\n"
#define HELP_LINE_12	"> Standard or extended CAN ID can be configured for transmission: [CRC] st [CmdId] [isExtended: 0=normal, 1=extended].\r\n"
#define HELP_LINE_13	">    Get currently used CAN ID type for transmission: [CRC] gt [CmdId]. Response: [CRC] cmd gt [CmdId] [isExtended].\r\n"
#define HELP_LINE_13_1	"> Silent mode (no dominant bits transmitted) may be enabled (IsSilent = 1) or disabled (IsSilent = 0).\r\n"
#define HELP_LINE_13_2	">    Set silent mode: [CRC] ss [CmdId] [IsSilent] [ResponseWait_ms].\r\n"
#define HELP_LINE_13_3	">    If ResponseWait_ms = 0 silent mode is disabled permanently after first requested transmission.\r\n"
#define HELP_LINE_13_4	">    If ResponseWait_ms = n (1 to 60 000) silent mode is disabled only for n ms after all next requested transmissions.\r\n"
#define HELP_LINE_14	"> Enjoy the tool.\r\n"


typedef struct{
        char* Name;
        err_Td (*CallbackFn)(char *cmdName, int32_t cmdId);
}CmdTd;

static const char CMD_DELIMS[] = " \r\n\t";

static char *cmd_saveptr = NULL;
static char *cmd_seed = NULL;

static void cmd_prepare_tokens(char *seed, char *continuation, bool include_seed)
{
	cmd_seed = include_seed ? seed : NULL;
	cmd_saveptr = continuation;
}

static char *cmd_next_token(void)
{
	if (cmd_seed != NULL) {
		char *first = cmd_seed;
		cmd_seed = NULL;
		return first;
}
	if (cmd_saveptr == NULL) {
		return NULL;
}
	return strtok_r(NULL, CMD_DELIMS, &cmd_saveptr);
}


/*
 * Get information about the instrument.
 * Syntax: [CRC] [CmdName] [CmdId]\r\n
 * Response: [CRC] cmd [CmdName] [CmdId] [Company] [Author] [Device] [HWVer] [FWVer] [Id] [CalDate]\r\n
 */
static err_Td GetInfoCb(char *cmdName, int32_t cmdId){
	comu_SendF("0 cmd %s %d %s %s %s %s %s %s %s\r\n", cmdName, cmdId, APPINFO_COMPANY, APPINFO_AUTHOR, APPINFO_DEVICE, APPINFO_HWVER, APPINFO_FWVER, APPINFO_ID, APPINFO_CALDATE);
	return err_Td_Ok;
}


/*
 * Get help file.
 * Syntax: [CRC] [CmdName] [CmdId]\r\n
 * Response: series of separate lines will deliver the content of help (standard ack will indicate end).
 */
static err_Td GetHelpCb(char *cmdName, int32_t cmdId){
	comu_SendF(HELP_LINE_1);
	comu_SendF(HELP_LINE_2);
	comu_SendF(HELP_LINE_3);
	comu_SendF(HELP_LINE_3_1);
	comu_SendF(HELP_LINE_3_2);
	comu_SendF(HELP_LINE_3_3);
	comu_SendF(HELP_LINE_4);
	comu_SendF(HELP_LINE_5);
	comu_SendF(HELP_LINE_6);
	comu_SendF(HELP_LINE_7);
	comu_SendF(HELP_LINE_7_1);
	comu_SendF(HELP_LINE_7_2);
	comu_SendF(HELP_LINE_7_3);
	comu_SendF(HELP_LINE_8);
	comu_SendF(HELP_LINE_9);
	comu_SendF(HELP_LINE_10);
	comu_SendF(HELP_LINE_11);
	comu_SendF(HELP_LINE_12);
	comu_SendF(HELP_LINE_13);
	comu_SendF(HELP_LINE_13_1);
	comu_SendF(HELP_LINE_13_2);
	comu_SendF(HELP_LINE_13_3);
	comu_SendF(HELP_LINE_13_4);
	comu_SendF(HELP_LINE_14);
	return err_Td_Ok;
}


static const char* app_state_to_str(app_state_t st)
{
        switch (st) {
        case APP_STATE_BOOT: return "BOOT";
        case APP_STATE_INIT: return "INIT";
        case APP_STATE_MANUAL: return "MANUAL";
        case APP_STATE_REMOTE: return "REMOTE";
        case APP_STATE_TEST: return "TEST";
        case APP_STATE_FAULT: return "FAULT";
        default: return "?";
        }
}

static err_Td ModeCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

if (strcmp(action, "get") == 0) {
app_status_t st = app_get_status();
comu_SendF("0 cmd %s %d %s\r\n", cmdName, cmdId, app_state_to_str(st.state));
                return err_Td_Ok;
        }

if (strcmp(action, "set") == 0) {
char *mode = cmd_next_token();
if (!mode) return err_Td_Param;
app_event_t evt = {0};
                if (strcmp(mode, "manual") == 0) {
                        evt.type = APP_EVT_CMD_MODE_MANUAL;
                } else if (strcmp(mode, "remote") == 0) {
                        evt.type = APP_EVT_CMD_MODE_REMOTE;
                } else if (strcmp(mode, "test") == 0) {
                        evt.type = APP_EVT_CMD_MODE_TEST;
                } else {
                        return err_Td_NotValid;
                }
                return app_post_event(evt) ? err_Td_Ok : err_Td_Busy;
        }

        return err_Td_NotValid;
}

static err_Td RelayCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

const io_state_t *cur = io_get();
io_state_t next = *cur;

if (strcmp(action, "set") == 0) {
char *idx_s = cmd_next_token();
char *val_s = cmd_next_token();
if (!idx_s || !val_s) return err_Td_Param;
int idx = atoi(idx_s);
if (idx < 1 || idx > 4) return err_Td_Range;
next.relays[idx - 1] = (atoi(val_s) != 0);
io_apply(&next);
return err_Td_Ok;
} else if (strcmp(action, "get") == 0) {
char *idx_s = cmd_next_token();
if (idx_s) {
int idx = atoi(idx_s);
if (idx < 1 || idx > 4) return err_Td_Range;
comu_SendF("0 cmd %s %d %d %d\r\n", cmdName, cmdId, idx, cur->relays[idx - 1] ? 1 : 0);
                } else {
                        comu_SendF("0 cmd %s %d %d %d %d %d\r\n", cmdName, cmdId,
                                        cur->relays[0] ? 1 : 0,
                                        cur->relays[1] ? 1 : 0,
                                        cur->relays[2] ? 1 : 0,
                                        cur->relays[3] ? 1 : 0);
                }
return err_Td_Ok;
} else if (strcmp(action, "all") == 0) {
for (int i = 0; i < 4; ++i) {
char *val_s = cmd_next_token();
if (!val_s) return err_Td_Param;
next.relays[i] = (atoi(val_s) != 0);
}
io_apply(&next);
                return err_Td_Ok;
        }

        return err_Td_NotValid;
}

static err_Td MosfetCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

const io_state_t *cur = io_get();
io_state_t next = *cur;

if (strcmp(action, "set") == 0) {
char *idx_s = cmd_next_token();
char *val_s = cmd_next_token();
                if (!idx_s || !val_s) return err_Td_Param;
                int idx = atoi(idx_s);
                if (idx < 1 || idx > 2) return err_Td_Range;
                next.mosfet[idx - 1] = (atoi(val_s) != 0);
                io_apply(&next);
                return err_Td_Ok;
        } else if (strcmp(action, "get") == 0) {
                comu_SendF("0 cmd %s %d %d %d\r\n", cmdName, cmdId, cur->mosfet[0] ? 1 : 0, cur->mosfet[1] ? 1 : 0);
                return err_Td_Ok;
        }
        return err_Td_NotValid;
}

static err_Td MuxCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

        const io_state_t *cur = io_get();
        io_state_t next = *cur;

if (strcmp(action, "set") == 0) {
char *sel_s = cmd_next_token();
                if (!sel_s) return err_Td_Param;
                if (strcmp(sel_s, "int") == 0) {
                        next.mux = MUX_INT;
                } else if (strcmp(sel_s, "ext") == 0) {
                        next.mux = MUX_EXT;
                } else {
                        int val = atoi(sel_s);
                        if (val == 0) next.mux = MUX_EXT;
                        else if (val == 1) next.mux = MUX_INT;
                        else return err_Td_Range;
                }
                io_apply(&next);
                return err_Td_Ok;
        } else if (strcmp(action, "get") == 0) {
                comu_SendF("0 cmd %s %d %d\r\n", cmdName, cmdId, (cur->mux == MUX_INT) ? 1 : 0);
                return err_Td_Ok;
        }
        return err_Td_NotValid;
}

static err_Td CurrentCmdCb(char *cmdName, int32_t cmdId)
{
char *mode = cmd_next_token();
char *ch_s = cmd_next_token();
        if (!mode || !ch_s) return err_Td_Param;
        int idx = atoi(ch_s);
        if (idx < 1 || idx > 4) return err_Td_Range;
        current_chTd ch = (current_chTd)(idx - 1);

        if (strcmp(mode, "raw") == 0) {
                uint16_t raw = Current_ReadRaw(ch);
                comu_SendF("0 cmd %s %d %d %u\r\n", cmdName, cmdId, idx, (unsigned)raw);
                return (raw == 0xFFFFU) ? err_Td_General : err_Td_Ok;
        } else if (strcmp(mode, "ma") == 0) {
                uint32_t ma = Current_Read_mA(ch);
                comu_SendF("0 cmd %s %d %d %lu\r\n", cmdName, cmdId, idx, (unsigned long)ma);
                return err_Td_Ok;
        }
        return err_Td_NotValid;
}

static err_Td RelayCountCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

        if (strcmp(action, "get") == 0) {
char *idx_s = cmd_next_token();
                if (idx_s) {
                        int idx = atoi(idx_s);
                        if (idx < 1 || idx > 4) return err_Td_Range;
                        comu_SendF("0 cmd %s %d %d %llu\r\n", cmdName, cmdId, idx, (unsigned long long)relay_counter_get((uint8_t)(idx - 1)));
                } else {
                        uint64_t counts[4] = {0};
                        relay_counter_get_all(counts, 4);
                        comu_SendF("0 cmd %s %d %llu %llu %llu %llu\r\n", cmdName, cmdId,
                                        (unsigned long long)counts[0],
                                        (unsigned long long)counts[1],
                                        (unsigned long long)counts[2],
                                        (unsigned long long)counts[3]);
                }
                return err_Td_Ok;
        } else if (strcmp(action, "reset") == 0) {
                relay_counter_reset();
                return err_Td_Ok;
        }
        return err_Td_NotValid;
}

static err_Td TestCmdCb(char *cmdName, int32_t cmdId)
{
char *action = cmd_next_token();
if (!action) return err_Td_Param;

        if (strcmp(action, "start") == 0) {
                return app_post_event((app_event_t){ .type = APP_EVT_TEST_START }) ? err_Td_Ok : err_Td_Busy;
        } else if (strcmp(action, "stop") == 0) {
                return app_post_event((app_event_t){ .type = APP_EVT_TEST_STOP }) ? err_Td_Ok : err_Td_Busy;
        } else if (strcmp(action, "status") == 0) {
                app_status_t st = app_get_status();
                comu_SendF("0 cmd %s %d %s %d %lu\r\n", cmdName, cmdId, app_state_to_str(st.state), (int)st.test_state, (unsigned long)st.fault_code);
                return err_Td_Ok;
        }
        return err_Td_NotValid;
}


/*
 * Send the message via CAN interface. All the parameters from MsgId forward are in hexadecimal form:
 * An optional prefix indicating octal or hexadecimal base ("0" or "0x"/"0X" respectively)
 * Syntax B0 to B7 might be inverted using SetByteOrder command to B7 to B0.
 * Syntax: [CRC] [CmdName] [CmdId] [MsgId] [ByteCnt] [B0] [B1] [B2] [B3] [B4] [B5] [B6] [B7]\r\n
 * Response: none

static err_Td TransmitCb(char *cmdName, int32_t cmdId){
	uint32_t i, Id, ByteCnt, TxMailbox;
	uint8_t Data[8];
	CAN_TxHeaderTypeDef TxHeader = {0};

	Id = strtol(strtok(0, " \r"), 0, 16);
	ByteCnt = strtol(strtok(0, " \r"), 0, 16);
	for( i=0; i<UT_SIZEOFARRAY(Data); i++){

		if(com_GetByteOrder()){						//If inverted syntax is requested
			Data[UT_SIZEOFARRAY(Data)-1-i] = strtol(strtok(0, " \r"), 0, 16);
		}
		else{										//Else if normal syntax is requested
			Data[i] = strtol(strtok(0, " \r"), 0, 16);
		}

	}


	if(com_IsExtIdUsed()){
		TxHeader.StdId = 0x00;
		TxHeader.ExtId = Id;
		TxHeader.IDE = CAN_ID_EXT;
	}
	else{
		TxHeader.StdId = Id;
		TxHeader.ExtId = 0x00;
		TxHeader.IDE = CAN_ID_STD;
	}


	TxHeader.RTR = CAN_RTR_DATA;

	TxHeader.DLC = ByteCnt;
	TxHeader.TransmitGlobalTime = DISABLE;



	uint8_t IsSil = com_IsSilent();		//Check if silent mode is configured (transmission not possible, must be changed to normal mode)
	if(IsSil){
		com_SetCANSpeedAndMode(com_GetBaudrate(), 0, com_GetResponseWaitMs());	//Configure normal mode
		HAL_Delay(2);
	}

	HAL_StatusTypeDef ErrCode = HAL_CAN_AddTxMessage(&hcan, &TxHeader, Data, &TxMailbox);


	if(IsSil){
		com_SetSilentAsync();//Set silent mode in defined number of ms.
	}


	if( ErrCode == HAL_OK){
		return err_Td_Ok;
	}
	else{
		return err_Td_General;
	}
}
 */


/*
 * Sets CAN baud rate. Possible values: 250, 500, 1000 kbps.
 * Syntax: [CRC] [CmdName] [CmdId] [BR_kbps]\r\n
 * Response: none

static err_Td SetBaudRateCb(char *cmdName, int32_t cmdId){
	uint32_t BR;
	BR = strtol(strtok(0, " \r"), 0, 10);
	return com_SetCANSpeedAndMode(BR, com_IsSilent(), com_GetResponseWaitMs());
}
 */


/*
 * Get CAN baud rate. Possible values: 250, 500, 1000 kbps or 0 in case of initialization failure (invalid parameter etc).
 * Syntax: [CRC] [CmdName] [CmdId]\r\n
 * Response: [CRC] cmd [CmdName] [CmdId] [BR_kbps]\r\n

static err_Td GetBaudRateCb(char *cmdName, int32_t cmdId){
	com_SendF("0 cmd %s %d %d\r\n", cmdName, cmdId, com_GetBaudrate());
	return err_Td_Ok;
}
 */



/*
 * Sets, resets or blinks defined led diode.
 * Syntax: [CRC] [CmdName] [CmdId] [LedIndex] [StateOrPeriodInMs]\r\n
 * Response: none
*/
//static err_Td LedSetCb(char *cmdName, int32_t cmdId){
//	uint32_t LedIndex = strtol(strtok(0, " \r"), 0, 10);
//	uint32_t StateOrPeriodInMs = strtol(strtok(0, " \r"), 0, 10);
//
//	if(LedIndex<0 || LedIndex >= LEDS_HANDLECOUNT){
//		return err_Td_Range;
//	}
//
//	if(StateOrPeriodInMs == 0){
//		leds_Reset(leds_Array[LedIndex]);
//	}
//	else if(StateOrPeriodInMs == 1){
//		leds_Set(leds_Array[LedIndex]);
//	}
//	else{
//		leds_Blink(leds_Array[LedIndex], StateOrPeriodInMs);
//	}
//	return err_Td_Ok;
//}




/*
 * Debug command
 * Syntax: [CRC] [CmdName] [CmdId] [intparam1] [intparam2]\r\n
 * Response: none
*/
//static err_Td DbgCb(char *cmdName, int32_t cmdId){
//	int32_t IntParam1 = strtol(strtok(0, " \r"), 0, 10);
//	int32_t IntParam2 = strtol(strtok(0, " \r"), 0, 10);
//
//
//	if(IntParam1 == 0){	// RESET pin of ftdi
//		if(IntParam2 == 0){
//			HAL_GPIO_WritePin(FTDI_RESET_GPIO_Port, FTDI_RESET_Pin, GPIO_PIN_RESET);
//		}
//		else {
//			HAL_GPIO_WritePin(FTDI_RESET_GPIO_Port, FTDI_RESET_Pin, GPIO_PIN_SET);
//		}
//	}
//	else if(IntParam1 == 1){	// ENABLE pin of ftdi
//		if(IntParam2 == 0){
//			HAL_GPIO_WritePin(USART_EN_GPIO_Port, USART_EN_Pin, GPIO_PIN_RESET);
//		}
//		else {
//			HAL_GPIO_WritePin(USART_EN_GPIO_Port, USART_EN_Pin, GPIO_PIN_SET);
//		}
//	}
//
//	return err_Td_Ok;
//}


/*
 * List of all available commands. Syntax is specified in each callback function separately (also in toltip).
 */
static CmdTd CmdList[] = {
        {"gi", GetInfoCb},
        {"gh", GetHelpCb},
        {"mode", ModeCmdCb},
        {"relay", RelayCmdCb},
        {"mosfet", MosfetCmdCb},
        {"mux", MuxCmdCb},
        {"curr", CurrentCmdCb},
        {"rcnt", RelayCountCmdCb},
        {"test", TestCmdCb},
        //{"dbg", DbgCb},
        //{"gr", GetBaudRateCb},
};


/*	Command parsing core, expects commands in following order:
 *	CRC Name Id Param1 Param2 ... ParamN\r\n
 */
void cmd_Handle(char *str){
        char *Name, AckStr[64];
        uint32_t i = 0, RecCRC = 0, CalCRC = 0, Id = 0, ErrNo = err_Td_Ok;
        bool IsNumber;
        char *save = NULL;

        Name = strtok_r(str, CMD_DELIMS, &save);                                                        // Read name from beginning of command (or it could be CRC if it is number)
        if (Name == NULL) {
                return;                                                                         // Empty line
        }
        RecCRC = atoi(Name);                                                               // Try to convert cmd name to number

        IsNumber = true;                                                                   // Here we need to check whether returned zero is because CRC is zero or because CRC is not used and we tried to convert cmd name to number
        for( i=0; i<strlen(Name); i++ ){                                                   // Check whole command name (or it could be CRC)
                if(!isdigit((unsigned char)Name[i])){                                   // If any letter does not contain number
                        IsNumber = false;                                                    // Set flag that we have command name and not CRC number
                        break;                                                               // No need to continue check
                }
        }

        if( IsNumber ){                                                                    // If IsNumber flag is set we have found RecCRC and name will follow in next token.
                i = strlen(Name);                                                          // Length of CRC
                CalCRC = HAL_CRC_Calculate(&hcrc, (uint32_t*)(Name + i + 1), strlen(Name + i + 1)); //Calculate CRC starting with CmdName and ending with \n
                CalCRC = ~CalCRC;
                Name = strtok_r(NULL, CMD_DELIMS, &save);                                      // Get command name
                if( Name == NULL){
                        ErrNo = err_Td_Param;                                                 // Command missing after CRC
                }
                else if( RecCRC != (CalCRC & 0xFFFF)){                                     // Compare received and calculated CRCs
                        ErrNo = err_Td_CRC;                                                   // If it doesnt match return error
                }
        }

        char *IdToken = strtok_r(NULL, CMD_DELIMS, &save);
        if (IdToken && IdToken[0] != '\0') {
                bool id_is_number = true;
                for (i = 0; i < strlen(IdToken); i++){
                        if(!isdigit((unsigned char)IdToken[i])){
                                id_is_number = false;
                                break;
                        }
                }
                if (id_is_number){
                        Id = atoi(IdToken);                                                  // Get command ID
                        cmd_prepare_tokens(NULL, save, false);                               // Continue from remaining buffer
                } else {
                        Id = 0;                                                              // Default ID when omitted
                        cmd_prepare_tokens(IdToken, save, true);                             // Treat token as first parameter
                }
        } else {
                Id = 0;                                                                // Default ID when omitted completely
                cmd_prepare_tokens(NULL, save, false);
        }

        if( ErrNo == err_Td_Ok && Name != NULL ){                                  // If there is no error in CRC, parse rest of command
                ErrNo = err_Td_NotExist;
                for( i=0; i<UT_SIZEOFARRAY(CmdList); i++ ){                             // Repeat through whole list of defined commands
                        if( strcmp(CmdList[i].Name, Name ) == 0 ){                      // If command name from defined list is equal to current command name
                                if( CmdList[i].CallbackFn != 0 ){
                                        ErrNo = CmdList[i].CallbackFn(Name, Id);             // Call callback function to parse remaining data
                                }
                                break;                                                       // Break lookup cycle if command has been found
                        }
                }
        }

        if (Name == NULL){
                Name = "?";
        }

        i = snprintf(AckStr, sizeof(AckStr), "ack %s %u %u %u\r\n", Name, (unsigned)Id, (unsigned)ErrNo, (unsigned)HAL_GetTick());      //Send ack with errorcode
        CalCRC = HAL_CRC_Calculate(&hcrc, (uint32_t*)(AckStr), i);
        CalCRC = ~CalCRC;
        comu_SendF("%05d %s", CalCRC & 0xFFFF, AckStr);
}



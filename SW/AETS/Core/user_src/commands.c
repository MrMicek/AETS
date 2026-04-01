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
#include "app_params.h"
#include "app_menu.h"
#include "profile_store.h"
#include "buzzer.h"
#include "relay_health_store.h"



#define HELP_LINE_SPACE "\r\n"
#define HELP_LINE_SEPARATOR "----------------------------------------\r\n"
#define HELP_LINE_ERROR1 "> 4. Error Codes:\r\n> 0 = Success / no error\r\n> 1 = Generic failure (used e.g., when a low-level call fails).\r\n> 2 = Missing or invalid parameter (parsing error).\r\n> 3 = ACK error (defined but not used in command parsing logic).\r\n> 4 = NACK error (defined but not used in command parsing logic).\r\n> 5 = Initialization error (defined; not directly returned in command parsing).\r\n> 6 = Busy / cannot accept command now (e.g., event queue full).\r\n"
#define HELP_LINE_ERROR2 "> 7 = Timeout (defined; not directly returned in command parsing).\r\n> 8 = Saturation / overflow condition (defined; not directly used).\r\n> 9 = Parameter out of range (used in index checks).\r\n> 10 = Command not found (set before dispatch when name not matched).\r\n> 11 = NULL pointer or invalid pointer (defined; not directly used in command parsing).\r\n> 12 = Not implemented (defined; not directly used).\r\n> 13 = Resource not found (used, e.g., load failure).\r\n"
#define HELP_LINE_ERROR3 "> 14 = Not supported (defined; not directly used).\r\n> 15 = Unsupported/invalid command or action (used in command handlers).\r\n> 16 = Overflow condition (defined; not directly used).\r\n> 17 = CRC mismatch in command (set during parsing).\r\n> 18 = Command disabled (e.g., blocked unless in REMOTE).\r\n>"

#define HELP_LINE_ACK "> 3. Standard Response:\r\n> ack <cmdName> <cmdId> <errorCode> <timestamp>\r\n"

#define HELP_LINE_1		"> AETS help.\r\n"

#define HELP_LINE_2		"> INPUT COMMANDS:\r\n"

#define HELP_LINE_3		"> 1. HELP/INFO Commands: \r\n"
#define HELP_LINE_3_1	"> - gh (help)(Expect: multi-line help text, then ""ack gh <cmdId> 0 <timestamp>"".)\r\n"
#define HELP_LINE_3_2	"> - gi (device info)(Expect: ""cmd gi <cmdId> <device info...>"" then ""ack gi <cmdId> 0 <timestamp>"".)\r\n"

#define HELP_LINE_3_3	"> 2. Remote-mode gating: \r\n"
#define HELP_LINE_4		"> - When not in REMOTE mode, only the ""mode"" command or ""test stop"" command is accepted. Other commands should return ack error code 18 (disabled).\r\n"
#define HELP_LINE_5		"> - Use the menu Connectivity -> Remote Mode to enter REMOTE, or ""mode set remote"" if allowed.\r\n"

#define HELP_LINE_6		"> 5. Mode Control Commands: \r\n"
#define HELP_LINE_7		"> - mode get (return current mode)(Expect: ""cmd mode <cmdId> <state>"" then ack.)\r\n"
#define HELP_LINE_7_1	"> - mode set [<manual|remote|test>] (set current mode) (params: 1. mode) (Expect: ack with error 0 if transition allowed; a later ""evt state <state>"" event.)\r\n"

// Upravené a nové řádky pro sekci 7.2 RELAY Commands
#define HELP_LINE_7_2	"> 6. RELAY Commands: \r\n"
#define HELP_LINE_7_3	"> - relay get (returns current state of relays 1-4)(Expect: ""cmd relay <cmdId> r1 r2 r3 r4"" then ack.)\r\n"
#define HELP_LINE_7_4	"> - relay set [<1-4>] [<0|1>] (params: 1. relay channel, 2. state for relay [0 = OFF, 1 = ON]) (Expect: ""ack relay <cmdId> 0 <timestamp>"")\r\n"
#define HELP_LINE_8		"> - rcnt get [<1-4>] (params: 1. relay channel [optional]) (Expect: counters for all or single relay.)\r\n"
#define HELP_LINE_8_1	"> - rcnt set [<1-4>] [<value>] (params: 1. relay channel, 2. value in k-cycles) (Sets remaining cycles for specific relay.)\r\n"
#define HELP_LINE_8_2	"> - rcnt reset (Resets all relay counters to default values.)\r\n"
#define HELP_LINE_8_3	"> - rcnt save (Saves current counter values to flash memory.)\r\n"
#define HELP_LINE_9		"> - curr ma  [<1-4>] (return current measurement in mA on selected channel) (params: 1. relay channel) (Expect: ""cmd current <cmdId> <ma>"" (or equivalent) then ack.)\r\n"

#define HELP_LINE_10	"> 7. MOSFET & MUX Commands:\r\n"
#define HELP_LINE_11	"> - mosfet get (return state of mosfets m1 and m2) (Expect: ""cmd mosfet <cmdId> m1 m2"" then ack.)\r\n"
#define HELP_LINE_11_1	"> - mosfet set [<1|2>] [<0|1>] (params: 1. FET channel, 2. state for FET [0 = OFF, 1 = ON]) (Expect: ""ack mosfet <cmdId> 0 <timestamp>"")\r\n"
#define HELP_LINE_12	"> - mux get (returns state of the multiplexers. Can be in states INTERNAL or EXTERNAL) (Expect: ""cmd mux <cmdId> <mux1> <mux2>"" with 0=INT, 1=EXT.)\r\n"
#define HELP_LINE_13	"> - mux set [<1|2>] [<0|1>] (params: 1. mux channel, 2. state for mux [0 = INT, 1 = EXT]) (Expect: selected mux channel switches (Indicated by LED).)\r\n"

#define HELP_LINE_13_1	"> 8. TEST CONTROL Commands: \r\n"
#define HELP_LINE_13_2	"> - test start current (starts test according to current settings) (Expect: ack; ""evt state TEST""; outputs may follow test sequence set by current settings.)\r\n"
#define HELP_LINE_13_3	"> - test start profile [<1-6>] (starts test according to selected profile settings) (params: 1. profile number) (Expect: ack; ""evt state TEST""; outputs may follow test sequence set by selected profile settings.)\r\n"
#define HELP_LINE_13_4	"> - test stop (stop running test) (Expect: ack; ""evt state REMOTE"" (if stopped remotely, if stopped manually expect: ""evt state MANUAL"")\r\n"

#define HELP_LINE_13_5  "> 9. PARAMETER SETTING Commands: \r\n"
#define HELP_LINE_13_6  "> - param relay get [<1-4>] (params: 1. relay channel) (Expect: one line per relay (or single relay) in the form ""cmd param <cmdId> relay <id> <enable> <ton_ms> <toff_ms> <imax_ma> <sw_count_k>"".)\r\n"
#define HELP_LINE_13_7  "> - param relay set [<1-4>] [<0|1>] [<ton_ms>] [<toff_ms>] [<imax_ma>] [<sw_count_k>] (params: 1. relay channel, 2. enable, 3. On time [ms], 4. Off time [ms], 5. Maximum current [mA], 6. Number of cycles in thousands) (Expect: ack)\r\n"
#define HELP_LINE_13_8  "> - param mosfet get [<1|2>] (params: 1. relay channel) (Expect: ""cmd param <cmdId> mosfet <id> <enable> <ext> <ton_ms> <toff_ms> <sw_count>"".)\r\n"
#define HELP_LINE_13_9  "> - param mosfet set [<1-2>] [<0|1>] [<0|1>] [<ton_ms>] [<toff_ms>] [<sw_count>] (params: 1. Mosfet channel, 2. enable, 3. Internal/External Control , 4. On time [ms], 5. Off time [ms], 6. Number of cycles) (Expect: ack; ext=1 forces mux to EXT and MOSFET output off.)\r\n"
#define HELP_LINE_14	"> - param trigger get (Expect: ""cmd param <cmdId> trigger <enable> <Relay channel>"".)\r\n"
#define HELP_LINE_15	"> - param trigger set [<0|1>] [<1-4>] (params: 1. Enable, 2. Relay channel) (Expect: ack; channel must be 1-4.)\r\n"
#define HELP_LINE_16	"> - param conn get (Expect: ""cmd param <cmdId> conn <remote mode> <can_output> <usb_output> <comm_period>"".)\r\n"
#define HELP_LINE_17	"> - param conn set [<0|1>] [<0|1>] [<0|1>] [<period_ms>] (params: 1. Enable Remote Mode,, 2. Enable CAN Output, 3. Enable USB Output 4. telemetry output period) (Expect: ack; settings reflected in menu.) \r\n"
#define HELP_LINE_18	"> - param buzzer get (Expect: ""cmd param <cmdId> buzzer <enable>"".) \r\n"
#define HELP_LINE_19	"> - param buzzer set [<0|1>] (params: 1. buzzer Enable) (Expect: ack; menu should reflect new value.)\r\n"
#define HELP_LINE_18_2	"> - param monitor get (Expect: ""cmd param <cmdId> monitor <enable>"".) \r\n"
#define HELP_LINE_18_3	"> - param monitor set [<0|1>] (params: 1. current monitoring Enable) (Expect: ack; menu should reflect new value.)\r\n"
#define HELP_LINE_19_1	"> - param reset (Resets all the current parameters to their default values) (Expect: ack; menu should reflect new value.)\r\n"
#define HELP_LINE_19_2	"> - param list profile [<1-6>] (params: 1. profile number) (List every parameter from selected profile) (Expect: ack)\r\n"

#define HELP_LINE_20	"> 9.1 DEFAULT BUZZER/CONN/TRIGGER PARAMETERS: .\r\n"
#define HELP_LINE_21	"> .buzzer_enable = 1, .connectivity = {.enable = 0,.can_enable = 0,.usb_enable = 0,},.trigger = {.enable = 0,.channel = 1,} .\r\n"

#define HELP_LINE_34	"> 10. PROFILE MANAGEMENT Commands: \r\n"
#define HELP_LINE_35	"> - save profile [<1-6>] (Save current parameters to flash profile) (Expect: ack)\r\n"
#define HELP_LINE_36	"> - load profile [<1-6>] (Load parameters from flash profile and apply) (Expect: ack)\r\n"

#define HELP_LINE_22	"> USB OUTPUT DESCRIPTION:\r\n"
#define HELP_LINE_23	"> - Test blocked. Not enough switches left. Relay: x (Relay doesnt have enough available switches for test, x represents the channel number )\r\n"
#define HELP_LINE_24	"> - out <timestamp> r1 <remaining_k> <state> <current_ma> r2 <remaining_k> <state> <current_ma> r3 <remaining_k> <state> <current_ma> r4 <remaining_k> <state> <current_ma> m1 <state> m2 <state> (Output from Running Test).\r\n"
#define HELP_LINE_25	"> - Test Stopped (test stopped manually or remotely)\r\n"
#define HELP_LINE_26	"> - Test Fail: Zero Current (Maximum steps with 0 current measured was exceeded)\r\n"
#define HELP_LINE_27	"> - Test Fail: OverCurrent (Maximum steps with current above maximum threshold was exceeded)\r\n"
#define HELP_LINE_28	"> - Test Done (Test finnished succesfully).\r\n"

#define HELP_LINE_29	"> CAN OUTPUT DESCRIPTION:\r\n"
#define HELP_LINE_30	"> Frame 1 (ID = 49):  \r\n> 0 .. 12 : 	Current CH1 (13 bits) - Relay 1 current in mA clamped to 13 bits.\r\n> 13 .. 24 : 	Relay1 Count-k (12 bits) - Remaining switch count for Relay 1 in kilo-units.\r\n> 25 : 		Relay1 State (1 bit) - Relay 1 output state [ 0 = OFF ; 1 = ON]. \r\n> 26 : 		Relay1 Alive (1 bit) - Relay 1 health monitor [ 0 = remaining count <= 0 ; 1 = remaining count > 0]. \r\n> 27 .. 39 : 	Current CH2 (13 bits) - Relay 2 current in mA clamped to 13 bits.\r\n> 40 .. 51 : 	Relay2 Count-k (12 bits) - Remaining switch count for Relay 2 in kilo-units.\r\n"
#define HELP_LINE_31	"> 52 : 		Relay2 State (1 bit) - Relay 2 output state [ 0 = OFF ; 1 = ON]. \r\n> 53 : 		Relay2 Alive (1 bit) - Relay 1 health monitor [ 0 = remaining count <= 0 ; 1 = remaining count > 0].\r\n> 54 : 		MUX1 State (1 bit) - [0 = INT, 1 = EXT] \r\n> 55 : 		MOSFET1 State (1 bit) - MOSFET 1 output state [ 0 = OFF ; 1 = ON]. \r\n> 56 .. 63 : 	Up-Counter (8 bit) - 8-bit counter incremented every CAN frame \r\n"
#define HELP_LINE_32	"> Frame 2 (ID = 50):  \r\n> 0 .. 12 : 	Current CH3 (13 bits) - Relay 3 current in mA clamped to 13 bits.\r\n> 13 .. 24 : 	Relay3 Count-k (12 bits) - Remaining switch count for Relay 3 in kilo-units.\r\n> 25 : 		Relay3 State (1 bit) - Relay 3 output state [ 0 = OFF ; 1 = ON]. \r\n> 26 : 		Relay3 Alive (1 bit) - Relay 3 health monitor [ 0 = remaining count <= 0 ; 1 = remaining count > 0]. \r\n> 27 .. 39 : 	Current CH4 (13 bits) - Relay 4 current in mA clamped to 13 bits.\r\n> 40 .. 51 : 	Relay4 Count-k (12 bits) - Remaining switch count for Relay 4 in kilo-units.\r\n"
#define HELP_LINE_33	"> 52 : 		Relay4 State (1 bit) - Relay 4 output state [ 0 = OFF ; 1 = ON]. \r\n> 53 : 		Relay4 Alive (1 bit) - Relay 4 health monitor [ 0 = remaining count <= 0 ; 1 = remaining count > 0].\r\n> 54 : 		MUX2 State (1 bit) - [0 = INT, 1 = EXT] \r\n> 55 : 		MOSFET2 State (1 bit) - MOSFET 2 output state [ 0 = OFF ; 1 = ON]. \r\n> 56 .. 63 : 	Up-Counter (8 bit) - 8-bit counter incremented every CAN frame \r\n"

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
	comu_SendF("cmd %s %d %s %s %s %s %s %s %s\r\n", cmdName, cmdId, APPINFO_COMPANY, APPINFO_AUTHOR, APPINFO_DEVICE, APPINFO_HWVER, APPINFO_FWVER, APPINFO_ID, APPINFO_CALDATE);
	return err_Td_Ok;
}


/*
 * Get help file.
 * Syntax: [CRC] [CmdName] [CmdId]\r\n
 * Response: series of separate lines will deliver the content of help (standard ack will indicate end).
 */
static err_Td GetHelpCb(char *cmdName, int32_t cmdId){
	comu_SendF(HELP_LINE_1);
	comu_SendF(HELP_LINE_SEPARATOR);
	comu_SendF(HELP_LINE_2);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_3);
	comu_SendF(HELP_LINE_3_1);
	comu_SendF(HELP_LINE_3_2);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_3_3);
	comu_SendF(HELP_LINE_4);
	comu_SendF(HELP_LINE_5);
	comu_SendF(HELP_LINE_SPACE);

	comu_SendF(HELP_LINE_ACK);
	comu_HandleCommunication();
	comu_SendF(HELP_LINE_SPACE);

	comu_SendF(HELP_LINE_ERROR1);
	comu_HandleCommunication();
	comu_SendF(HELP_LINE_ERROR2);
	comu_HandleCommunication();
	comu_SendF(HELP_LINE_ERROR3);
	comu_HandleCommunication();
	comu_SendF(HELP_LINE_SPACE);


	comu_SendF(HELP_LINE_6);
	comu_SendF(HELP_LINE_7);
	comu_SendF(HELP_LINE_7_1);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_7_2);
		comu_SendF(HELP_LINE_7_3);
		comu_SendF(HELP_LINE_7_4);
		comu_SendF(HELP_LINE_8);
		comu_SendF(HELP_LINE_8_1);
		comu_HandleCommunication();
		comu_SendF(HELP_LINE_8_2);
		comu_SendF(HELP_LINE_8_3);
		comu_SendF(HELP_LINE_9);
		comu_SendF(HELP_LINE_SPACE);

		comu_HandleCommunication();
		HAL_Delay(10);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_10);
	comu_SendF(HELP_LINE_11);
	comu_SendF(HELP_LINE_11_1);
	comu_SendF(HELP_LINE_12);
	comu_SendF(HELP_LINE_13);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_13_1);
	comu_SendF(HELP_LINE_13_2);
	comu_SendF(HELP_LINE_13_3);
	comu_SendF(HELP_LINE_13_4);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_13_5);
	comu_SendF(HELP_LINE_13_6);
	comu_SendF(HELP_LINE_13_7);
	comu_SendF(HELP_LINE_13_8);
	comu_SendF(HELP_LINE_13_9);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_14);
	comu_SendF(HELP_LINE_15);
	comu_SendF(HELP_LINE_16);
	comu_SendF(HELP_LINE_17);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_18);
	comu_SendF(HELP_LINE_18_2);
	comu_SendF(HELP_LINE_18_3);
	comu_HandleCommunication();
	HAL_Delay(10);
	comu_SendF(HELP_LINE_19);
	comu_SendF(HELP_LINE_19_1);
	comu_SendF(HELP_LINE_19_2);
	comu_SendF(HELP_LINE_SPACE);
	comu_SendF(HELP_LINE_20);
	comu_SendF(HELP_LINE_21);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_34);
	comu_SendF(HELP_LINE_35);
	comu_SendF(HELP_LINE_36);
	comu_SendF(HELP_LINE_SPACE);
	comu_SendF(HELP_LINE_SEPARATOR);
	comu_HandleCommunication();

	comu_SendF(HELP_LINE_22);
	comu_SendF(HELP_LINE_23);
	comu_SendF(HELP_LINE_24);
	comu_SendF(HELP_LINE_25);
	comu_SendF(HELP_LINE_26);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_27);
	comu_SendF(HELP_LINE_28);
	comu_SendF(HELP_LINE_SEPARATOR);
	comu_SendF(HELP_LINE_SPACE);
	comu_SendF(HELP_LINE_29);
	comu_SendF(HELP_LINE_30);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_31);
	comu_SendF(HELP_LINE_SPACE);

	comu_HandleCommunication();
	HAL_Delay(10);

	comu_SendF(HELP_LINE_32);
	comu_SendF(HELP_LINE_33);

	comu_HandleCommunication();
	HAL_Delay(10);
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
comu_SendF("cmd %s %d %s\r\n", cmdName, cmdId, app_state_to_str(st.state));
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
g_app_params.relays[idx - 1].enabled = 1;
next.relays[idx - 1] = (atoi(val_s) != 0);
io_apply(&next);
return err_Td_Ok;
} else if (strcmp(action, "get") == 0) {
char *idx_s = cmd_next_token();
if (idx_s) {
int idx = atoi(idx_s);
if (idx < 1 || idx > 4) return err_Td_Range;
comu_SendF("cmd %s %d %d %d\r\n", cmdName, cmdId, idx, cur->relays[idx - 1] ? 1 : 0);
                } else {
                        comu_SendF("cmd %s %d %d %d %d %d\r\n", cmdName, cmdId,
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
                g_app_params.mosfets[idx - 1].enabled = 1;
                next.mosfet[idx - 1] = (atoi(val_s) != 0);
                io_apply(&next);
                return err_Td_Ok;
        } else if (strcmp(action, "get") == 0) {
                comu_SendF("cmd %s %d %d %d\r\n", cmdName, cmdId, cur->mosfet[0] ? 1 : 0, cur->mosfet[1] ? 1 : 0);
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
				char *idx_s = cmd_next_token();
				char *sel_s = cmd_next_token();
				mux_sel_t sel = MUX_INT;
				if (!idx_s || !sel_s) return err_Td_Param;
				if (strcmp(sel_s, "int") == 0) {
						sel = MUX_INT;
				} else if (strcmp(sel_s, "ext") == 0) {
						sel = MUX_EXT;
				} else {
						int val = atoi(sel_s);
						if (val == 0) sel = MUX_INT;
						else if (val == 1) sel = MUX_EXT;
						else return err_Td_Range;
				}
				if (strcmp(idx_s, "all") == 0 || strcmp(idx_s, "0") == 0) {
						next.mux[0] = sel;
						next.mux[1] = sel;
						g_app_params.mosfets[0].ext_control = (sel == MUX_EXT) ? 1 : 0;
						g_app_params.mosfets[1].ext_control = (sel == MUX_EXT) ? 1 : 0;
				} else {
						int idx = atoi(idx_s);
						if (idx < 1 || idx > 2) return err_Td_Range;
						next.mux[idx - 1] = sel;
						g_app_params.mosfets[idx - 1].ext_control = (sel == MUX_EXT) ? 1 : 0;
				}
				io_apply(&next);
				return err_Td_Ok;
		} else if (strcmp(action, "get") == 0) {
				char *idx_s = cmd_next_token();
				if (idx_s) {
						int idx = atoi(idx_s);
						if (idx < 1 || idx > 2) return err_Td_Range;
						comu_SendF("cmd %s %d %d %d\r\n", cmdName, cmdId, idx, (cur->mux[idx - 1] == MUX_EXT) ? 1 : 0);
				} else {
						comu_SendF("cmd %s %d %d %d\r\n", cmdName, cmdId, (cur->mux[0] == MUX_EXT) ? 1 : 0, (cur->mux[1] == MUX_EXT) ? 1 : 0);
				}
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
                comu_SendF("cmd %s %d %d %u\r\n", cmdName, cmdId, idx, (unsigned)raw);
                return (raw == 0xFFFFU) ? err_Td_General : err_Td_Ok;
        } else if (strcmp(mode, "ma") == 0) {
                uint32_t ma = Current_Read_mA(ch);
                comu_SendF("cmd %s %d %d %lu\r\n", cmdName, cmdId, idx, (unsigned long)ma);
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
                        comu_SendF("cmd %s %d %d %lu\r\n", cmdName, cmdId, idx, (unsigned long long)g_app_params.relay_health_remaining_k[idx - 1]);
                } else {
                        comu_SendF("cmd %s %d %d %d %d %d\r\n", cmdName, cmdId,
                                        g_app_params.relay_health_remaining_k[0],
                                        g_app_params.relay_health_remaining_k[1],
                                        g_app_params.relay_health_remaining_k[2],
                                        g_app_params.relay_health_remaining_k[3]);
                }
                return err_Td_Ok;
        }
        else if (strcmp(action, "set") == 0) {
        	char *idx_s = cmd_next_token();
        	char *val_s = cmd_next_token();
			if (!idx_s || !val_s) return err_Td_Param;

        		int idx = atoi(idx_s);
        		long val = atoi(val_s);
    			if(val < 0 || val > 300000) return err_Td_Range;
    			if (idx < 1 || idx > 4) return err_Td_Range;
        		g_app_params.relay_health_remaining_k[idx - 1] = val;
        		relay_health_request_pending();
        		return err_Td_Ok;
		}


        else if (strcmp(action, "reset") == 0) {
                relay_counter_reset();
                return err_Td_Ok;
        }
        else if (strcmp(action, "save") == 0) {
                HAL_StatusTypeDef st = relay_counter_save_now(200U);;
                return (st == HAL_OK) ? err_Td_Ok : err_Td_General;
        }
        /*else if (strcmp(action, "load") == 0) {
                HAL_StatusTypeDef st = relay_counter_load();
                return (st == HAL_OK) ? err_Td_Ok : err_Td_NotFound;
        }*/
        return err_Td_NotValid;
}

static err_Td ParamCmdCb(char *cmdName, int32_t cmdId)
{
    char *group = cmd_next_token();
    char *action = cmd_next_token();

    if (!group) return err_Td_Param;

    if(strcmp(group, "reset") == 0){
		app_params_init();
		io_apply(io_get());
		return err_Td_Ok;
   	}

    if(!action) return err_Td_Param;

    if (strcmp(group, "relay") == 0) {
        if (strcmp(action, "get") == 0) {
            char *idx_s = cmd_next_token();
            if (idx_s) {
                int idx = atoi(idx_s);
                if (idx < 1 || idx > 4) return err_Td_Range;
                relay_params_t *p = &g_app_params.relays[idx - 1];
                comu_SendF("cmd %s %d relay %d %d %d %d %d %d\r\n",
                           cmdName, cmdId, idx, p->enabled, p->ton_ms, p->toff_ms, p->imax_ma, p->sw_count_k);
                return err_Td_Ok;
            }
            for (int i = 0; i < 4; ++i) {
                relay_params_t *p = &g_app_params.relays[i];
                comu_SendF("cmd %s %d relay %d %d %d %d %d %d\r\n",
                           cmdName, cmdId, i + 1, p->enabled, p->ton_ms, p->toff_ms, p->imax_ma, p->sw_count_k);
            }
            return err_Td_Ok;
        }
        if (strcmp(action, "set") == 0) {
            char *idx_s = cmd_next_token();
            char *en_s = cmd_next_token();
            char *ton_s = cmd_next_token();
            char *toff_s = cmd_next_token();
            char *imax_s = cmd_next_token();
            char *cnt_s = cmd_next_token();
            if (!idx_s || !en_s || !ton_s || !toff_s || !imax_s || !cnt_s) return err_Td_Param;
            int idx = atoi(idx_s);
            if (idx < 1 || idx > 4) return err_Td_Range;
            relay_params_t *p = &g_app_params.relays[idx - 1];
            p->enabled = atoi(en_s) ? 1 : 0;
            p->ton_ms = atoi(ton_s);
            p->toff_ms = atoi(toff_s);
            p->imax_ma = atoi(imax_s);
            p->sw_count_k = atoi(cnt_s);
            io_apply(io_get());
            return err_Td_Ok;
        }
        return err_Td_NotValid;
    }

    if (strcmp(group, "mosfet") == 0) {
        if (strcmp(action, "get") == 0) {
            char *idx_s = cmd_next_token();
            if (idx_s) {
                int idx = atoi(idx_s);
                if (idx < 1 || idx > 2) return err_Td_Range;
                mosfet_params_t *p = &g_app_params.mosfets[idx - 1];
                comu_SendF("cmd %s %d mosfet %d %d %d %d %d %d\r\n",
                           cmdName, cmdId, idx, p->enabled, p->ext_control, p->ton_ms, p->toff_ms, p->sw_count);
                return err_Td_Ok;
            }
            for (int i = 0; i < 2; ++i) {
                mosfet_params_t *p = &g_app_params.mosfets[i];
                comu_SendF("cmd %s %d mosfet %d %d %d %d %d %d\r\n",
                           cmdName, cmdId, i + 1, p->enabled, p->ext_control, p->ton_ms, p->toff_ms, p->sw_count);
            }
            return err_Td_Ok;
        }
        if (strcmp(action, "set") == 0) {
            char *idx_s = cmd_next_token();
            char *en_s = cmd_next_token();
            char *ext_s = cmd_next_token();
            char *ton_s = cmd_next_token();
            char *toff_s = cmd_next_token();
            char *cnt_s = cmd_next_token();
            if (!idx_s || !en_s || !ext_s || !ton_s || !toff_s || !cnt_s) return err_Td_Param;
            int idx = atoi(idx_s);
            if (idx < 1 || idx > 2) return err_Td_Range;
            mosfet_params_t *p = &g_app_params.mosfets[idx - 1];
            p->enabled = atoi(en_s) ? 1 : 0;
            p->ext_control = atoi(ext_s) ? 1 : 0;
            p->ton_ms = atoi(ton_s);
            p->toff_ms = atoi(toff_s);
            p->sw_count = atoi(cnt_s);
            io_apply(io_get());
            return err_Td_Ok;
        }
        return err_Td_NotValid;
    }

    if (strcmp(group, "trigger") == 0) {
        if (strcmp(action, "get") == 0) {
            comu_SendF("cmd %s %d trigger %d %d\r\n", cmdName, cmdId, g_app_params.trigger.enable, g_app_params.trigger.channel);
            return err_Td_Ok;
        }
        if (strcmp(action, "set") == 0) {
            char *en_s = cmd_next_token();
            char *ch_s = cmd_next_token();
            if (!en_s || !ch_s) return err_Td_Param;
            int ch = atoi(ch_s);
            if (ch < 1 || ch > 4) return err_Td_Range;
            g_app_params.trigger.enable = atoi(en_s) ? 1 : 0;
            g_app_params.trigger.channel = ch;
            return err_Td_Ok;
        }
        return err_Td_NotValid;
    }

    if (strcmp(group, "conn") == 0) {
        if (strcmp(action, "get") == 0) {
            comu_SendF("cmd %s %d conn %d %d %d %d\r\n", cmdName, cmdId,
                       g_app_params.connectivity.enable,
                       g_app_params.connectivity.can_enable,
                       g_app_params.connectivity.usb_enable,
					   g_app_params.connectivity.telemetry_period_ms);
            return err_Td_Ok;
        }
        if (strcmp(action, "set") == 0) {
            char *en_s = cmd_next_token();
            char *can_s = cmd_next_token();
            char *usb_s = cmd_next_token();
            char *time_s = cmd_next_token();
            if (!en_s || !can_s || !usb_s || !time_s) return err_Td_Param;
            g_app_params.connectivity.enable = atoi(en_s) ? 1 : 0;
            g_app_params.connectivity.can_enable = atoi(can_s) ? 1 : 0;
            g_app_params.connectivity.usb_enable = atoi(usb_s) ? 1 : 0;
            g_app_params.connectivity.telemetry_period_ms = atoi(time_s);
            return err_Td_Ok;
        }
        return err_Td_NotValid;
    }

    if (strcmp(group, "buzzer") == 0) {
        if (strcmp(action, "get") == 0) {
            comu_SendF("cmd %s %d buzzer %d\r\n", cmdName, cmdId, g_app_params.buzzer_enable);
            return err_Td_Ok;
        }
        if (strcmp(action, "set") == 0) {
            char *en_s = cmd_next_token();
            if (!en_s) return err_Td_Param;
            g_app_params.buzzer_enable = atoi(en_s) ? 1 : 0;
            return err_Td_Ok;
        }
        return err_Td_NotValid;
    }

    if (strcmp(group, "monitor") == 0) {
            if (strcmp(action, "get") == 0) {
                comu_SendF("cmd %s %d monitor %d\r\n", cmdName, cmdId, g_app_params.current_monitoring_enabled);
                return err_Td_Ok;
            }
            if (strcmp(action, "set") == 0) {
                char *en_s = cmd_next_token();
                if (!en_s) return err_Td_Param;
                g_app_params.current_monitoring_enabled = atoi(en_s) ? 1 : 0;
                return err_Td_Ok;
            }
            return err_Td_NotValid;
        }


    if (strcmp(group, "list") == 0) {
		if (strcmp(action, "profile") == 0) {
			char *id_s = cmd_next_token(); // expect profile number 1..6

			int id = atoi(id_s);
			if (id < 1 || id > PROFILE_STORE_COUNT) return err_Td_Range;

			app_profile_t profile;
			if (!profile_store_load((uint8_t)id, &profile)) {
			   return err_Td_NotFound;
			   }


			// Print same format as param relay/mosfet/trigger/conn/buzzer get
				comu_SendF(HELP_LINE_SEPARATOR);
			    comu_SendF("Connectivity:\r\nRemote Mode Enabled: %d \r\nCAN Output Enabled: %d\r\nUSB Output Enabled: %d\r\n", profile.connectivity.enable, profile.connectivity.can_enable, profile.connectivity.usb_enable);
			    comu_SendF(HELP_LINE_SPACE);
			    comu_SendF("Buzzer enabled: %d\r\n",profile.buzzer_enable);
			    comu_SendF(HELP_LINE_SPACE);
			    comu_SendF("Trigger Enable: %d \r\nTrigger Channel: %d\r\n", profile.trigger.enable, profile.trigger.channel);
			    comu_SendF(HELP_LINE_SPACE);
			    comu_HandleCommunication();
			    comu_SendF(HELP_LINE_SEPARATOR);
			    for (int i = 0; i < 4; ++i) {
			        relay_params_t *p = &profile.relays[i];
			        comu_SendF("Relay %d:\r\nEnable: %d\r\nT_on: %d ms\r\nT_off: %d ms\r\nMaximum Current: %d mA\r\nSwitch Count: %d k\r\n",
			                   i + 1, p->enabled, p->ton_ms, p->toff_ms, p->imax_ma, p->sw_count_k);
			        comu_SendF(HELP_LINE_SPACE);
			    }
			    comu_SendF(HELP_LINE_SEPARATOR);
			    for (int i = 0; i < 2; ++i) {
			        mosfet_params_t *p = &profile.mosfets[i];
			        comu_SendF("Mosfet %d:\r\nEnable: %d\r\nExternal control: %d\r\nT_on: %d ms\r\nT_off: %d ms\r\nSwitch Count: %d\r\n",
			                   i + 1, p->enabled, p->ext_control, p->ton_ms, p->toff_ms, p->sw_count);
			        comu_SendF(HELP_LINE_SPACE);
			    }
			    return err_Td_Ok;
			}
    }

    return err_Td_NotValid;
}

static err_Td TestCmdCb(char *cmdName, int32_t cmdId)
{
	char *group = cmd_next_token();
	char *action = cmd_next_token();
	if (!group) return err_Td_Param;
        if (strcmp(group, "start") == 0) {
        	if (!action) return err_Td_Param;
        	if (strcmp(action, "current") == 0) {
        		act_test_current();
                app_menu_set_test_screen(APP_TEST_SCREEN_START);
                return app_post_event((app_event_t){ .type = APP_EVT_TEST_START }) ? err_Td_Ok : err_Td_Busy;
        	}
        	else if (strcmp(action, "profile") == 0) {
        	        char *idx_s = cmd_next_token();
        	        int idx = atoi(idx_s);
        			act_test_profile(idx);
        			app_menu_set_test_screen(APP_TEST_SCREEN_START);
        			return app_post_event((app_event_t){ .type = APP_EVT_TEST_START }) ? err_Td_Ok : err_Td_Busy;
        	}
        }

        if (strcmp(group, "stop") == 0) {
                app_menu_set_test_screen(APP_TEST_SCREEN_STOP);

                if(app_post_event((app_event_t){ .type = APP_EVT_TEST_STOP })){
                	app_menu_draw_test_screen(APP_TEST_SCREEN_STOP);
                	app_post_event((app_event_t){ .type = APP_EVT_CMD_MODE_REMOTE });
                	return err_Td_Ok;
                }
				else {
					return err_Td_Busy;
				}
        }
        return err_Td_NotValid;
}

static err_Td SaveCmdCb(char *cmdName, int32_t cmdId) {
    char *sub = cmd_next_token();
    // Kontrola, zda následuje klíčové slovo "profile"
    if (!sub || strcmp(sub, "profile") != 0) return err_Td_Param;

    char *idx_s = cmd_next_token();
    if (!idx_s) return err_Td_Param;

    int idx = atoi(idx_s);
    // Kontrola rozsahu (podle menu 1-6)
    if (idx < 1 || idx > 6) return err_Td_Range;

    if (profile_store_save((uint8_t)idx, &g_app_params)) {
        if (g_app_params.buzzer_enable) Buzzer_PlayPattern(BUZZER_SAVE);
        return err_Td_Ok;
    }
    return err_Td_General;
}

/**
 * @brief Command to load parameters from a profile.
 * Syntax: load profile <index>
 */
static err_Td LoadCmdCb(char *cmdName, int32_t cmdId) {
    char *sub = cmd_next_token();
    // Kontrola, zda následuje klíčové slovo "profile"
    if (!sub || strcmp(sub, "profile") != 0) return err_Td_Param;

    char *idx_s = cmd_next_token();
    if (!idx_s) return err_Td_Param;

    int idx = atoi(idx_s);
    if (idx < 1 || idx > 6) return err_Td_Range;

    app_profile_t profile;
    if (profile_store_load((uint8_t)idx, &profile)) {
        profile_store_apply(&profile, &g_app_params);
        // Aplikace načteného nastavení na hardware (shodné s app_menu.c)
        io_apply(io_get());
        if (g_app_params.buzzer_enable) Buzzer_PlayPattern(BUZZER_LOAD);
        return err_Td_Ok;
    }
    return err_Td_NotExist; // Profil nebyl nalezen
}

/*
 * List of all available commands. Syntax is specified in each callback function separately (also in toltip).
 */
static CmdTd CmdList[] = {
        {"gi", GetInfoCb},
        {"gh", GetHelpCb},
        {"mode", ModeCmdCb},
        {"relay", RelayCmdCb},
		{"health", RelayCountCmdCb},
        {"mosfet", MosfetCmdCb},
        {"mux", MuxCmdCb},
        {"curr", CurrentCmdCb},
        {"rcnt", RelayCountCmdCb},
        {"param", ParamCmdCb},
        {"test", TestCmdCb},
		{"save", SaveCmdCb},
		{"load", LoadCmdCb},
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

	if (ErrNo == err_Td_Ok && Name != NULL) {
		app_status_t st = app_get_status();
		if (st.state != APP_STATE_REMOTE && strcmp(Name, "mode") != 0) {
			if (st.state != APP_STATE_REMOTE && strcmp(Name, "test") != 0)
				if (st.state != APP_STATE_REMOTE && strcmp(Name, "gh") != 0)
					if (st.state != APP_STATE_REMOTE && strcmp(Name, "gi") != 0)
						ErrNo = err_Td_Disabled;
		}
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

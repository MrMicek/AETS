/*
 * app_menu.c
 *
 *  Created on: Nov 6, 2025
 *      Author: uiv10467
 */




#include "menu.h"
#include "display.h"
#include "stm32g4xx_hal.h"
#include "app_params.h"
#include "io_control.h"
#include "app_sm.h"
#include "app_menu.h"
#include "test_seq.h"
#include "encoder.h"
#include <stdio.h>
#include <stdbool.h>
#include "buzzer.h"
#include "relay_health_store.h"
#include "profile_store.h"


// Common back action
void act_back(void) {
    menu_back();
}


// Common back action
static void saveToProfile(int n) {
    (void)profile_store_save((uint8_t)n, &g_app_params);
    (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_SAVE) : 0;
}

// Common back action
static void loadFromProfile(int n) {
    app_profile_t profile;
    if (profile_store_load((uint8_t)n, &profile)) {
        profile_store_apply(&profile, &g_app_params);
        io_apply(io_get());
        menu_draw_full(menu_get_active());
        (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_LOAD) : 0;
    }
}

static void saveToProfile1(void) { saveToProfile(1); act_back(); }
static void loadFromProfile1(void) { loadFromProfile(1); act_back();}

static void saveToProfile2(void) { saveToProfile(2); act_back();}
static void loadFromProfile2(void) { loadFromProfile(2); act_back();}

static void saveToProfile3(void) { saveToProfile(3); act_back();}
static void loadFromProfile3(void) { loadFromProfile(3); act_back();}

static void saveToProfile4(void) { saveToProfile(4); act_back();}
static void loadFromProfile4(void) { loadFromProfile(4); act_back();}

static void saveToProfile5(void) { saveToProfile(5); act_back();}
static void loadFromProfile5(void) { loadFromProfile(5); act_back();}

static void saveToProfile6(void) { saveToProfile(6); act_back();}
static void loadFromProfile6(void) { loadFromProfile(6); act_back();}


static uint8_t s_test_fail_relay = 0;
void app_menu_set_test_fail_relay(uint8_t relay_idx) { s_test_fail_relay = relay_idx; }

// ---------- Actions ----------
void act_test_current(void) {
    test_seq_set_params_current();
    app_post_event((app_event_t){ .type = APP_EVT_TEST_START });
}

void act_test_profile(uint8_t profile_id) {
	loadFromProfile(profile_id);
    test_seq_set_params_profile(profile_id);
    app_post_event((app_event_t){ .type = APP_EVT_TEST_START });
}

static void act_test_profile1(void) { act_test_profile(1U); }
static void act_test_profile2(void) { act_test_profile(2U); }
static void act_test_profile3(void) { act_test_profile(3U); }
static void act_test_profile4(void) { act_test_profile(4U); }
static void act_test_profile5(void) { act_test_profile(5U); }
static void act_test_profile6(void) { act_test_profile(6U); }


static void reset_params(void) {
	app_params_init();
	io_apply(io_get());
	menu_draw_full(menu_get_active());
	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_STARTUP) : 0;
	act_back();
}

static void set_trigger_channel(uint8_t ch)
{
    g_app_params.trigger.channel = ch;   // 1..4
    io_apply(io_get());                  // apply immediately
    menu_back();                         // return to Trigger menu
}

static void act_trigger_ch1(void) { set_trigger_channel(1); }
static void act_trigger_ch2(void) { set_trigger_channel(2); }
static void act_trigger_ch3(void) { set_trigger_channel(3); }
static void act_trigger_ch4(void) { set_trigger_channel(4); }


// ---------- SUBMENUS_3 ------------------------------------------------------------------------------
static const MenuItem RELAY1_ITEMS[] = {
    { "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.relays[0].enabled, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[0].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[0].toff_ms, 0, 1000000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[0].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[0].sw_count_k, 0, 1000000 },
};
static Menu gRelay1Menu = {
    .items = RELAY1_ITEMS,
    .count = sizeof(RELAY1_ITEMS)/sizeof(RELAY1_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RELAY2_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.relays[1].enabled, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[1].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[1].toff_ms, 0, 1000000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[1].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[1].sw_count_k, 0, 1000000 },
};
static Menu gRelay2Menu = {
    .items = RELAY2_ITEMS,
    .count = sizeof(RELAY2_ITEMS)/sizeof(RELAY2_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RELAY3_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.relays[2].enabled, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[2].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[2].toff_ms, 0, 1000000 },
	{ "I-max(mA)", NULL, NULL, &g_app_params.relays[2].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[2].sw_count_k, 0, 1000000 },
};
static Menu gRelay3Menu = {
    .items = RELAY3_ITEMS,
    .count = sizeof(RELAY3_ITEMS)/sizeof(RELAY3_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RELAY4_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.relays[3].enabled, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[3].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[3].toff_ms, 0, 1000000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[3].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[3].sw_count_k, 0, 1000000 },
};
static Menu gRelay4Menu = {
    .items = RELAY4_ITEMS,
    .count = sizeof(RELAY4_ITEMS)/sizeof(RELAY4_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_1[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile1, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile1 = {
    .items = LOAD_PROFILE_1,
    .count = sizeof(LOAD_PROFILE_1)/sizeof(LOAD_PROFILE_1[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_1[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile1, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile1 = {
    .items = SAVE_PROFILE_1,
    .count = sizeof(SAVE_PROFILE_1)/sizeof(SAVE_PROFILE_1[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_2[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile2, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile2 = {
    .items = LOAD_PROFILE_2,
    .count = sizeof(LOAD_PROFILE_2)/sizeof(LOAD_PROFILE_2[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_2[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile2, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile2 = {
    .items = SAVE_PROFILE_2,
    .count = sizeof(SAVE_PROFILE_2)/sizeof(SAVE_PROFILE_2[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_3[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile3, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile3 = {
    .items = LOAD_PROFILE_3,
    .count = sizeof(LOAD_PROFILE_3)/sizeof(LOAD_PROFILE_3[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_3[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile3, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile3 = {
    .items = SAVE_PROFILE_3,
    .count = sizeof(SAVE_PROFILE_3)/sizeof(SAVE_PROFILE_3[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_4[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile4, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile4 = {
    .items = LOAD_PROFILE_4,
    .count = sizeof(LOAD_PROFILE_4)/sizeof(LOAD_PROFILE_4[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_4[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile4, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile4 = {
    .items = SAVE_PROFILE_4,
    .count = sizeof(SAVE_PROFILE_4)/sizeof(SAVE_PROFILE_4[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_5[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile5, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile5 = {
    .items = LOAD_PROFILE_5,
    .count = sizeof(LOAD_PROFILE_5)/sizeof(LOAD_PROFILE_5[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_5[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile5, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile5 = {
    .items = SAVE_PROFILE_5,
    .count = sizeof(SAVE_PROFILE_5)/sizeof(SAVE_PROFILE_5[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem LOAD_PROFILE_6[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", loadFromProfile6, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gLoadProfile6 = {
    .items = LOAD_PROFILE_6,
    .count = sizeof(LOAD_PROFILE_6)/sizeof(LOAD_PROFILE_6[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SAVE_PROFILE_6[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", saveToProfile6, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gSaveProfile6 = {
    .items = SAVE_PROFILE_6,
    .count = sizeof(SAVE_PROFILE_6)/sizeof(SAVE_PROFILE_6[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

// ---------- SUBMENUS_2 ----------------------------------------------------------------------------------------
static const MenuItem PROFILE1_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile1, NULL },
	{ "Load from profile", NULL, &gLoadProfile1, NULL },
};
static Menu gProfile1Menu = {
    .items = PROFILE1_ITEMS,
    .count = sizeof(PROFILE1_ITEMS)/sizeof(PROFILE1_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE2_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile2, NULL },
	{ "Load from profile", NULL, &gLoadProfile2, NULL },
};
static Menu gProfile2Menu = {
    .items = PROFILE2_ITEMS,
    .count = sizeof(PROFILE2_ITEMS)/sizeof(PROFILE2_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE3_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile3, NULL },
	{ "Load from profile", NULL, &gLoadProfile3, NULL },
};
static Menu gProfile3Menu = {
    .items = PROFILE3_ITEMS,
    .count = sizeof(PROFILE3_ITEMS)/sizeof(PROFILE3_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE4_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile4, NULL },
	{ "Load from profile", NULL, &gLoadProfile4, NULL },
};
static Menu gProfile4Menu = {
    .items = PROFILE4_ITEMS,
    .count = sizeof(PROFILE4_ITEMS)/sizeof(PROFILE4_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE5_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile5, NULL },
	{ "Load from profile", NULL, &gLoadProfile5, NULL },
};
static Menu gProfile5Menu = {
    .items = PROFILE5_ITEMS,
    .count = sizeof(PROFILE5_ITEMS)/sizeof(PROFILE5_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE6_ITEMS[] = {
	{ "< Return", act_back, NULL },
	{ "Save to profile", NULL, &gSaveProfile6, NULL },
	{ "Load from profile", NULL, &gLoadProfile6, NULL },
};
static Menu gProfile6Menu = {
    .items = PROFILE6_ITEMS,
    .count = sizeof(PROFILE6_ITEMS)/sizeof(PROFILE6_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};


static const MenuItem TEST_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "Current Settings", act_test_current, NULL },
    { "Profile 1", act_test_profile1, NULL },
    { "Profile 2", act_test_profile2, NULL },
    { "Profile 3", act_test_profile3, NULL },
    { "Profile 4", act_test_profile4, NULL },
    { "Profile 5", act_test_profile5, NULL },
    { "Profile 6", act_test_profile6, NULL },
};
static Menu gTestMenu = {
    .items = TEST_ITEMS,
    .count = sizeof(TEST_ITEMS)/sizeof(TEST_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem MOSFET1_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.mosfets[0].enabled, 0, 1 },
	{ "Ext. Control", NULL, NULL, &g_app_params.mosfets[0].ext_control, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.mosfets[0].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.mosfets[0].toff_ms, 0, 1000000 },
	{ "SW. Cnt", NULL, NULL, &g_app_params.mosfets[0].sw_count, 0, 1000000 },
};
static Menu gMosfet1Menu = {
    .items = MOSFET1_ITEMS,
    .count = sizeof(MOSFET1_ITEMS)/sizeof(MOSFET1_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem MOSFET2_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.mosfets[1].enabled, 0, 1 },
	{ "Ext. Control", NULL, NULL, &g_app_params.mosfets[1].ext_control, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.mosfets[1].ton_ms, 0, 1000000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.mosfets[1].toff_ms, 0, 1000000 },
	{ "SW. Cnt", NULL, NULL, &g_app_params.mosfets[1].sw_count, 0, 1000000 },
};
static Menu gMosfet2Menu = {
    .items = MOSFET2_ITEMS,
    .count = sizeof(MOSFET2_ITEMS)/sizeof(MOSFET2_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SET_RELAY_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "Relay 1", NULL, &gRelay1Menu },
	{ "Relay 2", NULL, &gRelay2Menu },
	{ "Relay 3", NULL, &gRelay3Menu },
	{ "Relay 4", NULL, &gRelay4Menu },
};
static Menu gSetRelayMenu = {
    .items = SET_RELAY_ITEMS,
    .count = sizeof(SET_RELAY_ITEMS)/sizeof(SET_RELAY_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RELAY_HEALTH_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "Relay1 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health_remaining_k[0] },
	{ "Relay2 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health_remaining_k[1] },
	{ "Relay3 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health_remaining_k[2] },
	{ "Relay4 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health_remaining_k[3] },
};
static Menu gRelayHealth = {
    .items = RELAY_HEALTH_ITEMS,
    .count = sizeof(RELAY_HEALTH_ITEMS)/sizeof(RELAY_HEALTH_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SET_RELAY_HEALTH_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Relay1 (k)", NULL, NULL, &g_app_params.relay_health_set_k[0], 0, 30000},
	{ "Relay2 (k)", NULL, NULL, &g_app_params.relay_health_set_k[1], 0, 30000},
	{ "Relay3 (k)", NULL, NULL, &g_app_params.relay_health_set_k[2], 0, 30000},
	{ "Relay4 (k)", NULL, NULL, &g_app_params.relay_health_set_k[3], 0, 30000},
};
static Menu gSetRelayHealth = {
    .items = SET_RELAY_HEALTH_ITEMS,
    .count = sizeof(SET_RELAY_HEALTH_ITEMS)/sizeof(SET_RELAY_HEALTH_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RESET_PARAMETERS[] = {
	{ "Are you sure?", NULL, NULL },
	{ "YES", reset_params, NULL },
	{ "NO", act_back,        NULL },
};
static Menu gResetParameters = {
    .items = RESET_PARAMETERS,
    .count = sizeof(RESET_PARAMETERS)/sizeof(RESET_PARAMETERS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem TRIGGER_CH_ITEMS[] = {
    { "< Return", act_back, NULL },
    { "Relay 1",  act_trigger_ch1, NULL },
    { "Relay 2",  act_trigger_ch2, NULL },
    { "Relay 3",  act_trigger_ch3, NULL },
    { "Relay 4",  act_trigger_ch4, NULL },
};

static Menu gTriggerChannelMenu = {
    .items = TRIGGER_CH_ITEMS,
    .count = sizeof(TRIGGER_CH_ITEMS)/sizeof(TRIGGER_CH_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

// ---------- SUBMENUS_1 ----------------------------------------------------------------------------------------
static const MenuItem INFO_ITEMS[] = {
    { "< Return", act_back,        NULL },
	{ "Automated Electric", NULL, NULL},
	{ "Testing   System", NULL, NULL},
	{ "Version:  1.0", NULL, NULL},
	{ "Author: V.Micek", NULL, NULL},
	{ "SCHAEFFLER CZ sro", NULL, NULL},
};
static Menu gInfoMenu = {
    .items = INFO_ITEMS,
    .count = sizeof(INFO_ITEMS)/sizeof(INFO_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem SETTINGS_ITEMS[] = {
    { "< Return", act_back,        NULL },
	{ "Buzzer enable", NULL, NULL, &g_app_params.buzzer_enable, 0, 1},
	{ "Curr. monitor", NULL, NULL, &g_app_params.current_monitoring_enabled, 0, 1},
	{ "Reset parameters", NULL, &gResetParameters }
};
static Menu gSettingMenu = {
    .items = SETTINGS_ITEMS,
    .count = sizeof(SETTINGS_ITEMS)/sizeof(SETTINGS_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem PROFILE_ITEMS[] = {
    { "< Return", act_back,        NULL },
	{ "Profile 1",NULL,  &gProfile1Menu },
	{ "Profile 2",NULL,  &gProfile2Menu },
	{ "Profile 3",NULL,  &gProfile3Menu },
	{ "Profile 4",NULL,  &gProfile4Menu },
	{ "Profile 5",NULL,  &gProfile5Menu },
	{ "Profile 6",NULL,  &gProfile6Menu },
};
static Menu gProfileMenu = {
    .items = PROFILE_ITEMS,
    .count = sizeof(PROFILE_ITEMS)/sizeof(PROFILE_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem CONNECTIVITY_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "Remote Mode ",NULL, NULL, &g_app_params.connectivity.enable, 0, 1},
	{ "CAN Output ",NULL, NULL, &g_app_params.connectivity.can_enable, 0, 1},
	{ "USB Output ",NULL, NULL, &g_app_params.connectivity.usb_enable, 0, 1},
	{ "Period (ms)",NULL, NULL, &g_app_params.connectivity.telemetry_period_ms, 200, 100000},
};
static Menu gConectivityMenu = {
    .items = CONNECTIVITY_ITEMS,
    .count = sizeof(CONNECTIVITY_ITEMS)/sizeof(CONNECTIVITY_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem TRIGGER_ITEMS[] = {
    { "< Return", act_back, NULL },
    { "Enable ",  NULL, NULL, &g_app_params.trigger.enable, 0, 1 },

    // Show current channel value AND enter submenu to change it
    { "Relay Ch.", NULL, &gTriggerChannelMenu, NULL, 0, 0, &g_app_params.trigger.channel },
};

static Menu gTriggerMenu = {
    .items = TRIGGER_ITEMS,
    .count = sizeof(TRIGGER_ITEMS)/sizeof(TRIGGER_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem MOSFET_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "MOSFET 1",NULL,  &gMosfet1Menu },
	{ "MOSFET 2",NULL,  &gMosfet2Menu },
};
static Menu gMosfetMenu = {
    .items = MOSFET_ITEMS,
    .count = sizeof(MOSFET_ITEMS)/sizeof(MOSFET_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem RELAYS_ITEMS[] = {
    { "< Return", act_back,        NULL },
    { "Set Relay",NULL,  &gSetRelayMenu },
	{ "Remain Count",NULL,  &gRelayHealth },
	{ "Set Count",NULL,  &gSetRelayHealth },
};
static Menu gRelayMenu = {
    .items = RELAYS_ITEMS,
    .count = sizeof(RELAYS_ITEMS)/sizeof(RELAYS_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};



// ---------- Main menu ----------------------------------------------------------------------------------------
// Note: Items can either have an on_select action OR a .submenu

static const MenuItem MAIN_ITEMS[] = {
    { "Start test", NULL,          &gTestMenu },
    { "Relays", NULL,          &gRelayMenu },
	{ "Mosfets", NULL,          &gMosfetMenu },
	{ "Trigger", NULL,          &gTriggerMenu },
	{ "Connectivity", NULL,          &gConectivityMenu },
	{ "Profile", NULL,          &gProfileMenu },
	{ "Settings", NULL,          &gSettingMenu },
	{ "Info", NULL,          &gInfoMenu },
};

static Menu gMainMenu = {
    .items = MAIN_ITEMS,
    .count = sizeof(MAIN_ITEMS)/sizeof(MAIN_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = MENU_FLAG_WRAP, // main menu can wrap
};

static app_test_screen_t s_test_screen = APP_TEST_SCREEN_NONE;
static uint8_t s_remote_screen_drawn = 0;
static uint8_t s_test_screen_drawn = 0;
static uint8_t s_test_page = 0;

typedef struct {
    app_test_screen_t screen;
    uint8_t page;
    uint32_t relay_remaining[4];
    uint32_t relay_current_ma[4];
    uint32_t mosfet_remaining[2];
    uint8_t relay_on[4];
    uint8_t mosfet_on[2];
} test_screen_cache_t;

static test_screen_cache_t s_test_cache;

static void app_menu_draw_remote_screen(void) {
    oled_clear();
    oled_write_line_full(2, "REMOTE MODE");
    oled_write_line_full(3, "<RETURN MANUAL");
}

static void format_test_entry(char *line, size_t line_len, char prefix, uint8_t index,
                              bool enabled, uint32_t remaining, bool on)
{
    const char *state = on ? "ON " : "OFF";
    if (!enabled) {
        snprintf(line, line_len, "%c%u: ***** ***", prefix, (unsigned)(index + 1U));
    } else {
        uint32_t shown = (remaining > 99999U) ? 99999U : remaining;
        snprintf(line, line_len, "%c%u: %05lu %s", prefix, (unsigned)(index + 1U),
                 (unsigned long)shown, state);
    }
}

static void format_test_entry_relay(char *line, size_t line_len, char prefix, uint8_t index,
                                    bool enabled, uint32_t remaining, bool on, uint32_t current_ma)
{
    const char *state = on ? "ON " : "OFF";
    if (!enabled) {
        snprintf(line, line_len, "%c%u: ***** ***", prefix, (unsigned)(index + 1U));
    } else {
        uint32_t shown = (remaining > 99999U) ? 99999U : remaining;
        uint32_t shown_current = (current_ma > 9999U) ? 9999U : current_ma;
        snprintf(line, line_len, "%c%u: %05lu %s I:%4lu", prefix, (unsigned)(index + 1U),
                 (unsigned long)shown, state, (unsigned long)shown_current);
    }
}



static void app_menu_draw_test_running(uint8_t page)
{
    char line[21];
    const io_state_t *io = io_get();

    if (page == 0U) {
        snprintf(line, sizeof(line), "%c Stop Test", 0xDF);
        oled_write_line_full(1, line);
        oled_write_line_full(2, "Count Left:");
        format_test_entry_relay(line, sizeof(line), 'R', 0U, test_seq_relay_is_enabled(0U),
                                test_seq_get_relay_remaining(0U), io->relays[0],
                                app_get_relay_current_ma(0U));
        oled_write_line_full(3, line);
        format_test_entry_relay(line, sizeof(line), 'R', 1U, test_seq_relay_is_enabled(1U),
                                test_seq_get_relay_remaining(1U), io->relays[1],
                                app_get_relay_current_ma(1U));
        oled_write_line_full(4, line);
    } else {
    	format_test_entry_relay(line, sizeof(line), 'R', 2U, test_seq_relay_is_enabled(2U),
    	                                        test_seq_get_relay_remaining(2U), io->relays[2],
    	                                        app_get_relay_current_ma(2U));
    	        // Removed the unnecessary 'line1' copy
    	oled_write_line_full(1, line);

    	format_test_entry_relay(line, sizeof(line), 'R', 3U, test_seq_relay_is_enabled(3U),
    	                                test_seq_get_relay_remaining(3U), io->relays[3],
    	                                app_get_relay_current_ma(3U));
    	oled_write_line_full(2, line);
        format_test_entry(line, sizeof(line), 'M', 0U, test_seq_mosfet_is_enabled(0U),
                          test_seq_get_mosfet_remaining(0U), io->mosfet[0]);
        oled_write_line_full(3, line);
        format_test_entry(line, sizeof(line), 'M', 1U, test_seq_mosfet_is_enabled(1U),
                          test_seq_get_mosfet_remaining(1U), io->mosfet[1]);
        oled_write_line_full(4, line);
    }
}

 void app_menu_draw_test_screen(app_test_screen_t screen) {
    char line[21];

    switch (screen) {
    case APP_TEST_SCREEN_RUNNING:
        oled_clear();
        app_menu_draw_test_running(s_test_page);
        break;
    case APP_TEST_SCREEN_STOP:
        oled_clear();
        snprintf(line, sizeof(line), "%c < Return", 0xDF);
        oled_write_line_full(1, line);
        oled_write_line_full(2, "Test Stopped");
        oled_write_line_full(3, "Manually");
        oled_write_line_full(4, "");
        break;
    case APP_TEST_SCREEN_OK:
        oled_clear();
        snprintf(line, sizeof(line), "%c < Return", 0xDF);
        oled_write_line_full(1, line);
        oled_write_line_full(2, "Test Finished");
        oled_write_line_full(3, "Successfully");
        oled_write_line_full(4, "");
        (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_STARTUP) : 0;
        break;
    case APP_TEST_SCREEN_ERROR_MAX_CURRENT:
        oled_clear();
        snprintf(line, sizeof(line), "%c < Return", 0xDF);
        oled_write_line_full(1, line);
        oled_write_line_full(2, "Test Ended");
        oled_write_line_full(3, "Error:");
        oled_write_line_full(4, "Current Exceeded");
        (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_DANGER) : 0;
        break;
    case APP_TEST_SCREEN_ERROR_ZERO_CURRENT:
        oled_clear();
        snprintf(line, sizeof(line), "%c < Return", 0xDF);
        oled_write_line_full(1, line);
        oled_write_line_full(2, "Test Ended");
        oled_write_line_full(3, "Error:");
        oled_write_line_full(4, "No Current Flow");
        (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_DANGER) : 0;
        break;
    case APP_TEST_SCREEN_NONE:
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
    	act_back();
    	break;
    case APP_TEST_SCREEN_RELAY_COUNT_LOW:
    	oled_clear();
    	snprintf(line, sizeof(line), "%c < Return", 0xDF);
    	oled_write_line_full(1, line);
    	oled_write_line_full(2, "Can't Start Test");
    	snprintf(line, sizeof(line), "Relay %u:", (unsigned)s_test_fail_relay);
    	oled_write_line_full(3, line);
    	oled_write_line_full(4, "Not enough switches");
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_WARNING) : 0;
    	break;
    default:
    	(g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
    	act_back();
    }
}

void app_menu_set_test_screen(app_test_screen_t screen)
{
    s_test_screen = screen;
    s_test_screen_drawn = 0;
    s_test_page = 0;
    s_test_cache.screen = APP_TEST_SCREEN_NONE;
}

app_test_screen_t app_menu_get_test_screen(void)
{
    return s_test_screen;
}



void app_menu_init(void) {
    oled_init();
    menu_init(&gMainMenu, MAIN_ITEMS, sizeof(MAIN_ITEMS)/sizeof(MAIN_ITEMS[0]));
    menu_set_active(&gMainMenu);     // ensure active menu is main menu
    menu_set_wrap(&gMainMenu, 1);    // allow wrap only in main menu
}

void app_menu_task(void) {
    extern void menu_poll(Menu *menu); // from encoder_menu_glue.c
    extern uint8_t menu_encoder_take_press(void);
    app_status_t st = app_get_status();


    if (s_test_screen == APP_TEST_SCREEN_RELAY_COUNT_LOW) {
        if (!s_test_screen_drawn) {
            app_menu_draw_test_screen(s_test_screen);
            s_test_screen_drawn = 1;
            if (st.state == APP_STATE_REMOTE) {
            	comu_SendF("Test blocked. Not enough switches left. Relay: %u\r\n", (unsigned)s_test_fail_relay);
            }
        }

        // optional: handle encoder press to dismiss
        if (menu_encoder_take_press()) {
            app_menu_set_test_screen(APP_TEST_SCREEN_NONE);
            s_test_screen_drawn = 0;
            oled_clear();
            menu_draw_full(menu_get_active());
            (g_app_params.buzzer_enable) ? Buzzer_PlayPattern(BUZZER_INFO) : 0;
        }else
        	return; // avoid normal menu draw/poll while error screen is visible
    }


    if (st.state == APP_STATE_TEST) {
        if (s_test_screen == APP_TEST_SCREEN_RUNNING) {
            EncoderDirection_Td dir = Encoder_read();
            if (dir == UP || dir == DOWN) {
                s_test_page ^= 1U;
            }
        }

        if (menu_encoder_take_press()) {
            if (s_test_screen == APP_TEST_SCREEN_RUNNING) {
                app_menu_set_test_screen(APP_TEST_SCREEN_STOP);
                app_post_event((app_event_t){ .type = APP_EVT_TEST_STOP });
            } else {
                app_menu_set_test_screen(APP_TEST_SCREEN_NONE);
                app_post_event((app_event_t){ .type = APP_EVT_TEST_EXIT });
            }
        }

        uint8_t needs_redraw = 0;
        if (s_test_screen != s_test_cache.screen || s_test_page != s_test_cache.page) {
            needs_redraw = 1;
        }

        if (s_test_screen == APP_TEST_SCREEN_RUNNING) {
            const io_state_t *io = io_get();
            for (uint8_t i = 0; i < 4U; ++i) {
                uint32_t remaining = test_seq_get_relay_remaining(i);
                uint8_t on = io->relays[i] ? 1U : 0U;

                uint32_t current = app_get_relay_current_ma(i);

                if (remaining != s_test_cache.relay_remaining[i] ||
                                    on != s_test_cache.relay_on[i] ||
                                    current != s_test_cache.relay_current_ma[i]) { // Check Current!
                   needs_redraw = 1;
                }
            }
            for (uint8_t i = 0; i < 2U; ++i) {
                uint32_t remaining = test_seq_get_mosfet_remaining(i);
                uint8_t on = io->mosfet[i] ? 1U : 0U;
                if (remaining != s_test_cache.mosfet_remaining[i] || on != s_test_cache.mosfet_on[i]) {
                    needs_redraw = 1;
                }
            }
        }

        if (!s_test_screen_drawn || needs_redraw) {
            app_menu_draw_test_screen(s_test_screen);
            s_test_screen_drawn = 1;
            s_test_cache.screen = s_test_screen;
            s_test_cache.page = s_test_page;
            if (s_test_screen == APP_TEST_SCREEN_RUNNING) {
                const io_state_t *io = io_get();
                for (uint8_t i = 0; i < 4U; ++i) {
                    s_test_cache.relay_remaining[i] = test_seq_get_relay_remaining(i);
                    s_test_cache.relay_on[i] = io->relays[i] ? 1U : 0U;
                    s_test_cache.relay_current_ma[i] = app_get_relay_current_ma(i);
                }
                for (uint8_t i = 0; i < 2U; ++i) {
                    s_test_cache.mosfet_remaining[i] = test_seq_get_mosfet_remaining(i);
                    s_test_cache.mosfet_on[i] = io->mosfet[i] ? 1U : 0U;
                }
            }
        }
        return;
    }

    if (st.state == APP_STATE_REMOTE) {
        if (!s_remote_screen_drawn) {
            app_menu_draw_remote_screen();
            s_remote_screen_drawn = 1;
        }
        if (menu_encoder_take_press()) {
            g_app_params.connectivity.enable = 0;
            app_post_event((app_event_t){ .type = APP_EVT_CMD_MODE_MANUAL });
            s_remote_screen_drawn = 0;
            oled_clear();
            menu_draw_full(menu_get_active());
        }
        return;
    }

    s_remote_screen_drawn = 0;
    s_test_screen_drawn = 0;
    menu_poll(menu_get_active());
}

void app_menu_on_value_commit(const MenuItem *it)
{
    (void)it;
    io_apply(io_get());

    if (it && it->value_ptr == &g_app_params.connectivity.enable) {
        if (g_app_params.connectivity.enable != 0) {
            app_post_event((app_event_t){ .type = APP_EVT_CMD_MODE_REMOTE });
        } else {
            app_post_event((app_event_t){ .type = APP_EVT_CMD_MODE_MANUAL });
        }
    }

    if (it) {
        for (uint8_t i = 0; i < 4U; ++i) {
            if (it->value_ptr == &g_app_params.relay_health_set_k[i]) {
                g_app_params.relay_health_remaining_k[i] = g_app_params.relay_health_set_k[i];
                relay_health_save_now(200U);
                break;
            }
        }
    }
}

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

// Common back action
static void saveToProfile(int n) {

}

// Common back action
static void loadFromProfile(int n) {
}

static void saveToProfile1(void) { saveToProfile(1); }
static void loadFromProfile1(void) { loadFromProfile(1); }

static void saveToProfile2(void) { saveToProfile(2); }
static void loadFromProfile2(void) { loadFromProfile(2); }

static void saveToProfile3(void) { saveToProfile(3); }
static void loadFromProfile3(void) { loadFromProfile(3); }

static void saveToProfile4(void) { saveToProfile(4); }
static void loadFromProfile4(void) { loadFromProfile(4); }

static void saveToProfile5(void) { saveToProfile(5); }
static void loadFromProfile5(void) { loadFromProfile(5); }

static void saveToProfile6(void) { saveToProfile(6); }
static void loadFromProfile6(void) { loadFromProfile(6); }


// ---------- Actions ----------
static void act_start(void) {
    oled_clear();
    oled_write_line_full(2, "Starting...");
    HAL_Delay(600);
}

// Common back action
static void act_back(void) {
    menu_back();
}



// ---------- SUBMENUS_3 ------------------------------------------------------------------------------
static const MenuItem RELAY1_ITEMS[] = {
    { "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.relays[0].enabled, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[0].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[0].toff_ms, 0, 10000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[0].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[0].sw_count_k, 0, 1000 },
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
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[1].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[1].toff_ms, 0, 10000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[1].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[1].sw_count_k, 0, 1000 },
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
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[2].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[2].toff_ms, 0, 10000 },
	{ "I-max(mA)", NULL, NULL, &g_app_params.relays[2].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[2].sw_count_k, 0, 1000 },
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
	{ "Ton (ms)", NULL, NULL, &g_app_params.relays[3].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.relays[3].toff_ms, 0, 10000 },
	{ "I-max (mA)", NULL, NULL, &g_app_params.relays[3].imax_ma, 8, 4000 },
	{ "SW. Cnt (k)", NULL, NULL, &g_app_params.relays[3].sw_count_k, 0, 1000 },
};
static Menu gRelay4Menu = {
    .items = RELAY4_ITEMS,
    .count = sizeof(RELAY4_ITEMS)/sizeof(RELAY4_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

// ---------- SUBMENUS_2 ----------------------------------------------------------------------------------------
static const MenuItem PROFILE1_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile1,        NULL },
	{ "Load from profile", loadFromProfile1,        NULL },
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
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile2,        NULL },
	{ "Load from profile", loadFromProfile2,        NULL },
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
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile3,        NULL },
	{ "Load from profile", loadFromProfile3,        NULL },
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
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile4,        NULL },
	{ "Load from profile", loadFromProfile4,        NULL },
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
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile5,        NULL },
	{ "Load from profile", loadFromProfile5,        NULL },
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
	{ "< Return", act_back,        NULL },
	{ "Save to profile", saveToProfile6,        NULL },
	{ "Load from profile", loadFromProfile6,        NULL },
};
static Menu gProfile6Menu = {
    .items = PROFILE6_ITEMS,
    .count = sizeof(PROFILE6_ITEMS)/sizeof(PROFILE6_ITEMS[0]),
    .selected = 0, .top = 0,
    .last_drawn_selected = -1, .last_drawn_top = -1,
    .parent = NULL,
    .flags = 0,
};

static const MenuItem MOSFET1_ITEMS[] = {
	{ "< Return", act_back,        NULL },
	{ "Enable", NULL, NULL, &g_app_params.mosfets[0].enabled, 0, 1 },
	{ "Ext. Control", NULL, NULL, &g_app_params.mosfets[0].ext_control, 0, 1 },
	{ "Ton (ms)", NULL, NULL, &g_app_params.mosfets[0].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.mosfets[0].toff_ms, 0, 10000 },
	{ "SW. Cnt", NULL, NULL, &g_app_params.mosfets[0].sw_count, 0, 10000 },
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
	{ "Ton (ms)", NULL, NULL, &g_app_params.mosfets[1].ton_ms, 0, 10000 },
	{ "Toff (ms)", NULL, NULL, &g_app_params.mosfets[1].toff_ms, 0, 10000 },
	{ "SW. Cnt", NULL, NULL, &g_app_params.mosfets[1].sw_count, 0, 10000 },
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
    { "Relay1 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health[0] },
	{ "Relay2 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health[1] },
	{ "Relay3 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health[2] },
	{ "Relay4 (k)", NULL, NULL, NULL, 0, 0, &g_app_params.relay_health[3] },
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
	{ "Relay1 (k)", NULL, NULL, &g_app_params.relay_health[0], 0, 2000},
	{ "Relay2 (k)", NULL, NULL, &g_app_params.relay_health[1], 0, 2000},
	{ "Relay3 (k)", NULL, NULL, &g_app_params.relay_health[2], 0, 2000},
	{ "Relay4 (k)", NULL, NULL, &g_app_params.relay_health[3], 0, 2000},
};
static Menu gSetRelayHealth = {
    .items = SET_RELAY_HEALTH_ITEMS,
    .count = sizeof(SET_RELAY_HEALTH_ITEMS)/sizeof(SET_RELAY_HEALTH_ITEMS[0]),
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
	{ "Version:  0.1", NULL, NULL},
	{ "Author: V.Micek", NULL, NULL},
	{ "SCHAEFLER CZ sro", NULL, NULL},
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
    { "< Return", act_back,        NULL },
    { "Enable ",NULL, NULL, &g_app_params.trigger.enable, 0, 1},
	{ "Relay Ch.",NULL, NULL, &g_app_params.trigger.channel, 1, 4},
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
    { "Start Test",    act_start,     NULL },
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




void app_menu_init(void) {
    oled_init();
    menu_init(&gMainMenu, MAIN_ITEMS, sizeof(MAIN_ITEMS)/sizeof(MAIN_ITEMS[0]));
    menu_set_active(&gMainMenu);     // ensure active menu is main menu
    menu_set_wrap(&gMainMenu, 1);    // allow wrap only in main menu
}

void app_menu_task(void) {
    extern void menu_poll(Menu *menu); // from encoder_menu_glue.c
    menu_poll(menu_get_active());
}

void app_menu_on_value_commit(const MenuItem *it)
{
    (void)it;
    io_apply(io_get());
}


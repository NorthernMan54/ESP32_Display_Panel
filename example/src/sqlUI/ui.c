// This file was generated by SquareLine Studio
// SquareLine Studio version: SquareLine Studio 1.4.2
// LVGL version: 9.1.0
// Project name: SPA

#include "ui.h"
#include "ui_helpers.h"

///////////////////// VARIABLES ////////////////////


// SCREEN: ui_Loading_Screen
void ui_Loading_Screen_screen_init(void);
lv_obj_t *ui_Loading_Screen;
void ui_event_ThermostatLoading( lv_event_t * e);
lv_obj_t *ui_ThermostatLoading;
lv_obj_t *ui_HeatControlsLoading;
lv_obj_t *ui_uiHeatStateLoading;
lv_obj_t *ui_heatSwtichLabelLoading;
lv_obj_t *ui_heatStateSwitchlLoading;
lv_obj_t *ui_uiTempRangelLoading;
lv_obj_t *ui_tempRangeHighLabellLoading;
lv_obj_t *ui_tempRangeSwitchlLoading;
lv_obj_t *ui_tempRangeLowLabellLoading;
lv_obj_t *ui_uiPumpLoading;
lv_obj_t *ui_uiPumpLoading2;
lv_obj_t *ui_uiLightLoading;
lv_obj_t *ui_uiFilterLoading;
lv_obj_t *ui_uiTemperatureHistory2;
lv_obj_t *ui_uiTemperatureChart2;
lv_obj_t *ui_uiTemperatureChart2_Xaxis;
lv_obj_t *ui_uiTemperatureChart2_Yaxis1;
lv_obj_t *ui_uiTemperatureChart2_Yaxis2;
lv_obj_t *ui_temperatureChartLabel2;
lv_obj_t *ui_uiHeaterHistory2;
lv_obj_t *ui_uiHeaterChart2;
lv_obj_t *ui_uiHeaterChart2_Xaxis;
lv_obj_t *ui_uiHeaterChart2_Yaxis1;
lv_obj_t *ui_uiHeaterChart2_Yaxis2;
lv_obj_t *ui_heaterChartLabel2;
lv_obj_t *ui_uiClock2;
lv_obj_t *ui_uiClockLabel2;
lv_obj_t *ui_Container1;


// SCREEN: ui_Spa_Screen
void ui_Spa_Screen_screen_init(void);
void ui_event_Spa_Screen( lv_event_t * e);
lv_obj_t *ui_Spa_Screen;
void ui_event_uiThermostatPlaceholder( lv_event_t * e);
lv_obj_t *ui_uiThermostatPlaceholder;
lv_obj_t *ui_uiClock;
lv_obj_t *ui_uiClockLabel;
void ui_event_HeatControls( lv_event_t * e);
lv_obj_t *ui_HeatControls;
lv_obj_t *ui_uiHeatState;
lv_obj_t *ui_heatBlankLabel;
lv_obj_t *ui_heatStateSwitch;
lv_obj_t *ui_heatSwtichLabel;
lv_obj_t *ui_uiTempRange;
lv_obj_t *ui_tempRangeLowLabel1;
lv_obj_t *ui_tempRangeSwitch;
lv_obj_t *ui_tempRangeHighLabel;
lv_obj_t *ui_uiPump1;
void ui_event_uiPump1_Button( lv_event_t * e);
lv_obj_t *ui_uiPump2;
void ui_event_uiPump2_Button( lv_event_t * e);
lv_obj_t *ui_uiLight1;
void ui_event_uiLight1_Button( lv_event_t * e);
lv_obj_t *ui_uiFilter;
lv_obj_t *ui_uiTemperatureHistory;
lv_obj_t *ui_uiTemperatureChart;
lv_obj_t *ui_uiTemperatureChart_Xaxis;
lv_obj_t *ui_uiTemperatureChart_Yaxis1;
lv_obj_t *ui_uiTemperatureChart_Yaxis2;
lv_obj_t *ui_temperatureChartLabel;
lv_obj_t *ui_uiHeaterHistory;
lv_obj_t *ui_uiHeaterChart;
lv_obj_t *ui_uiHeaterChart_Xaxis;
lv_obj_t *ui_uiHeaterChart_Yaxis1;
lv_obj_t *ui_uiHeaterChart_Yaxis2;
lv_obj_t *ui_heaterChartLabel;
lv_obj_t *ui_Image8;


// SCREEN: ui_settingsAndAbout
void ui_settingsAndAbout_screen_init(void);
void ui_event_settingsAndAbout( lv_event_t * e);
lv_obj_t *ui_settingsAndAbout;
lv_obj_t *ui_TabView1;
lv_obj_t *ui_settingsPage;
lv_obj_t *ui_settingsContariner;
lv_obj_t *ui_brightnessContainer3;
lv_obj_t *ui_brightnessSlider3;
lv_obj_t *ui_brightnessLabel3;
lv_obj_t *ui_Checkbox2;
lv_obj_t *ui_aboutPage;
lv_obj_t *ui_Container9;
lv_obj_t *ui_aboutLabel;


// SCREEN: ui_adjustThermostatScreen
void ui_adjustThermostatScreen_screen_init(void);
void ui_event_adjustThermostatScreen( lv_event_t * e);
lv_obj_t *ui_adjustThermostatScreen;
void ui_event_uiThermostatPlaceholder3( lv_event_t * e);
lv_obj_t *ui_uiThermostatPlaceholder3;
lv_obj_t *ui_uiClock3;
lv_obj_t *ui_uiClockLabel3;
void ui_event_HeatControls2( lv_event_t * e);
lv_obj_t *ui_HeatControls2;
lv_obj_t *ui_uiHeatState2;
lv_obj_t *ui_heatBlankLabel2;
lv_obj_t *ui_heatStateSwitch2;
lv_obj_t *ui_heatSwtichLabel2;
lv_obj_t *ui_uiTempRange2;
lv_obj_t *ui_tempRangeLowLabel3;
lv_obj_t *ui_tempRangeSwitch2;
lv_obj_t *ui_tempRangeHighLabel2;
void ui_event_adjustThermostatContainer( lv_event_t * e);
lv_obj_t *ui_adjustThermostatContainer;
lv_obj_t *ui____initial_actions0;

///////////////////// TEST LVGL SETTINGS ////////////////////
#if LV_COLOR_DEPTH != 16
    #error "LV_COLOR_DEPTH should be 16bit to match SquareLine Studio's settings"
#endif

///////////////////// ANIMATIONS ////////////////////

///////////////////// FUNCTIONS ////////////////////
void ui_event_ThermostatLoading( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_adjustThermostatScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_adjustThermostatScreen_screen_init);
}
}
void ui_event_Spa_Screen( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_BOTTOM  ) {
lv_indev_wait_release(lv_indev_active());
      _ui_screen_change( &ui_settingsAndAbout, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_settingsAndAbout_screen_init);
}
}
void ui_event_uiThermostatPlaceholder( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_adjustThermostatScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_adjustThermostatScreen_screen_init);
}
}
void ui_event_HeatControls( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      uiHeatClick( e );
}
}
void ui_event_uiPump1_Button( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      uiPumpClick1( e );
}
}
void ui_event_uiPump2_Button( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      uiPumpClick2( e );
}
}
void ui_event_uiLight1_Button( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      uiLightClick1( e );
}
}
void ui_event_settingsAndAbout( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_TOP  ) {
lv_indev_wait_release(lv_indev_active());
      _ui_screen_change( &ui_Spa_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Spa_Screen_screen_init);
}
}
void ui_event_adjustThermostatScreen( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_GESTURE &&  lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_TOP  ) {
lv_indev_wait_release(lv_indev_active());
      _ui_screen_change( &ui_Spa_Screen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_Spa_Screen_screen_init);
}
}
void ui_event_uiThermostatPlaceholder3( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_adjustThermostatScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_adjustThermostatScreen_screen_init);
}
}
void ui_event_HeatControls2( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      uiHeatClick( e );
}
}
void ui_event_adjustThermostatContainer( lv_event_t * e) {
    lv_event_code_t event_code = lv_event_get_code(e);lv_obj_t * target = lv_event_get_target(e);
if ( event_code == LV_EVENT_CLICKED) {
      _ui_screen_change( &ui_adjustThermostatScreen, LV_SCR_LOAD_ANIM_FADE_ON, 500, 0, &ui_adjustThermostatScreen_screen_init);
}
}

///////////////////// SCREENS ////////////////////

void ui_init( void )
{LV_EVENT_GET_COMP_CHILD = lv_event_register_id();

lv_disp_t *dispp = lv_display_get_default();
lv_theme_t *theme = lv_theme_default_init(dispp, lv_palette_main(LV_PALETTE_BLUE), lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
lv_disp_set_theme(dispp, theme);
ui_Loading_Screen_screen_init();
// ui_Spa_Screen_screen_init();
// ui_settingsAndAbout_screen_init();
ui____initial_actions0 = lv_obj_create(NULL);
lv_disp_load_scr( ui_Loading_Screen);
}

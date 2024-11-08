#pragma once

#ifdef SPALVGL
#ifndef SPA_SHARED_H
#define SPA_SHARED_H



#ifdef __cplusplus
extern "C"
{
#endif
// #include "lvgl.h"

  extern lv_obj_t *currentTempNeedle;
  extern lv_obj_t *highTempNeedle;
  extern lv_obj_t *lowTempNeedle;
  extern lv_obj_t *temperatureGuage;

  lv_obj_t *thermostatArc(lv_obj_t *parent);

  void spaLvglClickButton(lv_event_t *e);
  void spaLvglClickPump(lv_event_t *e, int);
  void spaLvglClickLight(lv_event_t *e, int);
  void spaLvglClickHeat(lv_event_t *e);
  void adjustSpaScreen();

// uiMiscFunctions.c
  void spaButtonUpdate(lv_obj_t *component, uint8_t state);

#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif
#endif
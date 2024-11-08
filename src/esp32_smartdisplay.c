#include <esp32_smartdisplay.h>
#include "display.h"
#include "esp_bsp.h"
#include "lv_port.h"
#include "esp_log.h"

void smartdisplay_init()
{
  // log_d("smartdisplay_init");

  bsp_display_cfg_t cfg = {
      .lvgl_port_cfg = ESP_LVGL_PORT_INIT_CONFIG(),
      .buffer_size = EXAMPLE_LCD_QSPI_H_RES * EXAMPLE_LCD_QSPI_V_RES,
#if LVGL_PORT_ROTATION_DEGREE == 90
      .rotate = LV_DISPLAY_ROTATION_90,
#elif LVGL_PORT_ROTATION_DEGREE == 270
      .rotate = LV_DISPLAY_ROTATION_270,
#elif LVGL_PORT_ROTATION_DEGREE == 180
      .rotate = LV_DISPLAY_ROTATION_180,
#elif LVGL_PORT_ROTATION_DEGREE == 0
      .rotate = LV_DISPLAY_ROTATION_0,
#endif
  };

  bsp_display_start_with_config(&cfg);
  bsp_display_backlight_on();
  // bsp_display_lock(0);
}


/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include "esp_system.h"
#include "esp_log.h"
#include "esp_err.h"
#include "esp_check.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_interface.h"

#include "lv_port.h"
#include "lvgl.h"

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
#include "esp_lcd_touch.h"
#endif

#if (ESP_IDF_VERSION < ESP_IDF_VERSION_VAL(4, 4, 4)) || (ESP_IDF_VERSION == ESP_IDF_VERSION_VAL(5, 0, 0))
#define LVGL_PORT_HANDLE_FLUSH_READY 0
#else
#define LVGL_PORT_HANDLE_FLUSH_READY 1
#endif

static const char *TAG = "LVGL";

/*******************************************************************************
 * Types definitions
 *******************************************************************************/

typedef struct lvgl_port_ctx_s
{
    SemaphoreHandle_t lvgl_mux;
    esp_timer_handle_t tick_timer;
    bool running;
    int task_max_sleep_ms;
} lvgl_port_ctx_t;

typedef struct
{
    esp_lcd_panel_io_handle_t io_handle; /* LCD panel IO handle */
    esp_lcd_panel_handle_t panel_handle; /* LCD panel handle */
    lv_display_t *disp_drv;              /* LVGL display driver */

    uint32_t trans_size;              /* Maximum size for one transport */
    lv_color_t *trans_buf_1;          /* Buffer send to driver */
    lv_color_t *trans_buf_2;          /* Buffer send to driver */
    lv_color_t *trans_act;            /* Active buffer for sending to driver */
    SemaphoreHandle_t trans_done_sem; /* Semaphore for signaling idle transfer */
    lv_display_rotation_t sw_rotate;  /* Panel software rotation mask */

    lvgl_port_wait_cb draw_wait_cb; /* Callback function for drawing */
} lvgl_port_display_ctx_t;

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
typedef struct
{
    esp_lcd_touch_handle_t handle;   /* LCD touch IO handle */
    lv_indev_t *indev_drv;           /* LVGL input device driver */
    lvgl_port_wait_cb touch_wait_cb; /* Callback function for touch */
} lvgl_port_touch_ctx_t;
#endif

/*******************************************************************************
 * Local variables
 *******************************************************************************/
static lvgl_port_ctx_t lvgl_port_ctx;
static int lvgl_port_timer_period_ms = 5;

/*******************************************************************************
 * Function definitions
 *******************************************************************************/
static void lvgl_port_task(void *arg);
static esp_err_t lvgl_port_tick_init(void);
static void lvgl_port_task_deinit(void);

// LVGL callbacks
#if LVGL_PORT_HANDLE_FLUSH_READY
static bool lvgl_port_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
#endif
static void lvgl_port_flush_callback(lv_display_t *drv, const lv_area_t *area, lv_color_t *color_map);
#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
static void lvgl_port_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data);
#endif
/*******************************************************************************
 * Public API functions
 *******************************************************************************/

esp_err_t lvgl_port_init(const lvgl_port_cfg_t *cfg)
{
    esp_err_t ret = ESP_OK;
    ESP_GOTO_ON_FALSE(cfg, ESP_ERR_INVALID_ARG, err, TAG, "invalid argument");
    ESP_GOTO_ON_FALSE(cfg->task_affinity < (configNUM_CORES), ESP_ERR_INVALID_ARG, err, TAG, "Bad core number for task! Maximum core number is %d", (configNUM_CORES - 1));

    memset(&lvgl_port_ctx, 0, sizeof(lvgl_port_ctx));

    /* LVGL init */
    lv_init();
    /* Tick init */
    lvgl_port_timer_period_ms = cfg->timer_period_ms;
    ESP_RETURN_ON_ERROR(lvgl_port_tick_init(), TAG, "");
    /* Create task */
    lvgl_port_ctx.task_max_sleep_ms = cfg->task_max_sleep_ms;
    if (lvgl_port_ctx.task_max_sleep_ms == 0)
    {
        lvgl_port_ctx.task_max_sleep_ms = 500;
    }
    lvgl_port_ctx.lvgl_mux = xSemaphoreCreateRecursiveMutex();
    ESP_GOTO_ON_FALSE(lvgl_port_ctx.lvgl_mux, ESP_ERR_NO_MEM, err, TAG, "Create LVGL mutex fail!");

    BaseType_t res;
    if (cfg->task_affinity < 0)
    {
        res = xTaskCreate(lvgl_port_task, "LVGL task", cfg->task_stack, NULL, cfg->task_priority, NULL);
    }
    else
    {
        res = xTaskCreatePinnedToCore(lvgl_port_task, "LVGL task", cfg->task_stack, NULL, cfg->task_priority, NULL, cfg->task_affinity);
    }
    ESP_GOTO_ON_FALSE(res == pdPASS, ESP_FAIL, err, TAG, "Create LVGL task fail!");

err:
    if (ret != ESP_OK)
    {
        lvgl_port_deinit();
    }

    return ret;
}

esp_err_t lvgl_port_resume(void)
{
    esp_err_t ret = ESP_ERR_INVALID_STATE;

    if (lvgl_port_ctx.tick_timer != NULL)
    {
        lv_timer_enable(true);
        ret = esp_timer_start_periodic(lvgl_port_ctx.tick_timer, lvgl_port_timer_period_ms * 1000);
    }

    return ret;
}

esp_err_t lvgl_port_stop(void)
{
    esp_err_t ret = ESP_ERR_INVALID_STATE;

    if (lvgl_port_ctx.tick_timer != NULL)
    {
        lv_timer_enable(false);
        ret = esp_timer_stop(lvgl_port_ctx.tick_timer);
    }

    return ret;
}

esp_err_t lvgl_port_deinit(void)
{
    /* Stop and delete timer */
    if (lvgl_port_ctx.tick_timer != NULL)
    {
        esp_timer_stop(lvgl_port_ctx.tick_timer);
        esp_timer_delete(lvgl_port_ctx.tick_timer);
        lvgl_port_ctx.tick_timer = NULL;
    }

    /* Stop running task */
    if (lvgl_port_ctx.running)
    {
        lvgl_port_ctx.running = false;
    }
    else
    {
        lvgl_port_task_deinit();
    }

    return ESP_OK;
}

lv_display_t *lvgl_port_add_disp(const lvgl_port_display_cfg_t *disp_cfg)
{
    esp_err_t ret = ESP_OK;
    //    lv_display_t *disp = NULL;
    lv_color_t *buf1 = NULL;
    lv_color_t *buf2 = NULL;
    lv_color_t *buf3 = NULL;
    SemaphoreHandle_t trans_done_sem = NULL;

    assert(disp_cfg != NULL);
    assert(disp_cfg->io_handle != NULL);
    assert(disp_cfg->panel_handle != NULL);
    assert(disp_cfg->buffer_size > 0);
    assert(disp_cfg->hres > 0);
    assert(disp_cfg->vres > 0);

    /* Display context */
    lvgl_port_display_ctx_t *disp_ctx = malloc(sizeof(lvgl_port_display_ctx_t));
    ESP_GOTO_ON_FALSE(disp_ctx, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for display context allocation! %d", sizeof(lvgl_port_display_ctx_t));
    disp_ctx->io_handle = disp_cfg->io_handle;
    disp_ctx->panel_handle = disp_cfg->panel_handle;
    disp_ctx->trans_size = disp_cfg->trans_size;
    disp_ctx->sw_rotate = disp_cfg->sw_rotate;
    disp_ctx->draw_wait_cb = disp_cfg->draw_wait_cb;

    uint32_t buff_caps = MALLOC_CAP_DEFAULT;
    if (disp_cfg->flags.buff_dma)
    {
        buff_caps = MALLOC_CAP_DMA;
    }
    else if (disp_cfg->flags.buff_spiram)
    {
        buff_caps = MALLOC_CAP_SPIRAM;
    }

    /* alloc draw buffers used by LVGL */
    /* it's recommended to choose the size of the draw buffer(s) to be at least 1/10 screen sized */
    buf1 = heap_caps_malloc(disp_cfg->buffer_size * sizeof(lv_color_t), buff_caps);
    ESP_GOTO_ON_FALSE(buf1, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for LVGL buffer (buf1) allocation! %d", disp_cfg->buffer_size * sizeof(lv_color_t));

    if (disp_ctx->trans_size)
    {

        uint32_t caps = MALLOC_CAP_DMA;

        // buf2 = heap_caps_malloc(disp_ctx->trans_size * sizeof(lv_color_t), caps);

        buf2 = heap_caps_malloc(disp_cfg->buffer_size * sizeof(lv_color_t), buff_caps);
        ESP_GOTO_ON_FALSE(buf2, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for buffer2(transport) allocation! %d", disp_cfg->buffer_size * sizeof(lv_color_t));
        disp_ctx->trans_buf_1 = buf2;

        buf3 = heap_caps_malloc(disp_cfg->buffer_size * sizeof(lv_color_t), buff_caps);
        ESP_GOTO_ON_FALSE(buf3, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for buffer3(transport) allocation! %d", disp_cfg->buffer_size * sizeof(lv_color_t));
        disp_ctx->trans_buf_2 = buf3;

        trans_done_sem = xSemaphoreCreateCounting(1, 0);
        ESP_GOTO_ON_FALSE(trans_done_sem, ESP_ERR_NO_MEM, err, TAG, "Failed to create transport counting Semaphore");
        disp_ctx->trans_done_sem = trans_done_sem;
    }

    // lv_disp_draw_buf_t *disp_buf = malloc(sizeof(lv_disp_draw_buf_t));
    // ESP_GOTO_ON_FALSE(disp_buf, ESP_ERR_NO_MEM, err, TAG, "Not enough memory for LVGL display buffer allocation!");

    /* initialize LVGL draw buffers */
    // lv_disp_draw_buf_init(disp_buf, buf1, NULL, disp_cfg->buffer_size);

    lv_display_t *display = lv_display_create(disp_cfg->hres, disp_cfg->vres);
    lv_display_set_flush_cb(display, lvgl_port_flush_callback);
    lv_display_set_buffers(display, buf1, buf2, disp_cfg->buffer_size * sizeof(lv_color_t), LV_DISPLAY_RENDER_MODE_PARTIAL);

    ESP_LOGE(TAG, "Register display driver to LVGL buf1 %p, buf2 %p, buf3 %p", buf1, disp_ctx->trans_buf_1, disp_ctx->trans_buf_2);
    ESP_LOGE(TAG, "Register display driver to LVGL size %d", disp_cfg->buffer_size * sizeof(lv_color_t));
    ESP_LOGE(TAG, "Register display driver to LVGL trans_size %d", disp_ctx->trans_size * sizeof(lv_color_t));
    ESP_LOGD(TAG, "Register display driver to LVGL");

    //   ESP_LOGE(TAG, "Board: %s", BOARD_NAME);
    //   ESP_LOGE(TAG, "CPU: %s rev%d, CPU Freq: %d Mhz, %d core(s)", ESP.getChipModel(), ESP.getChipRevision(), getCpuFrequencyMhz(), ESP.getChipCores());
    ESP_LOGE(
        TAG,
        "Free Heap: %u bytes\n"
        "  MALLOC_CAP_8BIT      %7zu bytes\n"
        "  MALLOC_CAP_DMA       %7zu bytes\n"
        "  MALLOC_CAP_SPIRAM    %7zu bytes\n"
        "  MALLOC_CAP_INTERNAL  %7zu bytes\n"
        "  MALLOC_CAP_DEFAULT   %7zu bytes\n"
        "  MALLOC_CAP_IRAM_8BIT %7zu bytes\n"
        "  MALLOC_CAP_RETENTION %7zu bytes\n",
        xPortGetFreeHeapSize(),
        heap_caps_get_free_size(MALLOC_CAP_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_DMA),
        heap_caps_get_free_size(MALLOC_CAP_SPIRAM),
        heap_caps_get_free_size(MALLOC_CAP_INTERNAL),
        heap_caps_get_free_size(MALLOC_CAP_DEFAULT),
        heap_caps_get_free_size(MALLOC_CAP_IRAM_8BIT),
        heap_caps_get_free_size(MALLOC_CAP_RETENTION));

    // lv_disp_drv_init(&disp_ctx->disp_drv);
    //    disp_ctx->disp_drv.hor_res = disp_cfg->hres;
    //    disp_ctx->disp_drv.ver_res = disp_cfg->vres;
    //    disp_ctx->disp_drv.flush_cb = lvgl_port_flush_callback;

    //    disp_ctx->disp_drv.draw_buf = display;
    //    disp_ctx->disp_drv.user_data = disp_ctx;
    /* Force full_fresh */
    //    disp_ctx->disp_drv.full_refresh = 1;

#if LVGL_PORT_HANDLE_FLUSH_READY
    /* Register done callback */
    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = lvgl_port_flush_ready_callback,
    };
    esp_lcd_panel_io_register_event_callbacks(disp_ctx->io_handle, &cbs, &disp_ctx->disp_drv);
#endif

    lv_display_set_user_data(display, &disp_ctx);
    // disp = lv_disp_drv_register(&disp_ctx->disp_drv);

err:
    if (ret != ESP_OK)
    {
        if (buf1)
        {
            free(buf1);
        }
        if (buf2)
        {
            free(buf2);
        }
        if (buf3)
        {
            free(buf3);
        }
        if (trans_done_sem)
        {
            vSemaphoreDelete(trans_done_sem);
        }
        if (disp_ctx)
        {
            free(disp_ctx);
        }
    }

    return display;
}

esp_err_t lvgl_port_remove_disp(lv_display_t *disp)
{
    assert(disp);

    //    lv_display_t *disp_drv = disp->driver;
    //   assert(disp_drv);
    lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(disp);
    /*
            lv_disp_remove(disp);

            if (disp_drv)
            {
                if (disp_drv->draw_buf && disp_drv->draw_buf->buf1)
                {
                    free(disp_drv->draw_buf->buf1);
                    disp_drv->draw_buf->buf1 = NULL;
                }
                if (disp_drv->draw_buf && disp_drv->draw_buf->buf2)
                {
                    free(disp_drv->draw_buf->buf2);
                    disp_drv->draw_buf->buf2 = NULL;
                }
                if (disp_drv->draw_buf)
                {
                    free(disp_drv->draw_buf);
                    disp_drv->draw_buf = NULL;
                }
            }
    */
    free(disp_ctx);
    lv_display_delete(disp);
    return ESP_OK;
}

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
lv_indev_t *lvgl_port_add_touch(const lvgl_port_touch_cfg_t *touch_cfg)
{
    assert(touch_cfg != NULL);
    assert(touch_cfg->disp != NULL);
    assert(touch_cfg->handle != NULL);

    lv_indev_t *indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, lvgl_port_touchpad_read);

    /* Touch context */
    lvgl_port_touch_ctx_t *touch_ctx = malloc(sizeof(lvgl_port_touch_ctx_t));
    if (touch_ctx == NULL)
    {
        ESP_LOGE(TAG, "Not enough memory for touch context allocation!");
        return NULL;
    }
    touch_ctx->handle = touch_cfg->handle;
    touch_ctx->touch_wait_cb = touch_cfg->touch_wait_cb;

    /* Register a touchpad input device */
    // lv_indev_drv_init(&touch_ctx->indev_drv);
    // touch_ctx->indev_drv.type = LV_INDEV_TYPE_POINTER;
    // touch_ctx->indev_drv.disp = touch_cfg->disp;
    // touch_ctx->indev_drv.read_cb = lvgl_port_touchpad_read;
    // touch_ctx->indev_drv.user_data = touch_ctx;

    lv_indev_set_user_data(indev, touch_ctx);
    return indev;
}

esp_err_t lvgl_port_remove_touch(lv_indev_t *touch)
{
    assert(touch);
    //    lv_indev_t *indev_drv = touch->driver;
    //    assert(indev_drv);
    lvgl_port_touch_ctx_t *touch_ctx = (lvgl_port_touch_ctx_t *)lv_indev_get_user_data(touch);

    // Remove input device driver
    lv_indev_delete(touch);

    if (touch_ctx)
    {
        free(touch_ctx);
    }

    lv_indev_delete(touch);
    return ESP_OK;
}
#endif

bool lvgl_port_lock(uint32_t timeout_ms)
{
    assert(lvgl_port_ctx.lvgl_mux && "lvgl_port_init must be called first");

    const TickType_t timeout_ticks = (timeout_ms == 0) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTakeRecursive(lvgl_port_ctx.lvgl_mux, timeout_ticks) == pdTRUE;
}

void lvgl_port_unlock(void)
{
    assert(lvgl_port_ctx.lvgl_mux && "lvgl_port_init must be called first");
    xSemaphoreGiveRecursive(lvgl_port_ctx.lvgl_mux);
}

void lvgl_port_flush_ready(lv_display_t *disp)
{
    ESP_LOGD(TAG, "lvgl_port_flush_ready");
    // assert(disp);
    // assert(disp->driver);
    // lv_disp_flush_ready(disp->driver);
}

/*******************************************************************************
 * Private functions
 *******************************************************************************/

static void lvgl_port_task(void *arg)
{
    uint32_t task_delay_ms = lvgl_port_ctx.task_max_sleep_ms;

    ESP_LOGI(TAG, "Starting LVGL task");
    lvgl_port_ctx.running = true;
    while (lvgl_port_ctx.running)
    {
        if (lvgl_port_lock(0))
        {
            task_delay_ms = lv_timer_handler();
            lvgl_port_unlock();
        }
        if ((task_delay_ms > lvgl_port_ctx.task_max_sleep_ms) || (1 == task_delay_ms))
        {
            task_delay_ms = lvgl_port_ctx.task_max_sleep_ms;
        }
        else if (task_delay_ms < 1)
        {
            task_delay_ms = 1;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }

    lvgl_port_task_deinit();

    /* Close task */
    vTaskDelete(NULL);
}

static void lvgl_port_task_deinit(void)
{
    if (lvgl_port_ctx.lvgl_mux)
    {
        vSemaphoreDelete(lvgl_port_ctx.lvgl_mux);
    }
    memset(&lvgl_port_ctx, 0, sizeof(lvgl_port_ctx));
#if LV_ENABLE_GC || !LV_MEM_CUSTOM
    /* Deinitialize LVGL */
    lv_deinit();
#endif
}

#if LVGL_PORT_HANDLE_FLUSH_READY
static bool lvgl_port_flush_ready_callback(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    ESP_LOGE(TAG, "lvgl_port_flush_ready_callback");
    /*
    BaseType_t taskAwake = pdFALSE;

    lv_display_t *disp_drv = (lv_display_t *)user_ctx;
    assert(disp_drv != NULL);
    lvgl_port_display_ctx_t *disp_ctx = disp_drv->user_data;
    assert(disp_ctx != NULL);

    if (disp_ctx->trans_done_sem)
    {
        xSemaphoreGiveFromISR(disp_ctx->trans_done_sem, &taskAwake);
    }
*/
    return false;
}
#endif

static void lvgl_port_flush_callback(lv_display_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    ESP_LOGE(TAG, "lvgl_port_flush_callback");
    assert(drv != NULL);
    lvgl_port_display_ctx_t *disp_ctx = (lvgl_port_display_ctx_t *)lv_display_get_user_data(drv);
    assert(disp_ctx != NULL);

    const int x_start = area->x1;
    const int x_end = area->x2;
    const int y_start = area->y1;
    const int y_end = area->y2;
    const int width = x_end - x_start + 1;
    const int height = y_end - y_start + 1;

    lv_color_t *from = color_map;
    lv_color_t *to = NULL;

    ESP_LOGE(TAG, "lvgl_port_flush_callback from %p", from);
    ESP_LOGE(TAG, "lvgl_port_flush_callback trans_buf_1 %p", disp_ctx->trans_buf_1);
    ESP_LOGE(TAG, "lvgl_port_flush_callback trans_buf_2 %p", disp_ctx->trans_buf_2);
    ESP_LOGE(TAG, "lvgl_port_flush_callback trans_size %d", disp_ctx->trans_size);
    if (disp_ctx->trans_size)
    {
        assert(disp_ctx->trans_buf_1 != NULL);

        int x_draw_start = 0;
        int x_draw_end = 0;
        int y_draw_start = 0;
        int y_draw_end = 0;
        int trans_count = 0;

        disp_ctx->trans_act = disp_ctx->trans_buf_1;
        int rotate = disp_ctx->sw_rotate;

        int x_start_tmp = 0;
        int x_end_tmp = 0;
        int max_width = 0;
        int trans_width = 0;

        int y_start_tmp = 0;
        int y_end_tmp = 0;
        int max_height = 0;
        int trans_height = 0;

        if (LV_DISPLAY_ROTATION_270 == rotate || LV_DISPLAY_ROTATION_90 == rotate)
        {
            max_width = ((disp_ctx->trans_size / height) > width) ? (width) : (disp_ctx->trans_size / height);
            trans_count = width / max_width + (width % max_width ? (1) : (0));

            x_start_tmp = x_start;
            x_end_tmp = x_end;
        }
        else
        {
            max_height = ((disp_ctx->trans_size / width) > height) ? (height) : (disp_ctx->trans_size / width);
            trans_count = height / max_height + (height % max_height ? (1) : (0));

            y_start_tmp = y_start;
            y_end_tmp = y_end;
        }

        for (int i = 0; i < trans_count; i++)
        {

            if (LV_DISPLAY_ROTATION_90 == rotate)
            {
                trans_width = (x_end - x_start_tmp + 1) > max_width ? max_width : (x_end - x_start_tmp + 1);
                x_end_tmp = (x_end - x_start_tmp + 1) > max_width ? (x_start_tmp + max_width - 1) : x_end;
            }
            else if (LV_DISPLAY_ROTATION_270 == rotate)
            {
                trans_width = (x_end_tmp - x_start + 1) > max_width ? max_width : (x_end_tmp - x_start + 1);
                x_start_tmp = (x_end_tmp - x_start + 1) > max_width ? (x_end_tmp - trans_width + 1) : x_start;
            }
            else if (LV_DISPLAY_ROTATION_0 == rotate)
            {
                trans_height = (y_end - y_start_tmp + 1) > max_height ? max_height : (y_end - y_start_tmp + 1);
                y_end_tmp = (y_end - y_start_tmp + 1) > max_height ? (y_start_tmp + max_height - 1) : y_end;
            }
            else
            {
                trans_height = (y_end_tmp - y_start + 1) > max_height ? max_height : (y_end_tmp - y_start + 1);
                y_start_tmp = (y_end_tmp - y_start + 1) > max_height ? (y_end_tmp - max_height + 1) : y_start;
            }

            disp_ctx->trans_act = (disp_ctx->trans_act == disp_ctx->trans_buf_1) ? (disp_ctx->trans_buf_2) : (disp_ctx->trans_buf_1);
            to = disp_ctx->trans_act;

            ESP_LOGE(TAG, "LV_DISPLAY_ROTATION_0 to %p", to);
            if (to != NULL)
            {
                ESP_LOGE(TAG, "To target not set");
            }
            else
            {
                switch (rotate)
                {
                case LV_DISPLAY_ROTATION_90:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < trans_width; x++)
                        {
                            *(to + x * height + (height - y - 1)) = *(from + y * width + x_start_tmp + x);
                        }
                    }

                    x_draw_start = lv_display_get_vertical_resolution(drv) - y_end - 1;
                    x_draw_end = lv_display_get_vertical_resolution(drv) - y_start - 1;
                    y_draw_start = x_start_tmp;
                    y_draw_end = x_end_tmp;
                    break;
                case LV_DISPLAY_ROTATION_270:
                    for (int y = 0; y < height; y++)
                    {
                        for (int x = 0; x < trans_width; x++)
                        {
                            *(to + (trans_width - x - 1) * height + y) = *(from + y * width + x_start_tmp + x);
                        }
                    }
                    x_draw_start = y_start;
                    x_draw_end = y_end;
                    y_draw_start = lv_display_get_horizontal_resolution(drv) - x_end_tmp - 1;
                    y_draw_end = lv_display_get_horizontal_resolution(drv) - x_start_tmp - 1;
                    break;
                case LV_DISPLAY_ROTATION_180:
                    for (int y = 0; y < trans_height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            *(to + (trans_height - y - 1) * width + (width - x - 1)) = *(from + y_start_tmp * width + y * (width) + x);
                        }
                    }
                    x_draw_start = lv_display_get_horizontal_resolution(drv) - x_end - 1;
                    x_draw_end = lv_display_get_horizontal_resolution(drv) - x_start - 1;
                    y_draw_start = lv_display_get_vertical_resolution(drv) - y_end_tmp - 1;
                    y_draw_end = lv_display_get_vertical_resolution(drv) - y_start_tmp - 1;
                    break;
                case LV_DISPLAY_ROTATION_0:
                    ESP_LOGE(TAG, "LV_DISPLAY_ROTATION_0 %d %d %d ", trans_height, width, y_start_tmp);
                    for (int y = 0; y < trans_height; y++)
                    {
                        for (int x = 0; x < width; x++)
                        {
                            *(to + y * (width) + x) = *(from + y_start_tmp * width + y * (width) + x);
                        }
                    }
                    x_draw_start = x_start;
                    x_draw_end = x_end;
                    y_draw_start = y_start_tmp;
                    y_draw_end = y_end_tmp;
                    break;
                default:
                    break;
                }

                if (0 == i)
                {
                    if (disp_ctx->draw_wait_cb)
                    {
                        disp_ctx->draw_wait_cb(disp_ctx->panel_handle->user_data);
                    }
                    xSemaphoreGive(disp_ctx->trans_done_sem);
                }

                xSemaphoreTake(disp_ctx->trans_done_sem, portMAX_DELAY);
                esp_lcd_panel_draw_bitmap(disp_ctx->panel_handle, x_draw_start, y_draw_start, x_draw_end + 1, y_draw_end + 1, to);

                if (LV_DISPLAY_ROTATION_90 == rotate)
                {
                    x_start_tmp += max_width;
                }
                else if (LV_DISPLAY_ROTATION_270 == rotate)
                {
                    x_end_tmp -= max_width;
                }
                if (LV_DISPLAY_ROTATION_0 == rotate)
                {
                    y_start_tmp += max_height;
                }
                else
                {
                    y_end_tmp -= max_height;
                }
            }
        }
    }
    else
    {
        esp_lcd_panel_draw_bitmap(disp_ctx->panel_handle, x_start, y_start, x_end + 1, y_end + 1, color_map);
    }
    lv_disp_flush_ready(drv);
}

#ifdef ESP_LVGL_PORT_TOUCH_COMPONENT
static void lvgl_port_touchpad_read(lv_indev_t *indev_drv, lv_indev_data_t *data)
{
    assert(indev_drv);
    lvgl_port_touch_ctx_t *touch_ctx = (lvgl_port_touch_ctx_t *)lv_indev_get_user_data(indev_drv);
    assert(touch_ctx->handle);

    uint16_t touchpad_x[1] = {0};
    uint16_t touchpad_y[1] = {0};
    uint8_t touchpad_cnt = 0;

    /* Read data from touch controller into memory */
    bool touch_int = false;
    if (touch_ctx->touch_wait_cb)
    {
        touch_int = touch_ctx->touch_wait_cb(touch_ctx->handle->config.user_data);
    }
    if (touch_int)
    {
        esp_lcd_touch_read_data(touch_ctx->handle);
        /* Read data from touch controller */
        bool touchpad_pressed = esp_lcd_touch_get_coordinates(touch_ctx->handle, touchpad_x, touchpad_y, NULL, &touchpad_cnt, 1);

        if (touchpad_pressed && touchpad_cnt > 0)
        {
            data->point.x = touchpad_x[0];
            data->point.y = touchpad_y[0];
            data->state = LV_INDEV_STATE_PRESSED;
            esp_rom_printf("Touchpad pressed: x=%d, y=%d\n", data->point.x, data->point.y);
        }
        else
        {
            data->state = LV_INDEV_STATE_RELEASED;
        }
    }
}
#endif

static void lvgl_port_tick_increment(void *arg)
{
    /* Tell LVGL how many milliseconds have elapsed */
    lv_tick_inc(lvgl_port_timer_period_ms);
}

static esp_err_t lvgl_port_tick_init(void)
{
    // Tick interface for LVGL (using esp_timer to generate 2ms periodic event)
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_port_tick_increment,
        .name = "LVGL tick",
    };
    ESP_RETURN_ON_ERROR(esp_timer_create(&lvgl_tick_timer_args, &lvgl_port_ctx.tick_timer), TAG, "Creating LVGL timer filed!");
    return esp_timer_start_periodic(lvgl_port_ctx.tick_timer, lvgl_port_timer_period_ms * 1000);
}

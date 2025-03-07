#ifndef USER_LVGL_H
#define USER_LVGL_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_timer.h"

#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/spi_master.h"

#include "lvgl.h"

bool lvgl_lock(int timeout_ms);
void lvgl_unlock(void);
void lvgl_port_task(void *arg);

void lcd_spi_panel_Init(void);
void lcd_lvgl_Init(void);

//////////////////////////////////////按键相关///////////////////////////////////////////
#define KEYPAD_EN 
   #define KEY_LEFT_IO_NUM       GPIO_NUM_41       // 左
   #define KEY_RIGHT_IO_NUM      GPIO_NUM_39       // 右
   #define KEY_ENTER_IO_NUM      GPIO_NUM_40       // 确定

//#define BUTTON_EN

lv_group_t* lv_port_indev_init(void);
extern uint8_t event_code, data_key;

//////////////////////////////////////按键相关///////////////////////////////////////////

#ifdef __cplusplus
}
#endif

#endif

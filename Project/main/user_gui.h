#ifndef USER_GUI_H
#define USER_GUI_H

#ifdef __cplusplus
extern "C" {
#endif

#if         1
#include "user_lvgl.h"
#else
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "freertos/semphr.h"

#include "esp_timer.h"

#include "esp_err.h"
#include "esp_log.h"

#include "driver/gpio.h"

#include "lvgl.h"
#endif


/// @brief 带上下限限制的手动滚动条
void scroll_list_by(lv_obj_t * boj, int y_offset);
/// @brief 获取当前活跃的 Tab 对象
lv_obj_t *get_active_tab_obj(lv_obj_t *tabview);

void power_main_ui_init(void);

void gui_init(void);


extern lv_obj_t *tabview;
extern lv_obj_t *tab1, *tab2, *tab3;
extern uint8_t tab_idx;



#ifdef __cplusplus
}
#endif

#endif

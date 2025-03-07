/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */

/********************************** Copyright **********************************
 *                 (C) Copyright {2023},CAIJIAMING,China, GD.
 *                            All Rights Reserved
 *                              
 *                     By CAIJIAMING (caijiaming@live.com)
 *                      https://github.com/caijiaming1996
 *    
 * FileName   : multi_button
 * Version    : v1.3.1
 * Author     : CAIJIAMING
 * Date       : 2025-02-07
 * Changelog  ： 
 *	v1.3.1		2025-02-07		Author: CAIJIAMING		Description:	修复：连按触发max_repeat_cnt后，会导致额外触发一次SINGLE_CLICK事件；
 *																		优化：事件名称调整；
 *	v1.3.0		2024-09-06		Author: CAIJIAMING		Description:	优化：独立函数配置连按功能，并且增加最大连按次数，用于快速响应；
 *																		优化：活跃电平改为宏定义，节省1bit，可用于分配到其他变量；
 *																		优化：优化注释；
 *	v1.2.0		2024-09-06		Author: CAIJIAMING		Description:	新增：按键事件回调的代码裁剪宏;
 * 																		修复：原先PRESS_REPEAT事件每次按按键都会触发，无法区分多个不同次数的连按。现在只有在停止连按后触发一次（逻辑和原先的双击事件一样）；
 *																		优化：去掉双击事件，合并到PRESS_REPEAT连续按下事件中；
 *																		优化：PRESS_REPEAT和LONG_PRESS_UP事件可通过轮序event检测，不强制要求使用回调函数触发；
 *																		优化：减少event总数量（不能超过 8 个），增加repea_cnt连按的次数范围(7 → 15)；
 *	v1.1.2		2024-07-29		Author: CAIJIAMING		Description:	新增：使用CONTAIN_USER_BUTTON宏来选择是否用包含用户业务代码程序在当前文件中（简单程序建议包含）。
 * 	v1.1.1		2023-07-04		Author: CAIJIAMING		Description:	优化：写法优化。
 * 	v1.1.0		2023-04-16		Author: CAIJIAMING		Description:	新增：ADC按键；
 *																		优化：TICKS_INTERVAL调整；
 * 	v1.0.0		2023-02-13		Author: CAIJIAMING		Description:	新增：REPEAT部分代码裁剪宏；
 * 																		新增：LONG_PRESS_UP按键事件；
 * 																		优化：增加是否为连按的按键的标志，如果不是则直跳过SHORT_TICKS，快速响应SINGLE_CLICK。cnt(原repeat)改为3bit，分出1bit做是否为连按按键标记；
 *																		优化：增加repea_cnt计数溢出保护；
 *																		优化：解决按下按键后立马松开，然后马上再次按下按键并长按时，会处于多次按下状态，无法识别最后次长按的问题；
*******************************************************************************/

#include "multi_button.h"

#include <stdio.h>


//button handle list head.
static struct button_obj* head_handle = NULL;


/// @brief 初始化按钮结构句柄
/// @param handle 按键对象句柄
/// @param pin_level 绑定按键读取函数
void button_init(struct button_obj* handle, uint8_t(*pin_level)())
{
	memset(handle, 0, sizeof(struct button_obj));
	handle->event = (uint8_t)NONE_PRESSED;
	handle->hal_button_Level = pin_level;
	handle->button_level = handle->hal_button_Level();
}

#if COMPILE_REPEAT_FUN == 1
/// @brief 配置按键连按功能
/// @param handle 按键对象句柄
/// @param repeat_en 是否启用连按功能
/// @param max_repeat_cnt 最大支持连按次数（到最大次数后立马响应连按事件）
void button_repeat_config(struct button_obj* handle, uint8_t repeat_en, uint8_t max_repeat_cnt)
{
	handle->repeat_en = repeat_en;

	if(handle->repeat_en)
	{
		handle->max_repeat_cnt = max_repeat_cnt;

		if(handle->max_repeat_cnt == 0)
			handle->max_repeat_cnt = 2;
		else
		if(handle->max_repeat_cnt > 0x0f)
			handle->max_repeat_cnt = 15;
	}
}
#endif

#if COMPILE_CALL_BACK_FUN == 1
/// @brief 绑定按键事件回调函数
/// @param handle 按键对象句柄
/// @param event 回调函数触发事件类型
/// @param cb 回调函数指针
void button_attach_cb(struct button_obj* handle, press_event event, BtnCallback cb)
{
	handle->cb[event] = cb;
}

#define  EVENT_CB(ev)   if(handle->cb[ev]) handle->cb[ev]()
#else
#define  EVENT_CB(ev)	
#endif


/// @brief 查询发生的按钮事件
/// @param handle 按键对象句柄
/// @return 按键事件
press_event get_button_event(struct button_obj* handle)
{
	return (press_event)(handle->event);
}


/// @brief 按键驱动处理函数
/// @param handle 按键对象句柄
static void button_handler(struct button_obj* handle)
{
	uint8_t read_gpio_level = handle->hal_button_Level();

	//ticks counter working..
	if((handle->state) > 0) 
		handle->ticks++;

	/*------------button debounce handle---------------*/
	if(read_gpio_level != handle->button_level) //not equal to prev one
	{ 
		//continue read 3 times same new level change
		if(++(handle->debounce_cnt) > DEBOUNCE_TICKS) 
		{
			handle->button_level = read_gpio_level;
			handle->debounce_cnt = 0;
			// printf("%d\n", read_gpio_level);
		}
	} 
	else 
	{	//leved not change ,counter reset.
		handle->debounce_cnt = 0;
	}

	/*-----------------State machine-------------------*/
	switch (handle->state) 
	{
		case 0:
			if(handle->button_level == ACTIVE_LEVEL)	 	// 开始按下
			{	
				handle->event = (uint8_t)PRESSED;
				EVENT_CB(PRESSED);
				handle->ticks = 0;
				handle->repea_cnt = 1;
				handle->state = 1;
			} 
			else 
			{
				handle->event = (uint8_t)NONE_PRESSED;		// 未按下
			}
		break;

		case 1:
			if(handle->button_level != ACTIVE_LEVEL) 		// 释放松开
			{ 	
				handle->event = (uint8_t)RELEASED;
				EVENT_CB(RELEASED);
				handle->ticks = 0;
				handle->state = 2;
			}
			else 
			if(handle->ticks > LONG_TICKS) 					// 持续保持按下
			{
				handle->event = (uint8_t)LONG_PRESSED;
				EVENT_CB(LONG_PRESSED);
				handle->state = 5;
			}
		break;

		case 2:
#if COMPILE_REPEAT_FUN == 1
			if(handle->repeat_en == 0)	// 判断是否为多次点击的按键，如果不是则快速响应SINGLE_CLICK。
#endif
			{
				if(handle->button_level != ACTIVE_LEVEL) 	// 释放松开
				{
					handle->event = (uint8_t)PRESSED_SINGLE;
					EVENT_CB(PRESSED_SINGLE);

					handle->state = 0;	
				}
				else
				if(handle->ticks > LONG_TICKS) 				// 持续保持按下
				{
					handle->event = (uint8_t)LONG_PRESSED;
					EVENT_CB(LONG_PRESSED);
					handle->state = 5;
				}
			}
#if COMPILE_REPEAT_FUN == 1
			else 
			{
				if(handle->button_level == ACTIVE_LEVEL)  	// 再次按下（连按）
				{
					handle->event = (uint8_t)PRESSED;
					EVENT_CB(PRESSED);

					handle->ticks = 0;
					handle->state = 3;

					if(handle->repea_cnt < 15) 
					{
						if(++handle->repea_cnt == handle->max_repeat_cnt)
						{
							handle->event = (uint8_t)PRESSED_REPEAT;
							EVENT_CB(PRESSED_REPEAT);
							handle->state = 6;
						}
					}
				}
				else
				if(handle->ticks > SHORT_TICKS) 					// 释放超时
				{
					if(handle->repea_cnt == 1) 
					{
						handle->event = (uint8_t)PRESSED_SINGLE;		// 单击
						EVENT_CB(PRESSED_SINGLE);
					} 
					else 
					{
						handle->event = (uint8_t)PRESSED_REPEAT;
						EVENT_CB(PRESSED_REPEAT); 					// 连续按下触发
					}
					handle->state = 0;
				}
			}
#endif
		break;
			
#if COMPILE_REPEAT_FUN == 1
		case 3:
			if(handle->button_level != ACTIVE_LEVEL) 		// 释放松开
			{
				handle->event = (uint8_t)RELEASED;
				EVENT_CB(RELEASED);
				if(handle->ticks < SHORT_TICKS) 
				{
					handle->ticks = 0;
					handle->state = 2; //repeat press
				}
				else 
				{
					handle->state = 0;
				}
			}
			// 解决按下按键后立马松开，然后马上再次按下按键并长按时，会处于多次按下状态，无法识别最后次长按的问题。
			else
			if(handle->ticks > LONG_TICKS) 					// 持续保持按下
			{
				handle->event = (uint8_t)LONG_PRESSED;
				EVENT_CB(LONG_PRESSED);
				handle->state = 5;
			}
		break;
#endif
		case 5:
			if(handle->button_level == ACTIVE_LEVEL) 		// 长按持续触发
			{
				handle->event = (uint8_t)LONG_PRESSED_HOLD;
				EVENT_CB(LONG_PRESSED_HOLD);
			}
			else  											// 长按释放后触发一次
			{
				// handle->event = (uint8_t)RELEASED;
				// EVENT_CB(RELEASED);

				handle->event = (uint8_t)LONG_PRESSED_RELEASED;
				EVENT_CB(LONG_PRESSED_RELEASED);
				
				handle->state = 0;
			}
		break;

		case 6:
			handle->event = NONE_PRESSED;
			if(handle->button_level != ACTIVE_LEVEL) 		// 释放松开
			{
				handle->state = 0;
			}
		break;
	}
}


/// @brief 添加按键对象句柄到工作列表中，并且启动按键工作
/// @param handle 按键对象
/// @return 0: succeed. -1: already exist.
int button_start(struct button_obj* handle)
{
	struct button_obj* target = head_handle;
	while(target) {
		if(target == handle) return -1;	//already exist.
		target = target->next;
	}
	handle->next = head_handle;
	head_handle = handle;
	return 0;
}


/// @brief 停止按键工作，将从工作列表中移除
/// @param handle 按键对象
void button_stop(struct button_obj* handle)
{
	struct button_obj** curr;
	for(curr = &head_handle; *curr; ) {
		struct button_obj* entry = *curr;
		if (entry == handle) {
			*curr = entry->next;
//			free(entry);
		} else
			curr = &entry->next;
	}
}


#if 	COMPILE_ADC_KEY_FUN == 1
void key_id_read(void);
#endif

/// @brief 按键事件处理函数
/// @note 后台计时，计时器重复调用，间隔 TICKS_INTERVAL ms。
void button_ticks()
{
	struct button_obj* target;
	
	#if		COMPILE_ADC_KEY_FUN == 1
	key_id_read();
	#endif
	
	for(target=head_handle; target; target=target->next) {
		button_handler(target);
	}
}


/*********************************************************************************************************************/
#if		CONTAIN_USER_BUTTON == 1

#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

// AD按键
#if		COMPILE_ADC_KEY_FUN == 1
volatile uint16_t key_ad_value;
uint8_t	key_id;


void key_id_read(void)
{
	key_ad_value = GET_KEY_AD_VALUE();

	if((key_ad_value > (KEY1_AD_VAL - ADC_KEY_BIAS)) && (key_ad_value < (KEY1_AD_VAL + ADC_KEY_BIAS)))
		key_id = 1;
	else
	if((key_ad_value > (KEY2_AD_VAL - ADC_KEY_BIAS)) && (key_ad_value < (KEY2_AD_VAL + ADC_KEY_BIAS)))
		key_id = 2;
	else
	if((key_ad_value > (KEY3_AD_VAL - ADC_KEY_BIAS)) && (key_ad_value < (KEY3_AD_VAL + ADC_KEY_BIAS)))
		key_id = 3;
	else
	if((key_ad_value > (KEY4_AD_VAL - ADC_KEY_BIAS)) && (key_ad_value < (KEY4_AD_VAL + ADC_KEY_BIAS)))
		key_id = 4;
	else
		key_id = 0;
}


button_obj key1;
uint8_t key1_get_level(void)
{
	if(key_id == 1)	return 0;
	else	return 1;
}

button_obj key2;
uint8_t get_key2_level(void)
{
	if(key_id == 2)	return 0;
	else	return 1;
}

button_obj key3;
uint8_t get_key3_level(void)
{
	if(key_id == 3)	return 0;
	else	return 1;
}

button_obj key4;
uint8_t get_key4_level(void)
{
	if(key_id == 4)	return 0;
	else	return 1;
}

#endif


#ifdef KEY1_GPIO_NUM
button_obj key1;
uint8_t key1_get_level(void)
{
	if(gpio_get_level(KEY1_GPIO_NUM) == 0) 
		return ACTIVE_LEVEL;
	else 
		return !ACTIVE_LEVEL;

	// return KEY1_GPIO == 0 ? ACTIVE_LEVEL : !ACTIVE_LEVEL;
}

// void key1_press_repeat_cb(void)
// {

// }
#endif

#ifdef KEY2_GPIO_NUM
button_obj key2;
uint8_t key2_get_level(void)
{
	if(gpio_get_level(KEY2_GPIO_NUM) == 0) 
		return ACTIVE_LEVEL;
	else 
		return !ACTIVE_LEVEL;

	// return KEY1_GPIO == 0 ? ACTIVE_LEVEL : !ACTIVE_LEVEL;
}

// void key1_press_repeat_cb(void)
// {

// }
#endif

#ifdef KEY3_GPIO_NUM
button_obj key3;
uint8_t key3_get_level(void)
{
	if(gpio_get_level(KEY3_GPIO_NUM) == 0) 
		return ACTIVE_LEVEL;
	else 
		return !ACTIVE_LEVEL;

	// return KEY1_GPIO == 0 ? ACTIVE_LEVEL : !ACTIVE_LEVEL;
}

// void key1_press_repeat_cb(void)
// {

// }
#endif


void key_init(void)
{
	// GPIO配置
	gpio_config_t io_conf = {
		.mode = GPIO_MODE_INPUT,
		.pull_up_en = 1,
	};

#ifdef KEY1_GPIO_NUM
	io_conf.pin_bit_mask = BIT64(KEY1_GPIO_NUM);
	gpio_config(&io_conf);

	// 配置并注册按键
	button_init(&key1, key1_get_level);
#if COMPILE_REPEAT_FUN == 1
	button_repeat_config(&key1, 1, 2);
#endif
#if COMPILE_CALL_BACK_FUN == 1	
	button_attach_cb(&key1, PRESSED_REPEAT, key1_press_repeat_cb);
#endif
	button_start(&key1);
#endif

#ifdef KEY2_GPIO_NUM
	io_conf.pin_bit_mask = BIT64(KEY2_GPIO_NUM);
	gpio_config(&io_conf);

	// 配置并注册按键
	button_init(&key2, key2_get_level);
#if COMPILE_REPEAT_FUN == 1
	button_repeat_config(&key2, 1, 3);
#endif
#if COMPILE_CALL_BACK_FUN == 1	
	button_attach_cb(&key2, PRESSED_REPEAT, key2_press_repeat_cb);
#endif
	button_start(&key2);
#endif

#ifdef KEY3_GPIO_NUM
	io_conf.pin_bit_mask = BIT64(KEY3_GPIO_NUM);
	gpio_config(&io_conf);

	// 配置并注册按键
	button_init(&key3, key3_get_level);
#if COMPILE_REPEAT_FUN == 1
	button_repeat_config(&key3, 1, 4);
#endif
#if COMPILE_CALL_BACK_FUN == 1	
	button_attach_cb(&key3, PRESSED_REPEAT, key3_press_repeat_cb);
#endif
	button_start(&key3);
#endif

	void key_task(void *param);
	xTaskCreate(&key_task, "key_task", 1024 * 1, NULL, configMAX_PRIORITIES - 1, NULL);
}



#include "user_gui.h"
#include "user_source.h"


void key_task(void *param)
{
	while(1)
	{
		button_ticks();
		
		switch(get_button_event(&key1))
		{
			case LONG_PRESSED:					// 
			{
				ESP_LOGI("BUTTON", "key1 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				ESP_LOGI("BUTTON", "key1 single_click\n");

			}break;
			
			case PRESSED_REPEAT:						// 
			{
				ESP_LOGI("BUTTON", "key1 press_repeat %d\n", key1.repea_cnt);
			
				// if(key1.repea_cnt == 2)
				// {}
				// else
				// if(key1.repea_cnt == 3)
				// {}
			}break;

			default: break;
		}
		switch(get_button_event(&key2))
		{
			case LONG_PRESSED:					// 
			{
				// ESP_LOGI("BUTTON", "key2 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				// ESP_LOGI("BUTTON", "key2 single_click\n");

				lv_obj_scroll_by(pow_ui1, 0, -20, LV_ANIM_OFF);  // 向上滚动

			}break;

			case LONG_PRESSED_HOLD:					// 
			{
				lv_obj_scroll_by(pow_ui1, 0, -3, LV_ANIM_OFF);  // 向上滚动
			}break;
			
			case PRESSED_REPEAT:						// 
			{
				// ESP_LOGI("BUTTON", "key2 press_repeat %d\n", key2.repea_cnt);
			
				// if(key2.repea_cnt == 2)
				// {}
				// else
				// if(key2.repea_cnt == 3)
				// {}
			}break;

			default: break;
		}
		switch(get_button_event(&key3))
		{
			case LONG_PRESSED:					// 
			{
				// ESP_LOGI("BUTTON", "key3 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				// ESP_LOGI("BUTTON", "key3 single_click\n");

				lv_obj_scroll_by(pow_ui1, 0, 20, LV_ANIM_OFF);  // 向上滚动

			}break;

			case LONG_PRESSED_HOLD:					// 
			{
				lv_obj_scroll_by(pow_ui1, 0, 3, LV_ANIM_OFF);  // 向上滚动
			}break;
			
			case PRESSED_REPEAT:						// 
			{
				// ESP_LOGI("BUTTON", "key3 press_repeat %d\n", key3.repea_cnt);
			
				// if(key3.repea_cnt == 2)
				// {}
				// else
				// if(key3.repea_cnt == 3)
				// {}
			}break;

			default: break;
		}

		vTaskDelay(pdMS_TO_TICKS(TICKS_INTERVAL));
	}
}

#endif // CONTAIN_USER_BUTTON == 1

/*********************************************************************************************************************/
/*
// DEMO

#ifdef PAD_GPIO_KEY1
uint8_t key1_get_level(void)
{
	// if(KEY1_GPIO == 0)
	// 	return ACTIVE_LEVEL;
	// else
	// 	return !ACTIVE_LEVEL;

	return KEY1_GPIO == 0 ? ACTIVE_LEVEL : !ACTIVE_LEVEL;
}

void key1_press_repeat_cb(void)
{
	 if(key1.repea_cnt == 3)
	 {

	 }
}
#endif

// 1、初始化
void key_init(void)
{
	// GPIO配置.....	

	// 配置并注册按键
#ifdef PAD_GPIO_KEY1
	button_init(&key1, key1_get_level);
#if COMPILE_REPEAT_FUN == 1
	button_repeat_config(&key1, 1, debug_button_en==true? 10 : 3);
#endif
#if COMPILE_CALL_BACK_FUN == 1	
	button_attach_cb(&key1, PRESSED_REPEAT, key1_press_repeat_cb);
#endif
	button_start(&key1);
#endif
}

// 2、添加任务，按照TICKS_INTERVAL周期运行
void key_task(void)
{
	button_ticks();
    
    switch(get_button_event(&key1))
	{
		case LONG_PRESSED:					// 
		{

		}break;
		
		case PRESSED_SINGLE:						// 
		{

		}break;
        
        case PRESSED_REPEAT:						// 
        {			
			if(key1.repea_cnt == 2)
			{}
			else
			if(key1.repea_cnt == 3)
			{}
        }break;
	}
}
*/

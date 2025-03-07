/*
 * Copyright (c) 2016 Zibin Zheng <znbin@qq.com>
 * All rights reserved
 */
 
#ifndef _MULTI_BUTTON_H_
#define _MULTI_BUTTON_H_

#include "stdint.h"
#include "string.h"


#define	CONTAIN_USER_BUTTON					0		// 是否包含用户业务代码程序在当前文件中

#define	COMPILE_REPEAT_FUN					1		// code裁剪，是否编译 连按 功能相关代码（code ≈ 265 Byte, data ≈ 0 Byte)
#define	COMPILE_CALL_BACK_FUN				0		// code裁剪，是否编译 回调 功能相关代码（两个按键实例时，可减少code ≈ 76 Byte, data ≈ 68 Byte)


#define ACTIVE_LEVEL						0								// 按键活跃电平

#define TICKS_INTERVAL   				 	(10)							// button_ticks对应的运行周期，单位1ms				建议：5 ~ 10
#define DEBOUNCE_TICKS   				 	(2)								// 防抖次数，范围：0~7（防抖时间 = TICKS_INTERVAL * (DEBOUNCE_TICKS + 1)）
#define SHORT_TICKS     				 	((300) / TICKS_INTERVAL)		// 短按和连按的生效时间，单位1ms					 建议：200 ~ 300
#define LONG_TICKS       				 	((1000) / TICKS_INTERVAL)		// 长按的生效时间，单位1ms							建议：1000 ~ 1500


typedef void (*BtnCallback)(void);

typedef enum {
	PRESSED = 0,  							// 按键按下，每次按下都触发
	RELEASED,        						// 按键释放，每次松开都触发（如果是长按后松开，则是LONG_PRESSED_RELEASED）
	
	PRESSED_SINGLE,    						// 单击按键事件
	PRESSED_REPEAT,    						// 连续多次按下(含双击）触发，repea_cnt 记录连击次数（使用时需要将 repeat_en 配置为1）
	
	LONG_PRESSED,							// 达到长按时间阈值时触发一次
	LONG_PRESSED_HOLD, 						// 长按期间持续触发
	LONG_PRESSED_RELEASED,					// 长按释放后触发一次（考虑去掉改事件，新增点击后持续触发时间）
	
	NONE_PRESSED = 7,       				// 没有按下

	KEY_EVENT_MAX = 8,						// 事件数量不能超过 8 个
}press_event;

typedef struct button_obj {
	uint16_t ticks;                     	// 计时计数	
	uint8_t  event : 3;                 	// 按键事件（范围：0-7）
#if COMPILE_REPEAT_FUN == 1
	uint8_t  repeat_en : 1;					// 是否为连按按键标记（0：非连按按键（可提高非连按按键的响应速度） 1：连按按键）
	uint8_t  repea_cnt : 4;             	// 连按次数（范围：0-15）
	uint8_t  max_repeat_cnt;				// 最大连按次数（用于快速响应）
#endif
	uint8_t  state : 3;                 	// 状态机状态
	uint8_t  debounce_cnt : 3;          	// 防抖计数
	// uint8_t  active_level : 1;          	// 有效电平
	uint8_t  button_level : 1;	        	// 当前按键电平	
	uint8_t  (*hal_button_Level)(void);		// 按键电平获取函数指针
#if COMPILE_CALL_BACK_FUN == 1	
	BtnCallback  cb[KEY_EVENT_MAX];			// 按键事件的回调函数指针
#endif
	struct button_obj* next;
}button_obj;


#ifdef __cplusplus  
extern "C" {  
#endif  

/// @brief 初始化按钮结构句柄
void button_init(struct button_obj* handle, uint8_t(*pin_level)());
#if COMPILE_REPEAT_FUN == 1
/// @brief 配置按键连按功能
void button_repeat_config(struct button_obj* handle, uint8_t repeat_en, uint8_t max_repeat_cnt);
#endif
#if COMPILE_CALL_BACK_FUN == 1
/// @brief 绑定按键事件回调函数
void button_attach_cb(struct button_obj* handle, press_event event, BtnCallback cb);
#endif
/// @brief 查询发生的按钮事件
press_event get_button_event(struct button_obj* handle);
/// @brief 添加按键对象句柄到工作列表中，并且启动按键工作
int  button_start(struct button_obj* handle);
/// @brief 停止按键工作，将从工作列表中移除
void button_stop(struct button_obj* handle);
/// @brief 按键事件处理函数
void button_ticks(void);

#ifdef __cplusplus
} 
#endif


#if CONTAIN_USER_BUTTON == 1

#include "driver/gpio.h"

// 电源按键
#define	KEY1_GPIO_NUM		GPIO_NUM_39
#define	KEY2_GPIO_NUM		GPIO_NUM_40
#define	KEY3_GPIO_NUM		GPIO_NUM_41


#define	COMPILE_ADC_KEY_FUN					0		// code裁剪，是否编译ADC按键相关代码

#if		COMPILE_ADC_KEY_FUN == 1
#include "ADC.h"

// 获取ADC按键AD值
#define		GET_KEY_AD_VALUE()				GetADValue(KEY_AD_CHAN, Vref_VDD, 5)

// 按键对应AD值
#define		ADC_KEY_BIAS		50				// 允许公差

#define		KEY_NC_VAL			4095			// 没按键按下时

#define		KEY1_AD_VAL			398
#define		KEY2_AD_VAL			779
#define		KEY3_AD_VAL			1365
#define		KEY4_AD_VAL			2048

extern volatile uint16_t Key_AD_Value;
extern uint8_t	Key_ID;

#endif


#ifdef KEY1_GPIO_NUM
extern button_obj key1;
#endif

#ifdef KEY2_GPIO_NUM
extern button_obj key2;
#endif

#ifdef KEY3_GPIO_NUM
extern button_obj key3;
#endif


void key_init(void);

#endif	// CONTAIN_USER_BUTTON

#endif 	// _MULTI_BUTTON_H_

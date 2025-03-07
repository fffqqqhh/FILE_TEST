#ifndef _USER_BUTTON_H_
#define _USER_BUTTON_H_

#include "multi_button.h"
#include "driver/gpio.h"

// 电源按键
#define	KEY1_GPIO_NUM		GPIO_NUM_42
#define	KEY2_GPIO_NUM		GPIO_NUM_41
#define	KEY3_GPIO_NUM		GPIO_NUM_40


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


#endif 	// _USER_BUTTON_H_

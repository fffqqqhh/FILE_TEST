#include "user_button.h"


#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"


static const char *TAG = "user_button";



#include "user_gui.h"
#include "user_source.h"


void scroll_list_by(lv_obj_t * boj, int y_offset);

void key_task(void *param)
{
	while(1)
	{
		button_ticks();
		lv_obj_t *tab = get_active_tab_obj(tabview);
		switch(get_button_event(&key1))
		{
			case LONG_PRESSED:					// 
			{
				// ESP_LOGI(TAG, "key1 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				// ESP_LOGI(TAG, "key1 single_click\n");

				if (lvgl_lock(-1)) 
				{
					if(++tab_idx == 3)
                  	  tab_idx = 0;
                	lv_tabview_set_act(tabview, tab_idx, LV_ANIM_ON); 

					lvgl_unlock();
				}
			}break;
			
			case PRESSED_REPEAT:						// 
			{
				// ESP_LOGI(TAG, "key1 press_repeat %d\n", key1.repea_cnt);
			
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
				// ESP_LOGI(TAG, "key2 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				// ESP_LOGI(TAG, "key2 single_click\n");

				if (lvgl_lock(-1)) 
				{
					scroll_list_by(tab, 200);  // 向上滚动

					lvgl_unlock();
				}

			}break;

			case LONG_PRESSED_HOLD:					// 
			{
				if (lvgl_lock(-1)) 
				{
					scroll_list_by(tab, 10);  // 向上滚动

					lvgl_unlock();
				}
			}break;
			
			case PRESSED_REPEAT:						// 
			{
				// ESP_LOGI(TAG, "key2 press_repeat %d\n", key2.repea_cnt);
			
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
				// ESP_LOGI(TAG, "key3 long_press_start\n");
				
			}break;
			
			case PRESSED_SINGLE:						// 
			{
				// ESP_LOGI(TAG, "key3 single_click\n");

				if (lvgl_lock(-1)) 
				{
					scroll_list_by(tab, -200);  // 向下滚动

					lvgl_unlock();
				}

			}break;

			case LONG_PRESSED_HOLD:					// 
			{
				if (lvgl_lock(-1)) 
				{
					scroll_list_by(tab, -10);  // 向下滚动

					lvgl_unlock();
				}
			}break;
			
			case PRESSED_REPEAT:						// 
			{
				// ESP_LOGI(TAG, "key3 press_repeat %d\n", key3.repea_cnt);
			
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

// void key2_press_repeat_cb(void)
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

// void key3_press_repeat_cb(void)
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
	button_repeat_config(&key1, false, 2);
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
	button_repeat_config(&key2, false, 2);
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
	button_repeat_config(&key3, false, 2);
#endif
#if COMPILE_CALL_BACK_FUN == 1	
	button_attach_cb(&key3, PRESSED_REPEAT, key3_press_repeat_cb);
#endif
	button_start(&key3);
#endif

	xTaskCreate(&key_task, "key_task", 1024 * 4, NULL, configMAX_PRIORITIES - 1, NULL);
}

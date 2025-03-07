#ifndef _USER_ADC_H_
#define _USER_ADC_H_

#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"



#define NTC_ADC_CHAN0           ADC_CHANNEL_0
#define NTC_ADC_ATTEN           ADC_ATTEN_DB_12



void adc1_deinit(void);


void adc_ntc_init(void);
uint16_t adc_ntc_read(void);


#endif

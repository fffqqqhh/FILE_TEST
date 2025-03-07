#ifndef _USER_I2C_H_
#define _USER_I2C_H_

#include "driver/i2c.h"
// #include "driver/gpio.h"



esp_err_t i2c_master_init(void);

/// @brief 打印I2C总线上的所有设备
void i2c_find_all_device(i2c_port_t i2c_port);

/// @brief 打印所有寄存器的数据
void i2c_printf_all_reg(i2c_port_t i2c_port, uint8_t dev_addr);


#endif

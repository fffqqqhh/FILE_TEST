#ifndef _USER_UART_H_
#define _USER_UART_H_

#include "driver/uart.h"

#include "driver/gpio.h"

#include <string.h>

#define UART0_TX_PIN        GPIO_NUM_43
#define UART0_RX_PIN        GPIO_NUM_44

#define UART1_TX_PIN        GPIO_NUM_4
#define UART1_RX_PIN        GPIO_NUM_5


/// @brief 串口初始化配置
void uart_init_config(uart_port_t uart_port, uint32_t baud_rate, int tx_pin, int rx_pin);

/// @brief 串口0 接收任务
void uart0_rx_task(void);
/// @brief 串口1 接收任务
void uart1_rx_task(void);
/// @brief 串口2 接收任务
void uart2_rx_task(void);


#endif

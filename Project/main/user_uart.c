#include "user_uart.h"


#define RX_BUFFER_SIZE      200 //接收缓冲区大小
#define TX_BUFFER_SIZE      200 //发送缓冲区大小



/// @brief 串口初始化配置
/// @param uart_port UART_NUM_0 ~ (UART_NUM_MAX -1)
/// @param baud_rate 波特率
/// @param tx_pin tx_pin
/// @param rx_pin rx_pin
void uart_init_config(uart_port_t uart_port, uint32_t baud_rate, int tx_pin, int rx_pin)
{
    //定义 串口配置结构体，必须赋初值，否则无法实现
    uart_config_t uart_config={0};

    uart_config.baud_rate = baud_rate;                  // 配置波特率
    uart_config.data_bits = UART_DATA_8_BITS;           // 配置数据位为8位
    uart_config.parity = UART_PARITY_DISABLE;           // 配置校验位为不需要校验
    uart_config.stop_bits = UART_STOP_BITS_1;           // 配置停止位为 一位
    uart_config.flow_ctrl = UART_HW_FLOWCTRL_DISABLE;   // 禁用硬件流控制

    // 将以上参数加载到串口1的寄存器
    uart_param_config(uart_port, &uart_config);

    // 绑定引脚  TX=tx_pin RX=rx_pin RTS=不使用 CTS=不使用
    uart_set_pin(uart_port, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    // 安装 串口 驱动程序
    uart_driver_install(uart_port, RX_BUFFER_SIZE, TX_BUFFER_SIZE, 0, NULL, 0);
}


/// @brief 串口0 接收任务
void uart0_rx_task(void)
{
    uint8_t rx_data[200]={0};
    uint8_t temp[50]={0};
    while(1)
    {
        //接收串口数据收到的数据长度
        int rx_bytes = uart_read_bytes(UART_NUM_0, rx_data, 200, 10 / portTICK_PERIOD_MS);
        if( rx_bytes > 0 )//数据长度大于0，说明接收到数据
        {
            rx_data[rx_bytes] = 0;//将串口数据的最后一个设置为0，形成字符串



            //输出接收数据的长度
            sprintf((const char*)temp,"uart1 string length : %d\r\n", rx_bytes);
            uart_write_bytes(UART_NUM_0, (const char*)temp, strlen((const char*)temp));
            //通过串口2输出接受到的数据
            uart_write_bytes(UART_NUM_0, (const char*)"uart1 received : ", strlen("uart1 received : "));
            uart_write_bytes(UART_NUM_0, (const char*)rx_data, strlen((const char*)rx_data));
            //UART环缓冲区刷新。丢弃UART RX缓冲区中的所有数据，准备下次接收
            uart_flush(UART_NUM_0);
        }
    }
}

/// @brief 串口1 接收任务
void uart1_rx_task(void)
{
    uint8_t rx_data[200]={0};
    uint8_t temp[50]={0};
    while(1)
    {
        //接收串口数据收到的数据长度
        int rx_bytes = uart_read_bytes(UART_NUM_1, rx_data, 200, 10 / portTICK_PERIOD_MS);
        if( rx_bytes > 0 )//数据长度大于0，说明接收到数据
        {
            rx_data[rx_bytes] = 0;//将串口数据的最后一个设置为0，形成字符串



            //输出接收数据的长度
            sprintf((const char*)temp,"uart1 string length : %d\r\n", rx_bytes);
            uart_write_bytes(UART_NUM_1, (const char*)temp, strlen((const char*)temp));
            //通过串口2输出接受到的数据
            uart_write_bytes(UART_NUM_1, (const char*)"uart1 received : ", strlen("uart1 received : "));
            uart_write_bytes(UART_NUM_1, (const char*)rx_data, strlen((const char*)rx_data));
            //UART环缓冲区刷新。丢弃UART RX缓冲区中的所有数据，准备下次接收
            uart_flush(UART_NUM_1);
        }
    }
}

/// @brief 串口2 接收任务
void uart2_rx_task(void)
{
    uint8_t rx_data[200]={0};
    uint8_t temp[50]={0};
    while(1)
    {
        //接收串口数据收到的数据长度
        int rx_bytes = uart_read_bytes(UART_NUM_2, rx_data, 200, 10 / portTICK_PERIOD_MS);
        if( rx_bytes > 0 )//数据长度大于0，说明接收到数据
        {
            rx_data[rx_bytes] = 0;//将串口数据的最后一个设置为0，形成字符串



            //输出接收数据的长度
            sprintf((const char*)temp,"uart1 string length : %d\r\n", rx_bytes);
            uart_write_bytes(UART_NUM_2, (const char*)temp, strlen((const char*)temp));
            //通过串口2输出接受到的数据
            uart_write_bytes(UART_NUM_2, (const char*)"uart1 received : ", strlen("uart1 received : "));
            uart_write_bytes(UART_NUM_2, (const char*)rx_data, strlen((const char*)rx_data));
            //UART环缓冲区刷新。丢弃UART RX缓冲区中的所有数据，准备下次接收
            uart_flush(UART_NUM_2);
        }
    }
}


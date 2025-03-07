#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"

#include "esp_gatts_api.h"


#include "user_SW3516.h"
#include "user_IP2730.h"
//#include "user_HUSB238A.h"
#include "user_source.h"
#include "user_adc.h"

#include "user_oper_data.h"

//=================================================  configuration  =================================================//
#define UART_TEST 0// 串口输入对应数据模拟设备充电,用于辅助测试双C口智能功率分配功能
// #define SUPPORT_WIFI_GET_TIME// 通过WIFI校准时间,用于定时开关机,定时涓流/快充切换功能
// #define SUPPORT_NTC// NTC降功率功能
#define SUPPORT_NoteBook// 笔记本盲插65W功能
// #define SUPPORT_OTA_HTTP// 支持HTTP OTA升级
// #define SUPPORT_VBUS_DETECT// 支持VBUS充满关断
//=================================================  configuration  =================================================//

#define GATTS_TABLE_TAG "【GATTS_TABLE】"

uint8_t POWER_ALL = 100;      // 最大功率
uint8_t POWER_C1_MAX;   // 双口状态下C1口最大功率
uint8_t POWER_C2_MAX;   // 双口状态下C2口最大功率

uint8_t ReportedData[20];// 蓝牙上报
uint8_t ComdData[3];// 蓝牙接收
StateMachine_struct MyPower;
ChargingInformation_struct PORT[2];

#if UART_TEST
#include "driver/uart.h"
#include "string.h"

static const int RX_BUF_SIZE = 1024;
#define RXD_PIN (GPIO_NUM_5)
#endif

#ifdef SUPPORT_WIFI_GET_TIME
#include "esp_wifi.h"
#include "sntp.h"  
#include "time.h" 

#define EXAMPLE_ESP_WIFI_SSID      "Xiaomi 14"
#define EXAMPLE_ESP_WIFI_PASS      "88888888"
#define EXAMPLE_ESP_MAXIMUM_RETRY  5

#define ESP_WIFI_SAE_MODE WPA3_SAE_PWE_BOTH
#define EXAMPLE_H2E_IDENTIFIER ""

#define ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_WPA2_PSK

static EventGroupHandle_t s_wifi_event_group;

#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;

#endif

#ifdef SUPPORT_OTA_HTTP
#include <sys/socket.h>        // 套接字相关库
#include "esp_crt_bundle.h"     // 使用证书捆绑时需要包含的头文件
#include "esp_ota_ops.h"       // OTA(在线升级)操作相关 API
#include "esp_http_client.h"   // HTTP 客户端库
#include "esp_https_ota.h"     // HTTPS OTA(在线升级)操作相关 API
#include "user_ota.h"          // 一些通用协议示例
#include "string.h"            // C 字符串操作相关库
#include "esp_wifi.h"          // WiFi 相关 API
#include "driver/gpio.h"       // GPIO(通用输入输出)相关 API

#define HASH_LEN 32            // 定义 SHA-256 哈希值长度为 32 字节
#define UPDATE_IO 40           // 定义用于 OTA 更新的 GPIO 引脚号为 40

static const char *bind_interface_name = "example_netif_sta"; // 绑定 Wi-Fi STA 接口

extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start"); // 证书起始位置
extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");     // 证书结束位置

#define OTA_URL_SIZE 256        // OTA 固件 URL 大小

#define CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL "http://192.168.137.1:8070/build/hello_world.bin" // 固件升级 URL

#endif

#ifdef SUPPORT_VBUS_DETECT

typedef struct
{
    uint8_t LowLoadStart;
    uint8_t VBusSta;
    uint8_t LEDSta;

    uint16_t VBusOFF_TIMER;
    uint16_t LoadStart_TIMER;
    uint32_t VBusOFF_COUNT;
}VBusOFF_struct;
VBusOFF_struct VBusCtl[2];

// 待定义 确定控制关断的IO口 或者I2C指令

#endif

#if UART_TEST
void uart_init(void)
{
    const uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };
    
    // Configure UART parameters
    uart_param_config(UART_NUM_1, &uart_config);
    
    // Set UART pin (only RX: RXD_PIN)
    uart_set_pin(UART_NUM_1, UART_PIN_NO_CHANGE, RXD_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    
    // Install UART driver with RX buffer size
    uart_driver_install(UART_NUM_1, RX_BUF_SIZE * 2, 0, 0, NULL, 0);
}

static void rx_task(void *arg)
{
    static const char *RX_TASK_TAG = "RX_TASK";
    esp_log_level_set(RX_TASK_TAG, ESP_LOG_INFO);
    uint8_t* data = (uint8_t*) malloc(RX_BUF_SIZE + 1);
    uint8_t data_buffer[8];
    while (1) {
        const int rxBytes = uart_read_bytes(UART_NUM_1, data, RX_BUF_SIZE, 1000 / portTICK_PERIOD_MS);
        // if (rxBytes > 0) {
        //     data[rxBytes] = 0;
        //     ESP_LOGI(RX_TASK_TAG, "Read %d bytes: '%d'", rxBytes, data[0]);
        //     data_buffer[0] = (int)data[0];
        //     ESP_LOGI(RX_TASK_TAG, "bytes 1 : '%d'", data_buffer[0]);
        //     ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        // }

        if (rxBytes>0)
        {
            data[rxBytes] = 0;
            if (rxBytes == 8)
            {
                for(int i=0; i<8; i++)
                {
                    data_buffer[i] = (int)data[i];
                    ESP_LOGI(RX_TASK_TAG, "bytes %d : '%d'", i, data_buffer[i]);
                }
                PORT[0].curr_protocol[2] = (int)data[1]; // 读取协议
                PORT[1].curr_protocol[2] = (int)data[5]; // 读取协议

                PORT[0].curr_vout = (int)data[2]*1000;                                              // 读取输出电压
                PORT[1].curr_vout = (int)data[6]*1000;                                              // 读取输出电压
                PORT[0].curr_iout = (int)data[3]*100;                                             // 读取输出电流
                PORT[1].curr_iout = (int)data[7]*100;                                             // 读取输出电流

                PORT[0].curr_power = (PORT[0].curr_vout * PORT[0].curr_iout) / 1000000; // 读取输出功率
                PORT[1].curr_power = (PORT[1].curr_vout * PORT[1].curr_iout) / 1000000; // 读取输出功率

                if ((int)data[0] == 1)
                    PORT[0].port_state = 1; // PortC1 is on
                else
                    PORT[0].port_state = 0; // PortC1 is off

                if ((int)data[4] == 1)
                    PORT[1].port_state = 1; // PortC2 is on
                else
                    PORT[1].port_state = 0; // PortC2 is off 
            }
            ESP_LOG_BUFFER_HEXDUMP(RX_TASK_TAG, data, rxBytes, ESP_LOG_INFO);
        }
    }
    free(data);
}
#endif

#ifdef SUPPORT_WIFI_GET_TIME
/**
 * 事件处理函数
 * 处理WiFi连接、断开和获取IP的事件
 */
static void event_handler(void* arg, esp_event_base_t event_base,
                          int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect(); // 当WiFi STA启动时,尝试连接AP
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < EXAMPLE_ESP_MAXIMUM_RETRY) {
            esp_wifi_connect(); // 尝试重新连接
            s_retry_num++;
            ESP_LOGI(GATTS_TABLE_TAG, "retry to connect to the AP");
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT); // 设置连接失败的事件位
        }
        ESP_LOGI(GATTS_TABLE_TAG, "connect to the AP fail");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(GATTS_TABLE_TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip)); // 成功获取IP地址
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT); // 设置连接成功的事件位
    }
}

/**
 * 初始化WiFi STA模式
 */
void wifi_init_sta(void) {
    s_wifi_event_group = xEventGroupCreate(); // 创建事件组

    ESP_ERROR_CHECK(esp_netif_init()); // 初始化底层网络接口
    ESP_ERROR_CHECK(esp_event_loop_create_default()); // 创建默认事件循环
    esp_netif_create_default_wifi_sta(); // 创建默认的WiFi STA

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT(); // 获取默认WiFi配置
    ESP_ERROR_CHECK(esp_wifi_init(&cfg)); // 初始化WiFi驱动

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                                        &event_handler, NULL, &instance_any_id)); // 注册WiFi事件处理程序
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                                        &event_handler, NULL, &instance_got_ip)); // 注册IP事件处理程序

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = EXAMPLE_ESP_WIFI_SSID, // 设置SSID
            .password = EXAMPLE_ESP_WIFI_PASS, // 设置密码
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD, // 设置最低认证模式阈值
            .sae_pwe_h2e = ESP_WIFI_SAE_MODE, // 设置WPA3 SAE模式
            .sae_h2e_identifier = EXAMPLE_H2E_IDENTIFIER, // 设置WPA3 SAE标识符
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA)); // 设置WiFi模式为STA
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config)); // 配置WiFi
    ESP_ERROR_CHECK(esp_wifi_start()); // 启动WiFi

    ESP_LOGI(GATTS_TABLE_TAG, "wifi_init_sta finished."); // 打印初始化完成日志

    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE, pdFALSE, portMAX_DELAY); // 等待WiFi连接或失败事件

    if (bits & WIFI_CONNECTED_BIT) {
        ESP_LOGI(GATTS_TABLE_TAG, "connected to ap SSID:%s password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS); // 连接成功
    } else if (bits & WIFI_FAIL_BIT) {
        ESP_LOGI(GATTS_TABLE_TAG, "Failed to connect to SSID:%s, password:%s",
                 EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS); // 连接失败
    } else {
        ESP_LOGE(GATTS_TABLE_TAG, "UNEXPECTED EVENT"); // 处理意外事件
    }
}

void print_time_with_timezone(int timezone_hours)  
{  
    // 获取当前时间(UTC)  
    time_t now;  
    time(&now);  
  
    // 将时间转换为本地时间(实际上是 UTC,因为还没有应用时区)  
    struct tm timeinfo = {0};  
    localtime_r(&now, &timeinfo);  
  
    // 应用时区偏移(注意:这里简单地将小时数加减,没有考虑夏令时等因素)  
    timeinfo.tm_hour += timezone_hours;  
    // 如果超出了23小时,需要进位  
    if (timeinfo.tm_hour >= 24) {  
        timeinfo.tm_hour -= 24;  
        timeinfo.tm_mday += 1;  
        // 检查日期是否需要进位(没有处理月份和年份的进位)  
    } else if (timeinfo.tm_hour < 0) {  
        timeinfo.tm_hour += 24;  
        timeinfo.tm_mday -= 1;  
        // 同样,这里省略了月份和年份的退位处理  
    }  
  
    // 格式化并打印时间  
    char strftime_buf[64];  
    strftime(strftime_buf, sizeof(strftime_buf), "%Y-%m-%d %H:%M:%S", &timeinfo);  
    ESP_LOGE(GATTS_TABLE_TAG, "%s",strftime_buf);
    // printf("The current date/time (with timezone offset) is: %s\n", strftime_buf);  
} 

void initialize_sntp(void)  
{  
    // 初始化 SNTP  
    sntp_setoperatingmode(SNTP_OPMODE_POLL);  

    // 设置 SNTP 服务器  
    sntp_setservername(0, "pool.ntp.org");  

    // 初始化 SNTP 并等待时间同步  
    sntp_init();  

    // 等待时间同步完成  
    while (sntp_get_sync_status() == SNTP_SYNC_STATUS_RESET) {  
        vTaskDelay(2000 / portTICK_PERIOD_MS);  
    }  
}
#endif

#ifdef SUPPORT_OTA_HTTP
// HTTP 客户端事件处理函数
esp_err_t _http_event_handler(esp_http_client_event_t *evt)
{
    // 根据事件类型进行处理
    switch (evt->event_id) {
    case HTTP_EVENT_ERROR:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_ERROR"); // 处理 HTTP 错误事件
        break;
    case HTTP_EVENT_ON_CONNECTED:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_ON_CONNECTED"); // 处理 HTTP 连接事件
        break;
    case HTTP_EVENT_HEADER_SENT:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_HEADER_SENT"); // 处理 HTTP 头部已发送事件
        break;
    case HTTP_EVENT_ON_HEADER:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_ON_HEADER, key=%s, value=%s", evt->header_key, evt->header_value); // 处理收到的 HTTP 头部事件
        break;
    case HTTP_EVENT_ON_DATA:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_ON_DATA, len=%d", evt->data_len); // 处理收到数据事件
        break;
    case HTTP_EVENT_ON_FINISH:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_ON_FINISH"); // 处理 HTTP 完成事件
        break;
    case HTTP_EVENT_DISCONNECTED:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_DISCONNECTED"); // 处理 HTTP 断开连接事件
        break;
    case HTTP_EVENT_REDIRECT:
        ESP_LOGD(GATTS_TABLE_TAG, "HTTP_EVENT_REDIRECT"); // 处理 HTTP 重定向事件
        break;
    }
    return ESP_OK; // 返回处理成功
}

// OTA 示例任务
void simple_ota_example_task(void *pvParameter)
{
    ESP_LOGI(GATTS_TABLE_TAG, "Starting OTA example task"); // 开始 OTA 示例任务
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
    // 获取指定接口
    esp_netif_t *netif = get_example_netif_from_desc(bind_interface_name);
    if (netif == NULL) {
        ESP_LOGE(GATTS_TABLE_TAG, "Can't find netif from interface description"); // 找不到接口时,记录错误
        abort(); // 中止程序
    }
    struct ifreq ifr;
    esp_netif_get_netif_impl_name(netif, ifr.ifr_name);
    ESP_LOGI(GATTS_TABLE_TAG, "Bind interface name is %s", ifr.ifr_name); // 记录绑定接口名称
#endif
    // 配置 HTTP 客户端
    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL, // 设置 OTA URL
        // .url = "http://192.168.137.1:8070/build/hello_world.bin",
        // .url = "http://192.168.137.1:8070/build/ota_data_initial.bin",
#ifdef CONFIG_EXAMPLE_USE_CERT_BUNDLE
        .crt_bundle_attach = esp_crt_bundle_attach, // 如果使用证书捆绑,附加证书捆绑
#else
        .cert_pem = (char *)server_cert_pem_start, // 否则使用 PEM 格式证书
#endif /* CONFIG_EXAMPLE_USE_CERT_BUNDLE */
        .event_handler = _http_event_handler, // 指定 HTTP 事件处理函数
        .keep_alive_enable = true, // 启用 Keep-Alive
#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_BIND_IF
        .if_name = &ifr, // 绑定接口
#endif
    };

#ifdef CONFIG_EXAMPLE_FIRMWARE_UPGRADE_URL_FROM_STDIN
    // 如果固件升级 URL 是从标准输入获取的
    char url_buf[OTA_URL_SIZE];
    if (strcmp(config.url, "FROM_STDIN") == 0) {
        example_configure_stdin_stdout(); // 配置标准输入输出
        fgets(url_buf, OTA_URL_SIZE, stdin); // 从标准输入获取 URL
        int len = strlen(url_buf);
        url_buf[len - 1] = '\0'; // 去除换行符
        config.url = url_buf; // 设置 URL
    } else {
        ESP_LOGE(GATTS_TABLE_TAG, "Configuration mismatch: wrong firmware upgrade image url"); // 配置错误
        abort();
    }
#endif

#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
    config.skip_cert_common_name_check = true; // 如果配置了跳过证书公用名检查
#endif

    // 配置 OTA 操作
    esp_https_ota_config_t ota_config = {
        .http_config = &config,
    };
    ESP_LOGI(GATTS_TABLE_TAG, "Attempting to download update from %s", config.url); // 记录正在尝试下载固件更新
    esp_err_t ret = esp_https_ota(&ota_config); // 开始 OTA
    if (ret == ESP_OK) {
        ESP_LOGI(GATTS_TABLE_TAG, "OTA Succeed, Rebooting..."); // OTA 成功,重启设备
        esp_restart(); // 重启系统
    } else {
        ESP_LOGE(GATTS_TABLE_TAG, "Firmware upgrade failed"); // 记录固件升级失败
    }
    while (1) {
        vTaskDelay(1000 / portTICK_PERIOD_MS); // 延迟一秒
    }
}

// 打印 SHA-256 哈希值
static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1]; // 存储 SHA-256 哈希值的字符串
    hash_print[HASH_LEN * 2] = 0; // 终止符
    for (int i = 0; i < HASH_LEN; ++i) {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]); // 格式化为 16 进制字符串
    }
    ESP_LOGI(GATTS_TABLE_TAG, "%s %s", label, hash_print); // 打印哈希值
}

// 获取分区的 SHA-256 哈希值
static void get_sha256_of_partitions(void)
{
    uint8_t sha_256[HASH_LEN] = { 0 }; // 存储 SHA-256 哈希值
    esp_partition_t partition;

    // 获取引导程序的 SHA-256 哈希值
    partition.address   = ESP_BOOTLOADER_OFFSET; // 引导程序偏移量
    partition.size      = ESP_PARTITION_TABLE_OFFSET; // 分区表偏移量
    partition.type      = ESP_PARTITION_TYPE_APP; // 分区类型
    esp_partition_get_sha256(&partition, sha_256); // 获取引导程序的 SHA-256 哈希值
    print_sha256(sha_256, "SHA-256 for bootloader: "); // 打印引导程序的 SHA-256

    // 获取当前运行分区的 SHA-256 哈希值
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256); // 获取当前固件的 SHA-256
    print_sha256(sha_256, "SHA-256 for current firmware: "); // 打印当前固件的 SHA-256
}

// 初始化按键 IO
void key_io_init(void)
{
    gpio_config_t KEY; // 配置 GPIO
    KEY.mode = GPIO_MODE_INPUT; // 设置为输入模式
    KEY.pull_down_en = GPIO_PULLUP_ENABLE; // 启用上拉电阻
    KEY.pin_bit_mask = (1ULL << 45); // 设置 GPIO 45 为按键引脚

    // gpio_config_t KEY = {
    //     .pin_bit_mask = BIT64(UPDATE_IO),
    //     .mode = GPIO_MODE_INPUT_OUTPUT,
    //     .pull_up_en = 1,
    //     .pull_down_en = 0,
    // };
    gpio_config(&KEY);
    // gpio_set_level(UPDATE_IO, 1);
}

void xxx_functoin(void)
{
    ESP_LOGI(GATTS_TABLE_TAG, "xxx_functoin V1");
}

/// @brief 长按按钮3秒,执行OTA任务
/// @param pvParameter
void function_task(void *pvParameter)
{
    int ota_timer = 0; // 定义 OTA 计时器
    int ota_flag = 0; // 定义 OTA 标志
    TaskHandle_t ota_task_handle = NULL; // 定义 OTA 任务句柄
    key_io_init(); // 初始化按键

    while (1)
    {
        // 检测按键是否按下
        if (gpio_get_level(45) == 1) // GPIO 45 电平为高
        {
            ota_timer++;
            ESP_LOGI(GATTS_TABLE_TAG, "long press button 3s...");
            if (ota_timer > 3)
            {
                ESP_ERROR_CHECK(example_connect());

#if CONFIG_EXAMPLE_CONNECT_WIFI
                /* 确保禁用任何 WiFi 节能模式,确保 OTA 操作期间的最佳吞吐量 */
                esp_wifi_set_ps(WIFI_PS_NONE); // 禁用 WiFi 节能模式
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
                xTaskCreate(&simple_ota_example_task, "ota_example_task", 8192, NULL, 5, &ota_task_handle);
                ota_flag = 1;
            }
        }
        else
        {
            if(ota_flag)
            {
                // vTaskResume(ota_task_handle);
                // ota_flag = 0;
                // esp_wifi_stop();
                // esp_wifi_deinit();
            }
            ota_timer = 0;
            xxx_functoin();
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
#endif

/// @brief 打印端口、功率信息
/// @param PortNum:端口,C1(0)或C2(1)
void PortAndPowerInformationPrint(uint8_t PortNum)
{
    ESP_LOGE(GATTS_TABLE_TAG, "C%d power need change: %d", PortNum, PORT[PortNum].change_flag);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d ON or OFF: %d", PortNum, PORT[PortNum].port_state);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d quickly charge sta: %d", PortNum, PORT[PortNum].curr_protocol[0]);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d quickly charge vol: %d", PortNum, PORT[PortNum].curr_protocol[1]);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d quickly charge mode: %d", PortNum, PORT[PortNum].curr_protocol[2]);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d current vin: %d", PortNum, PORT[PortNum].curr_vin);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d current vout: %d", PortNum, PORT[PortNum].curr_vout);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d current iout: %d", PortNum, PORT[PortNum].curr_iout);
    ESP_LOGE(GATTS_TABLE_TAG, "C%d current power: %f", PortNum, PORT[PortNum].curr_power);
}

/// @brief 配置IIC的IO口
/// @param PortNum:端口,C1(0)或C2(1)
void GPIOConfigSW35xxI2C(uint8_t PortNum)
{
    if (PortNum == 0)
    {
        IIC_MASTER_SCL_PIN = 38;
        IIC_MASTER_SDA_PIN = 37;
        IIC_MASTER_NUM = 0;
        SW3516_InitConfig();
        SW3516_SetPDO(20, (POWER_ALL * 1000) / 20);
        SW3516_SetPPS0(11, 5000);
    }
    else
    {
        IIC_MASTER_SCL_PIN = 36;
        IIC_MASTER_SDA_PIN = 35;
        IIC_MASTER_NUM = 1;
        SW3516_InitConfig();
        SW3516_SetPDO(20, (POWER_ALL * 1000) / 20);
        SW3516_SetPPS0(11, 5000);
    }
}

/// @brief 获取并上传协议、电压、电流信息
/// @param PortNum:端口,C1(0)或C2(1)
void PowerRead(uint8_t PortNum)
{
    uint8_t reg_0x06, reg_0x07;//, reg_0xB7, reg_0xBE;

    IIC_MASTER_NUM = PortNum;

    reg_0x06 = SW3516_I2C_READ_BYTE(0x06);

    if (uGetBits(reg_0x06, 7, 7) == 1)
        PORT[PortNum].curr_protocol[0] = 1; // In quick charge state
    else
        PORT[PortNum].curr_protocol[0] = 0; // Not quick charge state

    if (uGetBits(reg_0x06, 6, 6) == 1)
        PORT[PortNum].curr_protocol[1] = 1; // In quick charge vol
    else
        PORT[PortNum].curr_protocol[1] = 0; // Not quick charge vol

    PORT[PortNum].curr_protocol[2] = uGetBits(reg_0x06, 3, 0); // 读取协议

    PORT[PortNum].curr_vin = SW3516_Get_VIN();                                                // 读取输入电压
    PORT[PortNum].curr_vout = SW3516_Get_VOUT();                                              // 读取输出电压
    PORT[PortNum].curr_iout = SW3516_Get_IOUT1();                                             // 读取输出电流
    PORT[PortNum].curr_power = (PORT[PortNum].curr_vout * PORT[PortNum].curr_iout) / 1000000; // 读取输出功率

    reg_0x07 = SW3516_I2C_READ_BYTE(0x07);
    if (uGetBits(reg_0x07, 0, 0) == 1)
        PORT[PortNum].port_state = 1; // PortC is on
    else
        PORT[PortNum].port_state = 0; // PortC is off

    // reg_0xB7 = SW3516_I2C_READ_BYTE(0xB7);// 2:9V 3:12V 4:15V 5:20V 6:PPS0 7:PPS1
    // reg_0xBE =  SW3516_I2C_READ_BYTE(0xBE);// PPS1 5:4  PPS0 1:0
    // 蓝牙上报数据
    ReportedData[4] = PORT[0].curr_protocol[2];// C1口快充协议
    ReportedData[5] = MyPower.currPortSta;// 端口状态 
    ReportedData[6] = PORT[0].curr_vin;// 输入电压低8位   
    ReportedData[7] = PORT[0].curr_vin>>8;// 输入电压高8位  
    ReportedData[8] = PORT[0].curr_vout;// 输出电压低8位 
    ReportedData[9] = PORT[0].curr_vout>>8;// C1口输出电压高8位 
    ReportedData[10] = PORT[0].curr_iout;// C1口输出电流低8位 
    ReportedData[11] = PORT[0].curr_iout>>8;// C1口输出电流高8位 
    ReportedData[12] = PORT[0].targ_power_pd;// C1口PD广播功率 
    ReportedData[13] |= BIT0|BIT1|BIT2|BIT3|BIT4;// 5/9/12/15/20/PPS0

    // if (PortNum==0)
    // {
    //     ReportedData[13] = (reg_0xBE&(BIT0|BIT1))<<4;// PPS0
    //     ReportedData[13] = ReportedData[13]|((reg_0xBE&(BIT4|BIT5))<<2);// PPS1
    //     ReportedData[13] |= (reg_0xB7&BIT2)?BIT0:0;// 9V
    //     ReportedData[13] |= (reg_0xB7&BIT3)?BIT1:0;// 12V
    //     ReportedData[13] |= (reg_0xB7&BIT4)?BIT2:0;// 15V
    //     ReportedData[13] |= (reg_0xB7&BIT5)?BIT3:0;// 20V
    // }
    // else
    // {
    //     ReportedData[19] = (reg_0xBE&(BIT0|BIT1))<<4;// PPS0
    //     ReportedData[19] = ReportedData[13]|((reg_0xBE&(BIT4|BIT5))<<2);// PPS1
    //     ReportedData[19] |= (reg_0xB7&BIT2)?BIT0:0;// 9V
    //     ReportedData[19] |= (reg_0xB7&BIT3)?BIT1:0;// 12V
    //     ReportedData[19] |= (reg_0xB7&BIT4)?BIT2:0;// 15V
    //     ReportedData[19] |= (reg_0xB7&BIT5)?BIT3:0;// 20V   
    // }

    ReportedData[14] = PORT[1].curr_protocol[2];// C2口快充协议
    ReportedData[15] = PORT[1].curr_vout;// C2口输出电压低8位 
    ReportedData[16] = PORT[1].curr_vout>>8;// C2口输出电压高8位 
    ReportedData[17] = PORT[1].curr_iout;// C2口输出电流低8位 
    ReportedData[18] = PORT[1].curr_iout>>8;// C2口输出电流高8位 
    ReportedData[19] |= BIT0|BIT1|BIT2|BIT3|BIT4;// 5/9/12/15/20/PPS0 
}

/// @brief 调整协议、电压、电流状态
/// @param PortNum:端口,C1(0)或C2(1)
void PowerWrite(uint8_t PortNum)
{
    IIC_MASTER_NUM = PortNum;

#ifdef SOURCE_IP2730
    if (POWER_ALL < 30)
    {
        SW3516_AllOff();
        //SW3516_ResetPower();
    }
    else
#endif
    {
        if (MyPower.LowSpeedCharg == 1)
        {
            SW3516_AllOff();
        }
        else
        {
            SW3516_AllOn();
            // FIX
            SW3516_SetPDO(20, (PORT[PortNum].targ_power_pd * 1000) / 20);
            // PPS
            // if(PORT[PortNum].targ_power_pd>55)
            // {
            //     SW3516_SetPPS0(21, (PORT[PortNum].targ_power_pd * 1000) / 20);
            //     if (PortNum==0)
            //         ReportedData[13] |= BIT5;// PPS0 21V
            //     else
            //         ReportedData[19] |= BIT5;// PPS0 21V
            // }
            // else
            {
                if (PORT[PortNum].targ_power_pd >= 55)
                    SW3516_SetPPS0(11, 5000);
                else
                    SW3516_SetPPS0(11, (PORT[PortNum].targ_power_pd * 1000) / 11);
                if (PortNum == 0)
                    ReportedData[13] &= ~BIT5; // PPS0 11V
                else
                    ReportedData[19] &= ~BIT5; // PPS0 11V
            }
        }
    }
}

/// @brief 状态机状态位更新
/// @param NULL
bool FlushState(void)
{
    if (MyPower.State != MyPower.NexState)
    {
        // ESP_LOGE("STATE_STATE", "MyPower.State : %d", MyPower.State);
        MyPower.State = MyPower.NexState;
        // ESP_LOGE("STATE_STATE", "MyPower.NexState : %d", MyPower.NexState);
        return true;
    }
    else
        return false;
}

/// @brief 定时器节拍
/// @param NULL
void TimerTick(void)
{

}

/// @brief 定时器节拍任务函数
/// @param NULL
void UserTimer_task(void *parm)
{
    while (1)
    {
        if(MyPower.ADCCheck_TIMER)
            MyPower.ADCCheck_TIMER--;
        if (MyPower.NTCLowPower_TIMER)
            MyPower.NTCLowPower_TIMER--;
        if (MyPower.Check_TIMER)
            MyPower.Check_TIMER--;
        if (MyPower.PortCheck_TIMER)
            MyPower.PortCheck_TIMER--;
        if (MyPower.PowerCheck_TIMER)
            MyPower.PowerCheck_TIMER--;
        if (MyPower.SystemI2CCheck_TIMER)
            MyPower.SystemI2CCheck_TIMER--;
        if (MyPower.Notify_TIMER)
            MyPower.Notify_TIMER--;
        if (MyPower.FirstIn_TIMER)
            MyPower.FirstIn_TIMER--;
        if (MyPower.NoteBookCheck_TIMER)
            MyPower.NoteBookCheck_TIMER--;
#ifdef SUPPORT_VBUS_DETECT
        for (int i = 0; i < 2; i++)
        {
            if (VBusCtl[i].VBusOFF_TIMER)
                VBusCtl[i].VBusOFF_TIMER--;
            if (VBusCtl[i].LoadStart_TIMER>1)
                VBusCtl[i].LoadStart_TIMER--;
            if (VBusCtl[i].VBusOFF_COUNT)
                VBusCtl[i].VBusOFF_COUNT--;
        }
#endif
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

#ifdef SUPPORT_NoteBook
/// @brief 笔记本充电功率设置
/// @param NULL
void SetNoteBookPower(void)
{
    // NoteBookCheck();
    // 双口转单口时刻,当没关掉的口有申请20V时,另一个口初始化为POWER_ALL-65W,否则初始化为65W
    if ((MyPower.OldPortSta == 3) && ((MyPower.currPortSta == 1) || (MyPower.currPortSta == 2)))
    {
        // 剩下的口为PD协议
        if ((PORT[MyPower.currPortSta - 1].curr_protocol[2] == SRC_FIX) || (PORT[MyPower.currPortSta - 1].curr_protocol[2] == SRC_PPS))
        {
            // 还剩下C1口,为20V
            if ((MyPower.currPortSta == 1) && ((MyPower.NoteBookSta == 1) || (MyPower.NoteBookSta == 3)))
            {
                PORT[0].temp_targ_power_pd = 65;
                PORT[1].temp_targ_power_pd = POWER_ALL - 65;
            }
            // 还剩下C2口,为20V
            else if ((MyPower.currPortSta == 2) && ((MyPower.NoteBookSta == 2) || (MyPower.NoteBookSta == 3)))
            {
                PORT[1].temp_targ_power_pd = 65;
                PORT[0].temp_targ_power_pd = POWER_ALL - 65;
            }
            else// 剩下的口为非20V PD协议 则预分配关掉的口为65W,避免后续插笔记本不兼容
            {
                if (MyPower.currPortSta == 1)
                {
                    PORT[1].temp_targ_power_pd = 65;
                    PORT[0].temp_targ_power_pd = POWER_ALL - 65;
                }
                else if (MyPower.currPortSta == 2)
                {
                    PORT[0].temp_targ_power_pd = 65;
                    PORT[1].temp_targ_power_pd = POWER_ALL - 65;
                }
            }
        }
        
        // 剩下的口为非PD协议
        else if (PORT[MyPower.currPortSta - 1].curr_protocol[2] == 0)
        {
            PORT[MyPower.currPortSta - 1].temp_targ_power_pd = 15;
            PORT[!(MyPower.currPortSta - 1)].temp_targ_power_pd = POWER_ALL - 15;
        }
        else if (PORT[MyPower.currPortSta - 1].curr_protocol[2] == SRC_SCP)
        {
            PORT[MyPower.currPortSta - 1].temp_targ_power_pd = 25;
            PORT[!(MyPower.currPortSta - 1)].temp_targ_power_pd = POWER_ALL - 25;
        }
        else
        {
            PORT[MyPower.currPortSta - 1].temp_targ_power_pd = 20;
            PORT[!(MyPower.currPortSta - 1)].temp_targ_power_pd = POWER_ALL - 20;
        }
    }
    // 单口转双口时,根据提前分配的功率来分配
    else if((MyPower.currPortSta == 3)&&((MyPower.OldPortSta == 1)||(MyPower.OldPortSta == 2)))
    {
        ;
    }
    // 其它情况,初始化为50W
    else
    {
        PORT[0].targ_power_pd = POWER_ALL/2;
        PORT[1].targ_power_pd = POWER_ALL/2;
    }
}

/// @brief 检测20V设备情况
/// @param NULL 
void NoteBookCheck(void)
{
    if ((PORT[0].curr_vout >= 18000) && (PORT[1].curr_vout <= 16000))
        MyPower.NewNoteBookSta = 1;
    else if ((PORT[1].curr_vout >= 18000) && (PORT[0].curr_vout <= 16000))
        MyPower.NewNoteBookSta = 2;
    else if ((PORT[1].curr_vout >= 18000) && (PORT[0].curr_vout >= 18000))
        MyPower.NewNoteBookSta = 3;
    else
        MyPower.NewNoteBookSta = 0;

    // 新增20V设备则直接更新,减小则延迟2.5s更新
    if ((MyPower.NoteBookSta != MyPower.NewNoteBookSta)&&(MyPower.NoteBookStaChange == 0))
    {
        MyPower.NoteBookStaChange = 1;
        // 减少
    if (((MyPower.NoteBookSta==3)&&(MyPower.NoteBookSta!=3))||((MyPower.NoteBookSta>=1)&&(MyPower.NewNoteBookSta==0)))
            MyPower.NoteBookCheck_TIMER = 250;// 2.5s
        // 新增 或 转移
        else
        {
            MyPower.NoteBookCheck_TIMER = 0;// 0s

            switch (MyPower.currPortSta)
            {
                case 1:// 单C1口后续申请了20V
                    PORT[0].temp_targ_power_pd = 65;
                    PORT[1].temp_targ_power_pd = POWER_ALL - 65;
                    MyPower.NexState = SING_MODE;
                    break;
                case 2:// 单C2口后续申请了20V
                    PORT[1].temp_targ_power_pd = 65;
                    PORT[0].temp_targ_power_pd = POWER_ALL - 65;
                    MyPower.NexState = SING_MODE;
                    break;
                default:
                    break;
            }
        }
    }

    if ((MyPower.NoteBookCheck_TIMER==0)&&(MyPower.NoteBookStaChange == 1))
    {
        MyPower.NoteBookStaChange = 0;
        MyPower.NoteBookSta = MyPower.NewNoteBookSta;
    }
}
#endif

/// @brief 设置功率变化上限
/// @param NULL
void SetPowerMAX(void)
{
#ifdef SUPPORT_NoteBook
    // NoteBookCheck();
    // 设置功率调节上限,防止充笔记本电脑时出现45W以下广播导致断充
    switch (MyPower.NoteBookSta)
    {
    case 1:// 仅C1有笔电
        POWER_C2_MAX = POWER_ALL - 65;
        POWER_C1_MAX = POWER_ALL - 20;
        break;

    case 2:// 仅C2有笔电
        POWER_C1_MAX = POWER_ALL - 65;
        POWER_C2_MAX = POWER_ALL - 20;
        break;
    case 3:// 都有
        POWER_C1_MAX = POWER_ALL - 65;
        POWER_C2_MAX = POWER_ALL - 65;
        break;

    default:// 无笔电
        POWER_C1_MAX = POWER_ALL - 20;
        POWER_C2_MAX = POWER_ALL - 20;
        break;
    }
#else
    // 设置功率调节上限,防止充笔记本电脑时出现45W以下广播导致断充
    if ((PORT[0].curr_vout >= 18000) && (PORT[1].curr_vout <= 16000))
    {
        POWER_C2_MAX = POWER_ALL-45;
        POWER_C1_MAX = POWER_ALL-20;
    }
    else if ((PORT[1].curr_vout >= 18000) && (PORT[0].curr_vout <= 16000))
    {
        POWER_C1_MAX = POWER_ALL-45;
        POWER_C2_MAX = POWER_ALL-20;
    }
    else if ((PORT[1].curr_vout >= 18000) && (PORT[0].curr_vout >= 18000))
    {
        POWER_C1_MAX = POWER_ALL-45;
        POWER_C2_MAX = POWER_ALL-45;
    }
    else
    {
        POWER_C1_MAX = POWER_ALL-20;
        POWER_C2_MAX = POWER_ALL-20;
    }
#endif
    //C1进入非PD非DCP协议,C2进入PD时,C1广播小功率
    if (((PORT[0].curr_protocol[2] != SRC_FIX) && (PORT[0].curr_protocol[2] != SRC_PPS) && (PORT[0].curr_protocol[2] != 0))&&((PORT[1].curr_protocol[2] == SRC_FIX) || (PORT[1].curr_protocol[2] == SRC_PPS)) )
    {
        PORT[0].targ_power_pd = 25;
        //if(PORT[0].curr_protocol[2] == SRC_SCP)//HSCP 40W
            //PORT[0].targ_power_pd = 40;
        PORT[1].targ_power_pd = POWER_ALL-PORT[0].targ_power_pd;
        PORT[0].change_flag = 1;
        PORT[1].change_flag = 1;
        MyPower.NexState = TURN_MODE;
    }
    //C2进入非PD非DCP协议,C1进入PD时,C2广播小功率
    else if (((PORT[1].curr_protocol[2] != SRC_FIX) && (PORT[1].curr_protocol[2] != SRC_PPS) && (PORT[1].curr_protocol[2] != 0))&&((PORT[0].curr_protocol[2] == SRC_FIX) || (PORT[0].curr_protocol[2] == SRC_PPS)) )
    {
        PORT[1].targ_power_pd = 25;
        //if(PORT[1].curr_protocol[2] == SRC_SCP)//HSCP 40W
            //PORT[1].targ_power_pd = 40;
        PORT[0].targ_power_pd = POWER_ALL-PORT[1].targ_power_pd;
        PORT[1].change_flag = 1;
        PORT[0].change_flag = 1;
        MyPower.NexState = TURN_MODE;
    }
    //C1和C2进入非PD非DCP协议,C1 C2均分功率
    else if (((PORT[0].curr_protocol[2] != SRC_FIX) && (PORT[0].curr_protocol[2] != SRC_PPS) && (PORT[0].curr_protocol[2] != 0))&&((PORT[1].curr_protocol[2] != SRC_FIX) && (PORT[1].curr_protocol[2] != SRC_PPS) && (PORT[1].curr_protocol[2] != 0)) )
    {
        PORT[1].targ_power_pd = POWER_ALL/2;
        PORT[0].targ_power_pd = POWER_ALL/2;
        PORT[1].change_flag = 1;
        PORT[0].change_flag = 1;
        MyPower.NexState = TURN_MODE;
    }
    //仅C1进入DCP协议时,C1分配20W
    else if( (PORT[0].curr_protocol[2] == 0)&&(PORT[1].curr_protocol[2] != 0) )
    {
        PORT[0].targ_power_pd = 15;
        PORT[1].targ_power_pd = POWER_ALL-PORT[0].targ_power_pd;
        PORT[0].change_flag = 1;
        PORT[1].change_flag = 1;
        MyPower.NexState = TURN_MODE;
    }
    //仅C2进入DCP协议时,C2分配20W
    else if( (PORT[1].curr_protocol[2] == 0)&&(PORT[0].curr_protocol[2] != 0) )
    {
        PORT[1].targ_power_pd = 15;
        PORT[0].targ_power_pd = POWER_ALL-PORT[1].targ_power_pd;
        PORT[0].change_flag = 1;
        PORT[1].change_flag = 1;
        MyPower.NexState = TURN_MODE;
    }
}

/// @brief 响应命令,定时检查NTC变化、功率变化、端口状态变化、定时蓝牙上报数据、定时检查ERR
/// @param NULL
void StateCheck(void)
{
    // 控制响应
    if (SW3516_Info.SetPDO_f == true)
    {
        if (MyPower.currPortSta==3)// 双口状态
        {
            if (ComdData[2] < 15)
                ComdData[2] = 15;
            else if (ComdData[2] > 85)
                ComdData[2] = 85;
        }
        else
        {
            if (ComdData[2] < 15)
                ComdData[2] = 15;
        }

        PORT[ComdData[0]].targ_power_pd = ComdData[2];
        PORT[!ComdData[0]].targ_power_pd = (100 - ComdData[2]);
        // ComdData[1];
        PORT[0].change_flag = 1;
        PORT[1].change_flag = 1;
        MyPower.NexState = TURN_MODE;
        SW3516_Info.SetPDO_f = false;
    }

#ifdef SUPPORT_NTC 
    // NTC检测降功率
    if (MyPower.ADCCheck_TIMER==0)
    {
        if (adc_ntc_read()<1300)
        {
            MyPower.NTCSta = 1;
            POWER_ALL = 80;
            MyPower.NTCLowPower_TIMER = 500; // 5s后恢复功率
        }
        else if (adc_ntc_read()>1450)
        {
            if (MyPower.NTCLowPower_TIMER==0)
            {
                if(MyPower.currPortSta==3)
                    POWER_ALL = 120;
                else
                    POWER_ALL = 100;
                MyPower.NTCSta = 0;
            }
        }

        if (MyPower.OldNTCSta != MyPower.NTCSta)
        {
            MyPower.OldPortSta = 0;// 促使重新广播
            MyPower.OldNTCSta = MyPower.NTCSta;
            ESP_LOGE(GATTS_TABLE_TAG, "current port MAX power is: %dW", POWER_ALL);
        }

        MyPower.ADCCheck_TIMER = 100;
    }
#endif

#if UART_TEST==0
    // 读取功率、状态
    // if ((MyPower.Check_TIMER == 0) && (MyPower.NexState == IDLE) && (MyPower.OldPortSta == MyPower.currPortSta))
    if ((MyPower.Check_TIMER == 0) && (MyPower.NexState == IDLE))
    {
        PowerRead(0);
        PowerRead(1);

        MyPower.Check_TIMER = 20; // 200ms一次
    }
#endif

#ifdef SUPPORT_NoteBook
    // 20V笔记本电脑检测
    NoteBookCheck();
#endif

    // 端口检查
    if (MyPower.PortCheck_TIMER == 0)
    {
        if ((PORT[0].port_state) && (PORT[1].port_state))
            MyPower.currPortSta = 3;
        else if ((PORT[0].port_state == 0) && (PORT[1].port_state == 1))
            MyPower.currPortSta = 2;
        else if ((PORT[0].port_state == 1) && (PORT[1].port_state == 0))
            MyPower.currPortSta = 1;
        else
            MyPower.currPortSta = 0;

        // 进入对应端口状态
        if (MyPower.OldPortSta != MyPower.currPortSta)
        {
            if (MyPower.currPortSta < MyPower.OldPortSta) // 检测到有设备拔出
                MyPower.PortCheckDelay_COUNT++;
            else
                MyPower.PortCheckDelay_COUNT += 5;// 检测到插入设备则快速响应 

            if (MyPower.PortCheckDelay_COUNT >= 10) // 多次确认端口变化 10*500ms
            {
            #ifdef SUPPORT_NoteBook
                SetNoteBookPower();
            #endif
                switch (MyPower.currPortSta)
                {
                case 0:
                    MyPower.NexState = IDLE;
                    break;

                case 3:
                {
                    MyPower.FirstIn = 1;
                    MyPower.FirstIn_TIMER = 4000; // 40s后进行首次智能功率分配
                    #ifdef SOURCE_IP2730 
                        POWER_ALL = (IP2730.PD_PMAX>100)?100:IP2730.PD_PMAX;
                    #else
                        POWER_ALL = 100;
                    #endif
                    MyPower.ToMultSta = MyPower.OldPortSta;
                    MyPower.NexState = MULT_MODE;
                }
                    break;

                default:
                    MyPower.FirstIn = 0;
                    MyPower.FirstIn_TIMER = 0;
                    #ifdef SOURCE_IP2730 
                        POWER_ALL = (IP2730.PD_PMAX>100)?100:IP2730.PD_PMAX;
                    #else
                        POWER_ALL = 100;
                    #endif
                    MyPower.NexState = SING_MODE;
                    break;
                }
                MyPower.OldPortSta = MyPower.currPortSta;// 更新端口状态
                MyPower.PortCheckDelay_COUNT = 0;

#ifdef SUPPORT_VBUS_DETECT
                // 待定义 需要初始化对应的VBus控制IO

                if (PORT[0].port_state)
                    VBusCtl[0].LoadStart_TIMER = 30000; // 5min后开始检测轻载电流
                else
                    VBusCtl[0].LowLoadStart = 0; // 轻载电流检测结束

                if (PORT[1].port_state)
                    VBusCtl[1].LoadStart_TIMER = 30000; // 5min后开始检测轻载电流
                else
                    VBusCtl[1].LowLoadStart = 0; // 轻载电流检测结束
#endif
            }

            // // 端口发生变化则刷新功率分配时间
            // if ((PORT[0].curr_protocol[2] == SRC_PPS) || (PORT[1].curr_protocol[2] == SRC_PPS))
            //     MyPower.PowerCheck_TIMER = 2500; // 30s一次
            // else
            //     MyPower.PowerCheck_TIMER = 1100; // 12s一次
        }
        else
        {
#ifdef SUPPORT_NoteBook
            // NoteBookCheck();
#endif
            MyPower.PortCheckDelay_COUNT = 0;
            MyPower.NexState = IDLE;
        }

        MyPower.PortCheck_TIMER = 50; // 500ms一次
    }

    // 功率检查,连接蓝牙或者功率受限后停止分配
    if ((MyPower.PowerCheck_TIMER == 0) && (MyPower.NexState == IDLE) && (SW3516_Info.Notify_f == false)&&(POWER_ALL >= 65))
    {
        uint8_t BigPowerPort = 0;

        //双口状态下,设置功率变化上限,如果有一个口进入非PD模式,则直接更新功率
        if((MyPower.currPortSta==3)&&(MyPower.OldPortSta==3))
            SetPowerMAX();

        // 两个口均处于PD协议
        if ((PORT[0].curr_protocol[2] == SRC_FIX || PORT[0].curr_protocol[2] == SRC_PPS) && (PORT[1].curr_protocol[2] == SRC_FIX || PORT[1].curr_protocol[2] == SRC_PPS))
        {
            // 判定大功率口 和 小功率口
            if (PORT[0].curr_power > PORT[1].curr_power)
                BigPowerPort = 0;
            else if (PORT[0].curr_power < PORT[1].curr_power)
                BigPowerPort = 1;
            else
                BigPowerPort = 3;

            // 首次进入智能功率分配模式时,等待较长时间,分配较大幅度功率
            if (MyPower.FirstIn)
            {
                if (MyPower.FirstIn_TIMER == 0)
                {
                    MyPower.FirstIn = 0;
                    // 只有一个端口的实际功率接近分配的PD功率时,才进行分配
                    if ((((PORT[0].curr_power + 2 * POWER_STEEP) > POWER_ALL / 2) && ((PORT[1].curr_power + 4 * POWER_STEEP) < POWER_ALL / 2)) || (((PORT[1].curr_power + 2 * POWER_STEEP) > POWER_ALL / 2) && ((PORT[0].curr_power + 4 * POWER_STEEP) < POWER_ALL / 2)))
                    {
                        if (BigPowerPort == 0)
                        {
                            // if ((POWER_ALL-(PORT[1].curr_power - ((int)PORT[1].curr_power % 5) + 10)) < POWER_C1_MAX)
                            #ifdef SUPPORT_NoteBook
                            if (MyPower.NewNoteBookSta != 3)// 待优化
                            #endif
                            {
                                PORT[1].targ_power_pd = PORT[1].curr_power - ((int)PORT[1].curr_power % 5) + 10;
                                if (PORT[1].targ_power_pd < 20)
                                    PORT[1].targ_power_pd = 20;
                                // if ((PORT[1].curr_protocol[2] == SRC_PPS)&&(PORT[1].targ_power_pd<35))
                                //     PORT[1].targ_power_pd = 45;
                                PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;

                                // ESP_LOGE(GATTS_TABLE_TAG, "FIRST FIRST FIRST FIRST,PORT[0].targ_power_pd is: %dW",PORT[0].targ_power_pd);
                            }
                        }
                        else if (BigPowerPort == 1)
                        {
                            // if ((POWER_ALL-(PORT[0].curr_power - ((int)PORT[0].curr_power % 5) + 10)) < POWER_C2_MAX)
                            #ifdef SUPPORT_NoteBook
                            if (MyPower.NewNoteBookSta != 3)// 待优化
                            #endif
                            {
                                PORT[0].targ_power_pd = PORT[0].curr_power - ((int)PORT[0].curr_power % 5) + 10;
                                if (PORT[0].targ_power_pd < 20)
                                    PORT[0].targ_power_pd = 20;
                                // if ((PORT[0].curr_protocol[2] == SRC_PPS)&&(PORT[0].targ_power_pd<35))
                                //     PORT[0].targ_power_pd = 45;
                                PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
                            }
                        }

                        PORT[0].change_flag = 1;
                        PORT[1].change_flag = 1;
                        MyPower.NexState = TURN_MODE;
                    }
                }
            }
            // 后续进入智能功率分配模式时,等待较短时间,分配较小幅度功率
            else
            {
                // C1大于C2,且C2没抽满
                //if ((BigPowerPort == 0))
                // {
                    // C1调节的功率在范围内,且C1实际抽载快接近广播功率,且C2实际抽载远小于广播功率
                    if ((PORT[0].targ_power_pd < POWER_C1_MAX) && ((PORT[0].curr_power + POWER_STEEP) >= PORT[0].targ_power_pd) && ((PORT[1].curr_power + 2 * POWER_STEEP) < PORT[1].targ_power_pd))
                    {
                        PORT[0].targ_power_pd = PORT[0].targ_power_pd + POWER_STEEP;
                        PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
                        PORT[0].change_flag = 1;
                        PORT[1].change_flag = 1;
                        MyPower.NexState = TURN_MODE;
                    }
                // }
                // C2大于C1,且C1没抽满
                //else if ((BigPowerPort == 1))
                // {
                    // C2调节的功率在范围内,且C2实际抽载快接近广播功率,且C1实际抽载远小于广播功率
                    else if ((PORT[1].targ_power_pd < POWER_C2_MAX) && ((PORT[1].curr_power + POWER_STEEP) >= PORT[1].targ_power_pd) && ((PORT[0].curr_power + 2 * POWER_STEEP) < PORT[0].targ_power_pd))
                    {
                        PORT[1].targ_power_pd = PORT[1].targ_power_pd + POWER_STEEP;
                        PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;
                        PORT[1].change_flag = 1;
                        PORT[0].change_flag = 1;
                        MyPower.NexState = TURN_MODE;
                    }
                // }
            }
            // // 异常功率复位
            // if ((PORT[0].targ_power_pd > POWER_C1_MAX) || (PORT[1].targ_power_pd > POWER_C2_MAX))
            //     MyPower.NexState = MULT_MODE;
        }

        // PPS协议时间拉长
        if ( (PORT[0].curr_protocol[2] == SRC_PPS)|| (PORT[1].curr_protocol[2] == SRC_PPS) )
            MyPower.PowerCheck_TIMER = 2500; // 30s一次
        else
            MyPower.PowerCheck_TIMER = 1100; // 12s一次
    }

#ifdef SUPPORT_VBUS_DETECT
    // 充满断电检测
    for (int i = 0; i < 2; i++)
    {
        // 轻载关断检测
        if (VBusCtl[i].VBusOFF_TIMER == 0)
        {
            // 启机5分钟后开始检测电流
            if (VBusCtl[i].LoadStart_TIMER == 1)
            {
                VBusCtl[i].LowLoadStart = 1;
                VBusCtl[i].LoadStart_TIMER = 0;
                // ESP_LOGE(GATTS_TABLE_TAG, "CHECK CURRENT BRGIN");
            }

            // SCP且开始了轻载检测
            if ((VBusCtl[i].LowLoadStart) && (PORT[i].curr_protocol[2] == SRC_SCP))
                VBusCtl[i].VBusSta = 3;
            // DCP且开始了轻载检测
            else if ((PORT[i].curr_protocol[2] == 0) && (VBusCtl[i].LowLoadStart))
                VBusCtl[i].VBusSta = 2;
            // 非SCP且非DCP且开始了轻载检测
            else if ((VBusCtl[i].LowLoadStart) && (PORT[i].curr_protocol[2] != SRC_SCP))
                VBusCtl[i].VBusSta = 1;

            // 未开始轻载检测
            else if (VBusCtl[i].LowLoadStart == 0)
            {
                VBusCtl[i].VBusOFF_COUNT = 0;
                VBusCtl[i].LEDSta = 0;
                VBusCtl[i].VBusSta = 0;
            }

            if ((VBusCtl[i].VBusSta == 1) && (VBusCtl[i].LEDSta == 0))
            {
                if (PORT[i].curr_iout < 100)
                    VBusCtl[i].VBusOFF_COUNT++;
            }
            else if ((VBusCtl[i].VBusSta == 2) && (VBusCtl[i].LEDSta == 0))
            {
                if (PORT[i].curr_iout < 30)
                {
                    VBusCtl[i].VBusOFF_COUNT++;
                    // ESP_LOGE(GATTS_TABLE_TAG, "MODE:DCP  VBusOFF_COUNT: %d", (int)VBusCtl[i].VBusOFF_COUNT);
                }

                if (PORT[i].curr_iout < 20)
                {
                    VBusCtl[i].VBusOFF_COUNT++;
                    // ESP_LOGE(GATTS_TABLE_TAG, "MODE:DCP  VBusOFF_COUNT: %d", (int)VBusCtl[i].VBusOFF_COUNT);
                }

                if (PORT[i].curr_iout < 10)
                {
                    VBusCtl[i].VBusOFF_COUNT++;
                    // ESP_LOGE(GATTS_TABLE_TAG, "MODE:DCP  VBusOFF_COUNT: %d", (int)VBusCtl[i].VBusOFF_COUNT);
                }
            }
            else if ((VBusCtl[i].VBusSta == 3) && (VBusCtl[i].LEDSta == 0))
            {
                if (PORT[i].curr_iout < 200) // 200mA
                    VBusCtl[i].VBusOFF_COUNT++;
            }

            // 快充小电流5分钟
            if ((VBusCtl[i].VBusOFF_COUNT >= 30000) && ((VBusCtl[i].VBusSta == 1 || VBusCtl[i].VBusSta == 3)))
            {
                VBusCtl[i].VBusOFF_COUNT = 0;
                VBusCtl[i].LEDSta = 1;
            }
            // DCP小电流20分钟
            else if ((VBusCtl[i].VBusOFF_COUNT >= 120000) && (VBusCtl[i].VBusSta == 2))
            {
                VBusCtl[i].VBusOFF_COUNT = 0;
                VBusCtl[i].LEDSta = 1;
            }

            if (VBusCtl[i].LEDSta)
            {
                // 待定义 开启电平关断VBus
            }

            VBusCtl[i].VBusOFF_TIMER = 100; // 1S一次
        }
    }
#endif

    //数据上报
    if ((MyPower.NotifySta) && (MyPower.Notify_TIMER==0))
    {
        *(uint16_t*)ReportedData = DEVICE_ID_ESP32S3;
        ReportedData[2]          = 0x00; //crc result
        ReportedData[3]          = 0x00;
        //ReportedData[4]          = 0x00; //password state
        //ReportedData[5]          = 0x00;
        //ReportedData[6]          = 0x00; //new password
        //ReportedData[7]          = 0x00;
        if(true == SW3516_Info.Notify_f)
        {
            esp_ble_gatts_send_indicate(sw3516_gatts.gatts_if, sw3516_gatts.conn_id, sw3516_gatts.service_handle,
                                    sizeof(ReportedData), ReportedData, false);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXXXXXXXXXXXX: %d", ReportedData[0]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXXXXXXXXXXXX: %d", ReportedData[1]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXXXXXXXXXXXX: %d", ReportedData[2]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXXXXXXXXXXXX: %d", ReportedData[3]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX1XXXXXXXXXXX: %d", ReportedData[4]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX2XXXXXXXXXXX: %d", ReportedData[5]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX3XXXXXXXXXXX: %d", ReportedData[6]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX4XXXXXXXXXXX: %d", ReportedData[7]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX5XXXXXXXXXXX: %d", ReportedData[8]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX6XXXXXXXXXXX: %d", ReportedData[9]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX7XXXXXXXXXXX: %d", ReportedData[10]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX8XXXXXXXXXXX: %d", ReportedData[11]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX9XXXXXXXXXXX: %d", ReportedData[12]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXX10XXXXXXXXXXX: %d", ReportedData[13]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX11XXXXXXXXXX: %d", ReportedData[14]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX12XXXXXXXXXX: %d", ReportedData[15]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX13XXXXXXXXXX: %d", ReportedData[16]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX14XXXXXXXXXX: %d", ReportedData[17]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX15XXXXXXXXXX: %d", ReportedData[18]);
            // ESP_LOGE(GATTS_TABLE_TAG, "XXXXXXXXXXXX16XXXXXXXXXX: %d", ReportedData[19]);
        }
         MyPower.Notify_TIMER = 200;// 2秒一次
    }

    // 错误检查
    if (MyPower.SystemI2CCheck_TIMER == 0)
    {
        // if(MyPower.currPortSta==0)//查询不到
        {
            // ESP_LOGE(GATTS_TABLE_TAG, "current port is C:%d  current vin: %d",MyPower.currPortSta,PORT[0].curr_vin);
        }
        MyPower.SystemI2CCheck_TIMER = 200; // 2S一次
    }
}

/// @brief 默认状态处理函数
/// @param NULL
void IDLE_Progress(void)
{
    FlushState();
}

/// @brief 单口状态处理函数,广播满功率
/// @param NULL
void SING_MODE_Progress(void)
{
    FlushState();

    // 单C1
    if (MyPower.currPortSta==1)
    {
        PORT[0].change_flag = 1;
        PORT[0].targ_power_pd = POWER_ALL;
        ESP_LOGE(GATTS_TABLE_TAG, "current port is: C1 ,power is: %dW", PORT[0].targ_power_pd);
#ifdef SUPPORT_NoteBook
        PORT[1].targ_power_pd = PORT[1].temp_targ_power_pd;
        PORT[1].change_flag = 1;
#endif
        // PowerWrite(0);
    }
    
    // 单C2
    else if (MyPower.currPortSta==2)
    {
        PORT[1].change_flag = 1;
        PORT[1].targ_power_pd = POWER_ALL;
        ESP_LOGE(GATTS_TABLE_TAG, "current port is: C2 ,power is: %dW", PORT[1].targ_power_pd);
#ifdef SUPPORT_NoteBook
        PORT[0].targ_power_pd = PORT[0].temp_targ_power_pd;
        PORT[0].change_flag = 1;
#endif
        // PowerWrite(1);
    }

    MyPower.NexState = TURN_MODE;
}

/// @brief 多口状态位处理函数,初次进入双口状态均分功率
/// @param NULL
void MULT_MODE_Progress(void)
{
    FlushState();
    // if (MyPower.ToMultSta == 1) // C1 to Mult
    // {
    //     PORT[0].targ_power_pd = (uint16_t)(PORT[0].curr_power + 5);
    //     if (PORT[0].targ_power_pd >= 85)
    //         PORT[0].targ_power_pd = 85;
    //     PORT[0].change_flag = 1;

    //     PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
    //     PORT[1].change_flag = 1;
    // }
    // else if (MyPower.ToMultSta == 2) // C2 to Mult
    // {
    //     PORT[1].targ_power_pd = (uint16_t)(PORT[1].curr_power + 5);
    //     if (PORT[1].targ_power_pd >= 85)
    //         PORT[1].targ_power_pd = 85;
    //     PORT[1].change_flag = 1;

    //     PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;
    //     PORT[0].change_flag = 1;
    // }
    // else
    {
        // // 检测到只有一边是20V设备,则分配至少65W功率
        // if ((MyPower.NoteBookSta==1)||(MyPower.NoteBookSta==2))
        // {
        //     if (PORT[0].curr_vout >= 18000)
        //     {
        //         PORT[0].targ_power_pd = 65;
        //         PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
        //     }
        //     else
        //     {
        //         PORT[1].targ_power_pd = 65;
        //         PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;
        //     }
        //     PORT[0].change_flag = 1;
        //     PORT[1].change_flag = 1;
        // }
        // // 否则均分功率
        // else
        // {
        //     PORT[0].targ_power_pd = POWER_ALL / 2;
        //     PORT[0].change_flag = 1;

        //     PORT[1].targ_power_pd = POWER_ALL / 2;
        //     PORT[1].change_flag = 1;
        // }
#ifdef SUPPORT_NoteBook

        if (PORT[0].temp_targ_power_pd == 0)// 防止分配功率为0
        {
            PORT[0].temp_targ_power_pd = POWER_ALL/2;
            PORT[1].temp_targ_power_pd = POWER_ALL/2;
        }

        PORT[0].targ_power_pd = PORT[0].temp_targ_power_pd;
        PORT[1].targ_power_pd = PORT[1].temp_targ_power_pd;
        // switch (MyPower.NoteBookSta)
        // {
        // case 1:
        //         PORT[0].targ_power_pd = 65;
        //         PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
        //     break;
        
        // case 2:
        //         PORT[1].targ_power_pd = 65;
        //         PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;
        //     break;

        // case 3:
        //         // PORT[0].targ_power_pd = 65;
        //         // PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;

        //         // 根据20V设备的情况来分配功率,先插的20V设备分配65W,后插的20V设备分配剩余的功率,同插优先C1
        //         {
        //             if (MyPower.NoteBookSta == 2)
        //             {
        //                 PORT[1].targ_power_pd = 65;
        //                 PORT[0].targ_power_pd = POWER_ALL - PORT[1].targ_power_pd;
        //             }
        //             else
        //             {
        //                 PORT[0].targ_power_pd = 65;
        //                 PORT[1].targ_power_pd = POWER_ALL - PORT[0].targ_power_pd;
        //             }
        //         }
        //     break;

        // default:
        //     PORT[0].targ_power_pd = POWER_ALL / 2;
        //     PORT[1].targ_power_pd = POWER_ALL / 2;       
        //     break;
        // }

        PORT[0].change_flag = 1; 
        PORT[1].change_flag = 1; 
#else
        {
            PORT[0].targ_power_pd = POWER_ALL / 2;
            PORT[0].change_flag = 1;

            PORT[1].targ_power_pd = POWER_ALL / 2;
            PORT[1].change_flag = 1;
        }
#endif
    }

    MyPower.PowerCheck_TIMER = 1000;
    MyPower.NexState = TURN_MODE;
}

/// @brief 切换功率状态处理函数,进行功率切换操作
/// @param NULL
void TURN_MODE_Progress(void)
{
    FlushState();

    if ((MyPower.currPortSta) && (PORT[0].change_flag) && (PORT[0].curr_power_pd != PORT[0].targ_power_pd))
    {
        PowerWrite(0);
        PORT[0].change_flag = 0;
        PORT[0].curr_power_pd = PORT[0].targ_power_pd;
        ESP_LOGE(GATTS_TABLE_TAG, "turn current port is: C1 ,power is: %dW", PORT[0].targ_power_pd);
    }

    if ((MyPower.currPortSta) && (PORT[1].change_flag) && (PORT[1].curr_power_pd != PORT[1].targ_power_pd))
    {
        PowerWrite(1);
        PORT[1].change_flag = 0;
        PORT[1].curr_power_pd = PORT[1].targ_power_pd;
        ESP_LOGE(GATTS_TABLE_TAG, "turn current port is: C2 ,power is: %dW", PORT[1].targ_power_pd);
    }

    MyPower.NexState = IDLE;
}

/// @brief 功率自动分配任务函数
/// @param NULL
void PowerAllocation_task(void *parm)
{
#ifdef SUPPORT_WIFI_GET_TIME
    wifi_init_sta(); // 初始化并启动WiFi STA模式
    initialize_sntp();// 初始化并启动SNTP时间同步
    esp_wifi_stop(); // 停止WiFi STA模式
    // while(1)
    // {
    //     print_time_with_timezone(8); // 打印当前时间
    //     vTaskDelay(1000 / portTICK_PERIOD_MS); // 延时1秒
    // }
#endif
    xTaskCreate(UserTimer_task, "UserTimer_task", 1024, NULL, 7, NULL);
#if UART_TEST
    uart_init();
    xTaskCreate(rx_task, "uart_rx_task", 2048 * 2, NULL, configMAX_PRIORITIES - 1, NULL);
#endif
    MyPower.NexState = IDLE;

    // init C1
    GPIOConfigSW35xxI2C(0);
    // init C2
    GPIOConfigSW35xxI2C(1);

// #ifdef SUPPORT_NTC
    // init ADC
    adc_ntc_init();
// #endif

    while (1)
    {
        //TimerTick();

        StateCheck();

        switch (MyPower.NexState)
        {
        case IDLE:
            IDLE_Progress();
            break;

        case SING_MODE: // 单口 100W
            // MyPower.LowSpeedCharg = 0;
            SING_MODE_Progress();
            break;

        case MULT_MODE: // 双口功率动态分配
            // MyPower.LowSpeedCharg = 1;
            MULT_MODE_Progress();
            break;

        case TURN_MODE: // 改变功率
            TURN_MODE_Progress();
            break;

        default:
            break;
        }

        //vTaskDelay(pdMS_TO_TICKS(10)); 
        vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms/loop
    }
}

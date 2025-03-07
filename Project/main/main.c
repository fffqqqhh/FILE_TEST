/*
 *SPDX-文件版权文本:2021-2023 乐鑫系统(上海)有限公司
 *
 *SPDX-许可证-标识符:无许可证或 CC0-1.0
 */
/****************************************************************************
 *
 *该演示展示了使用预定义属性表创建 GATT 数据库.
 *它充当GATT服务器,可以发送adv数据,由客户端连接.
 *运行gatt_client demo,客户端demo将自动连接到gatt_server_service_table demo.
 *客户端演示将在连接后启用 GATT 服务器的通知.然后两个设备将交换
 *数据.
 *
 ****************************************************************************/

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "driver/gpio.h"

#include "main.h"

#include "user_adc.h"
#include "user_uart.h"

#include "user_gui.h"
// #include "user_SW3566.h"
//#include "user_HUSB238A.h"
#include "user_IP2730.h"
#include "user_oper_data.h"

#include "user_letter_shell.h"

// #include "user_source.h"
#include "user_new_source.h"
#include "user_button.h"


#define LEDG_PIN GPIO_NUM_48

// Log的标签
#define GATTS_TABLE_TAG "【GATTS_TABLE】"

#define GATT_DEVICE_NAME "ESP-POWER-DEMO" // Device Name(苹果扫描的时候看到的device name)

#define PROFILE_APP_NUM 1 // 应用程序的配置文件数量

#define PROFILE_APP0_IDX 0 // 应用程序0索引号
#define ESP_APP0_ID 0x55   // 应用程序0 ID号

// 特征值的最大长度.当GATT客户端执行写或准备写操作时,数据长度必须小于GATTS_CHAR_VAL_LEN_MAX
#define GATTS_CHAR_VAL_LEN_MAX (100)

// Server MTU最大长度(客户端的MTU设置不能小于服务端的MTU)
#define SERVER_MTU_LEN_MAX (GATTS_CHAR_VAL_LEN_MAX + 3 + 20)

// 准备写入(长写入)对应的缓存大小(不小于MTU)
#define PREPARE_BUF_MAX_SIZE 1024

// 特征声明的大小(不需要修改)
#define CHAR_DECLARATION_SIZE (sizeof(uint8_t))

#define ADV_CONFIG_FLAG (1 << 0)      // 广播数据设置完成
#define SCAN_RSP_CONFIG_FLAG (1 << 1) // 扫描响应数据设置完成
static uint8_t adv_config_done = 0;   // 用来检测当 广播数据 和 扫描响应数据 都设置完成后,才开启广播, 起到逻辑与的作用.

// 准备写入的缓存
typedef struct
{
    uint8_t *prepare_buf;
    int prepare_len;
} prepare_type_env_t;
static prepare_type_env_t prepare_write_env;

#define CONFIG_SET_RAW_ADV_DATA
#ifdef CONFIG_SET_RAW_ADV_DATA // 使用原始格式的广告数据设置
// 格式由长度(Length)、类型(Type)和值(Value)组成,也就是我们说的LTV格式.
// 广播数据
static uint8_t raw_adv_data[] = {
    /* flags */
    2, ESP_BLE_AD_TYPE_FLAG, 0x06,
    /* tx power*/
    2, ESP_BLE_AD_TYPE_TX_PWR, 0xeb,

    /* 16 位服务类 UUID 的完整列表 */
    // 3, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xFF, 0x00,

    /* 广播中的设备名称 Local Name(安卓扫描的时候看到的是local name)*/
    11, ESP_BLE_AD_TYPE_NAME_CMPL, 'L', 'H', '-', 'E', 'S', 'P', '3', '2', 'S', '3',
    9, ESP_BLE_AD_MANUFACTURER_SPECIFIC_TYPE, 0x00, 0xce, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
};
// 广播扫描响应数据
static uint8_t raw_scan_rsp_data[] = {
    /* flags */
    2, ESP_BLE_AD_TYPE_FLAG, 0x06,
    /* tx power */
    2, ESP_BLE_AD_TYPE_TX_PWR, 0xeb,

    /* 16 位服务类 UUID 的完整列表 */
    // 3, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xFF, 0x00

	11, ESP_BLE_AD_TYPE_NAME_CMPL, 'L', 'H', '-', 'E', 'S', 'P', '3', '2', 'S', '3',
};

#else  // 使用标准格式的广告数据设置
static uint8_t service_uuid[16] = {
    /* LSB <--------------------------------------------------------------------------------> MSB */
    // first uuid, 16bit, [12],[13] is the value
    0xfb,
    0x34,
    0x9b,
    0x5f,
    0x80,
    0x00,
    0x00,
    0x80,
    0x00,
    0x10,
    0x00,
    0x00,
    0xFF,
    0x00,
    0x00,
    0x00,
};

/* 广播数据 长度必须小于31字节 */
static esp_ble_adv_data_t adv_data = {
    .set_scan_rsp = false,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006, // slave connection min interval, Time = min_interval * 1.25 msec
    .max_interval = 0x0010, // slave connection max interval, Time = max_interval * 1.25 msec
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, // test_manufacturer,
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(service_uuid),
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};

// 扫描响应数据
static esp_ble_adv_data_t scan_rsp_data = {
    .set_scan_rsp = true,
    .include_name = true,
    .include_txpower = true,
    .min_interval = 0x0006,
    .max_interval = 0x0010,
    .appearance = 0x00,
    .manufacturer_len = 0,       // TEST_MANUFACTURER_DATA_LEN,
    .p_manufacturer_data = NULL, //&test_manufacturer[0],
    .service_data_len = 0,
    .p_service_data = NULL,
    .service_uuid_len = sizeof(service_uuid),
    .p_service_uuid = service_uuid,
    .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
};
#endif /* CONFIG_SET_RAW_ADV_DATA */

// 广播参数
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,                                    // 最小广播间隔
    .adv_int_max = 0x40,                                    // 最大广播间隔
    .adv_type = ADV_TYPE_IND,                               // 广播类型
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,                  // 对端设备蓝牙设备地址类型
    .channel_map = ADV_CHNL_ALL,                            // 广播通道
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, // 广播过滤政策
};

// GATT服务的配置文件,用于回调事件中提供有关事件的详细信息和上下文
struct gatts_profile_inst
{
    esp_gatts_cb_t gatts_cb;       // GATT服务器回调函数指针
    esp_gatt_if_t gatts_if;        // GATT服务器操作接口
    uint16_t app_id;               // 应用程序ID
    uint16_t conn_id;              // 连接ID
    uint16_t service_handle;       // 服务句柄
    esp_gatt_srvc_id_t service_id; // 服务ID
    uint16_t char_handle;          // 特征句柄
    esp_bt_uuid_t char_uuid;       // 特征值UUID
    esp_gatt_perm_t perm;          // 特征值属性
    esp_gatt_char_prop_t property; // 描述符句柄
    uint16_t descr_handle;         // 描述符UUID
    esp_bt_uuid_t descr_uuid;
};

// GATT 事件处理函数,这个函数通常作为回调在GATT服务实现时被调用,以响应不同的BLE事件.
static void gatts_profile_event_handler(esp_gatts_cb_event_t event,
                                        esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);

/*1个基于gatt的配置文件1个app_id和1个gatts_if,该数组将存储ESP_GATTS_REG_EVT返回的gatts_if*/
static struct gatts_profile_inst heart_rate_profile_tab[PROFILE_APP_NUM] = {
    [PROFILE_APP0_IDX] = {
        .gatts_cb = gatts_profile_event_handler, // GATT服务回调函数
        .gatts_if = ESP_GATT_IF_NONE,            // 未获取 gatt_if,因此初始值为 ESP_GATT_IF_NONE
    },
};

static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;                // 服务UUID
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;         // 特征声明UUID
static const uint16_t character_client_config_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG; // 客户端特征配置UUID

// 特征权限
static const uint8_t char_prop_read = ESP_GATT_CHAR_PROP_BIT_READ;
static const uint8_t char_prop_write = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t char_prop_read_write_notify = ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_NOTIFY;

// 服务和特征 Default
static const uint16_t GATTS_SERVICE_UUID_DEFA = 0xFF00;
static const uint16_t GATTS_CHAR_UUID_DEFA_A = 0xFF01;
static const uint16_t GATTS_CHAR_UUID_DEFA_B = 0xFF02;
static const uint16_t GATTS_CHAR_UUID_DEFA_C = 0xFF03; // 上报充电数据
static const uint16_t GATTS_CHAR_UUID_DEFA_D = 0xFF04; // 设置PDO

// 服务和特征 1
static const uint16_t GATTS_SERVICE_UUID_1 = ESP_GATT_UUID_DEVICE_INFO_SVC;
static const uint16_t GATTS_CHAR_UUID_A = ESP_GATT_UUID_SW_VERSION_STR;

// 特征值
static const uint8_t CharDefault1Value[] = {0x11, 0x22, 0x33, 0x44}; // 特征的初始值(可选)
static const uint8_t CharDefault2Value[] = "Hello LH-ESP32";         // 特征的初始值(可选)
static const uint8_t CharDefaultCfg[2] = {0x00, 0x00};               // 特征的描述符初始值

static const uint8_t SW_Ver[] = {"000.000.001"};

// 储存服务的Att属性句柄
uint16_t services_handle_table[HRS_IDX_NB];
uint16_t services_1_handle_table[HRS_IDX_NB1];

/* 数据库 -用于将属性添加到数据库中*/
static const esp_gatts_attr_db_t gatt_db[HRS_IDX_NB] =
    {
        /*
        [索引]=
        {
            {属性控制类型}, {UUID长度, *UUID值, 属性权限, 属性值最大长度, 当前属性值长度, *当前属性的初始值}
        }
        */
        // 服务声明
        [IDX_SVC] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_DEFA), (uint8_t *)&GATTS_SERVICE_UUID_DEFA}},

        /*特征声明*/
        [IDX_CHAR_A] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /*特征值*/
        [IDX_CHAR_VAL_A] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_DEFA_A, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(CharDefault1Value), (uint8_t *)CharDefault1Value}},
        /*客户端特征配置描述符*/
        [IDX_CHAR_CFG_A] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(CharDefaultCfg), sizeof(CharDefaultCfg), (uint8_t *)CharDefaultCfg}},

        /*特征声明*/
        [IDX_CHAR_B] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},
        /*特征值*/
        [IDX_CHAR_VAL_B] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_DEFA_B, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(CharDefault1Value), (uint8_t *)CharDefault1Value}},

        /*特征声明*/
        [IDX_CHAR_C] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read_write_notify}},
        /*特征值*/
        [IDX_CHAR_VAL_C] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_DEFA_C, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(CharDefault1Value), (uint8_t *)CharDefault1Value}},
        /*客户端特征配置描述符*/
        [IDX_CHAR_CFG_C] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_client_config_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(CharDefaultCfg), sizeof(CharDefaultCfg), (uint8_t *)CharDefaultCfg}},

        /*特征声明*/
        [IDX_CHAR_D] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_write}},
        /*特征值*/
        [IDX_CHAR_VAL_D] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_DEFA_D, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, GATTS_CHAR_VAL_LEN_MAX, sizeof(CharDefault2Value), (uint8_t *)CharDefault2Value}},
};

/* 数据库 -用于将属性添加到数据库中*/
static const esp_gatts_attr_db_t gatt_db1[HRS_IDX_NB1] =
    {
        /*
        [索引]=
        {
            {属性控制类型}, {UUID长度, *UUID值, 属性权限, 属性值最大长度, 当前属性值长度, *当前属性的初始值}
        }
        */
        // 服务声明
        [IDX_SVC_1] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ, sizeof(uint16_t), sizeof(GATTS_SERVICE_UUID_1), (uint8_t *)&GATTS_SERVICE_UUID_1}},

        /*特征声明*/
        [IDX_CHAR_A1] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ, CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&char_prop_read}},
        /*特征值*/
        [IDX_CHAR_VAL_A1] =
            {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&GATTS_CHAR_UUID_A, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE, sizeof(SW_Ver), sizeof(SW_Ver), (uint8_t *)SW_Ver}},

};


// /**
//  * @brief 通过句柄查找数据库的下标
//  * @param svc_inst_ID: 服务实例ID
//  * @param handle: 属性句柄
//  * @return uint8_t: 在数据库中的下标
//  */
// static uint8_t find_char_and_desr_index(uint8_t svc_inst_ID, uint16_t handle)
// {
//     esp_err_t err = 0xff;

//     if(svc_inst_ID == SVC_INST_ID)
//     {
//         for(int i = 0; i < HRS_IDX_NB ; i++){
//             if(handle == services_handle_table[i]){
//                 return i;
//             }
//         }
//     }
//     else if(svc_inst_ID == SVC1_INST_ID)
//     {
//         for(int i = 0; i < HRS_IDX_NB1 ; i++){
//             if(handle == services_1_handle_table[i]){
//                 return i;
//             }
//         }
//     }

//     return err;
// }

// GAP事件处理
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event)
    {
#ifdef CONFIG_SET_RAW_ADV_DATA
    // 原始广播数据设置完成
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params); // 开始广播扫描
        }
        break;
    // 原始扫描响应数据设置完成
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params); // 开始广播扫描
        }
        break;
#else
    // 广播数据设置完成
    case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~ADV_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params); // 开始广播扫描
        }
        break;
    // 扫描响应数据设置完成
    case ESP_GAP_BLE_SCAN_RSP_DATA_SET_COMPLETE_EVT:
        adv_config_done &= (~SCAN_RSP_CONFIG_FLAG);
        if (adv_config_done == 0)
        {
            esp_ble_gap_start_advertising(&adv_params); // 开始广播扫描
        }
        break;
#endif
    // 广播开始完成
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
        /* 广告开始完成事件,指示广告开始成功或失败*/
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "advertising start failed");
        }
        else
        {
            ESP_LOGI(GATTS_TABLE_TAG, "advertising start successfully");
            // ESP_LOGI(GATTS_TABLE_TAG, "GAP event 广播开启成功");
        }
        break;
    // 停止广播完成
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
        if (param->adv_stop_cmpl.status != ESP_BT_STATUS_SUCCESS)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "Advertising stop failed");
        }
        else
        {
            ESP_LOGI(GATTS_TABLE_TAG, "Stop adv successfully");
            // ESP_LOGI(GATTS_TABLE_TAG, "GAP event 停止广播成功");
        }
        break;
    // 更新连接参数完成
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:
        ESP_LOGI(GATTS_TABLE_TAG, "update connection params status = %d, min_int = %d, max_int = %d,conn_int = %d,latency = %d, timeout = %d",
                 param->update_conn_params.status,
                 param->update_conn_params.min_int,
                 param->update_conn_params.max_int,
                 param->update_conn_params.conn_int,
                 param->update_conn_params.latency,
                 param->update_conn_params.timeout);
        break;
    default:
        break;
    }
}

// 准备写事件环境
void example_prepare_write_event_env(esp_gatt_if_t gatts_if, prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    ESP_LOGI(GATTS_TABLE_TAG, "prepare write, handle = %d, value len = %d", param->write.handle, param->write.len);
    esp_gatt_status_t status = ESP_GATT_OK;

    // 如果偏移量超出
    if (param->write.offset > PREPARE_BUF_MAX_SIZE)
    {
        status = ESP_GATT_INVALID_OFFSET; // 无效偏移量
    }
    else if ((param->write.offset + param->write.len) > PREPARE_BUF_MAX_SIZE)
    {
        status = ESP_GATT_INVALID_ATTR_LEN; // 无效属性长度
    }

    // 创建一个内存作为缓存
    if (status == ESP_GATT_OK && prepare_write_env->prepare_buf == NULL)
    {
        prepare_write_env->prepare_buf = (uint8_t *)malloc(PREPARE_BUF_MAX_SIZE * sizeof(uint8_t));
        prepare_write_env->prepare_len = 0;
        if (prepare_write_env->prepare_buf == NULL)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "%s, Gatt_server prep no mem", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    }

    /* 当写操作是否需要响应时,发送响应 */
    if (param->write.need_rsp)
    {
        esp_gatt_rsp_t *gatt_rsp = (esp_gatt_rsp_t *)malloc(sizeof(esp_gatt_rsp_t));
        if (gatt_rsp != NULL)
        {
            gatt_rsp->attr_value.len = param->write.len;
            gatt_rsp->attr_value.handle = param->write.handle;
            gatt_rsp->attr_value.offset = param->write.offset;
            gatt_rsp->attr_value.auth_req = ESP_GATT_AUTH_REQ_NONE;

            memcpy(gatt_rsp->attr_value.value, param->write.value, param->write.len);
            esp_err_t response_err = esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, status, gatt_rsp); // 发送响应
            if (response_err != ESP_OK)
            {
                ESP_LOGE(GATTS_TABLE_TAG, "Send response error");
            }
            free(gatt_rsp);
        }
        else
        {
            ESP_LOGE(GATTS_TABLE_TAG, "%s, malloc failed", __func__);
            status = ESP_GATT_NO_RESOURCES;
        }
    }
    if (status != ESP_GATT_OK)
    {
        return;
    }

    // 将写入请求中的值复制到准备写入环境中的缓冲区中,并更新准备写入的长度.
    memcpy(prepare_write_env->prepare_buf + param->write.offset,
           param->write.value,
           param->write.len);
    prepare_write_env->prepare_len += param->write.len;
}

// 执行写,打印数据并且释放内存
void example_exec_write_event_env(prepare_type_env_t *prepare_write_env, esp_ble_gatts_cb_param_t *param)
{
    if (param->exec_write.exec_write_flag == ESP_GATT_PREP_WRITE_EXEC && prepare_write_env->prepare_buf)
    {
        esp_log_buffer_hex(GATTS_TABLE_TAG, prepare_write_env->prepare_buf, prepare_write_env->prepare_len);
    }
    else
    {
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATT_PREP_WRITE_CANCEL"); // 写准备被取消
    }

    // 释放写入环境的缓存和长度
    if (prepare_write_env->prepare_buf)
    {
        free(prepare_write_env->prepare_buf);
        prepare_write_env->prepare_buf = NULL;
    }
    prepare_write_env->prepare_len = 0;
}

static uint8_t CeartTabNum = 0;
// GATT 事件处理函数,这个函数通常作为回调在GATT服务实现时被调用,以响应不同的BLE事件.
static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    switch (event)
    {
    case ESP_GATTS_REG_EVT: // 当注册应用程序ID时,该事件触发
    {
        // 设置设备名称
        esp_err_t set_dev_name_ret = esp_ble_gap_set_device_name(GATT_DEVICE_NAME);
        if (set_dev_name_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "set device name failed, error code = %x", set_dev_name_ret);
        }

#ifdef CONFIG_SET_RAW_ADV_DATA
        // 配置原始广播数据
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= ADV_CONFIG_FLAG;

        // 配置原始扫描响应数据
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#else
        // 配置广播数据
        esp_err_t ret = esp_ble_gap_config_adv_data(&adv_data);
        if (ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config adv data failed, error code = %x", ret);
        }
        adv_config_done |= ADV_CONFIG_FLAG;

        // 配置扫描响应数据
        ret = esp_ble_gap_config_adv_data(&scan_rsp_data);
        if (ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "config scan response data failed, error code = %x", ret);
        }
        adv_config_done |= SCAN_RSP_CONFIG_FLAG;
#endif
        // 创建服务属性(指向服务属性选项卡的指针, GATT服务器访问接口, 添加到服务数据库的属性数量, 服务的实例ID)
        esp_err_t create_attr_ret = esp_ble_gatts_create_attr_tab(gatt_db, gatts_if, HRS_IDX_NB, SVC_INST_ID);
        if (create_attr_ret)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", create_attr_ret);
        }
    }
    break;

    case ESP_GATTS_READ_EVT: // 当GATT客户端请求读操作时,该事件触发
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_READ_EVT");
        // ESP_LOGI(GATTS_TABLE_TAG, "GATTS读事件 handle = %d", param->read.handle);
        break;

    case ESP_GATTS_WRITE_EVT: // 当GATT客户端请求写操作时,该事件触发
        /*
         当出现写请求事件时,两个事件 ESP_GATTS_WRITE_EVT 和 ESP_GATTS_EXEC_WRITE_EVT 可能都会成立,具体取决于客户端发送的写请求的类型和数据大小.
         如果客户端发送的是一个完整的写入请求,即一次性发送完所有数据,则只会触发 ESP_GATTS_WRITE_EVT 事件.
         如果客户端发送的是一个准备写入请求,即分多次发送数据,然后再执行写入请求,则会先触发 ESP_GATTS_WRITE_EVT 事件,并在后续的写入请求完成时触发 ESP_GATTS_EXEC_WRITE_EVT 事件.
         因此,在处理写入请求时,您需要考虑这两个事件的可能性,并根据具体情况来进行相应的处理.例如,在处理 ESP_GATTS_WRITE_EVT 事件时,可能需要检查 is_prep 字段的值,以确定是否需要等待后续的执行写入请求.
        */
        if (!param->write.is_prep) // 检查本次WRITE EVT是否为准备写入(写入的数据长度小于GATTS_DEMO_CHAR_VAL_LEN_MAX)
        {
            ESP_LOGI(GATTS_TABLE_TAG, "GATT_WRITE_EVT, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
            // ESP_LOGI(GATTS_TABLE_TAG, "GATTS写事件, handle = %d, value len = %d, value :", param->write.handle, param->write.len);
            esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);

            if (services_handle_table[IDX_CHAR_CFG_C] == param->write.handle && param->write.len == 2)
            {
                uint16_t descr_value = param->write.value[1] << 8 | param->write.value[0];

                if (descr_value == 0x0001)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "notify enable");

                    sw3516_gatts.gatts_if = gatts_if;
                    sw3516_gatts.conn_id = param->write.conn_id;
                    sw3516_gatts.service_handle = services_handle_table[IDX_CHAR_VAL_C];

                    SW3516_Info.Notify_f = true;

                    /* // 通过indicate或者notify发送一段数据内容
                    uint8_t notify_data[15];
                    for (int i = 0; i < sizeof(notify_data); ++i) {
                        notify_data[i] = i % 0xff;
                    }
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, services_handle_table[IDX_CHAR_VAL_A],
                                                sizeof(notify_data), notify_data, false); */
                }
                else if (descr_value == 0x0002)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "indicate enable");

                    // 通过indicate或者notify发送一段数据内容
                    uint8_t indicate_data[15];
                    for (int i = 0; i < sizeof(indicate_data); ++i)
                    {
                        indicate_data[i] = i % 0xff;
                    }

                    // 如果想更改服务器数据库中的值,请调用:
                    // esp_ble_gatts_set_attr_value(services_handle_table[IDX_CHAR_VAL_A], sizeof(indicate_data), indicates_data);
                    esp_ble_gatts_send_indicate(gatts_if, param->write.conn_id, services_handle_table[IDX_CHAR_VAL_A],
                                                sizeof(indicate_data), indicate_data, true);
                }
                else if (descr_value == 0x0000)
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "notify/indicate disable ");
                }
                else
                {
                    ESP_LOGE(GATTS_TABLE_TAG, "unknown descr value");
                    esp_log_buffer_hex(GATTS_TABLE_TAG, param->write.value, param->write.len);
                }
            }

            if (services_handle_table[IDX_CHAR_VAL_D] == param->write.handle && param->write.len == 3)
            {
                char data[3];

                memcpy(data, param->write.value, 3);

                // SW3516_Info.PDO_Vol = (int)data[0];
                // SW3516_Info.PDO_Curr = data[2] << 8 | data[1];
                ComdData[0] = (int)data[0];// 控制端口 0:C1 1:C2
                ComdData[1] = (int)data[1];// 最大电压 0:21V 1:16V 2:12V 
                ComdData[2] = (int)data[2];// 最大功率 0-100W

                ESP_LOGI(GATTS_TABLE_TAG, "get the data is %d %d.", SW3516_Info.PDO_Vol, SW3516_Info.PDO_Curr);
                SW3516_Info.SetPDO_f = true;
            }

            /* 当写操作需要响应时 */
            if (param->write.need_rsp)
            {
                esp_ble_gatts_send_response(gatts_if, param->write.conn_id, param->write.trans_id, ESP_GATT_OK, NULL);
            }
        }
        else
        {
            /*处理准备写入*/
            // 单次写入的数据长度超过GATTS_CHAR_VAL_LEN_MAX时触发(部分调试助手不会触发)
            example_prepare_write_event_env(gatts_if, &prepare_write_env, param);
        }
        break;

    case ESP_GATTS_EXEC_WRITE_EVT: // 当GATT客户端请求执行写操作时,该事件触发
        // gattc 准备写入数据的长度必须小于 GATTS_DEMO_CHAR_VAL_LEN_MAX.
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_EXEC_WRITE_EVT");

        example_exec_write_event_env(&prepare_write_env, param);
        break;

    case ESP_GATTS_MTU_EVT: // 当设置MTU完成时,该事件触发
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_MTU_EVT, MTU %d", param->mtu.mtu);
        break;

    case ESP_GATTS_CONF_EVT: // 当接收到确认时,该事件触发
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONF_EVT, status = %d, attr_handle %d", param->conf.status, param->conf.handle);
        break;

    case ESP_GATTS_START_EVT: // 当开始服务完成时,该事件触发
        ESP_LOGI(GATTS_TABLE_TAG, "SERVICE_START_EVT, status %d, service_handle %d", param->start.status, param->start.service_handle);

        // 创建剩余的服务(指向服务属性选项卡的指针, GATT服务器访问接口, 添加到服务数据库的属性数量, 服务的实例ID)
        esp_err_t err = ESP_OK;
        switch (CeartTabNum)
        {
        case 0:
            err = esp_ble_gatts_create_attr_tab(gatt_db1, gatts_if, HRS_IDX_NB1, SVC1_INST_ID);
            break;
        }

        if (err != ESP_OK)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attr table failed, error code = %x", err);
        }
        else
        {
            CeartTabNum++;
        }

        break;

    case ESP_GATTS_CONNECT_EVT: // 当GATT客户端连接时,该事件触发
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_CONNECT_EVT, conn_id = %d", param->connect.conn_id);
        // ESP_LOGI(GATTS_TABLE_TAG, "GATTS连接完成, conn_id = %d", param->connect.conn_id);
        esp_log_buffer_hex(GATTS_TABLE_TAG, param->connect.remote_bda, 6);

        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t)); // 复制对端蓝牙设备地址

        /*对于iOS系统,BLE连接参数限制请参考苹果官方文档.*/
        conn_params.latency = 0;
        conn_params.max_int = 0x20; // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10; // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;  // timeout = 400*10ms = 4000ms
        // 开始向对端设备发送更新的连接参数.
        esp_ble_gap_update_conn_params(&conn_params);

        // if (SW3516_Handle == NULL)
        // {
        //     //SW3516_InitConfig();
        //     xTaskCreate(&sw3516_task, "sw3516_task", 4096 * 2, NULL, 7, &SW3516_Handle);
        // }
        // else
        // {
        //     vTaskResume(SW3516_Handle);
        // }
        if(MyPower.NotifySta == 0)
        {
            MyPower.NotifySta = 1;
            MyPower.Notify_TIMER = 200;
        }
        break;

    case ESP_GATTS_DISCONNECT_EVT: // GATT客户端断开连接
        ESP_LOGI(GATTS_TABLE_TAG, "ESP_GATTS_DISCONNECT_EVT, reason = 0x%x", param->disconnect.reason);

        //vTaskSuspend(SW3516_Handle);
        SW3516_Info.Notify_f = false;
        MyPower.NotifySta = 0;
        MyPower.Notify_TIMER = 0;

        esp_ble_gap_start_advertising(&adv_params); // 开始广播扫描
        break;

    case ESP_GATTS_CREAT_ATTR_TAB_EVT: // GATT创建属性表完成
    {
        // 检查属性表是否创建成功
        if (param->add_attr_tab.status != ESP_GATT_OK)
        {
            ESP_LOGE(GATTS_TABLE_TAG, "create attribute table failed, error code=0x%x", param->add_attr_tab.status);
        }
        else
        {
            switch (CeartTabNum)
            {
            case 0:
            {
                // 检查属性表最大下标是否正常
                if (param->add_attr_tab.num_handle != HRS_IDX_NB)
                {
                    ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                             param->add_attr_tab.num_handle, HRS_IDX_NB);
                }
                // 成功创建
                else
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d", param->add_attr_tab.num_handle);

                    memcpy(services_handle_table, param->add_attr_tab.handles, sizeof(services_handle_table)); // 记录服务表句柄
                    esp_ble_gatts_start_service(services_handle_table[IDX_SVC]);                               // 开始GATTS服务
                }
            }
            break;

            case 1:
            {
                // 检查属性表最大下标是否正常
                if (param->add_attr_tab.num_handle != HRS_IDX_NB1)
                {
                    ESP_LOGE(GATTS_TABLE_TAG, "create attribute table abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB1(%d)",
                             param->add_attr_tab.num_handle, HRS_IDX_NB1);
                }
                // 成功创建
                else
                {
                    ESP_LOGI(GATTS_TABLE_TAG, "create attribute table successfully, the number handle = %d", param->add_attr_tab.num_handle);

                    memcpy(services_1_handle_table, param->add_attr_tab.handles, sizeof(services_1_handle_table)); // 记录服务表句柄
                    esp_ble_gatts_start_service(services_1_handle_table[IDX_SVC]);                                 // 开始GATTS服务
                }
            }
            break;
            }
        }
    }

    case ESP_GATTS_STOP_EVT:
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    case ESP_GATTS_UNREG_EVT:
    case ESP_GATTS_DELETE_EVT:
    default:
        break;
    }
}

static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /*注册事件,存储每个配置文件的 gatts_if*/
    if (event == ESP_GATTS_REG_EVT)
    {
        if (param->reg.status == ESP_GATT_OK)
        {
            heart_rate_profile_tab[PROFILE_APP0_IDX].gatts_if = gatts_if;
        }
        else
        {
            ESP_LOGE(GATTS_TABLE_TAG, "reg app failed, app_id %04x, status %d",
                     param->reg.app_id,
                     param->reg.status);
            return;
        }
    }
    do
    {
        // 遍历注册APP结构记录表,其实此代码就一个APP应用
        int idx;
        for (idx = 0; idx < PROFILE_APP_NUM; idx++)
        {
            /*ESP_GATT_IF_NONE,不指定某个gatt_if,需要调用每个profile的cb函数*/
            // 判断是已被注册,或者不指定某个gatt_if
            if (gatts_if == ESP_GATT_IF_NONE || gatts_if == heart_rate_profile_tab[idx].gatts_if)
            {
                // 执行注册APP的回调函数
                if (heart_rate_profile_tab[idx].gatts_cb)
                {
                    heart_rate_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

#ifdef SOURCE_IP2730
/// @brief 诱骗电压
/// @param NULL 
void Set_Vin(void)
{

    IP2730_Init();
    vTaskDelay(pdMS_TO_TICKS(500));
    
    //IP2730_Read_PDO_Current();
}
#endif

void app_main(void)
{
    esp_err_t ret;
    
    // 初始化NVS(非易失性存储)闪存.ESP32使用NVS来存储重要的配置信息.
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // 释放不使用的经典蓝牙(BR/EDR)模式所占用的内存,因为这里只使用BLE模式.
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    // 初始化蓝牙控制器
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    // 启用蓝牙控制器的BLE模式
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // 初始化Bluedroid堆栈的默认配置
    esp_bluedroid_config_t bluedroid_cfg = BT_BLUEDROID_INIT_CONFIG_DEFAULT();
    ret = esp_bluedroid_init_with_cfg(&bluedroid_cfg);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    // 启用Bluedroid堆栈
    ret = esp_bluedroid_enable();
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    // 注册GATT回调
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    // 注册GAP应用层的回调
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gap register error, error code = %x", ret);
        return;
    }

    // 使用指定的应用程序ID注册一个GATT应用.此操作会使得应用获得一个GATT接口,后续所有GATT相关的操作都将使用这个接口
    ret = esp_ble_gatts_app_register(ESP_APP0_ID);
    if (ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "gatts app register error, error code = %x", ret);
        return;
    }

    // 设置BLE通信的最大传输单元(MTU)大小为500字节.这决定了一次性能通过BLE传输的最大数据量,增大MTU可以提高数据传输效率.
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(SERVER_MTU_LEN_MAX);
    if (local_mtu_ret)
    {
        ESP_LOGE(GATTS_TABLE_TAG, "set local  MTU failed, error code = %s", esp_err_to_name(local_mtu_ret));
    }

#ifdef SOURCE_IP2730
    // 初始化sink模块的IO
    IP2730_IO_Init();
#endif
    
    // uart_init_config(UART_NUM_0, 115200, UART0_TX_PIN, UART0_RX_PIN);
    // uart_init_config(UART_NUM_1, 115200, UART1_TX_PIN, UART1_RX_PIN);
    // xTaskCreate(uart0_rx_task, "uart0_rx_task", 1024 * 2, NULL, 24, NULL);
    // xTaskCreate(uart1_rx_task, "uart1_rx_task", 1024 * 2, NULL, 24, NULL);

    // uart_write_bytes(UART_NUM_0, "uart0 test OK \r\n", strlen("uart0 test OK \r\n"));
    // uart_write_bytes(UART_NUM_1, "uart1 test OK \r\n", strlen("uart1 test OK \r\n"));

    lcd_spi_panel_Init();
    lcd_lvgl_Init();    
    gui_init();

    // gpio_reset_pin(LEDG_PIN);
    // gpio_set_direction(LEDG_PIN, GPIO_MODE_OUTPUT);

    key_init();

    user_shell_init();
    
#ifdef SOURCE_IP2730
    // 初始化HU1101A模块并申请最大功率
    // IP2730_Init();
    Set_Vin();
#endif
    
    xTaskCreate(&PowerAllocation_task, "PowerAllocation_task", 1024 * 8, NULL, 7, NULL);    
}

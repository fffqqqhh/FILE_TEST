/*
 * SPDX-FileCopyrightText: 2021 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#ifndef __MAIN_H__
#define __MAIN_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "esp_gatts_api.h"





// // #define SOURCE_HUSB238A// 使用C口接PD充电器供电
// #define SOURCE_IP2730// 使用C口接PD充电器供电

typedef struct
{
    esp_gatt_if_t gatts_if;                                 // GATT服务器操作接口
    uint16_t conn_id;                                       // 连接ID
    uint16_t service_handle;                                // 服务句柄
}gatts_profile_t;



/* Attributes State Machine */
#define SVC_INST_ID                     0       // 服务的实例ID
enum
{
    IDX_SVC,

    IDX_CHAR_A,
    IDX_CHAR_VAL_A,
    IDX_CHAR_CFG_A,

    IDX_CHAR_B,
    IDX_CHAR_VAL_B,

    IDX_CHAR_C,
    IDX_CHAR_VAL_C,
    IDX_CHAR_CFG_C,

    IDX_CHAR_D,
    IDX_CHAR_VAL_D,

    HRS_IDX_NB,
};


#define SVC1_INST_ID                    1       // 服务的实例ID
enum
{
    IDX_SVC_1,

    IDX_CHAR_A1,
    IDX_CHAR_VAL_A1,

    HRS_IDX_NB1,
};


#endif

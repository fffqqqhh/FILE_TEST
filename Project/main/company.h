#ifndef __COMPANY_H__
#define __COMPANY_H__
// 后续根据协议拿到的VID来匹配品牌去做显示
#include "main.h"

struct Company {
    int id;
    char name[50];
};

struct Company companies[] = {
    {1452, "Apple"},    // Apple
    {8983, "HUAWEI"},   // Huawei Device Co., Ltd.
    {4817, "HUAWEI"},   // Huawei Technologies Co., Ltd.
    {10007, "Xiaomi"},  // Xiaomi Communications Co., Ltd.
    {8921, "OPPO"},     // OPPO Mobile
    {11669, "VIVO"},    // Vivo Mobile Communication Co., Ltd.
    {7519, "VIVO"},     // ViVOtech, Inc.
    {6610, "ZTE"},      // ZTE Corporation
    {6127, "Lenovo"},   // Lenovo
    {8198, "Lenovo"},   // Lenovo Mobile Communication Technology Ltd.
    {8107, "Samsung"},  // SAMSUNG DIGITAL IMAGING CO., LTD.
    {1373, "Samsung"},  // Samsung Electro-Mechanics Co.
    {1402, "Samsung"},  // Samsung Electronics America
    {1256, "Samsung"},  // Samsung Electronics Co., Ltd.
    {6102, "Samsung"},  // Samsung Electronics Research Institute
    {1049, "Samsung"},  // Samsung Info. Systems America Inc.
    {1076, "Samsung"},  // Samsung Info. Systems America Inc.(2)
    {10034, "Samsung"}, // Samsung Medison Co., Ltd.
    {3788, "Samsung"},  // Samsung SDS
    {1170, "Samsung"}   // Samsung Semiconductor, Inc.
};

int num_companies = sizeof(companies) / sizeof(companies[0]);


#endif
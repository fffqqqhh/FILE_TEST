#ifndef __USER_IP2730_H__
#define __USER_IP2730_H__

#include "main.h"

#define IP2730_IIC_CH 1 // IIC_CH2

#define IP2730_ADDR 0x08//0x08

#define PPS1 0
#define PPS2 1
#define PPS3 2

#define SRC_PDO_5V 1
#define SRC_PDO_9V 2
#define SRC_PDO_12V 3
#define SRC_PDO_15V 4
#define SRC_PDO_20V 5
#define SRC_PDO_PPS1 6
#define SRC_PDO_PPS2 7
#define SRC_PDO_PPS3 8
#define SRC_PDO_AVS 9
#define QC2_5V 10
#define QC2_9V 11
#define QC2_12V 12
#define SRC_PDO_28V 13
#define SRC_PDO_36V 14
#define SRC_PDO_48V 15
#define SRC_EPR_AVS 16


typedef struct
{
    unsigned char PDO_Current[11];
    unsigned char PPS_Cap_Current[3];
    unsigned char PPS_Cap_VolMin[3];
    unsigned char PPS_Cap_VolMax[3];
    unsigned char Reg0x18, Reg0x19;
    unsigned int PD_FIX_VMAX,PD_PPS_VMAX,PD_FIX_PMAX,PD_PPS_PMAX,PD_PMAX;

    unsigned char UserPPSx; // 当前PPS档
    unsigned int UserPPS_MaxCurrent;

} IP2730_TypeDef;

extern IP2730_TypeDef IP2730;


void IP2730_IO_Init(void);
void IP2730_Init(void);
void IP2730_Read_PDO_Current(void);
esp_err_t IP2730_iic_master_init(void);
void IP2730_FixPDO_Request(unsigned int Voltage);
void IP2730_PPS_Request(unsigned char PPS_x, unsigned int RequestVol);
void IP2730_PPS_RequestMAX(unsigned int RequestVol);
unsigned char IP2730_Is_PPS_Ready(void);
unsigned int UserRequestVoil_PPS(unsigned int RequestVol);
unsigned int GetUserPPS_MaxCurrent(void);

#endif
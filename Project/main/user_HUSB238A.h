#ifndef __USER_HUSB238A_H__
#define __USER_HUSB238A_H__

#include "main.h"

#define HUSB238A_IIC_CH 1 // IIC_CH2

#define HUSB238A_ADDR 0x42

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

} HUSB238A_TypeDef;

extern HUSB238A_TypeDef HUSB238A;


void HUSB238A_IO_Init(void);
void HUSB238A_Init(void);
void HUSB238A_Read_PDO_Current(void);
esp_err_t husb238a_iic_master_init(void);
void HUSB238A_FixPDO_Request(unsigned int Voltage);
void HUSB238A_PPS_Request(unsigned char PPS_x, unsigned int RequestVol);
void HUSB238A_PPS_RequestMAX(unsigned int RequestVol);
unsigned char HUSB238A_Is_PPS_Ready(void);
unsigned int UserRequestVoil_PPS(unsigned int RequestVol);
unsigned int GetUserPPS_MaxCurrent(void);

#endif
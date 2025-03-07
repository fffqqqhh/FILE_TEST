#ifndef _SW3536_H_
#define _SW3536_H_


//#include "NuMicro.h"

#include "Sim_I2C.h"

#include "global.h"


//extern SimI2Cx SW3536_I2C;
extern SimI2Cx SW3536_U1;


// i2c时钟
#define	SW3536_SCL_PORT		        B
#define	SW3536_SCL_PIN		        5
// i2c数据
#define	SW3536_SDA_PORT		        B
#define	SW3536_SDA_PIN		        4


#define	SW3536_SCL_P_x		        STRCAT2(P, SW3536_SCL_PORT)                     // Px
#define	SW3536_SDA_P_x		        STRCAT2(P, SW3536_SDA_PORT)                     // Px

#define	SW3536_SCL_BIT_n		    STRCAT2(BIT, SW3536_SCL_PIN)                    // BITn
#define	SW3536_SDA_BIT_n		    STRCAT2(BIT, SW3536_SDA_PIN)                    // BITn

#define	SW3536_SCL_GPIO		        STRCAT3(P, SW3536_SCL_PORT, SW3536_SCL_PIN)     // Pxn
#define	SW3536_SDA_GPIO		        STRCAT3(P, SW3536_SDA_PORT, SW3536_SDA_PIN)     // Pxn

// 切换SCL 方向
#define SW3536_SCL_INPUT()          GPIO_SetMode(SW3536_SCL_P_x, SW3536_SCL_BIT_n, GPIO_MODE_QUASI)
#define SW3536_SCL_OUTPUT()         GPIO_SetMode(SW3536_SCL_P_x, SW3536_SCL_BIT_n, GPIO_MODE_OUTPUT)

// 切换SDA 方向
#define SW3536_SDA_INPUT()          GPIO_SetMode(SW3536_SDA_P_x, SW3536_SDA_BIT_n, GPIO_MODE_QUASI)
#define SW3536_SDA_OUTPUT()			GPIO_SetMode(SW3536_SDA_P_x, SW3536_SDA_BIT_n, GPIO_MODE_OUTPUT)

// SCK
#define SW3536_SCL_L()             	SW3536_SCL_GPIO = 0
#define SW3536_SCL_H()             	SW3536_SCL_GPIO = 1

// SDA
#define SW3536_SDA_L()             	SW3536_SDA_GPIO = 0
#define SW3536_SDA_H()             	SW3536_SDA_GPIO = 1

// 读SCL口线状态
#define SW3536_SCL_READ()          	SW3536_SCL_GPIO
// 读SDA口线状态
#define SW3536_SDA_READ()          	SW3536_SDA_GPIO


// #define		SW3536_I2C_WRITE_BYTE(REG_ADDR, DATA)		vSimI2C_WriteByte(&SW3536_I2C, SW3536_I2C_ADD, REG_ADDR, DATA)
// #define		SW3536_I2C_READ_BYTE(REG_ADDR)				u8SimI2C_ReadByte(&SW3536_I2C, SW3536_I2C_ADD, REG_ADDR)


#define PD_PPS0         0
#define PD_PPS1         1
#define PD_PPS2         2
#define PD_PPS3         3


typedef enum
{
    SW3536_ADC_IOUT1 = 1,
    SW3536_ADC_IOUT2,
    SW3536_ADC_VOUT = 5,
    SW3536_ADC_VIN,
    SW3536_ADC_VNTC,
    SW3536_ADC_VOUT_2 = 11,   // 单位转换后输出电压数据，单位：1mV/bit
} SW3536_AdcChannelDef;


typedef enum
{
    SW3536_PPS3_EN = 1 << 7,
    SW3536_PPS2_EN = 1 << 6,
    SW3536_PPS1_EN = 1 << 5,
    SW3536_PPS0_EN = 1 << 4,

    SW3536_PDO_20V_EN = 1 << 3,
    SW3536_PDO_15V_EN = 1 << 2,
    SW3536_PDO_12V_EN = 1 << 1,
    SW3536_PDO_9V_EN = 1 << 0,
} SW3536_PDO_CfgTypeDef;


// api 
void SW3536U1_IIC_Init(void);

void SW3536_InitConfig(void);

uint16_t SW3536_ADC_Read12bit(SW3536_AdcChannelDef AdcType);
uint32_t SW3536_Get_RNTC(void);
uint16_t SW3536_Get_VIN(void);
uint16_t SW3536_Get_VOUT(void);
uint16_t SW3536_Get_IOUT1(void);
uint16_t SW3536_Get_IOUT2(void);
uint32_t SW3536_Get_OUTPow(uint16_t u, uint16_t i);

void SW3536_PrintState(void);
void SW3536_ForceSetPort1Curr(uint16_t MaxIout);
void SW3536_ForceSetPort2Curr(uint16_t MaxIout);

void SW3536_RegW_Enable(uint16_t reg_add); // SW3536 写寄存器 使能
void SW3536_SrcChange(void);

void SW3536_SetPPS(uint8_t PPS_x, uint16_t MaxVout, uint16_t MaxIout);
void SW3536_SetPDO(uint8_t MaxVol, uint16_t MaxIout);

void SW3536_PDO_EnConfig(uint8_t vol, uint8_t state);
void SW3536_PDO_OnlyVol_EnConfig(uint8_t vol);
void SW3536_PPS0_EnConfig(uint8_t state);
void SW3536_PPS1_EnConfig(uint8_t state);
void SW3536_PPS2_EnConfig(uint8_t state);
void SW3536_PPS3_EnConfig(uint8_t state);
void SW3536_PD_EnConfig(uint8_t state);
void SW3536_QC2_EnConfig(uint8_t state);
void SW3536_QC3_EnConfig(uint8_t state);
void SW3536_QC3P_EnConfig(uint8_t state);
void SW3536_QC4P_EnConfig(uint8_t state);
void SW3536_TFCP_EnConfig(uint8_t state);
void SW3536_FCP_EnConfig(uint8_t state);
void SW3536_SFCP_EnConfig(uint8_t state);
void SW3536_LvSCP_EnConfig(uint8_t state);
void SW3536_HvSCP_EnConfig(uint8_t state);
void SW3536_PE_EnConfig(uint8_t state);
void SW3536_AFC_EnConfig(uint8_t state);
void SW3536_Apple2V4A_EnConfig(uint8_t state);
void SW3536_Sam1V2_EnConfig(uint8_t state);
void SW3536_AllOff(void);
void SW3536_AllOn(void);

// void SW3536_BuckEn(uint8_t state);
// void SW3536_ResetPower(void);

void SW3536_I2C_WRITE_BYTE(uint16_t reg, uint8_t dat);
uint8_t SW3536_I2C_READ_BYTE(uint16_t reg);


#endif

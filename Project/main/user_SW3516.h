#ifndef _SW3516_H_
#define _SW3516_H_


#include "main.h"

//#define SW3516_I2C_SIM

#ifdef SW3516_I2C_SIM
//extern SimI2Cx SW3516_I2C;
extern SimI2Cx SW3516_U1;

// i2c时钟
#define	SW3516_SCL_PORT		        B
#define	SW3516_SCL_PIN		        7
// i2c数据
#define	SW3516_SDA_PORT		        B
#define	SW3516_SDA_PIN		        6


#define	SW3516_SCL_P_x		        STRCAT2(P, SW3516_SCL_PORT)                     // Px
#define	SW3516_SDA_P_x		        STRCAT2(P, SW3516_SDA_PORT)                     // Px

#define	SW3516_SCL_BIT_n		    STRCAT2(BIT, SW3516_SCL_PIN)                    // BITn
#define	SW3516_SDA_BIT_n		    STRCAT2(BIT, SW3516_SDA_PIN)                    // BITn

#define	SW3516_SCL_GPIO		        STRCAT3(P, SW3516_SCL_PORT, SW3516_SCL_PIN)     // Pxn
#define	SW3516_SDA_GPIO		        STRCAT3(P, SW3516_SDA_PORT, SW3516_SDA_PIN)     // Pxn

// 切换SCL 方向
#define SW3516_SCL_INPUT()          GPIO_SetMode(SW3516_SCL_P_x, SW3516_SCL_BIT_n, GPIO_MODE_QUASI)
#define SW3516_SCL_OUTPUT()         GPIO_SetMode(SW3516_SCL_P_x, SW3516_SCL_BIT_n, GPIO_MODE_OUTPUT)

// 切换SDA 方向
#define SW3516_SDA_INPUT()          GPIO_SetMode(SW3516_SDA_P_x, SW3516_SDA_BIT_n, GPIO_MODE_QUASI)
#define SW3516_SDA_OUTPUT()			GPIO_SetMode(SW3516_SDA_P_x, SW3516_SDA_BIT_n, GPIO_MODE_OUTPUT)

// SCK
#define SW3516_SCL_L()             	SW3516_SCL_GPIO = 0
#define SW3516_SCL_H()             	SW3516_SCL_GPIO = 1

// SDA
#define SW3516_SDA_L()             	SW3516_SDA_GPIO = 0
#define SW3516_SDA_H()             	SW3516_SDA_GPIO = 1

// 读SCL口线状态
#define SW3516_SCL_READ()          	SW3516_SCL_GPIO
// 读SDA口线状态
#define SW3516_SDA_READ()          	SW3516_SDA_GPIO


// #define		SW3516_I2C_WRITE_BYTE(REG_ADDR, DATA)		vSimI2C_WriteByte(&SW3516_I2C, SW3516_I2C_ADD, REG_ADDR, DATA)
// #define		SW3516_I2C_READ_BYTE(REG_ADDR)				u8SimI2C_ReadByte(&SW3516_I2C, SW3516_I2C_ADD, REG_ADDR)


#else
// #define IIC_MASTER_SCL_PIN              36
// #define IIC_MASTER_SDA_PIN              35

// #define IIC_MASTER_NUM                  0
#define IIC_MASTER_FREQ_HZ              100000
#define IIC_MASTER_TX_BUF_DISABLE       0
#define IIC_MASTER_RX_BUF_DISABLE       0
#define IIC_MASTER_TIMEOUT_MS           1000

#endif

#define SLAVE_SW35_ADDR                 0x3C

#define DEVICE_ID_ESP32S3               0xCE00


#define PD_PPS0         0
#define PD_PPS1         1


typedef struct 
{
    bool SetPDO_f;
    uint16_t PDO_Vol;
    uint16_t PDO_Curr;

    bool Notify_f;
    uint8_t ChargeData[20];
}SW35xx_Info_t;


typedef enum
{
    SW3516_ADC_VIN = 1,
    SW3516_ADC_VOUT,
    SW3516_ADC_IOUT1,
    SW3516_ADC_IOUT2,
    SW3516_ADC_RNTC = 6,
} SW3516_AdcChannelDef;


typedef enum
{
    Max5V9 = 0,
    Max11V,
    Max16V,
    Max21V,
} SW3516_PPS_MaxVolTypeDef;

typedef enum
{
    SW3516_PPS1_EN = 1 << 7,
    SW3516_PPS0_EN = 1 << 6,

    SW3516_PDO_20V_EN = 1 << 5,
    SW3516_PDO_15V_EN = 1 << 4,
    SW3516_PDO_12V_EN = 1 << 3,
    SW3516_PDO_9V_EN = 1 << 2,
    
    SW3516_PDO_EMARKER_EN = 1 << 1,
} SW3516_PDO_CfgTypeDef;



extern TaskHandle_t SW3516_Handle;

extern SW35xx_Info_t SW3516_Info;
extern gatts_profile_t sw3516_gatts;
extern uint8_t IIC_MASTER_SCL_PIN;
extern uint8_t IIC_MASTER_SDA_PIN;
extern uint8_t IIC_MASTER_NUM;

// api 
uint16_t SW3516_ADC_Read12bit(SW3516_AdcChannelDef AdcType);
uint32_t SW3516_Get_RNTC(void);
uint16_t SW3516_Get_VIN(void);
uint16_t SW3516_Get_VOUT(void);
uint16_t SW3516_Get_IOUT1(void);
uint16_t SW3516_Get_IOUT2(void);
uint32_t SW3516_Get_OUTPow(uint16_t u, uint16_t i);

void SW3516_PrintState(void);

void SW3516_RegW_Enable(void);
void SW3516_SrcChange(void);

void SW3516_SetPPS0(uint8_t MaxVol, uint16_t MaxIout);
void SW3516_SetPPS1(uint8_t MaxVol, uint16_t MaxIout);
void SW3516_SetPDO(uint8_t MaxVol, uint16_t MaxIout);

void SW3516_PDO_EnConfig(uint8_t vol, uint8_t state);
void SW3516_PDO_OnlyVol_EnConfig(uint8_t vol);
void SW3516_PPS0_EnConfig(uint8_t state);
void SW3516_PPS1_EnConfig(uint8_t state);
void SW3516_PD_EnConfig(uint8_t state);
void SW3516_QC3_EnConfig(uint8_t state);
void SW3516_QC_EnConfig(uint8_t state);
void SW3516_FCP_EnConfig(uint8_t state);
void SW3516_LvSCP_EnConfig(uint8_t state);
void SW3516_HvSCP_EnConfig(uint8_t state);
void SW3516_PE_EnConfig(uint8_t state);
void SW3516_AFC_EnConfig(uint8_t state);
void SW3516_Sam1V2_EnConfig(uint8_t state);
void SW3516_AllOff(void);
void SW3516_AllOn(void);

void SW3516_BuckEn(uint8_t state);
void SW3516_ResetPower(void);



esp_err_t iic_master_init(void);
void SW3516_I2C_WRITE_BYTE(uint8_t reg, uint8_t dat);
uint8_t SW3516_I2C_READ_BYTE(uint8_t reg);

void SW3516_InitConfig(void);

void sw3516_task(void *param);


#endif

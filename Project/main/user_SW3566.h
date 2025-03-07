#ifndef _SW3566_H_
#define _SW3566_H_


#include "main.h"

//#define SW3566_I2C_SIM

#ifdef SW3566_I2C_SIM
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


// #define		sw3566_write_byte(REG_ADDR, DATA)		vSimI2C_WriteByte(&SW3516_I2C, SW3516_I2C_ADD, REG_ADDR, DATA)
// #define		sw3566_read_byte(REG_ADDR)				u8SimI2C_ReadByte(&SW3516_I2C, SW3516_I2C_ADD, REG_ADDR)


#else
// #define IIC_MASTER_SCL_PIN              36
// #define IIC_MASTER_SDA_PIN              35

// #define IIC_MASTER_NUM                  0
#define IIC_MASTER_FREQ_HZ              100000
#define IIC_MASTER_TX_BUF_DISABLE       0
#define IIC_MASTER_RX_BUF_DISABLE       0
#define IIC_MASTER_TIMEOUT_MS           1000

#endif



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






void SW3516_InitConfig(void);

void sw3516_task(void *param);



#define SW3566_U1_SDA_PIN       GPIO_NUM_13
#define SW3566_U1_SCL_PIN       GPIO_NUM_12
#define SW3566_U1_IRQ_PIN       GPIO_NUM_14

#define SW3566_U2_SDA_PIN       GPIO_NUM_47
#define SW3566_U2_SCL_PIN       GPIO_NUM_21
#define SW3566_U2_IRQ_PIN       GPIO_NUM_48

#define SW3566_U3_SDA_PIN       GPIO_NUM_35
#define SW3566_U3_SCL_PIN       GPIO_NUM_36
#define SW3566_U3_IRQ_PIN       GPIO_NUM_45

#define SW3566_U4_SDA_PIN       GPIO_NUM_37
#define SW3566_U4_SCL_PIN       GPIO_NUM_38
#define SW3566_U4_IRQ_PIN       GPIO_NUM_39

#define SW3566_U1_ADDR          0x45        // 0x8a
#define SW3566_U2_ADDR          0x45        // 0x8a
#define SW3566_U3_ADDR          0x45        // 0x8a
#define SW3566_U4_ADDR          0x45        // 0x8a


// cpc = Charging protocol chip 充电协议芯片

#define SW3566_I2C_NUM             I2C_NUM_0



typedef enum
{
    PPS_0 = 0,
    PPS_1,
    PPS_2,
    PPS_MAX,
} pps_num_t;


typedef enum
{
    SW_U1,  // default
    SW_U2,
    SW_U3,
    SW_U4, 

    SW_MAX,

    SW_NONE = 0xff,
} chip_obj_t;

extern chip_obj_t sw3566_idx_num;

extern QueueHandle_t sw3566_irq_queue;




extern char sw3566_protocol_str_list[][11];


/// @brief 切换I2C总线上的芯片
esp_err_t sw3566_i2c_switch_chip(chip_obj_t obj);

/// @brief 初始化I2C总线
esp_err_t sw3566_gpio_init(void);


/// @brief 查找I2C总线上的所有设备，并打印有效设备的地址（调试用）
void sw3566_i2c_find_addr(void);


/// @brief 3566写寄存器
void sw3566_write_byte(uint8_t reg, uint8_t dat);
/// @brief 3566读寄存器
uint8_t sw3566_read_byte(uint8_t reg, uint8_t *recv_data);


/// @brief 读取寄存器的第bit位
uint8_t sw3566_read_bit(uint8_t reg_addr, uint8_t bit);
/// @brief 读取寄存器的第h_bit~l_bit位
uint8_t sw3566_read_bits(uint8_t reg_addr, uint8_t h_bit, uint8_t l_bit);

/// @brief 设置寄存器的第bit位
uint8_t sw3566_set_bit(uint8_t reg_addr, uint8_t bit, uint8_t bit_val);
/// @brief 设置寄存器的第h_bit~l_bit位
uint8_t sw3566_set_bits(uint8_t reg_addr, uint8_t h_bit, uint8_t l_bit, uint8_t bit_val);

void sw3566_write_10bit_data(uint8_t reg_h, uint16_t data);
uint16_t sw3566_get_16bit_data(uint8_t reg_h);
/********************************************************************************************/

/// @brief 上电初始化
void sw3566_init(chip_obj_t obj, uint8_t en);

/// @brief irq处理
void sw3566_irq_handler(void);


/// @brief 获取vout
uint16_t sw3566_get_vout(void);
/// @brief 获取vin
uint16_t sw3566_get_vin(void);
/// @brief 获取port 1电流
uint16_t sw3566_get_port1_curr(void);
/// @brief 获取port 2电流
uint16_t sw3566_get_port2_curr(void);

/// @brief 重新广播PDO
void sw3566_update_pdo(void);
/// @brief 硬件复位
void sw3566_hard_reset(void);

/// @brief pps 使能控制
void sw3566_pps_en(pps_num_t num, uint8_t en);
/// @brief PDO 电压使能控制
void sw3566_pdo_vol_en(uint8_t vol, uint8_t state);
/// @brief PDO 电流设置
void sw3566_pdo_set_curr(uint8_t vol, uint16_t mA);


#endif

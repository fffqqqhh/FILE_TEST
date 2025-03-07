#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"

#include "esp_gatts_api.h"


#include "user_SW3516.h"

#include "user_oper_data.h"


#define XXX_TAG     "【SW3516_TABLE】"     



SW35xx_Info_t SW3516_Info;
gatts_profile_t sw3516_gatts={0, 0, 0};
uint8_t IIC_MASTER_SCL_PIN = 36;
uint8_t IIC_MASTER_SDA_PIN = 35;
uint8_t IIC_MASTER_NUM  = 0;


struct PDO_VolSwitch
{
    uint8_t PDO_5V_f:1;
    uint8_t PDO_9V_f:1;
    uint8_t PDO_12V_f:1;
    uint8_t PDO_15V_f:1;
    uint8_t PDO_20V_f:1;
}SW3516_PDO_VolState = {1, 1, 1 ,1, 1};       // 记录PDO各个电压挡位的开关状态，默认都开启


/**
 * @brief 如果要操作寄存器 reg0xA0~CF，需先解锁IIC写操作
 */
void SW3516_RegW_Enable(void)
{
    uint8_t Reg_0x12;

    Reg_0x12 = SW3516_I2C_READ_BYTE(0x12) & 0x1F;

    SW3516_I2C_WRITE_BYTE(0x12, Reg_0x12 | 0x20);
    SW3516_I2C_WRITE_BYTE(0x12, Reg_0x12 | 0x40);
    SW3516_I2C_WRITE_BYTE(0x12, Reg_0x12 | 0x80);
}


/**
 * @brief 输入 Vin 的 ADC 工作使能，只有在使能时，Vin 的数据才能读出
 */
void SW3516_VinADC_Enable(void)
{ 
    SW3516_I2C_WRITE_BYTE(0x13, SW3516_I2C_READ_BYTE(0x13) | 0x02);
}


/**
 * @brief 读取12位ADC
 * @param AdcType: 选择需要读取的内容
 * @return uint16_t: 12bit的AD值
 */
uint16_t SW3516_ADC_Read12bit(SW3516_AdcChannelDef AdcType)
{
    uint16_t SW3516_ADC;

    SW3516_I2C_WRITE_BYTE(0x3A, AdcType);    // 配置ADC通道

    vTaskDelay(10); // 等待数据锁存到Reg0x3B和Reg0x3C

    SW3516_ADC = SW3516_I2C_READ_BYTE(0x3B);
    SW3516_ADC <<= 4;
    SW3516_ADC |= SW3516_I2C_READ_BYTE(0x3C) & 0x0f;

    return SW3516_ADC;
}


/**
 * @brief 读取NTC阻值
 * @return uint32_t: 阻值 单位：1Ω
 */
uint32_t SW3516_Get_RNTC(void) {
    return (SW3516_ADC_Read12bit(SW3516_ADC_RNTC) * 5 - 2000);
}

/**
 * @brief 读取Vin电压
 * @return uint16_t: 电压值 单位1mV
 */
uint16_t SW3516_Get_VIN(void)
{
	// 输入 Vin 的 ADC 工作使能，只有在使能时，Vin 的数据才能读出
    SW3516_VinADC_Enable();        

    return (SW3516_ADC_Read12bit(SW3516_ADC_VIN) * 10);
}

/**
 * @brief 读取Vout电压
 * @return uint16_t: 电压值 单位1mV
 */
uint16_t SW3516_Get_VOUT(void) {
    return (SW3516_ADC_Read12bit(SW3516_ADC_VOUT) * 6);
}

/**
 * @brief 读取端口1输出电流
 * @return uint16_t: 电流值 单位1mA
 */
uint16_t SW3516_Get_IOUT1(void) {
    return (SW3516_ADC_Read12bit(SW3516_ADC_IOUT1) * 2.5);
}

/**
 * @brief 读取端口2输出电流
 * @return uint16_t: 电流值 单位1mA
 */
uint16_t SW3516_Get_IOUT2(void) {
    return (SW3516_ADC_Read12bit(SW3516_ADC_IOUT2) * 2.5);
}

/**
 * @brief 计算输出功率
 * @param u: 输出电压 单位：1mV
 * @param i: 输出电流 单位：1mA
 * @return uint32_t: 输出功率  单位：1mW
 */
uint32_t SW3516_Get_OUTPow(uint16_t u, uint16_t i)												// 单位：1mW
{
    return (u * i) / 1000;
}


/**
 * @brief 打印芯片相关状态
 */
// void SW3516_PrintState(void)
// {
//     uint16_t u, i1, i2;
//     uint32_t p;
    
//     u = SW3516_Get_VOUT();
//     i1 = SW3516_Get_IOUT1();
//     i2 = SW3516_Get_IOUT2();

//     printf("\n\n");
// #if	 				0				// m为单位显示
// 	printf("VIN:%5d mV  ", SW3516_Get_VIN());		// 输入电压
//     printf("VOUT:%5d mV  ", u);						// 输出电压
    
//     printf("R_NTC:%5d R", SW3516_Get_RNTC());		// NTC阻值
//     printf("\n");

//     p = SW3516_Get_OUTPow(u, i1);					// 端口1电流和功率
//     printf("Iout1:%5d mA  ", i1);      				
//     printf("IPow1:%5d mW\n", p);
    
//     p = SW3516_Get_OUTPow(u, i2);					// 端口2电流和功率
//     printf("Iout2:%5d mA  ", i2);					
//     printf("IPow2:%5d mW", p); 
// #else
//     printf("VIN:%6.3f V  ", (float)SW3516_Get_VIN()/1000);		// 输入电压
//     printf("VOUT:%6.3f V  ", (float)u/1000);					// 输出电压
    
//     printf("R_NTC:%5d R", SW3516_Get_RNTC());					// NTC阻值
//     printf("\n");

//     p = SW3516_Get_OUTPow(u, i1);								// 端口1电流和功率
//     printf("Iout1:%6.3f A  ", (float)i1/1000);      				
//     printf("IPow1:%6.3f W\n", (float)p/1000);
    
//     p = SW3516_Get_OUTPow(u, i2);								// 端口2电流和功率
//     printf("Iout2:%6.3f A  ", (float)i2/1000);					
//     printf("IPow2:%6.3f W", (float)p/1000);
// #endif
//     printf("\n");


//     uint8_t data;

//     data = SW3516_I2C_READ_BYTE(0x06);
//     printf("[0x06]: ");
//     if(uGetBit(data, 7) == 1)						// 是否快充
//         printf("[quick charge] ");
//     else
//         printf("[!quick charge] ");
//     if(uGetBit(data, 6) == 1)						// 是否快充电压
//         printf("[quick charge vol] ");
//     else
//         printf("[!quick charge vol] ");
//     if(uGetBits(data, 5, 4) == 1)					// PD版本
//         printf("[PD2.0] ");
//     else
//     if(uGetBits(data, 5, 4) == 2)
//         printf("[PD3.0] ");
//     switch (uGetBits(data, 3, 0))					// 当前正在使用的协议
//     {
//         case 1: printf("[QC2.0]");  break;
//         case 2: printf("[QC3.0]");  break;
//         case 3: printf("[FCP]");  break;
//         case 4: printf("[SCP]");  break;
//         case 5: printf("[PD FIX]");  break;
//         case 6: printf("[PD PPS]");  break;
//         case 7: printf("[PE1.1]");  break;
//         case 8: printf("[PE2.0]");  break;
//         case 10: printf("[SFCP]");  break;
//         case 11: printf("[AFC]");  break;
//         default: printf("[other]");break;
//     }
//     printf("\n");

//     data = SW3516_I2C_READ_BYTE(0x07);
//     printf("[0x07]: ");
//     if(uGetBit(data, 1) == 1)						// 端口2状态
//         printf("[COM2 ON] ");
//     else
//         printf("[COM2 OFF] ");
//     if(uGetBit(data, 0) == 1)						// 端口1状态
//         printf("[COM1 ON] ");
//     else
//         printf("[COM1 OFF] ");
//     printf("\n");

//     data = SW3516_I2C_READ_BYTE(0x08);				// 更详细的端口状态，详见寄存器手册0x80
//     printf("[0x08]: [7:0] = %d", uGetBits(data, 7, 4));
//     printf("\n");

// }


/**
 * @brief 发送 source capability 命令
 */
void SW3516_SrcChange(void)
{ 
    SW3516_I2C_WRITE_BYTE(0x73, SW3516_I2C_READ_BYTE(0x73) | 0x80);  
}


/**
 * @brief PPS最大电压电流设置
 * @param PPS_x ：PPS选择,PD_PPS0和PD_PPS1
 * @param MaxVout：PPS最大电压设置，详见SW3516_PPS_MaxVolTypeDef
 * @param MaxIout：PPS最大电流设置，[6-0]50mA/bit
 */
void SW3516_PPS_SetMaxVol(uint8_t PPS_x, uint8_t MaxVout, uint16_t MaxIout)
{
    uint8_t Temp;

    switch (MaxVout)
    {
        case 59:
            MaxVout = Max5V9;
            break;
        case 11:
            MaxVout = Max11V;
            break;
        case 16:
            MaxVout = Max16V;
            break;
        case 21:
            MaxVout = Max21V;
            break;
    }

    Temp = SW3516_I2C_READ_BYTE(0xBE);

    SW3516_RegW_Enable();

    if (PPS_x == PD_PPS0)
    {
        Temp = (Temp & 0xF0) | MaxVout;

        SW3516_I2C_WRITE_BYTE(0xB5, MaxIout);   // PPS电流设置
    }
    else if (PPS_x == PD_PPS1)
    {
        Temp = (Temp & 0xF) | (MaxVout << 4);

        SW3516_I2C_WRITE_BYTE(0xB6, MaxIout);   // PPS电流设置
    }
    else    // 同时设置PPS0以及PPS1
    {
        Temp = (MaxVout << 4) | MaxVout;

        SW3516_I2C_WRITE_BYTE(0xB5, MaxIout);   // PPS电流设置
    
        SW3516_RegW_Enable();
        SW3516_I2C_WRITE_BYTE(0xB6, MaxIout);   // PPS电流设置
    }

    SW3516_RegW_Enable();
    SW3516_I2C_WRITE_BYTE(0xBE, Temp);          // PPS电压设置

    // 发送 source capability 命令
    SW3516_SrcChange();
}


/**
 * @brief PPS0 电流设置
 * @param MaxVol: PPS最大电压范围，输入范围：59（5.9V）、11、16、21V
 * @param MaxIout: PPS最大电流范围，单位1mA，步进50mA
 */
void SW3516_SetPPS0(uint8_t MaxVol, uint16_t MaxIout)
{
    //SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    //SW3516_PPS_SetMaxVol(PD_PPS0, MaxVol, MaxIout / 50);
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN|SW3516_PPS1_EN) ));
    SW3516_PPS_SetMaxVol(PD_PPS0, MaxVol, MaxIout / 50);
    // SW3516_SrcChange();
}


/**
 * @brief PPS1 电流设置
 * @param MaxVol: PPS最大电压范围，输入范围：59（5.9V）、11、16、21V
 * @param MaxIout: PPS最大电流范围，单位1mA，步进50mA
 */
void SW3516_SetPPS1(uint8_t MaxVol, uint16_t MaxIout)
{
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    SW3516_PPS_SetMaxVol(PD_PPS1, MaxVol, MaxIout / 50);
}


/**
 * @brief PDO 电流电压设置
 * @param MaxVol: 选择设置电流的PDO电压档位，输入范围：5、9、12、15、20V
 * @param MaxIout: 设置vol对应的PDO电流，单位1mA，步进50mA
 */
void SW3516_SetPDO(uint8_t MaxVol, uint16_t MaxIout)
{
    SW3516_RegW_Enable();

    MaxIout /= 50;

    switch (MaxVol)
    {
        case 5:
            SW3516_I2C_WRITE_BYTE(0xB0, MaxIout);   // 5V i*50mA
            break;
        case 9:
            SW3516_I2C_WRITE_BYTE(0xB1, MaxIout);   // 9V i*50mA
            break;
        case 12:
            SW3516_I2C_WRITE_BYTE(0xB2, MaxIout);   // 12V i*50mA
            break;
        case 15:
            SW3516_I2C_WRITE_BYTE(0xB3, MaxIout);   // 15V i*50mA
            break;
        case 20:
            SW3516_I2C_WRITE_BYTE(0xB4, MaxIout);   // 20V i*50mA
            break;
        default:
            break;
    }
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    // 发送 source capability 命令
    //SW3516_I2C_WRITE_BYTE(0x73, SW3516_I2C_READ_BYTE(0x73) | (1 << 7));
}



/**
 * @brief PDO 电压使能控制
 * @param vol: 选择需要控制的PDO电压档位，输入范围：9、12、15、20V
 * @param state: 0：关闭当前电压档位
 *               1：打开当前电压档位
 */
void SW3516_PDO_EnConfig(uint8_t vol, uint8_t state)
{ 
    uint8_t RegAdd = 0xb7;
    uint8_t BitMask = 0;

    switch (vol)
    {
        case 9:
            BitMask = SW3516_PDO_9V_EN;
            SW3516_PDO_VolState.PDO_9V_f = (state != 0)? 1 : 0;
            break;
        case 12:
            BitMask = SW3516_PDO_12V_EN;
            SW3516_PDO_VolState.PDO_12V_f = (state != 0)? 1 : 0;
            break;
        case 15:
            BitMask = SW3516_PDO_15V_EN;
            SW3516_PDO_VolState.PDO_15V_f = (state != 0)? 1 : 0;
            break;
        case 20:
            BitMask = SW3516_PDO_20V_EN;
            SW3516_PDO_VolState.PDO_20V_f = (state != 0)? 1 : 0;
            break;
    }

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    SW3516_SrcChange();
}


/**
 * @brief PDO 电压选择，近保留选择的电压挡位
 * @param vol: 选择需要保留的PDO电压档位，输入范围：9、12、15、20V，其它：全开
 */
void SW3516_PDO_OnlyVol_EnConfig(uint8_t vol)
{
    switch (vol)
    {
        case 5:
            if(SW3516_PDO_VolState.PDO_9V_f) {
                SW3516_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_12V_f) {
                SW3516_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_15V_f) {
                SW3516_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_20V_f) {
                SW3516_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            break;

        case 9:
            if(SW3516_PDO_VolState.PDO_12V_f) {
                SW3516_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_15V_f) {
                SW3516_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_20V_f) {
                SW3516_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3516_PDO_EnConfig(9, 1);
            break;
            
        case 12:
            if(SW3516_PDO_VolState.PDO_9V_f) {
                SW3516_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_15V_f) {
                SW3516_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_20V_f) {
                SW3516_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3516_PDO_EnConfig(12, 1);
            break;

        case 15:
            if(SW3516_PDO_VolState.PDO_9V_f) {
                SW3516_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_12V_f) {
                SW3516_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_20V_f) {
                SW3516_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3516_PDO_EnConfig(15, 1);
            break;

        case 20:
            if(SW3516_PDO_VolState.PDO_9V_f) {
                SW3516_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_12V_f) {
                SW3516_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3516_PDO_VolState.PDO_15V_f) {
                SW3516_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3516_PDO_EnConfig(20, 1);
            break;            

        default:
            SW3516_PDO_EnConfig(9, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3516_PDO_EnConfig(12, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3516_PDO_EnConfig(15, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3516_PDO_EnConfig(20, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
    }
}


/**
 * @brief PPS0 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_PPS0_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb7;
    uint8_t BitMask = 0x01 << 7;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    SW3516_SrcChange();
}


/**
 * @brief PPS1 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_PPS1_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb7;
    uint8_t BitMask = 0x01 << 6;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }
    SW3516_I2C_WRITE_BYTE(0xB7, ( SW3516_I2C_READ_BYTE(0xB7)&~(SW3516_PDO_EMARKER_EN) ));
    SW3516_SrcChange();
}


/**
 * @brief PD 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_PD_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 5;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

    //SW3516_SrcChange();
}


/**
 * @brief QC3 协议使能控制，在QC2和QC3切换
 * @param state: 0：关闭QC3，切换到QC2
 *               1：打开QC3
 */
void SW3516_QC3_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xaa;
    uint8_t BitMask = 0x01 << 6;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }
}


/**
 * @brief QC 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_QC_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 4;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief FCP 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_FCP_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 3;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief 低压SCP 协议使能控制
 * @param state: 0：关闭该协议，高压SCP也跟着关闭
 *               1：打开该协议
 */
void SW3516_LvSCP_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 2;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief 高压SCP 协议使能控制
 * @param state: 0：关闭该协议，低压SCP仍然可使用
 *               1：打开该协议
 */
void SW3516_HvSCP_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xc5;
    uint8_t BitMask = 0x01 << 2;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief PE 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_PE_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 0;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief AFC 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_AFC_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xba;
    uint8_t BitMask = 0x01 << 6;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief 三星1.2V 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3516_Sam1V2_EnConfig(uint8_t state)
{ 
    uint8_t RegAdd = 0xad;
    uint8_t BitMask = 0x01 << 2;

    SW3516_RegW_Enable();

    if(state) {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

   // SW3516_SrcChange();
}


/**
 * @brief 关闭全部快充协议
 */
void SW3516_AllOff(void)
{
    // SW3516_PD_EnConfig(0);
    // //SW3516_PD_EnConfig(0);
    // SW3516_PPS0_EnConfig(0);
    // SW3516_PPS1_EnConfig(0);
    // SW3516_QC3_EnConfig(0);
    // SW3516_QC_EnConfig(0);
    // SW3516_FCP_EnConfig(0);
    // SW3516_LvSCP_EnConfig(0);
    // SW3516_HvSCP_EnConfig(0);
    // SW3516_PE_EnConfig(0);
    // SW3516_AFC_EnConfig(0);
    // SW3516_Sam1V2_EnConfig(0);

    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 7;

    SW3516_RegW_Enable();

    // SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
}


/**
 * @brief 打开全部快充协议
 */
void SW3516_AllOn(void)
{
    // SW3516_PD_EnConfig(1);
    // SW3516_PPS0_EnConfig(1);
    // SW3516_PPS1_EnConfig(1);
    // SW3516_QC3_EnConfig(1);    
    // SW3516_QC_EnConfig(1);
    // SW3516_FCP_EnConfig(1);
    // SW3516_LvSCP_EnConfig(1);
    // SW3516_HvSCP_EnConfig(1);
    // SW3516_PE_EnConfig(1);
    // SW3516_AFC_EnConfig(1);
    // SW3516_Sam1V2_EnConfig(1);

    uint8_t RegAdd = 0xb9;
    uint8_t BitMask = 0x01 << 7;

    SW3516_RegW_Enable();

    SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) | BitMask));  
    // SW3516_I2C_WRITE_BYTE(RegAdd, (SW3516_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
}


/**
 * @brief Buck强制开关
 * @param state: 0：关闭BUCK
 *               1：打开BUCK
 */
void SW3516_BuckEn(uint8_t state)
{ 
    if(state) 
    {
        SW3516_I2C_WRITE_BYTE(0x16, SW3516_I2C_READ_BYTE(0x16) & (~0x01));
	    SW3516_I2C_WRITE_BYTE(0x76, SW3516_I2C_READ_BYTE(0x76) & (~0x03));
    }
    else 
    {
        SW3516_I2C_WRITE_BYTE(0x16, SW3516_I2C_READ_BYTE(0x16) | 0x01);		// 强制关闭 Buck
	    SW3516_I2C_WRITE_BYTE(0x76, SW3516_I2C_READ_BYTE(0x76) | 0x03);		// 不使能BC1.2  强制不驱动CC
    }

   // SW3516_SrcChange();
}


#define	BLOCKED_RESER_POWER		// 阻塞方式复位/定时器方式复位

#ifndef BLOCKED_RESER_POWER
xTimerHandle RstPowTimer;
void ResetPowerTimer_CB(TimerHandle_t xTimer)
{
	SW3516_BuckEn(1);
}
#endif

/**
 * @brief 强制关闭BUCK 1s后再重启
 */
void SW3516_ResetPower(void)
{
	SW3516_BuckEn(0);

#ifndef BLOCKED_RESER_POWER
	RstPowTimer = xTimerCreate("RstPow", pdMS_TO_TICKS(1000), pdFALSE, (void *)0, ResetPowerTimer_CB);
	if(RstPowTimer != NULL) {
		xTimerStart(RstPowTimer, 0);
	}
#else
	vTaskDelay(pdMS_TO_TICKS(1000));

	SW3516_BuckEn(1);
#endif
}


/**
 * @brief 3516配置相关初始化
 */
void SW3516_InitConfig(void)
{
    iic_master_init();
    // SW3516_I2C_WRITE_BYTE(0x13, (SW3516_I2C_READ_BYTE(0x13)|(BIT6)));
    // SW3516_I2C_WRITE_BYTE(0xB8, ( SW3516_I2C_READ_BYTE(0xB8)|(BIT3) ));
    // vTaskDelay(200);
	// SW3516_PD_EnConfig(1);

    // // vTaskDelay(200);
    // // SW3516_PPS0_EnConfig(1);

    // vTaskDelay(200);
    // SW3516_PPS1_EnConfig(1);

    // vTaskDelay(200);
    // SW3516_PPS0_EnConfig(1);

    // //SW3516_AllOff();
	// //vTaskDelay(200);
	// //SW3516_PD_EnConfig(1);
    // //vTaskDelay(200);
    // //SW3516_PPS1_EnConfig(1);
    SW3516_AllOn();
    SW3516_I2C_WRITE_BYTE(0xB7, (SW3516_I2C_READ_BYTE(0xB7)&(~BIT3)));// 去掉12V，防止苹果电脑在广播低功率的情况下切12V
    ESP_LOGI(XXX_TAG, "SW3536 config ok");
}




void pack_chargeinfo(void)
{
    *(uint16_t*)SW3516_Info.ChargeData = DEVICE_ID_ESP32S3;
    SW3516_Info.ChargeData[2]          = 0x00; //crc result
    SW3516_Info.ChargeData[3]          = 0x00;
    SW3516_Info.ChargeData[4]          = 0x00; //password state
    SW3516_Info.ChargeData[5]          = 0x00;
    SW3516_Info.ChargeData[6]          = 0x00; //new password
    SW3516_Info.ChargeData[7]          = 0x00;
}

TaskHandle_t SW3516_Handle = NULL;

void sw3516_task(void *param)
{
    uint8_t send_data_tick = 0;

    while(1)
    {
        ESP_LOGI(XXX_TAG, "In sw3516 task.");

        //int ret;
        uint8_t reg_0x06;
        uint8_t reg_0x07;
        
        reg_0x06 = SW3516_I2C_READ_BYTE(0x06);
       // if(ESP_OK == ret)
        {
            if(uGetBits(reg_0x06,7,7) == 1) {
                // ESP_LOGI(XXX_TAG,"In quick charge state");
            }
            else {
                // ESP_LOGI(XXX_TAG,"Not quick charge state");
            }

            if(uGetBits(reg_0x06,6,6) == 1) {
                // ESP_LOGI(XXX_TAG,"In quick charge vol");
            }
            else {
                // ESP_LOGI(XXX_TAG,"Not quick charge vol");
            }

            if(uGetBits(reg_0x06,5,4) == 1) {
                // ESP_LOGI(XXX_TAG,"charge protocol is pd2.0");
                SW3516_Info.ChargeData[8] = 1;
            }
            else if(uGetBits(reg_0x06,5,4) == 2) {
                // ESP_LOGI(XXX_TAG,"charge protocol is pd3.0");
                SW3516_Info.ChargeData[8] = 2;
            }
            else {
                SW3516_Info.ChargeData[8] = 0xff;
            }

            switch(uGetBits(reg_0x06,3,0))
            {
                case 1:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is qc2.0");
                    SW3516_Info.ChargeData[9] = 1;
                }break;

                case 2:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is qc3.0");
                    SW3516_Info.ChargeData[9] = 2;
                }break;

                case 3:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is fcp");
                    SW3516_Info.ChargeData[9] = 3;
                }break;

                case 4:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is scp");
                    SW3516_Info.ChargeData[9] = 4;
                }break;

                case 5:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is pd fix");
                    SW3516_Info.ChargeData[9] = 5;
                }break;

                case 6:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is pd pps");
                    SW3516_Info.ChargeData[9] = 6;
                }break;

                case 7:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is pe1.1");
                    SW3516_Info.ChargeData[9] = 7;
                }break;

                case 8:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is pe2.0");
                    SW3516_Info.ChargeData[9] = 8;
                }break;

                case 10:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is sfcp");
                    SW3516_Info.ChargeData[9] = 9;
                }break;

                case 11:
                {
                    // ESP_LOGI(XXX_TAG,"charge protocol is afc");
                    SW3516_Info.ChargeData[9] = 10;
                }break;

                default:
                    // ESP_LOGI(XXX_TAG,"charge protocol is unknow");
                    SW3516_Info.ChargeData[9] = 0xff;
                break;
            }
        }
        // else
        // {
        //     SW3516_Info.ChargeData[9] = 0xff;
        // }

        reg_0x07 = SW3516_I2C_READ_BYTE(0x07);
      //  if(ESP_OK == ret)
        {
            bool com1_flag = false;
            bool com2_flag = false;

            if(uGetBits(reg_0x07,1,1) == 1) {
                // ESP_LOGI(XXX_TAG,"Com2 is on");
                com2_flag = true;
            }
            else {
                // ESP_LOGI(XXX_TAG,"Com2 is off");
                com2_flag = false;
            }

            if(uGetBits(reg_0x07,0,0) == 1) {
                // ESP_LOGI(XXX_TAG,"Com1 is on");
                com1_flag = true;
            }
            else {
                // ESP_LOGI(XXX_TAG,"Com1 is off");
                com1_flag = false;
            }

            if((true==com2_flag) && (true==com1_flag)) {
                SW3516_Info.ChargeData[10] = 3;
            }
            else
            {
                if(true == com1_flag)
                {
                    SW3516_Info.ChargeData[10] = 1;
                }
                else if(true == com2_flag)
                {
                    SW3516_Info.ChargeData[10] = 2;
                }
                else
                {
                    SW3516_Info.ChargeData[10] = 0;
                }
            }
        }
        // else
        // {
        //     SW3516_Info.ChargeData[10] = 0;
        // }

        ESP_LOGI(XXX_TAG,"Get the vin is        %d.",SW3516_Get_VIN());
        *(uint16_t*)(SW3516_Info.ChargeData + 11)  = SW3516_Get_VIN();
        ESP_LOGI(XXX_TAG,"Get the vout is       %d.",SW3516_Get_VOUT());
        *(uint16_t*)(SW3516_Info.ChargeData + 13)  = SW3516_Get_VOUT();
        ESP_LOGI(XXX_TAG,"Get the iout1 is      %d.",SW3516_Get_IOUT1());
        *(uint16_t*)(SW3516_Info.ChargeData + 15)  = SW3516_Get_IOUT1();
        ESP_LOGI(XXX_TAG,"Get the iout2 is      %d.",SW3516_Get_IOUT2());
        *(uint16_t*)(SW3516_Info.ChargeData + 17)  = SW3516_Get_IOUT2();

 
        if(true == SW3516_Info.SetPDO_f)
        {
            //SW3516_PDO_OnlyVol_EnConfig(SW3516_Info.PDO_Vol);
            //SW3516_SetPDO(SW3516_Info.PDO_Vol, SW3516_Info.PDO_Curr);

            SW3516_Info.SetPDO_f = false;
        }


        #if 1
        //每2s上报一次数据吧
        if(true == SW3516_Info.Notify_f)
        {
            pack_chargeinfo();
            if(send_data_tick++ >= 1)
            {
                send_data_tick = 0;

                esp_ble_gatts_send_indicate(sw3516_gatts.gatts_if, sw3516_gatts.conn_id, sw3516_gatts.service_handle,
                                        sizeof(SW3516_Info.ChargeData), SW3516_Info.ChargeData, false);
            }
        }
        #endif

        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}


/********************************************************************************************/
#ifdef SW3516_I2C_SIM       // 模拟I2C
// SimI2Cx SW3516_I2C;

SimI2Cx SW3516_U1;
//SimI2Cx SW3516_U2;
SimI2Cx *SW3516_Ux = &SW3516_U1;


static void SimI2C_SCL_L(void) {
    SW3516_SCL_L();
}

static void SimI2C_SCL_H(void) {
    SW3516_SCL_H();
}

static void SimI2C_SDA_L(void) {
    SW3516_SDA_L();
}

static void SimI2C_SDA_H(void) {
    SW3516_SDA_H();
}

static void SimI2C_SCL_Input(void) {
   SW3516_SCL_INPUT();
}

static void SimI2C_SCL_Output(void) {
	SW3516_SCL_OUTPUT();
}

static void SimI2C_SDA_Input(void) {
   SW3516_SDA_INPUT();
}

static void SimI2C_SDA_Output(void) {
	SW3516_SDA_OUTPUT();
}

static uint8_t SimI2C_SCL_Read(void) {
	return SW3516_SCL_READ();
}

static uint8_t SimI2C_SDA_Read(void) {
    return SW3516_SDA_READ();
}


static void bsp_InitI2C(void)
{
	GPIO_SetMode(SW3516_SCL_P_x, SW3516_SCL_BIT_n, GPIO_MODE_OUTPUT);		// i2c时钟
	GPIO_SetMode(SW3516_SDA_P_x, SW3516_SDA_BIT_n, GPIO_MODE_OUTPUT);		// i2c数据
}


// I2C读写
void SW3516_I2C_WRITE_BYTE(uint8_t reg, uint8_t dat)
{
    vSimI2C_WriteByte(SW3516_Ux, SW3516_I2C_ADD, reg, dat);
}		


uint8_t SW3516_I2C_READ_BYTE(uint8_t reg)	
{
   return u8SimI2C_ReadByte(SW3516_Ux, SW3516_I2C_ADD, reg);
}


void SW3516U1_IIC_Init(void)
{
    SimI2Cx *pI2C = &SW3516_U1;
    
	pI2C->GPIO_Init	 	= bsp_InitI2C;

    pI2C->SCL_Input 	= SimI2C_SCL_Input;
    pI2C->SCL_Output 	= SimI2C_SCL_Output;
    
    pI2C->SDA_Input 	= SimI2C_SDA_Input;
    pI2C->SDA_Output 	= SimI2C_SDA_Output;
    
    pI2C->SCL_L 		= SimI2C_SCL_L;
    pI2C->SCL_H 		= SimI2C_SCL_H;
    
    pI2C->SDA_H 		= SimI2C_SDA_H;
    pI2C->SDA_L 		= SimI2C_SDA_L;

    pI2C->SCL_Read	 	= SimI2C_SCL_Read;
	pI2C->SDA_Read 		= SimI2C_SDA_Read;


	vSimI2C_GPIO_Init(pI2C);


    if(xSimI2C_CheckDevice(pI2C, SW3516_I2C_ADD) == I2C_ACK) {
		printf("SW3516 IIC Ok\n");
	}
    else {
        printf("SW3516 IIC Faile\n");
    }
    // SW35XX IIC机制问题，不要只发送设备地址，但是不做任何读写操作。否则可能会影响2次的通讯不会应答。
    xSimI2C_CheckDevice(pI2C, SW3516_I2C_ADD);
    xSimI2C_CheckDevice(pI2C, SW3516_I2C_ADD);
}

#else
esp_err_t iic_master_init(void)
{
    int i2c_master_port = IIC_MASTER_NUM;

    i2c_config_t conf = 
    {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = IIC_MASTER_SDA_PIN,
        .scl_io_num = IIC_MASTER_SCL_PIN,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = IIC_MASTER_FREQ_HZ,
    };
    i2c_param_config(i2c_master_port,&conf);

    return i2c_driver_install(i2c_master_port,conf.mode,IIC_MASTER_RX_BUF_DISABLE,IIC_MASTER_TX_BUF_DISABLE,0);
}

void SW3516_I2C_WRITE_BYTE(uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = {reg, data};

    esp_err_t err = i2c_master_write_to_device(IIC_MASTER_NUM, SLAVE_SW35_ADDR, write_buf, sizeof(write_buf), IIC_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS); 

    if(err != ESP_OK) {
        ESP_LOGW(XXX_TAG, "SW3516 %d i2c fail err:%d, reg:%#x",IIC_MASTER_NUM,  err, reg);
    }
}

uint8_t SW3516_I2C_READ_BYTE(uint8_t reg)
{
    uint8_t data = 0;

    esp_err_t err = i2c_master_write_read_device(IIC_MASTER_NUM, SLAVE_SW35_ADDR, &reg, 1, &data, 1, IIC_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);

    if(err != ESP_OK) {
        ESP_LOGW(XXX_TAG, "SW3516 %d i2c fail err:%d, reg:%#x", IIC_MASTER_NUM, err, reg);
    }

    return data;
}

#endif




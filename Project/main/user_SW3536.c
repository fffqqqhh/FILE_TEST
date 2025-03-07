#include "SW3536.h"

#include "FreeRTOS.h"
#include "timers.h"

#include "user_oper_data.h"


struct PDO_VolSwitch
{
    uint8_t PDO_5V_f:1;
    uint8_t PDO_9V_f:1;
    uint8_t PDO_12V_f:1;
    uint8_t PDO_15V_f:1;
    uint8_t PDO_20V_f:1;
}SW3536_PDO_VolState = {1, 1, 1 ,1, 1};       // 记录PDO各个电压挡位的开关状态，默认都开启


static void _SW3536_I2C_WRITE_BYTE(uint16_t reg, uint8_t dat);
static uint8_t _SW3536_I2C_READ_BYTE(uint16_t reg);


/**
 * @brief I2C寄存器操作使能
 * @param reg_add: 需要读/写的寄存器地址
 */
void SW3536_RegW_Enable(uint16_t reg_add)
{
    static uint8_t Reg_f = 0;

    // 只有寄存器地址发生切换后，才重新使能对应区域寄存器
    if(((Reg_f == 0) && (reg_add > 0xff)) || ((Reg_f == 1) && (reg_add< 0x100)))
    {
        uint8_t Reg0x10 = _SW3536_I2C_READ_BYTE(0x10) & 0x1F;

        // 如果要写其他寄存器,需要先执行如下操作：
        _SW3536_I2C_WRITE_BYTE(0x10, Reg0x10 | 0x20);
        _SW3536_I2C_WRITE_BYTE(0x10, Reg0x10 | 0x40);
        _SW3536_I2C_WRITE_BYTE(0x10, Reg0x10 | 0x80); 

        if(reg_add > 0xff) 
        {
            // 若要操作 Reg0x100~Reg0x14F，写 Reg0x10=0x81
            _SW3536_I2C_WRITE_BYTE(0x10, Reg0x10 | 0x81);
            Reg_f = 1;
        }
        else 
        {
            // 要操作低于0x100的地址时，向Reg0x80写0即可将0x10 bit0清零
            _SW3536_I2C_WRITE_BYTE(0x80, 0x00);
            Reg_f = 0;
        }
    }
}


/**
 * @brief 强制寄存器操作使能，如果要操作寄存器Reg0xl6~Reg0x19，需要先执行
 */
void SW3536_ForceRegW_Enable(void)
{
    SW3536_I2C_WRITE_BYTE(0x15, 0x20);
    SW3536_I2C_WRITE_BYTE(0x15, 0x40);
    SW3536_I2C_WRITE_BYTE(0x15, 0x80);
}


/**
 * @brief 输入 Vin 的 ADC 工作使能，只有在使能时，Vin 的数据才能读出
 */
void SW3536_VinADC_Enable(void)
{ 
    SW3536_ForceRegW_Enable();
    
    // VIN ADC 默认关闭，需要设置 Reg0x18[6]打开相应 ADC 通路
    SW3536_I2C_WRITE_BYTE(0x18, uSetBit(SW3536_I2C_READ_BYTE(0x18), 6, 1));
}


/**
 * @brief 读取12位ADC
 * @param AdcType: 选择需要读取的内容
 * @return uint16_t: 12bit的AD值
 */
uint16_t SW3536_ADC_Read12bit(SW3536_AdcChannelDef AdcType)
{
    uint16_t SW3536_ADC;

    // 配置ADC通道
    SW3536_I2C_WRITE_BYTE(0x40, uWriteBits(SW3536_I2C_READ_BYTE(0x40), 3, 0, AdcType));
    
    vTaskDelay(10); // 等待数据锁存到Reg0x41和Reg0x42

    SW3536_ADC = uSetBit(SW3536_I2C_READ_BYTE(0x42), 7, 0);
    SW3536_ADC <<= 8;
    SW3536_ADC |= SW3536_I2C_READ_BYTE(0x41);

    return SW3536_ADC;
}


/**
 * @brief 读取NTC阻值
 * @return uint32_t: 阻值 单位：1Ω
 */
uint32_t SW3536_Get_RNTC(void) 
{
    /* 
     NTC 温度的计算如下：
     1)读取 ADC 的电压；
     2)根据 Reg0x44[7]判断当前使用的电流为 20uA 还是 40uA，计算得到电阻；
     3)根据 NTC 电阻的规格查表得到温度。 
    */
   
    uint16_t NtcVol = SW3536_ADC_Read12bit(SW3536_ADC_VNTC) * 1.2;       // NTC 通道的电压数据 1.2mV/bit

    if(uGetBit(SW3536_I2C_READ_BYTE(0x44), 7)) {
        // NTC 检测电流为 40uA
        return NtcVol / 0.04;
    }
    else {
        // NTC 检测电流为 20uA
        return NtcVol / 0.02;
    }
}

/**
 * @brief 读取Vin电压
 * @return uint16_t: 电压值 单位1mV
 */
uint16_t SW3536_Get_VIN(void)
{	
    SW3536_VinADC_Enable();        

    return (SW3536_ADC_Read12bit(SW3536_ADC_VIN) * 10);     // 输入电压数据，单位：10mV/bit
}

/**
 * @brief 读取Vout电压
 * @return uint16_t: 电压值 单位1mV
 */
uint16_t SW3536_Get_VOUT(void) {
    return (SW3536_ADC_Read12bit(SW3536_ADC_VOUT) * 6);     // 输出电压数据， 单位：6mV/bit
}

/**
 * @brief 读取端口1输出电流
 * @return uint16_t: 电流值 单位1mA
 */
uint16_t SW3536_Get_IOUT1(void) {
    return (SW3536_ADC_Read12bit(SW3536_ADC_IOUT1) * 2.5);  // 通路1电流数据 idis1， 单位：2.5mA/bit @5mohm
}

/**
 * @brief 读取端口2输出电流
 * @return uint16_t: 电流值 单位1mA
 */
uint16_t SW3536_Get_IOUT2(void) {
    return (SW3536_ADC_Read12bit(SW3536_ADC_IOUT2) * 2.5);  // 通路2电流数据 idis1， 单位：2.5mA/bit @5mohm
}

/**
 * @brief 计算输出功率
 * @param u: 输出电压 单位：1mV
 * @param i: 输出电流 单位：1mA
 * @return uint32_t: 输出功率  单位：1mW
 */
uint32_t SW3536_Get_OUTPow(uint16_t u, uint16_t i)												// 单位：1mW
{
    return (u * i) / 1000;
}


/**
 * @brief 打印芯片相关状态
 */
void SW3536_PrintState(void)
{
    uint16_t u, i1, i2;
    uint32_t p;
    
    u = SW3536_Get_VOUT();
    i1 = SW3536_Get_IOUT1();
    i2 = SW3536_Get_IOUT2();

    printf("\n\n");
#if	 				0				// m为单位显示
	printf("VIN:%5d mV  ", SW3536_Get_VIN());		// 输入电压
    printf("VOUT:%5d mV  ", u);						// 输出电压
    
    printf("R_NTC:%5d R", SW3536_Get_RNTC());		// NTC阻值
    printf("\n");

    p = SW3536_Get_OUTPow(u, i1);					// 端口1电流和功率
    printf("Iout1:%5d mA  ", i1);      				
    printf("IPow1:%5d mW\n", p);
    
    p = SW3536_Get_OUTPow(u, i2);					// 端口2电流和功率
    printf("Iout2:%5d mA  ", i2);					
    printf("IPow2:%5d mW", p); 
#else
    printf("VIN:%6.3f V  ", (float)SW3536_Get_VIN()/1000);		// 输入电压
    printf("VOUT:%6.3f V  ", (float)u/1000);					// 输出电压
    
    printf("R_NTC:%5d R", SW3536_Get_RNTC());					// NTC阻值
    printf("\n");

    p = SW3536_Get_OUTPow(u, i1);								// 端口1电流和功率
    printf("Iout1:%6.3f A  ", (float)i1/1000);      				
    printf("IPow1:%6.3f W\n", (float)p/1000);
    
    p = SW3536_Get_OUTPow(u, i2);								// 端口2电流和功率
    printf("Iout2:%6.3f A  ", (float)i2/1000);					
    printf("IPow2:%6.3f W", (float)p/1000);
#endif
    printf("\n");


    printf("Pmax: %d W\n", SW3536_I2C_READ_BYTE(0x02));


    uint8_t data;
    data = SW3536_I2C_READ_BYTE(0x09);
    printf("[0x09]: ");
    if(uGetBit(data, 6) == 1)						// 是否快充
        printf("[quick charge] ");
    else
        printf("[!quick charge] ");
    if(uGetBit(data, 7) == 1)						// 是否快充电压
        printf("[quick charge vol] ");
    else
        printf("[!quick charge vol] ");
    if(uGetBits(data, 5, 4) == 1)					// PD版本
        printf("[PD2.0] ");
    else
    if(uGetBits(data, 5, 4) == 2)
        printf("[PD3.0] ");
    switch (uGetBits(data, 3, 0))					// 当前正在使用的协议
    {
        case 0: printf("[Not]");  break;
        case 1: printf("[QC2.0]");  break;
        case 2: printf("[QC3.0]");  break;
        case 3: printf("[QC3+]");  break;
        case 4: printf("[FCP]");  break;
        case 5: printf("[SCP]");  break;
        case 6: printf("[PD FIX]");  break;
        case 7: printf("[PD PPS]");  break;
        case 8: printf("[PE1.1]");  break;
        case 9: printf("[PE2.0]");  break;
        case 13: printf("[SFCP]");  break;
        case 14: printf("[AFC]");  break;
        case 15: printf("[TFCP]");  break;
        default: printf("[other]");break;
    }
    printf("\n");


    data = SW3536_I2C_READ_BYTE(0x0A);
    printf("[0x0A]: ");
    if(uGetBit(data, 1) == 1)						// 端口2状态
        printf("[COM2 ON] ");
    else
        printf("[COM2 OFF] ");
    if(uGetBit(data, 0) == 1)						// 端口1状态
        printf("[COM1 ON] ");
    else
        printf("[COM1 OFF] ");
    printf("\n");

}


/**
 * @brief 发送 source capability 命令
 * @note 使用流程：1、SW3536_RegW_Enable解锁IIC，2、配置参数（0x100以下相关寄存器），3、发送src cap
 */
void SW3536_SrcChange(void)
{ 
    uint8_t Reg0xa7 = SW3536_I2C_READ_BYTE(0xa7) & 0xE0;

    SW3536_I2C_WRITE_BYTE(0xa7, Reg0xa7 | 0x0a);
}


/**
 * @brief PPS 电压电流设置
 * @param PPS_x: PPS编号，输入范围：0~3
 * @param MaxVol: PPS最大电压范围，输入范围：最小单位100mV
 * @param MaxIout: PPS最大电流范围，输入范围：最小单位50mA
 */
void SW3536_SetPPS(uint8_t PPS_x, uint16_t MaxVout, uint16_t MaxIout)
{
    if(PPS_x > 3)
        PPS_x = 3;

    // if(MaxVout > 5000)
    //     MaxVout = 5000;
    // if(MaxVout < 10)
    //     MaxVout = 10;

    // if(MaxIout > 5000)
    //     MaxIout = 5000;
    // if(MaxIout < 10)
    //     MaxIout = 10;

    MaxVout /= 100; // 100mV/位
    MaxIout /= 50;  // 50mA/位

    switch (PPS_x)
    {
        case PD_PPS0:
            SW3536_I2C_WRITE_BYTE(0x12C, MaxVout);
            SW3536_I2C_WRITE_BYTE(0x12E, uWriteBits(SW3536_I2C_READ_BYTE(0x12E), 6, 0, MaxIout));
            break;

        case PD_PPS1:
            SW3536_I2C_WRITE_BYTE(0x12F, MaxVout);
            SW3536_I2C_WRITE_BYTE(0x131, uWriteBits(SW3536_I2C_READ_BYTE(0x131), 6, 0, MaxIout));
            break;

        case PD_PPS2:
            SW3536_I2C_WRITE_BYTE(0x132, MaxVout);
            SW3536_I2C_WRITE_BYTE(0x134, uWriteBits(SW3536_I2C_READ_BYTE(0x134), 6, 0, MaxIout));
            break;

        case PD_PPS3:
            SW3536_I2C_WRITE_BYTE(0x135, MaxVout);
            SW3536_I2C_WRITE_BYTE(0x137, uWriteBits(SW3536_I2C_READ_BYTE(0x137), 6, 0, MaxIout));
            break;
    }

    SW3536_SrcChange();
}


/**
 * @brief PDO 电流电压设置
 * @param MaxVol: 选择设置电流的PDO电压档位，输入范围：5、9、12、15、20V
 * @param MaxIout: 设置MaxVol对应的PDO电流，单位1mA，步进10mA
 */
void SW3536_SetPDO(uint8_t MaxVol, uint16_t MaxIout)
{
    if(MaxIout > 5000)
        MaxIout = 5000;
    if(MaxIout < 10)
        MaxIout = 10;

    MaxIout /= 10;      // 10mA/位

    switch (MaxVol)
    {
        case 5:
            // pd_xV_cur[9:2]
            SW3536_I2C_WRITE_BYTE(0x126, MaxIout >> 2);
            // pd_xV_cur[1:0] 
            SW3536_I2C_WRITE_BYTE(0x125, uWriteBits(SW3536_I2C_READ_BYTE(0x125), 1, 0, (MaxIout & 0x03)));
            break;
        case 9:
            // pd_xV_cur[9:2]
            SW3536_I2C_WRITE_BYTE(0x127, MaxIout >> 2);
            // pd_xV_cur[1:0] 
            SW3536_I2C_WRITE_BYTE(0x12B, uWriteBits(SW3536_I2C_READ_BYTE(0x12B), 1, 0, (MaxIout & 0x03)));
            break;
        case 12:
            // pd_xV_cur[9:2]
            SW3536_I2C_WRITE_BYTE(0x128, MaxIout >> 2);
            // pd_xV_cur[1:0] 
            SW3536_I2C_WRITE_BYTE(0x12B, uWriteBits(SW3536_I2C_READ_BYTE(0x12B), 3, 2, (MaxIout & 0x03)));
            break;
        case 15:
            // pd_xV_cur[9:2]
            SW3536_I2C_WRITE_BYTE(0x129, MaxIout >> 2);
            // pd_xV_cur[1:0] 
            SW3536_I2C_WRITE_BYTE(0x12B, uWriteBits(SW3536_I2C_READ_BYTE(0x12B), 5, 4, (MaxIout & 0x03)));
            break;
        case 20:
            // pd_xV_cur[9:2]
            SW3536_I2C_WRITE_BYTE(0x12A, MaxIout >> 2);
            // pd_xV_cur[1:0] 
            SW3536_I2C_WRITE_BYTE(0x12B, uWriteBits(SW3536_I2C_READ_BYTE(0x12B), 7, 6, (MaxIout & 0x03)));
            break;
        default:
            break;
    }

    SW3536_SrcChange();
}

/**
 * @brief PDO 电压使能控制
 * @param vol: 选择需要控制的PDO电压档位，输入范围：9、12、15、20V
 * @param state: 0：关闭当前电压档位
 *               1：打开当前电压档位
 */
void SW3536_PDO_EnConfig(uint8_t vol, uint8_t state)
{ 
    uint16_t RegAdd = 0x124;
    uint8_t BitMask;

    switch (vol)
    {
        case 9:
            BitMask = SW3536_PDO_9V_EN;
            SW3536_PDO_VolState.PDO_9V_f = (state != 0)? 1 : 0;
            break;
        case 12:
            BitMask = SW3536_PDO_12V_EN;
            SW3536_PDO_VolState.PDO_12V_f = (state != 0)? 1 : 0;
            break;
        case 15:
            BitMask = SW3536_PDO_15V_EN;
            SW3536_PDO_VolState.PDO_15V_f = (state != 0)? 1 : 0;
            break;
        case 20:
            BitMask = SW3536_PDO_20V_EN;
            SW3536_PDO_VolState.PDO_20V_f = (state != 0)? 1 : 0;
            break;
    }

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, (SW3536_I2C_READ_BYTE(RegAdd) | BitMask));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, (SW3536_I2C_READ_BYTE(RegAdd) & (~BitMask)));  
    }

    SW3536_SrcChange();
}


/**
 * @brief PDO 电压选择，近保留选择的电压挡位
 * @param vol: 选择需要保留的PDO电压档位，输入范围：9、12、15、20V，其它：全开
 */
void SW3536_PDO_OnlyVol_EnConfig(uint8_t vol)
{
    switch (vol)
    {
        case 5:
            if(SW3536_PDO_VolState.PDO_9V_f) {
                SW3536_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_12V_f) {
                SW3536_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_15V_f) {
                SW3536_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_20V_f) {
                SW3536_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            break;

        case 9:
            if(SW3536_PDO_VolState.PDO_12V_f) {
                SW3536_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_15V_f) {
                SW3536_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_20V_f) {
                SW3536_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3536_PDO_EnConfig(9, 1);
            break;
            
        case 12:
            if(SW3536_PDO_VolState.PDO_9V_f) {
                SW3536_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_15V_f) {
                SW3536_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_20V_f) {
                SW3536_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3536_PDO_EnConfig(12, 1);
            break;

        case 15:
            if(SW3536_PDO_VolState.PDO_9V_f) {
                SW3536_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_12V_f) {
                SW3536_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_20V_f) {
                SW3536_PDO_EnConfig(20, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3536_PDO_EnConfig(15, 1);
            break;

        case 20:
            if(SW3536_PDO_VolState.PDO_9V_f) {
                SW3536_PDO_EnConfig(9, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_12V_f) {
                SW3536_PDO_EnConfig(12, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            if(SW3536_PDO_VolState.PDO_15V_f) {
                SW3536_PDO_EnConfig(15, 0);
                vTaskDelay(pdMS_TO_TICKS(200));
            }
            SW3536_PDO_EnConfig(20, 1);
            break;            

        default:
            SW3536_PDO_EnConfig(9, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3536_PDO_EnConfig(12, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3536_PDO_EnConfig(15, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            SW3536_PDO_EnConfig(20, 1);
            vTaskDelay(pdMS_TO_TICKS(200));
            break;
    }
}

/**
 * @brief PPS0 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PPS0_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x124;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief PPS1 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PPS1_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x124;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 5, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 5, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief PPS2 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PPS2_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x124;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 6, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 6, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief PPS3 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PPS3_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x124;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 7, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 7, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief PD 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PD_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x122;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief QC2 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_QC2_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 0));  
    }

    SW3536_SrcChange();
}

/**
 * @brief QC3 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_QC3_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 3, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 3, 0));  
    }

    SW3536_SrcChange();
}

/**
 * @brief QC3+ 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_QC3P_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 2, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 2, 0));  
    }

    SW3536_SrcChange();
}

/**
 * @brief QC4+ 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_QC4P_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 1, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 1, 0));  
    }

    SW3536_SrcChange();
}


/**
 * @brief 传音TFCP 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_TFCP_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief FCP 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_FCP_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x119;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 2, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 2, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief SFCP 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_SFCP_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 6, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 6, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief 低压SCP 协议使能控制
 * @param state: 0：关闭该协议，高压SCP也跟着关闭
 *               1：打开该协议
 */
void SW3536_LvSCP_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x119;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 1, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 1, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief 高压SCP 协议使能控制
 * @param state: 0：关闭该协议，低压SCP仍然可使用
 *               1：打开该协议
 */
void SW3536_HvSCP_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x119;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 0, 0));  
    }


   // SW3536_SrcChange();
}


/**
 * @brief PE 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_PE_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 7, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 7, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief AFC 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_AFC_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x11A;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 5, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 5, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief 苹果 2.4A 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_Apple2V4A_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x116;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 4, 0));  
    }

   SW3536_SrcChange();
}


/**
 * @brief 三星1.2V 协议使能控制
 * @param state: 0：关闭该协议
 *               1：打开该协议
 */
void SW3536_Sam1V2_EnConfig(uint8_t state)
{ 
    uint16_t RegAdd = 0x116;

    if(state) {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 3, 1));  
    }
    else {
        SW3536_I2C_WRITE_BYTE(RegAdd, uSetBit(SW3536_I2C_READ_BYTE(RegAdd), 3, 0));  
    }

   SW3536_SrcChange();
}


typedef void (*AgreeEnFunction)(uint8_t);

AgreeEnFunction AgreeEnFunctionTab[]={
    SW3536_PD_EnConfig,
    SW3536_PPS0_EnConfig,
    SW3536_PPS1_EnConfig,
    SW3536_PPS2_EnConfig,
    SW3536_PPS3_EnConfig,
    SW3536_QC2_EnConfig,
    SW3536_QC3_EnConfig,
    SW3536_QC3P_EnConfig,
    SW3536_QC4P_EnConfig,
    SW3536_TFCP_EnConfig,
    SW3536_FCP_EnConfig,
    SW3536_SFCP_EnConfig,
    SW3536_LvSCP_EnConfig,
    SW3536_HvSCP_EnConfig,
    SW3536_PE_EnConfig,
    SW3536_AFC_EnConfig,
    SW3536_Apple2V4A_EnConfig,
    SW3536_Sam1V2_EnConfig
};

/**
 * @brief 关闭全部快充协议
 */
void SW3536_AllOff(void)
{
    uint8_t i;

    for(i=0; i<(sizeof(AgreeEnFunctionTab) / sizeof(AgreeEnFunction)); i++)
    {
        AgreeEnFunctionTab[i](DISABLE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 打开全部快充协议
 */
void SW3536_AllOn(void)
{
    uint8_t i;

    for(i=0; i<(sizeof(AgreeEnFunctionTab) / sizeof(AgreeEnFunction)); i++)
    {
        AgreeEnFunctionTab[i](ENABLE);
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

/**
 * @brief 强制设置通路1限流
 * @param MaxIout: 最大电流值（最小1000mA/500mA）
 */
void SW3536_ForceSetPort1Curr(uint16_t MaxIout)
{
    // reg 0x07 bit7  0: 通路1限流值正常 1: 通路1限流值减半   （实测bit7=1时限流值正常）
    #define    SW3536_REG0x07_BIT7     1

    #if SW3536_REG0x07_BIT7 == 1
    // 限流值正常，1000mA+50mA*0x07[6:0]
    if(MaxIout < 1000)
        MaxIout = 1000;
    MaxIout -= 1000;
    MaxIout /= 50;
    
    #elif SW3536_REG0x07_BIT7 == 0
    // 限流值减半，500mA+25mA*0x07[6:0]
    if(MaxIout < 500)
        MaxIout = 500;
    MaxIout -= 500;
    MaxIout /= 25;
    #endif

    MaxIout &= 0x7f;

    /* 强制设置通路1限流的流程如下：
    1) I2C 使能，操作 Reg0x10 寄存器实现
    2) 强制操作使能，操作 reg0x15 寄存器实现
    3) 强制设置限流使能，操作 Reg0x16[0]实现
    4) 设置通路限流值，操作 Reg0x38 实现
    5) 限流生效，操作 Reg0x17[6]实现 */

    SW3536_ForceRegW_Enable();

    SW3536_I2C_WRITE_BYTE(0x16, uSetBit(SW3536_I2C_READ_BYTE(0x16), 0, 1));     // 强制设置 DAC 限流

    SW3536_I2C_WRITE_BYTE(0x38, uWriteBits(SW3536_I2C_READ_BYTE(0x38), 6, 0, MaxIout));

    SW3536_I2C_WRITE_BYTE(0x17, uSetBit(SW3536_I2C_READ_BYTE(0x17), 6, 1));
}


/**
 * @brief 强制设置通路2限流
 * @param MaxIout: 最大电流值（最小1000mA/500mA）
 */
void SW3536_ForceSetPort2Curr(uint16_t MaxIout)
{
    // reg 0x08 bit7  0: 通路1限流值正常 1: 通路1限流值减半   （实测bit7=1时限流值正常）
    #define    SW3536_REG0x08_BIT7     1

    #if SW3536_REG0x08_BIT7 == 1
    // 限流值正常，1000mA+50mA*0x08[6:0]
    if(MaxIout < 1000)
        MaxIout = 1000;
    MaxIout -= 1000;
    MaxIout /= 50;
    
    #elif SW3536_REG0x08_BIT7 == 0
    // 限流值减半，500mA+25mA*0x08[6:0]
    if(MaxIout < 500)
        MaxIout = 500;
    MaxIout -= 500;
    MaxIout /= 25;
    #endif

    MaxIout &= 0x7f;

    /* 强制设置通路2限流的流程如下：
    1) I2C 使能，操作 Reg0x10 寄存器实现
    2) 强制操作使能，操作 reg0x15 寄存器实现
    3) 强制设置限流使能，操作 Reg0x16[0]实现
    4) 设置通路限流值，操作 Reg0x39 实现
    5) 限流生效，操作 Reg0x17[7]实现 */

    SW3536_ForceRegW_Enable();

    SW3536_I2C_WRITE_BYTE(0x16, uSetBit(SW3536_I2C_READ_BYTE(0x16), 0, 1));     // 强制设置 DAC 限流

    SW3536_I2C_WRITE_BYTE(0x39, uWriteBits(SW3536_I2C_READ_BYTE(0x39), 6, 0, MaxIout));

    SW3536_I2C_WRITE_BYTE(0x17, uSetBit(SW3536_I2C_READ_BYTE(0x17), 7, 1));
}


/**
 * @brief 3536配置相关初始化
 */
void SW3536_InitConfig(void)
{
    uint16_t RegAdd;
    
    // I2C 模式下需要改变系统功率时，可以通过将此 bit 设置为 0，然后通过 Reg0x12A 来修改系统总功率。
    RegAdd = 0x115;
    SW3536_I2C_WRITE_BYTE(RegAdd, uWriteBits(SW3536_I2C_READ_BYTE(RegAdd), 4, 4, 0));

    // PD FIX档位电流值设置  0：手动设置     1：自动设置，依据系统功率自动计算
    RegAdd = 0x125;
    SW3536_I2C_WRITE_BYTE(RegAdd, uWriteBits(SW3536_I2C_READ_BYTE(RegAdd), 7, 7, 0));

    // PD PPS档位电流值设置  0：手动设置     1：自动设置，依据系统功率自动计算
    RegAdd = 0x125;
    SW3536_I2C_WRITE_BYTE(RegAdd, uWriteBits(SW3536_I2C_READ_BYTE(RegAdd), 6, 6, 0));

    // PD PPS自动配置模式下是否支持数字恒功率  0：不支持       1：支持
    RegAdd = 0x125;
    SW3536_I2C_WRITE_BYTE(RegAdd, uWriteBits(SW3536_I2C_READ_BYTE(RegAdd), 4, 4, 0));

    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_AllOff();

    vTaskDelay(pdMS_TO_TICKS(200));
    SW3536_PD_EnConfig(1);

    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPDO(5, 3000);
    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPDO(9, 3000);
    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPDO(12, 3000);
    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPDO(15, 3000);
    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPDO(20, 5000);
    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_SetPPS(0, 20000, 5000);
    

    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_PPS0_EnConfig(1);

    vTaskDelay(pdMS_TO_TICKS(100));
    SW3536_PDO_OnlyVol_EnConfig(0xff);

    printf("SW3536 config ok\n");
}

/********************************************************************************************/
//extern SimI2Cx SW3536_I2C;
SimI2Cx SW3536_U1;
//SimI2Cx SW3536_U2;
SimI2Cx *SW3536_Ux = &SW3536_U1;


static void SimI2C_SCL_L(void) {
    SW3536_SCL_L();
}

static void SimI2C_SCL_H(void) {
    SW3536_SCL_H();
}

static void SimI2C_SDA_L(void) {
    SW3536_SDA_L();
}

static void SimI2C_SDA_H(void) {
    SW3536_SDA_H();
}

static void SimI2C_SCL_Input(void) {
   SW3536_SCL_INPUT();
}

static void SimI2C_SCL_Output(void) {
	SW3536_SCL_OUTPUT();
}

static void SimI2C_SDA_Input(void) {
   SW3536_SDA_INPUT();
}

static void SimI2C_SDA_Output(void) {
	SW3536_SDA_OUTPUT();
}

static uint8_t SimI2C_SCL_Read(void) {
	return SW3536_SCL_READ();
}

static uint8_t SimI2C_SDA_Read(void) {
    return SW3536_SDA_READ();
}


static void bsp_InitI2C(void)
{
	GPIO_SetMode(SW3536_SCL_P_x, SW3536_SCL_BIT_n, GPIO_MODE_OUTPUT);		// i2c时钟
	GPIO_SetMode(SW3536_SDA_P_x, SW3536_SDA_BIT_n, GPIO_MODE_OUTPUT);		// i2c数据
}


void SW3536U1_IIC_Init(void)
{
    SimI2Cx *pI2C = &SW3536_U1;
    
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

     
    if(xSimI2C_CheckDevice(pI2C, SW3536_I2C_ADD) == I2C_ACK) {
		printf("SW3536 IIC Ok\n");
	}
    else {
        printf("SW3536 IIC Faile\n");
    }
    // SW35XX IIC机制问题，不要只发送设备地址，但是不做任何读写操作。否则可能会影响2次的通讯不会应答。
    xSimI2C_CheckDevice(pI2C, SW3536_I2C_ADD);
    xSimI2C_CheckDevice(pI2C, SW3536_I2C_ADD);
}


// 基础I2C读写
static void _SW3536_I2C_WRITE_BYTE(uint16_t reg, uint8_t dat)
{
    vSimI2C_WriteByte(SW3536_Ux, SW3536_I2C_ADD, reg, dat);
}

static uint8_t _SW3536_I2C_READ_BYTE(uint16_t reg)	
{
    return u8SimI2C_ReadByte(SW3536_Ux, SW3536_I2C_ADD, reg);
}


// 包含了I2C寄存器读写使能后的读写
void SW3536_I2C_WRITE_BYTE(uint16_t reg, uint8_t dat)
{
    SW3536_RegW_Enable(reg);

    _SW3536_I2C_WRITE_BYTE(reg & 0x00FF, dat);
}

uint8_t SW3536_I2C_READ_BYTE(uint16_t reg)	
{
    SW3536_RegW_Enable(reg);

    return _SW3536_I2C_READ_BYTE(reg & 0x00FF);
}

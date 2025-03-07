// 以下驱动还没调 用不了
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "user_IP2730.h"
#include "esp_log.h"

#define I2C_SCL_GPIO 19
#define I2C_SDA_GPIO 20
#define TAG "【IP2730】"
// uint8_t IP2730_ADDR = 0x00;

IP2730_TypeDef IP2730 = {0};

#if 0
void SET_SDA_OUTPUT()
{
    gpio_set_direction(I2C_SDA_GPIO, GPIO_MODE_OUTPUT);
}

void SET_SDA_INPUT()
{
    gpio_set_direction(I2C_SDA_GPIO, GPIO_MODE_INPUT);
}

void SET_IIC_CLK(unsigned char Stau)
{
    if (Stau)
        gpio_set_level(I2C_SCL_GPIO, 1);
    else
        gpio_set_level(I2C_SCL_GPIO, 0);
}

void SET_IIC_SDA(unsigned char Stau)
{
    if (Stau)
        gpio_set_level(I2C_SDA_GPIO, 1);
    else
        gpio_set_level(I2C_SDA_GPIO, 0);      
}

void  IIC_Start(void)
{
	SET_SDA_OUTPUT();
    SET_IIC_SDA(1);    
    SET_IIC_CLK(1);    
    SET_IIC_SDA(0);    
    SET_IIC_CLK(0);    
}

void  IIC_Stop(void)
{
	SET_SDA_OUTPUT();
    SET_IIC_CLK(0);    
    SET_IIC_SDA(0);    
    SET_IIC_CLK(1);   
    SET_IIC_SDA(1);   

}

void  IIC_ACK(void)
{
    SET_IIC_CLK(0);
    SET_IIC_SDA(0);   
	SET_SDA_OUTPUT();    
    SET_IIC_CLK(1);  
    SET_IIC_CLK(0); 
}

void  IIC_NACK(void)
{
    SET_IIC_CLK(0);
    SET_IIC_SDA(1);   
	SET_SDA_OUTPUT();    
    SET_IIC_CLK(1);  
    SET_IIC_CLK(0);   
}

void  IIC_Read_ACK(void)
{   
    SET_IIC_CLK(0);
	SET_SDA_INPUT();    
    SET_IIC_CLK(1);  
    SET_IIC_CLK(0);   
}

unsigned char  READ_IIC_SDA(void)
{
	// ets_delay_us(10);
    return gpio_get_level(I2C_SDA_GPIO);
	// ets_delay_us(10);
}

void IIC_WriteByte(unsigned char Dat)
{
    unsigned char i;
    
	SET_SDA_OUTPUT();   // ets_delay_us(5);
	
    for(i=0;i<8;i++)    //8bit data
    {
       SET_IIC_CLK(0);
       if(Dat & 0x80)   
       {
           SET_IIC_SDA(1);
       }
       else  
       {
           SET_IIC_SDA(0);
       } 
       SET_IIC_CLK(1);  
       Dat <<= 1; 			 
    }
   
		IIC_Read_ACK();
}

unsigned char  IIC_ReadByte(void)
{
    unsigned char Dat = 0;
    unsigned char i = 0;
    
    SET_SDA_INPUT();    // ets_delay_us(5);
    
    for(i=0;i<8;i++)              //8bit data 
   {
       SET_IIC_CLK(0);    
       Dat <<= 1;
       SET_IIC_CLK(1);    
       if(READ_IIC_SDA())  Dat|=1;
   }
   
	IIC_ACK();      //ACK
	 
   return Dat;
}

void  IIC_Write(unsigned char Ch, unsigned char DeviceAddr, unsigned char ArgAddr, unsigned char Dat)
{
    DeviceAddr<<=1;
			
    IIC_Start();
    IIC_WriteByte(DeviceAddr);
    IIC_WriteByte(ArgAddr);
    IIC_WriteByte(Dat);
    IIC_Stop();
}

unsigned char  IIC_Read(unsigned char Ch, unsigned char DeviceAddr, unsigned char ArgAddr)
{
    unsigned char Sdat = 0;
	
    DeviceAddr<<=1;
	
    IIC_Start();
    IIC_WriteByte(DeviceAddr);
    IIC_WriteByte(ArgAddr);
    IIC_Start();
    IIC_WriteByte(DeviceAddr | 1);   //RD mode
    Sdat = IIC_ReadByte();
    IIC_Stop();
	
    return Sdat;
}
#else
#include "driver/i2c.h"
#include "driver/i2c_master.h"
#include "driver/gpio.h"

#define IP2730_IIC_MASTER_NUM 1

/// @brief IP2730_iic_master_init
/// @param NULL 
/// @return: ESP_OK: succeed, other: fail
esp_err_t IP2730_iic_master_init(void)
{
    int i2c_master_port = IP2730_IIC_MASTER_NUM;

    i2c_config_t conf =
        {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = I2C_SDA_GPIO,
            .scl_io_num = I2C_SCL_GPIO,
            .sda_pullup_en = GPIO_PULLUP_ENABLE,
            .scl_pullup_en = GPIO_PULLUP_ENABLE,
            .master.clk_speed = 100000,
        };
    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

/// @brief IIC_Write
/// @param reg: register address
/// @param data: data
void IIC_Write(uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = {reg, data};

    esp_err_t err = i2c_master_write_to_device(IP2730_IIC_MASTER_NUM, IP2730_ADDR, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "IP2730 i2c fail err:%d, reg:%#x, addr:%d", err, reg, IP2730_ADDR);
        // IP2730_ADDR++;
        // if(IP2730_ADDR>=128)
        //     IP2730_ADDR = 0;
    }
    else
    {
        ESP_LOGW(TAG, "IP2730 addr is OK:%d", IP2730_ADDR);
    }
}

/// @brief IIC_Read
/// @param reg: register address
/// @return: data
uint8_t IIC_Read(uint8_t reg)
{
    uint8_t data = 0;

    esp_err_t err = i2c_master_write_read_device(IP2730_IIC_MASTER_NUM, IP2730_ADDR, &reg, 1, &data, 1, 1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "IP2730 i2c fail err:%d, reg:%#x", err, reg);
    }

    return data;
}
#endif

/// @brief IP2730 io init
/// @param NULL 
void IP2730_IO_Init(void)
{
    // 初始化INT_N, EN_N脚
    gpio_config_t INT_N = {
        .pin_bit_mask = BIT64(18),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 1,
    };
    gpio_config_t EN_N = {
        .pin_bit_mask = BIT64(8),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 1,
    };

    // 初始化SDA, SCL脚
    gpio_config_t SNK_SDA = {
        .pin_bit_mask = BIT64(20),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
    };
    gpio_config_t SNK_SCL = {
        .pin_bit_mask = BIT64(19),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
    };

    gpio_config(&INT_N);
    gpio_config(&EN_N);
    gpio_config(&SNK_SDA);
    gpio_config(&SNK_SCL);
    gpio_set_level(8, 0);
}

// /// @brief IP2730初始化
// /// @param NULL 
// void IP2730_Init(void)
// {
//     IP2730_iic_master_init();
//     IIC_Write(0x02, IIC_Read(0x02) | (1 << 3));    // CONTROL1 Reg0x02[3]=1 (I2C 使能)
//     IIC_Write(0x0E, IIC_Read(0x0E) | (1 << 2));    // PD_PRIOR Reg0x0E[2]= 1(默认PD优先级打开)
//     IIC_Write(0x0F, IIC_Read(0x0F) | (1 << 6));    // Reg0x0F[6]=1b PPS Capability is supported
// }

/// @brief IP2730初始化
/// @param NULL 
void IP2730_Init(void)
{
    uint8_t data_temp = 0;
    IP2730_iic_master_init();

    vTaskDelay(pdMS_TO_TICKS(500));

    data_temp = IIC_Read(0x03) &~ (BIT0+BIT1);
    vTaskDelay(pdMS_TO_TICKS(1));
    IIC_Write(0x03, data_temp);    // TypeC_CTL :UFP

    vTaskDelay(pdMS_TO_TICKS(1));
    data_temp = (IIC_Read(0x43) | (BIT3+BIT2))&~(BIT1+BIT0);
    IIC_Write(0x43, data_temp);    // SNK_EVALUATE_CAP
    vTaskDelay(pdMS_TO_TICKS(1));

    ESP_LOGW(TAG, "RECEIVE_CAP1_00 DATA is: %d",IIC_Read(0x27));
    
    vTaskDelay(pdMS_TO_TICKS(500));
    IP2730.PD_PMAX = 100;
    i2c_driver_delete(IP2730_IIC_MASTER_NUM);
}

/// @brief 获取PPS最大电压和最大功率
/// @param num:PPS档位 
void GetPPSVmaxPmax(unsigned char num)
{
    switch (IP2730.PPS_Cap_VolMax[num])
    {
    case 0:
        IP2730.PD_PPS_VMAX = 5900;
        break;

    case 1:
        IP2730.PD_PPS_VMAX = 11000;
        break;

    case 2:
        IP2730.PD_PPS_VMAX = 16000;
        break;

    case 3:
        IP2730.PD_PPS_VMAX = 21000;
        break;

    default:
        break;
    }
    IP2730.PD_PPS_PMAX = IP2730.PD_PPS_VMAX*IP2730.PPS_Cap_Current[num]/10000;
}

/// @brief 获取sink PDO
/// @param NULL 
void IP2730_Read_PDO_Current(void)
{
    unsigned int Addr;
    IP2730.PPS_Cap_Current[0] = IIC_Read(0x72) & 0x7f;
    IP2730.PPS_Cap_Current[1] = IIC_Read(0x73) & 0x7f;
    IP2730.PPS_Cap_Current[2] = IIC_Read(0x74) & 0x7f;

    IP2730.PPS_Cap_VolMax[0] = IIC_Read(0x75); //
    IP2730.PPS_Cap_VolMax[1] = (IP2730.PPS_Cap_VolMax[0] >> 4) & 0x3;
    IP2730.PPS_Cap_VolMax[2] = (IP2730.PPS_Cap_VolMax[0] >> 2) & 0x3;
    IP2730.PPS_Cap_VolMin[0] = IP2730.PPS_Cap_VolMax[0] & 0x3;
    IP2730.PPS_Cap_VolMax[0] >>= 6;

    if (IP2730.PPS_Cap_Current[2])
        GetPPSVmaxPmax(2);
    else if (IP2730.PPS_Cap_Current[1])
        GetPPSVmaxPmax(1);
    else if (IP2730.PPS_Cap_Current[0])
        GetPPSVmaxPmax(0);
    else
        IP2730.PD_PPS_VMAX = 0;

    for (Addr = 0x6A; Addr <= 0x71; Addr++)
    {
        IP2730.PDO_Current[Addr - 0x6A] = (IIC_Read(Addr)&~BIT7);
        // ESP_LOGE(TAG, "PDO %d Current is :%d",(Addr-0x6A),IP2730.PDO_Current[Addr - 0x6A]);
    }

    IP2730.Reg0x18 = IIC_Read(0x18);
    IP2730.Reg0x19 = IIC_Read(0x19);

    if (IP2730.PDO_Current[4])
    {
        IP2730.PD_FIX_VMAX = 20000;
        IP2730.PD_FIX_PMAX = IP2730.PD_FIX_VMAX*IP2730.PDO_Current[4]/10000;
    }
    else if (IP2730.PDO_Current[3])
    {
        IP2730.PD_FIX_VMAX = 15000;
        IP2730.PD_FIX_PMAX = IP2730.PD_FIX_VMAX*IP2730.PDO_Current[3]/10000;
    }
    else if (IP2730.PDO_Current[2])
    {
        IP2730.PD_FIX_VMAX = 12000;
        IP2730.PD_FIX_PMAX = IP2730.PD_FIX_VMAX*IP2730.PDO_Current[2]/10000;
    }
    else if (IP2730.PDO_Current[1])
    {
        IP2730.PD_FIX_VMAX = 9000;
        IP2730.PD_FIX_PMAX = IP2730.PD_FIX_VMAX*IP2730.PDO_Current[1]/10000;
    }
    else if (IP2730.PDO_Current[0])
    {
        IP2730.PD_FIX_VMAX = 5000;
        IP2730.PD_FIX_PMAX = IP2730.PD_FIX_VMAX*IP2730.PDO_Current[0]/10000;
    }
    
    // ESP_LOGE(TAG, "PD_FIX_VMAX: %d",IP2730.PD_FIX_VMAX);
    // ESP_LOGE(TAG, "PD_FIX_PMAX: %d",IP2730.PD_FIX_PMAX);
    // ESP_LOGE(TAG, "PD_PPS_VMAX: %d",IP2730.PD_PPS_VMAX);
    // ESP_LOGE(TAG, "PD_PPS_PMAX: %d",IP2730.PD_PPS_PMAX);

    if ((IP2730.PD_PPS_VMAX>IP2730.PD_FIX_VMAX)&&(IP2730.PD_PPS_PMAX>IP2730.PD_FIX_PMAX))
    {
        IP2730_PPS_RequestMAX(IP2730.PD_PPS_VMAX);
        IP2730.PD_PMAX = IP2730.PD_PPS_PMAX;
    }
    else
    {
        IP2730_FixPDO_Request(IP2730.PD_FIX_VMAX);
        IP2730.PD_PMAX = IP2730.PD_FIX_PMAX;
    }
    i2c_driver_delete(IP2730_IIC_MASTER_NUM);
}

/// @brief 获取PPS是否就绪
/// @param NULL 
/// @return 1:就绪 0:未就绪
unsigned char IP2730_Is_PPS_Ready(void)
{
    if (IP2730.PPS_Cap_Current[0] || IP2730.PPS_Cap_Current[1] || IP2730.PPS_Cap_Current[2])
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

/// @brief 通过PPS请求电压
/// @param PPS_x 
/// @param RequestVol 
void IP2730_PPS_Request(unsigned char PPS_x, unsigned int RequestVol)
{
    unsigned char ValueH, ValueL;

    if (IP2730.PPS_Cap_Current[PPS_x] == 0)
        return; // PPS not supported

    RequestVol = (RequestVol - 3000) / 20;
    ValueL = RequestVol & 0xFF;
    ValueH = (RequestVol >> 8) & 0x3;

    IP2730.Reg0x19 = (IP2730.Reg0x19 & 0xFC) | ValueH;
    IIC_Write(0x19, IP2730.Reg0x19);                     //
    IIC_Write(0x1A, ValueL);                               //
    IIC_Write(0x1B, (IIC_Read(0x72 + PPS_x) & 0x7F) << 1); //
    IP2730.Reg0x19 = (IP2730.Reg0x19 & 0x07) | ((0x6 + PPS_x) << 3);
    IIC_Write(0x19, IP2730.Reg0x19); // PPS1
    IP2730.Reg0x18 = (IP2730.Reg0x19 & 0xE0) | 1;
    IIC_Write(0x18, IP2730.Reg0x18); // Go Command Reg0x18[4:0]=00001b, 请求PPS 5V
}

/// @brief 请求最高档位PPS的电压
/// @param RequestVol 
void IP2730_PPS_RequestMAX(unsigned int RequestVol)
{
    for (int PPS_x = 2; PPS_x >= 0; PPS_x--)
    {
        if (IP2730.PPS_Cap_Current[(unsigned char)PPS_x])
        {
            IP2730_PPS_Request(PPS_x, RequestVol);
            break;
        }
    }
}

/// @brief 通过请求满足要求的PPS电压
/// @param RequestVol:mV
/// @return 返回当前PPS的最大电流值,0表示没有找到满足要求的PPS
unsigned int UserRequestVoil_PPS(unsigned int RequestVol)
{
    unsigned char i;
    unsigned int Vmax, Vmin;

    for (i = 0; i < 3; i++)
    {
        if (IP2730.PPS_Cap_Current[i]) // 查询PPS是否支持
        {
            if (IP2730.PPS_Cap_VolMax[i] == 0)
                Vmax = 5900;
            else if (IP2730.PPS_Cap_VolMax[i] == 1)
                Vmax = 11000;
            else if (IP2730.PPS_Cap_VolMax[i] == 2)
                Vmax = 16000;
            else
                Vmax = 21000;

            if (IP2730.PPS_Cap_VolMin[0] == 0)
                Vmin = 3000;
            else if (IP2730.PPS_Cap_VolMin[0] == 1)
                Vmin = 3300;
            else if (IP2730.PPS_Cap_VolMin[0] == 2)
                Vmin = 5000;
            else
                Vmin = 5000;

            if (Vmin <= RequestVol && RequestVol < Vmax)
            {
                IP2730_PPS_Request(i, RequestVol);
                IP2730.UserPPSx = i;
                IP2730.UserPPS_MaxCurrent = 100 * (unsigned int)IP2730.PPS_Cap_Current[i];
                return 100 * (unsigned int)IP2730.PPS_Cap_Current[i];
            }
        }
    }
    return 0;
}

/// @brief 获取当前PPS的最大电流值
/// @param NULL
/// @return 返回当前PPS的最大电流值
unsigned int GetUserPPS_MaxCurrent(void)
{
    return IP2730.UserPPS_MaxCurrent;
}

/// @brief 请求对应PD档位
/// @param PDO_x:PDO档位
void IP2730_FixPDO_Request(unsigned int Voltage)
{
    unsigned char PDO_x;

    if (Voltage == 5000)    
        PDO_x = 1;
    else if (Voltage == 9000)
        PDO_x = 2;  
    else if (Voltage == 12000)
        PDO_x = 3;
    else if (Voltage == 15000)
        PDO_x = 4;
    else if (Voltage == 20000)
        PDO_x = 5;
    else
        return;

    IIC_Write(0x19, PDO_x << 3); //
    IP2730.Reg0x18 = (IP2730.Reg0x19 & 0xE0) | 1;
    IIC_Write(0x18, BIT0); // Go Command Reg0x18[4:0]=00001b, 请求PPS 5V
}

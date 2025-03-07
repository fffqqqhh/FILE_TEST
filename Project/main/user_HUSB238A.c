#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_system.h"
#include "sdkconfig.h"
#include "user_HUSB238A.h"
#include "esp_log.h"

#define I2C_SCL_GPIO 19
#define I2C_SDA_GPIO 20
#define TAG "【HUSB238A】"


HUSB238A_TypeDef HUSB238A = {0};

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

#define HUSB238A_IIC_MASTER_NUM 1

/// @brief husb238a_iic_master_init
/// @param NULL 
/// @return: ESP_OK: succeed, other: fail
esp_err_t husb238a_iic_master_init(void)
{
    int i2c_master_port = HUSB238A_IIC_MASTER_NUM;

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

    esp_err_t err = i2c_master_write_to_device(HUSB238A_IIC_MASTER_NUM, HUSB238A_ADDR, write_buf, sizeof(write_buf), 1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "HUSB238A i2c fail err:%d, reg:%#x", err, reg);
    }
}

/// @brief IIC_Read
/// @param reg: register address
/// @return: data
uint8_t IIC_Read(uint8_t reg)
{
    uint8_t data = 0;

    esp_err_t err = i2c_master_write_read_device(HUSB238A_IIC_MASTER_NUM, HUSB238A_ADDR, &reg, 1, &data, 1, 1000 / portTICK_PERIOD_MS);

    if (err != ESP_OK)
    {
        ESP_LOGW(TAG, "HUSB238A i2c fail err:%d, reg:%#x", err, reg);
    }

    return data;
}
#endif

/// @brief HUSB238A io init
/// @param NULL 
void HUSB238A_IO_Init(void)
{
    // 初始化INT_N, EN_N脚
    gpio_config_t INT_N = {
        .pin_bit_mask = BIT64(18),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 1,
        .pull_down_en = 0,
    };
    gpio_config_t EN_N = {
        .pin_bit_mask = BIT64(8),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = 0,
        .pull_down_en = 1,
    };

    gpio_config(&INT_N);
    gpio_config(&EN_N);
    gpio_set_level(8, 0);
}

/// @brief HUSB238A初始化
/// @param NULL 
void HUSB238A_Init(void)
{
    husb238a_iic_master_init();
    IIC_Write(0x02, IIC_Read(0x02) | (1 << 3));    // CONTROL1 Reg0x02[3]=1 (I2C 使能)
    IIC_Write(0x0E, IIC_Read(0x0E) | (1 << 2));    // PD_PRIOR Reg0x0E[2]= 1(默认PD优先级打开)
    IIC_Write(0x0F, IIC_Read(0x0F) | (1 << 6));    // Reg0x0F[6]=1b PPS Capability is supported
}

/// @brief 获取PPS最大电压和最大功率
/// @param num：PPS档位 
void GetPPSVmaxPmax(unsigned char num)
{
    switch (HUSB238A.PPS_Cap_VolMax[num])
    {
    case 0:
        HUSB238A.PD_PPS_VMAX = 5900;
        break;

    case 1:
        HUSB238A.PD_PPS_VMAX = 11000;
        break;

    case 2:
        HUSB238A.PD_PPS_VMAX = 16000;
        break;

    case 3:
        HUSB238A.PD_PPS_VMAX = 21000;
        break;

    default:
        break;
    }
    HUSB238A.PD_PPS_PMAX = HUSB238A.PD_PPS_VMAX*HUSB238A.PPS_Cap_Current[num]/10000;
}

/// @brief 获取sink PDO
/// @param NULL 
void HUSB238A_Read_PDO_Current(void)
{
    unsigned int Addr;
    HUSB238A.PPS_Cap_Current[0] = IIC_Read(0x72) & 0x7f;
    HUSB238A.PPS_Cap_Current[1] = IIC_Read(0x73) & 0x7f;
    HUSB238A.PPS_Cap_Current[2] = IIC_Read(0x74) & 0x7f;

    HUSB238A.PPS_Cap_VolMax[0] = IIC_Read(0x75); //
    HUSB238A.PPS_Cap_VolMax[1] = (HUSB238A.PPS_Cap_VolMax[0] >> 4) & 0x3;
    HUSB238A.PPS_Cap_VolMax[2] = (HUSB238A.PPS_Cap_VolMax[0] >> 2) & 0x3;
    HUSB238A.PPS_Cap_VolMin[0] = HUSB238A.PPS_Cap_VolMax[0] & 0x3;
    HUSB238A.PPS_Cap_VolMax[0] >>= 6;

    if (HUSB238A.PPS_Cap_Current[2])
        GetPPSVmaxPmax(2);
    else if (HUSB238A.PPS_Cap_Current[1])
        GetPPSVmaxPmax(1);
    else if (HUSB238A.PPS_Cap_Current[0])
        GetPPSVmaxPmax(0);
    else
        HUSB238A.PD_PPS_VMAX = 0;

    for (Addr = 0x6A; Addr <= 0x71; Addr++)
    {
        HUSB238A.PDO_Current[Addr - 0x6A] = (IIC_Read(Addr)&~BIT7);
        // ESP_LOGE(TAG, "PDO %d Current is :%d",(Addr-0x6A),HUSB238A.PDO_Current[Addr - 0x6A]);
    }

    HUSB238A.Reg0x18 = IIC_Read(0x18);
    HUSB238A.Reg0x19 = IIC_Read(0x19);

    if (HUSB238A.PDO_Current[4])
    {
        HUSB238A.PD_FIX_VMAX = 20000;
        HUSB238A.PD_FIX_PMAX = HUSB238A.PD_FIX_VMAX*HUSB238A.PDO_Current[4]/10000;
    }
    else if (HUSB238A.PDO_Current[3])
    {
        HUSB238A.PD_FIX_VMAX = 15000;
        HUSB238A.PD_FIX_PMAX = HUSB238A.PD_FIX_VMAX*HUSB238A.PDO_Current[3]/10000;
    }
    else if (HUSB238A.PDO_Current[2])
    {
        HUSB238A.PD_FIX_VMAX = 12000;
        HUSB238A.PD_FIX_PMAX = HUSB238A.PD_FIX_VMAX*HUSB238A.PDO_Current[2]/10000;
    }
    else if (HUSB238A.PDO_Current[1])
    {
        HUSB238A.PD_FIX_VMAX = 9000;
        HUSB238A.PD_FIX_PMAX = HUSB238A.PD_FIX_VMAX*HUSB238A.PDO_Current[1]/10000;
    }
    else if (HUSB238A.PDO_Current[0])
    {
        HUSB238A.PD_FIX_VMAX = 5000;
        HUSB238A.PD_FIX_PMAX = HUSB238A.PD_FIX_VMAX*HUSB238A.PDO_Current[0]/10000;
    }
    
    // ESP_LOGE(TAG, "PD_FIX_VMAX: %d",HUSB238A.PD_FIX_VMAX);
    // ESP_LOGE(TAG, "PD_FIX_PMAX: %d",HUSB238A.PD_FIX_PMAX);
    // ESP_LOGE(TAG, "PD_PPS_VMAX: %d",HUSB238A.PD_PPS_VMAX);
    // ESP_LOGE(TAG, "PD_PPS_PMAX: %d",HUSB238A.PD_PPS_PMAX);

    if ((HUSB238A.PD_PPS_VMAX>HUSB238A.PD_FIX_VMAX)&&(HUSB238A.PD_PPS_PMAX>HUSB238A.PD_FIX_PMAX))
    {
        HUSB238A_PPS_RequestMAX(HUSB238A.PD_PPS_VMAX);
        HUSB238A.PD_PMAX = HUSB238A.PD_PPS_PMAX;
    }
    else
    {
        HUSB238A_FixPDO_Request(HUSB238A.PD_FIX_VMAX);
        HUSB238A.PD_PMAX = HUSB238A.PD_FIX_PMAX;
    }
    i2c_driver_delete(HUSB238A_IIC_MASTER_NUM);
}

/// @brief 获取PPS是否就绪
/// @param NULL 
/// @return 1:就绪 0:未就绪
unsigned char HUSB238A_Is_PPS_Ready(void)
{
    if (HUSB238A.PPS_Cap_Current[0] || HUSB238A.PPS_Cap_Current[1] || HUSB238A.PPS_Cap_Current[2])
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
void HUSB238A_PPS_Request(unsigned char PPS_x, unsigned int RequestVol)
{
    unsigned char ValueH, ValueL;

    if (HUSB238A.PPS_Cap_Current[PPS_x] == 0)
        return; // PPS not supported

    RequestVol = (RequestVol - 3000) / 20;
    ValueL = RequestVol & 0xFF;
    ValueH = (RequestVol >> 8) & 0x3;

    HUSB238A.Reg0x19 = (HUSB238A.Reg0x19 & 0xFC) | ValueH;
    IIC_Write(0x19, HUSB238A.Reg0x19);                     //
    IIC_Write(0x1A, ValueL);                               //
    IIC_Write(0x1B, (IIC_Read(0x72 + PPS_x) & 0x7F) << 1); //
    HUSB238A.Reg0x19 = (HUSB238A.Reg0x19 & 0x07) | ((0x6 + PPS_x) << 3);
    IIC_Write(0x19, HUSB238A.Reg0x19); // PPS1
    HUSB238A.Reg0x18 = (HUSB238A.Reg0x19 & 0xE0) | 1;
    IIC_Write(0x18, HUSB238A.Reg0x18); // Go Command Reg0x18[4:0]=00001b, 请求PPS 5V
}

/// @brief 请求最高档位PPS的电压
/// @param RequestVol 
void HUSB238A_PPS_RequestMAX(unsigned int RequestVol)
{
    for (int PPS_x = 2; PPS_x >= 0; PPS_x--)
    {
        if (HUSB238A.PPS_Cap_Current[(unsigned char)PPS_x])
        {
            HUSB238A_PPS_Request(PPS_x, RequestVol);
            break;
        }
    }
}

/// @brief 通过请求满足要求的PPS电压
/// @param RequestVol：mV
/// @return 返回当前PPS的最大电流值,0表示没有找到满足要求的PPS
unsigned int UserRequestVoil_PPS(unsigned int RequestVol)
{
    unsigned char i;
    unsigned int Vmax, Vmin;

    for (i = 0; i < 3; i++)
    {
        if (HUSB238A.PPS_Cap_Current[i]) // 查询PPS是否支持
        {
            if (HUSB238A.PPS_Cap_VolMax[i] == 0)
                Vmax = 5900;
            else if (HUSB238A.PPS_Cap_VolMax[i] == 1)
                Vmax = 11000;
            else if (HUSB238A.PPS_Cap_VolMax[i] == 2)
                Vmax = 16000;
            else
                Vmax = 21000;

            if (HUSB238A.PPS_Cap_VolMin[0] == 0)
                Vmin = 3000;
            else if (HUSB238A.PPS_Cap_VolMin[0] == 1)
                Vmin = 3300;
            else if (HUSB238A.PPS_Cap_VolMin[0] == 2)
                Vmin = 5000;
            else
                Vmin = 5000;

            if (Vmin <= RequestVol && RequestVol < Vmax)
            {
                HUSB238A_PPS_Request(i, RequestVol);
                HUSB238A.UserPPSx = i;
                HUSB238A.UserPPS_MaxCurrent = 100 * (unsigned int)HUSB238A.PPS_Cap_Current[i];
                return 100 * (unsigned int)HUSB238A.PPS_Cap_Current[i];
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
    return HUSB238A.UserPPS_MaxCurrent;
}

/// @brief 请求对应PD档位
/// @param PDO_x：PDO档位
void HUSB238A_FixPDO_Request(unsigned int Voltage)
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
    HUSB238A.Reg0x18 = (HUSB238A.Reg0x19 & 0xE0) | 1;
    IIC_Write(0x18, BIT0); // Go Command Reg0x18[4:0]=00001b, 请求PPS 5V
}

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/i2c_master.h"

#include "esp_gatts_api.h"
#include "esp_log.h"

#include "user_oper_data.h"
#include "user_SW3566.h"

#define TAG      "【SW3566】"     


SW35xx_Info_t SW3516_Info;
gatts_profile_t sw3516_gatts={0, 0, 0};

chip_obj_t sw3566_idx_num;
uint8_t sw3566_idx_addr;


QueueHandle_t sw3566_irq_queue;

enum sw3566_protocol_enum_list 
{
    NONE = 0,
    QC2_0, QC3_0, QC3_PLUS,
    SFCP, AFC,
    FCP, SCP,
    VOOC1_0, VOOC4_0, SVOOC2_0,
    TFCP,
    UFCS,
    PE1_0, PE2_0,
    PD_FIX_5V,
    PD_FIX_HV,
    PD_SPR_AVS,
    PD_PPS,
    PD_EPR,
    PD_AVS,
    OTHER,
};
char sw3566_protocol_str_list[][11] = {
    "NONE",
    "QC2.0","QC3.0","QC3+",
    "SFCP","AFC",
    "FCP","SCP",
    "VOOC1.0","VOOC4.0","SVOOC2.0",
    "TFCP",
    "UFCS",
    "PE1.0","PE2.0",
    "PD FIX 5V",
    "PD FIX HV",
    "PD SPR AVS",
    "PD PPS",
    "PD EPR",
    "PD AVS",
    "other"
};

/********************************************************************************************/
#ifdef SW3566_I2C_SIM       // 模拟I2C
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
void sw3566_write_byte(uint8_t reg, uint8_t dat)
{
    vSimI2C_WriteByte(SW3516_Ux, SW3516_I2C_ADD, reg, dat);
}		


uint8_t sw3566_read_byte(uint8_t reg)	
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
    // SW35XX IIC机制问题,不要只发送设备地址,但是不做任何读写操作.否则可能会影响2次的通讯不会应答.
    xSimI2C_CheckDevice(pI2C, SW3516_I2C_ADD);
    xSimI2C_CheckDevice(pI2C, SW3516_I2C_ADD);
}

#else

/********************************************************************************************/

i2c_config_t i2c_cfg = { 
    0
    // .mode = I2C_MODE_MASTER,
    // .sda_io_num = IIC_MASTER_SDA_PIN,
    // .scl_io_num = IIC_MASTER_SCL_PIN,
    // .sda_pullup_en = GPIO_PULLUP_ENABLE,
    // .scl_pullup_en = GPIO_PULLUP_ENABLE,
    // .master.clk_speed = IIC_MASTER_FREQ_HZ,
};


/// @brief 切换I2C总线上的芯片
/// @param chip_obj  
esp_err_t sw3566_i2c_switch_chip(chip_obj_t obj)
{
    switch(obj)
    {
        case SW_U1:
        {
            i2c_cfg.sda_io_num = SW3566_U1_SDA_PIN;
            i2c_cfg.scl_io_num = SW3566_U1_SCL_PIN;
            sw3566_idx_addr = SW3566_U1_ADDR;
            sw3566_idx_num = SW_U1;
        }break;
        case SW_U2:
        {
            i2c_cfg.sda_io_num = SW3566_U2_SDA_PIN;
            i2c_cfg.scl_io_num = SW3566_U2_SCL_PIN;
            sw3566_idx_addr = SW3566_U2_ADDR;
            sw3566_idx_num = SW_U2;
        }break;
        case SW_U3:
        {
            i2c_cfg.sda_io_num = SW3566_U3_SDA_PIN;
            i2c_cfg.scl_io_num = SW3566_U3_SCL_PIN;
            sw3566_idx_addr = SW3566_U3_ADDR;
            sw3566_idx_num = SW_U3;
        }break;
        case SW_U4:
        {
            i2c_cfg.sda_io_num = SW3566_U4_SDA_PIN;
            i2c_cfg.scl_io_num = SW3566_U4_SCL_PIN;
            sw3566_idx_addr = SW3566_U4_ADDR;
            sw3566_idx_num = SW_U4;
        }break;
        default:
            return ESP_FAIL;
    }

    //printf("%s: switch to chip %d\n", TAG, sw3566_idx_num);

    return i2c_param_config(SW3566_I2C_NUM, &i2c_cfg);
}


static void sw3566_isr(void *arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    uint8_t sw3566_irq = 0;

    if(gpio_num == SW3566_U1_IRQ_PIN) 
    {
        sw3566_irq = SW_U1 + 1;
        xQueueSendFromISR(sw3566_irq_queue, &sw3566_irq, NULL);

    }else if(gpio_num == SW3566_U2_IRQ_PIN) 
    {
        sw3566_irq = SW_U2 + 1;
        xQueueSendFromISR(sw3566_irq_queue, &sw3566_irq, NULL);

    }else if(gpio_num == SW3566_U3_IRQ_PIN) 
    {
        sw3566_irq = SW_U3 + 1;
        xQueueSendFromISR(sw3566_irq_queue, &sw3566_irq, NULL);

    }
    else if(gpio_num == SW3566_U4_IRQ_PIN) 
    {
        sw3566_irq = SW_U4 + 1;
        xQueueSendFromISR(sw3566_irq_queue, &sw3566_irq, NULL);

    }
        
    // 清除中断标志(针对电平触发需手动清除,边沿触发可忽略)
    // gpio_intr_clear(gpio_num);
}

/// @brief 初始化I2C总线
esp_err_t sw3566_gpio_init(void)
{
    gpio_config_t io_conf = {        
        .pin_bit_mask = BIT64(SW3566_U1_IRQ_PIN)|BIT64(SW3566_U2_IRQ_PIN)|BIT64(SW3566_U3_IRQ_PIN)|BIT64(SW3566_U4_IRQ_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = 0,
        .pull_down_en = 0,
        .intr_type = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io_conf);

    sw3566_irq_queue = xQueueCreate(5, sizeof(uint8_t));

    gpio_install_isr_service(0);
    gpio_isr_handler_add(SW3566_U1_IRQ_PIN, sw3566_isr, (void*)SW3566_U1_IRQ_PIN);
    gpio_isr_handler_add(SW3566_U2_IRQ_PIN, sw3566_isr, (void*)SW3566_U2_IRQ_PIN);
    gpio_isr_handler_add(SW3566_U3_IRQ_PIN, sw3566_isr, (void*)SW3566_U3_IRQ_PIN);
    gpio_isr_handler_add(SW3566_U4_IRQ_PIN, sw3566_isr, (void*)SW3566_U4_IRQ_PIN);


    i2c_cfg.mode = I2C_MODE_MASTER,
    // i2c_cfg.sda_io_num = IIC_MASTER_SDA_PIN,
    // i2c_cfg.scl_io_num = IIC_MASTER_SCL_PIN,
    i2c_cfg.sda_pullup_en = 1,
    i2c_cfg.scl_pullup_en = 1,
    i2c_cfg.master.clk_speed = IIC_MASTER_FREQ_HZ,

    sw3566_i2c_switch_chip(SW_U1);

    return i2c_driver_install(SW3566_I2C_NUM, i2c_cfg.mode, IIC_MASTER_RX_BUF_DISABLE, IIC_MASTER_TX_BUF_DISABLE, 0);
}


/// @brief 查找I2C总线上的所有设备,并打印有效设备的地址(调试用)
void sw3566_i2c_find_addr(void)
{
    uint8_t reg = 0x00;
    uint8_t data = 0;

	for(uint8_t addr = 0x00; addr <= 0x7f; addr++)
	{
        esp_err_t err = i2c_master_write_read_device(SW3566_I2C_NUM, addr, &reg, 1, &data, 1, IIC_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);

        if(err != ESP_OK) {
            ESP_LOGW(TAG, "%d i2c fail err:%d, reg:%#x, addr:%#x", SW3566_I2C_NUM, err, reg, addr);
        }
        else {
            ESP_LOGW(TAG, "Device valid addr: 0x%x(0x%x)\n", addr, addr<<1);
            break;
        }
	}
}


/// @brief 3566写寄存器
/// @param reg 寄存器地址
/// @param data 写入数据
void sw3566_write_byte(uint8_t reg, uint8_t data)
{
    uint8_t write_buf[2] = {reg, data};

    esp_err_t err = i2c_master_write_to_device(SW3566_I2C_NUM, sw3566_idx_addr, write_buf, sizeof(write_buf), IIC_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS); 

    if(err != ESP_OK) {
        ESP_LOGW(TAG, "U%d i2c write fail err:%d, reg:%#x, data:%#x", (uint8_t)sw3566_idx_num, err, reg, data);
    }
}

/// @brief 3566读寄存器
/// @param reg 寄存器地址
/// @param recv_data 读出数据
/// @return 0读取失败 1读取成功
uint8_t sw3566_read_byte(uint8_t reg, uint8_t *recv_data)
{
    uint8_t data[3] = {0};
    
    esp_err_t err = i2c_master_write_read_device(SW3566_I2C_NUM, sw3566_idx_addr, &reg, 1, &data[0], 3, IIC_MASTER_TIMEOUT_MS/portTICK_PERIOD_MS);

    if(err != ESP_OK) {
        ESP_LOGW(TAG, "U%d i2c read fail err:%d, reg:%#x\n", (uint8_t)sw3566_idx_num, err, reg);
        return 0;
    }

    /*
        第一个字节为寄存器地址,第二个字节为寄存器数据,第三个字节为读数据校验位
        校验位数据 = ~(寄存器地址 + 寄存器数据)
    */
    if(data[0] != reg || (data[2] != ((uint8_t)~(data[0]+data[1])))) {    // 3566需要按3字节读取,并且校验
        ESP_LOGW(TAG, "U%d read check fail, reg:%#x, data:%#x %#x %#x\n", (uint8_t)sw3566_idx_num, reg, data[0], data[1], data[2]);
        return 0;
    }

    *recv_data = data[1];
    
    return 1;
}

/* /// @brief 读取并打印连续的寄存器数据
/// @param i2c_x :操作的SIM I2C句柄
/// @param start_reg_add :起始寄存器地址
/// @param end_reg_add :结束寄存器地址
void sw3566_printf_reg(sim_i2c *i2c_x, uint8_t start_reg_add, uint8_t end_reg_add)
{
	uint8_t buff[255];

	sw3566_read_data(i2c_x, start_reg_add, buff, end_reg_add - start_reg_add + 1);
	 
	for(uint16_t i = start_reg_add; i <= end_reg_add; i++)	
		print_reg(i, buff[i]);	
}
 */

/// @brief 读取寄存器的第bit位
/// @param reg_addr 目标寄存器地址
/// @param bit 位编号
/// @return 读取到的bit数据
uint8_t sw3566_read_bit(uint8_t reg_addr, uint8_t bit)
{
	uint8_t reg_val;

    sw3566_read_byte(reg_addr, &reg_val);

	return uGetBit(reg_val, bit);
}

/// @brief 读取寄存器的第h_bit~l_bit位
/// @param reg_addr 目标寄存器地址
/// @param h_bit 高位
/// @param l_bit 低位
/// @return 读取到的bit数据
uint8_t sw3566_read_bits(uint8_t reg_addr, uint8_t h_bit, uint8_t l_bit)
{
	uint8_t reg_val;

    sw3566_read_byte(reg_addr, &reg_val);

	return uGetBits(reg_val, h_bit, l_bit);
}

/// @brief 设置寄存器的第bit位
/// @param reg_addr 目标寄存器地址
/// @param bit 位编号
/// @param bit_val 新的位值
/// @return 写入的新值
uint8_t sw3566_set_bit(uint8_t reg_addr, uint8_t bit, uint8_t bit_val)
{
	uint8_t reg_val;

    sw3566_read_byte(reg_addr, &reg_val);

	if(bit_val)
		bit_val = 1;
	
	reg_val = uSetBit(reg_val, bit, bit_val);

	sw3566_write_byte(reg_addr, reg_val);

	return reg_val;
}

/// @brief 设置寄存器的第h_bit~l_bit位
/// @param reg_addr 目标寄存器地址
/// @param h_bit 高位
/// @param l_bit 低位
/// @param bit_val 新的位值
/// @return 写入的新值
uint8_t sw3566_set_bits(uint8_t reg_addr, uint8_t h_bit, uint8_t l_bit, uint8_t bit_val)
{
	uint8_t reg_val;

    sw3566_read_byte(reg_addr, &reg_val);

	reg_val = uWriteBits(reg_val, h_bit, l_bit, bit_val);

	sw3566_write_byte(reg_addr, reg_val);

	return reg_val;
}

void sw3566_write_10bit_data(uint8_t reg_h, uint16_t data)
{
    if(data > 0x03ff)
        data &= 0x03ff;
    
    uint8_t data_h = (data>>8) & 0x0003;
    uint8_t data_l = data & 0x00ff;

    sw3566_set_bits(reg_h, 1, 0, data_h);
    sw3566_write_byte(reg_h + 1, data_l);
}

uint16_t sw3566_get_16bit_data(uint8_t reg_h)
{
    uint8_t data_h, data_l;
    sw3566_read_byte(reg_h, &data_h);
    sw3566_read_byte(reg_h + 1, &data_l);
    return (data_h<<8) | data_l;
}

/********************************************************************************************/

/// @brief 上电初始化
/// @param obj 选择的芯片对象
void sw3566_init(chip_obj_t obj, uint8_t max_power)
{
    sw3566_i2c_switch_chip(obj);
    
    sw3566_set_bits(0x17, 7,7, 0);                  // 3566程序暂停

    sw3566_set_bits(0x01, 7,5, 1);                  // 单C模式
    sw3566_write_byte(0x02, max_power);             // PD最大功率

    // sw3566_pps_en(PPS_MAX, 0);                      
    // sw3566_pps_en(PPS_0, 1);                     

    sw3566_set_bits(0x17, 7,7, 1);                  // 3566程序开始运行
}

/// @brief irq处理
void sw3566_irq_handler(void)
{
    uint8_t irq_num;

    if (xQueueReceive(sw3566_irq_queue, &irq_num, 0)) {
        // ESP_LOGI(TAG, "U%d intr\n", (int)irq_num);
        irq_num = 0; // 清除标志
    }
}


/// @brief 获取vout
/// @return voltage unit:mV
uint16_t sw3566_get_vout(void) {
    return sw3566_get_16bit_data(0x26);
}
/// @brief 获取vin
/// @return voltage unit:mV
uint16_t sw3566_get_vin(void) {
    return sw3566_get_16bit_data(0x28);
}
/// @brief 获取port 1电流
/// @return curr unit:mA
uint16_t sw3566_get_port1_curr(void) {
    return sw3566_get_16bit_data(0x2a);
}
/// @brief 获取port 2电流
/// @return curr unit:mA
uint16_t sw3566_get_port2_curr(void) {
    return sw3566_get_16bit_data(0x0e);
}

uint8_t sw3566_get_pdo_base_addr(uint8_t vol)
{
    switch (vol)
    {
        case 5:     return 0x05;
        case 9:     return 0x07;
        case 12:    return 0x0b;
        case 15:    return 0x0f;
        case 20:    return 0x13;
        case 28:    return 0x1d;
    }
    return 0;
}

/// @brief 重新广播PDO
void sw3566_update_pdo(void)
{
    sw3566_set_bit(0x01, 0, 1);
}

/// @brief 硬件复位
void sw3566_hard_reset(void)
{
    sw3566_set_bit(0x01, 1, 1);
}



/// @brief pps 使能控制
/// @param num pps编号
/// @param state 0:关闭,1:打开
void sw3566_pps_en(pps_num_t num, uint8_t en)
{ 
    uint8_t reg = 0x03;
    uint8_t val = 0;

    if(en)
        en = 1;
        
    sw3566_read_byte(reg, &val);
    
    switch (num)
    {
        case PPS_0:            
            val = uSetBit(val, 7, en);
        break;
        case PPS_1:            
            val = uSetBit(val, 6, en);
        break;
        case PPS_2:            
            val = uSetBit(val, 5, en);
        break;
        case PPS_MAX:
            val = uSetBits(val, 7, 5, en==1? 0x07 : 0x00);            
        break;
    }
    sw3566_write_byte(reg, val);
}


/// @brief PDO 电压使能控制
/// @param vol 选择需要控制的PDO电压档位,输入范围:9、12、15、20、28V
/// @param state 0:关闭当前电压档位,1:打开当前电压档位
void sw3566_pdo_vol_en(uint8_t vol, uint8_t en)
{ 
    uint8_t reg = sw3566_get_pdo_base_addr(vol);
    sw3566_set_bit(reg, 2, en);
}


/// @brief PDO 电流设置
/// @param vol 选择设置电流的PDO电压档位,输入范围:5、9、12、15、20V
/// @param mA 设置vol对应的PDO电流,单位10mA
void sw3566_pdo_set_curr(uint8_t vol, uint16_t mA)
{
    uint8_t reg = sw3566_get_pdo_base_addr(vol);

    if(vol != 5)
        reg += 2;

    sw3566_write_10bit_data(reg, mA/10);

    printf("set pdo %d curr %d\n", vol, mA);
}

#endif

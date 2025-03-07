
#include "user_letter_shell.h"

#define     TAG     "shell"

/*
    三种移植方法介绍
    有操作系统:
        使能 SHELL_USING_TASK 和 SHELL_TASK_WHILE
        绑定 shell.writ 和 shell.read 用于收发数据
        创建 shellTask() 任务 xTaskCreate(shellTask, "shellTask", 2048, &shell, tskIDLE_PRIORITY + 2, NULL);	// FreeRTOS Task Create demo

    无操作系统 1:
        不使能 SHELL_USING_TASK 和 SHELL_TASK_WHILE
        绑定 shell.writ,以及在uart接收中断中调用shellInput(&shell, data),用于收发数据

    无操作系统 2:
        使能 SHELL_USING_TASK ,不使能 SHELL_TASK_WHILE
        绑定 shell.writ 和 shell.read 用于收发数据
        在主循环中不断调用 shellTask()
*/

#define SHELL_PORTING_METHOD             1       // 移植方法选择,1:有操作系统,2:无操作系统 1,3:无操作系统 2


// 定义shell对象
// SHELL_TypeDef shell;


// #include "user_sw3566.h"

/// @brief 打印寄存器数据
/// @param reg :寄存器地址
/// @param data :寄存器数据
void print_reg(uint8_t reg, uint8_t data) 
{
    printf("0x%02x = %3d, 0x%02x", reg, data, data);

    #define DATA_BITS   8
    printf(", ");
    for(uint8_t i = 0; i < DATA_BITS; i++)
    {
        if((i%4 == 0) && (i != 0))
           printf(" ");
        printf("%d", (data >> (DATA_BITS - 1 - i)) & 1);     // 高位在前
    }
    printf("\n");
}


void shell_sw_read_byte(chip_obj_t obj, uint8_t reg_addr)
{
    uint8_t rev_data;
    sw3566_i2c_switch_chip(obj);
    sw3566_read_byte(reg_addr, &rev_data);
    print_reg(reg_addr, rev_data);
}


void shell_sw_write_byte(chip_obj_t obj, uint8_t reg_addr, uint8_t data)
{
    sw3566_i2c_switch_chip(obj);
    sw3566_write_byte(reg_addr, data);
    print_reg(reg_addr, data);
}


// shell 命令表
const SHELL_CommandTypeDef shellCommandList[] =
{
	{"---------------------", NULL, "----------------------------"},	
	SHELL_CMD_ITEM_EX(help,     shellHelp,          command help,       help [cmd] -show help info of command),
	SHELL_CMD_ITEM(clear,       shellClear,         clear command line),
    #if SHELL_USING_VAR == 1
	SHELL_CMD_ITEM_EX(vars,     shellListVariables, show all vars,      read $[name]; write $[name] [val]; ),
    #endif
	
	// {"reset",           (shellFunction)NVIC_SystemReset,                "reset mcu"},        
    // { "ver",            (shellFunction)shell_version,                   "version info"},
    
    { "findSw",         (shellFunction)sw3566_i2c_find_addr,                "sw3566_i2c_find_addr"},

    { "sw",             (shellFunction)sw3566_i2c_switch_chip,              "switch chip 0-3"},
    { "swr",            (shellFunction)shell_sw_read_byte,                  "shell_sw_read_byte"},
    { "sww",            (shellFunction)shell_sw_write_byte,                 "shell_sw_write_byte"},
    { "swr16",          (shellFunction)sw3566_get_16bit_data,               "get_16bit_data"},
    { "sww10",          (shellFunction)sw3566_write_10bit_data,             "write_10bit_data"},

    { "ppsEn",          (shellFunction)sw3566_pps_en,                       "set pps en[num][en]"},
    { "pdoEn",          (shellFunction)sw3566_pdo_vol_en,                   "set pdo 9,12,15,20,28V en[vol][en])"},
    { "pdoI",           (shellFunction)sw3566_pdo_set_curr,                 "set pdo curr[vol][mA]"},


    // 以下添加自定义命令
	// 格式:{命令名称(不能用空格),命令函数指针,命令描述,命令长帮助信息(可选)},等同SHELL_CMD_ITEM和SHELL_CMD_ITEM_EX
	// {"---------------------", NULL, "----------------------------"},
};


const SHELL_KeyFunctionDef keyFuncList[] =
{
	// 注意:不要重复使用shellDefaultKeyFunctionList[]表里已使用的默认按键.
	// 格式:{按键提示,按键类型,回调函数}
    {"update pdo", SHELL_KEY_CTRL_U, (keyFunction)sw3566_update_pdo},
    {"update pdo", SHELL_KEY_CTRL_R, (keyFunction)sw3566_hard_reset},
};


#if SHELL_USING_VAR == 1
uint8_t demo_val;
// shell 变量表
const SHELL_VaribaleTypeDef shellVariableList[] = 
{
	// 格式:对应变量类型的导入命令(变量名称,变量值,变量描述)
	SHELL_VAR_ITEM(demo, &demo_val, 0~255, SHELL_VAR_CHAR),

};
#endif



#if SHELL_PORTING_METHOD == 1

static void shell_uart_write(uint8_t data)
{
    uart_write_bytes(UART_NUM_0, &data, 1);
}

static signed char shell_uart_read(uint8_t *data) 
{
    // if(xQueueReceive(xUart0RxQueue, data, (TickType_t)portMAX_DELAY) == pdPASS) 
    if(uart_read_bytes(UART_NUM_0, data, 1, portMAX_DELAY))          //数据长度大于0,说明接收到数据
		return 0;
	else 
		return 1;

    // *data = '7';
    // return 0;
}

void user_shell_init(void)
{
    uart_init_config(UART_NUM_0, 115200, UART0_TX_PIN, UART0_RX_PIN);


    shell.write = (shellWrite)shell_uart_write;
    shell.read = (shellRead)shell_uart_read;

    shellInit(&shell);

    shellSetCommandList(&shell, (SHELL_CommandTypeDef *)shellCommandList, sizeof(shellCommandList) / sizeof(SHELL_CommandTypeDef));
    shellSetKeyFuncList(&shell, (SHELL_KeyFunctionDef *)keyFuncList, sizeof(keyFuncList) / sizeof(SHELL_KeyFunctionDef));
    #if SHELL_USING_VAR == 1
    shellSetVariableList(&shell,(SHELL_VaribaleTypeDef *)shellVariableList, sizeof(shellVariableList) / sizeof(SHELL_VaribaleTypeDef));
    #endif


    xTaskCreate(shellTask, "shellTask", 4096, &shell, tskIDLE_PRIORITY + 2, NULL);
}


#elif SHELL_PORTING_METHOD == 2
void shell_uart_write(uint8_t data) 
{
    drv_usart_ex_write(OM_USART0, &data, 1, 10);
}

void user_shell_init(void)
{
    shell.write = (shellWrite)shell_uart_write;
    // shell.read = (shellRead)Uart0_ShellReceive;      // 中断接收方式
    shellInit(&shell);

    shellSetCommandList(&shell, (SHELL_CommandTypeDef *)shellCommandList, sizeof(shellCommandList) / sizeof(SHELL_CommandTypeDef));
    shellSetKeyFuncList(&shell, (SHELL_KeyFunctionDef *)keyFuncList, sizeof(keyFuncList) / sizeof(SHELL_KeyFunctionDef));
    #if SHELL_USING_VAR == 1
    shellSetVariableList(&shell,(SHELL_VaribaleTypeDef *)shellVariableList, sizeof(shellVariableList) / sizeof(SHELL_VaribaleTypeDef));
    #endif
}

#elif SHELL_PORTING_METHOD == 3

#endif


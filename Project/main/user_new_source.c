#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_log.h"
#include "esp_gatts_api.h"

#include "user_new_source.h"
#include "user_oper_data.h"

#define TAG "【source】"




uint8_t POWER_ALL = DEVICE_MAX_POWER;   // 最大功率
uint8_t POWER_C1_MAX;                   // 双口状态下C1口最大功率
uint8_t POWER_C2_MAX;                   // 双口状态下C2口最大功率


uint8_t ReportedData[20];// 蓝牙上报
uint8_t ComdData[3];// 蓝牙接收
StateMachine_struct MyPower;
ChargingInformation_struct PORT[PORT_MAX];

/// @brief 定时器节拍任务函数
/// @param NULL
void user_timer_task(void *parm)
{
    while (1)
    {
        // if(MyPower.ADCCheck_TIMER)
        //     MyPower.ADCCheck_TIMER--;
        // if (MyPower.NTCLowPower_TIMER)
        //     MyPower.NTCLowPower_TIMER--;
        if (MyPower.Check_TIMER)
            MyPower.Check_TIMER--;
        // if (MyPower.PortCheck_TIMER)
        //     MyPower.PortCheck_TIMER--;
        // if (MyPower.PowerCheck_TIMER)idf.py build
        //     MyPower.PowerCheck_TIMER--;
//         if (MyPower.SystemI2CCheck_TIMER)
//             MyPower.SystemI2CCheck_TIMER--;
        if (MyPower.Notify_TIMER)
            MyPower.Notify_TIMER--;
//         if (MyPower.FirstIn_TIMER)
//             MyPower.FirstIn_TIMER--;
//         if (MyPower.NoteBookCheck_TIMER)
//             MyPower.NoteBookCheck_TIMER--;
// #ifdef SUPPORT_VBUS_DETECT
//         for (int i = 0; i < 2; i++)
//         {
//             if (VBusCtl[i].VBusOFF_TIMER)
//                 VBusCtl[i].VBusOFF_TIMER--;
//             if (VBusCtl[i].LoadStart_TIMER>1)
//                 VBusCtl[i].LoadStart_TIMER--;
//             if (VBusCtl[i].VBusOFF_COUNT)
//                 VBusCtl[i].VBusOFF_COUNT--;
//         }
// #endif
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void cpc_init(void)
{
    sw3566_gpio_init();

    // for(chip_obj_t obj = SW_U1; obj<SW_MAX; obj++) {
    //     sw3566_init(obj, POWER_ALL);
    // }
    sw3566_init(SW_U1, 100);
    sw3566_init(SW_U2, 100);
    sw3566_init(SW_U3, 100);
    sw3566_init(SW_U4, 100);
    
}

/// @brief 获取并上传协议、电压、电流信息
/// @param PortNum:端口号
void cpc_read_info(port_num_t PortNum)
{
    sw3566_i2c_switch_chip(PortNum); // 切换到对应端口的芯片
    
    uint8_t state = 0;
    sw3566_read_byte(0x4b, &state);

    if (uGetBits(state, 7, 7) == 1)
        PORT[PortNum].quick_charge_f = 1;
    else
        PORT[PortNum].quick_charge_f = 0;

    PORT[PortNum].protocol = uGetBits(state, 6, 0); // 读取协议

    PORT[PortNum].curr_vin = sw3566_get_vin();                                             // 读取输入电压
    PORT[PortNum].curr_vout = sw3566_get_vout();                                           // 读取输出电压
    PORT[PortNum].curr_iout = sw3566_get_port1_curr();                                     // 读取输出电流
    PORT[PortNum].curr_power = (float)(PORT[PortNum].curr_vout * PORT[PortNum].curr_iout) / 1000; // 读取输出功率

   if (sw3566_read_bit(0x34, 6) == 1)
        PORT[PortNum].port_state = 1;
    else
        PORT[PortNum].port_state = 0;
}


/// @brief 周期检测
void period_check(void)
{
    // 读取功率、状态
    // if ((MyPower.Check_TIMER == 0) && (MyPower.NexState == IDLE))
    if (MyPower.Check_TIMER == 0)
    {
        cpc_read_info(PORT_1);
        cpc_read_info(PORT_2);
        cpc_read_info(PORT_3);
        cpc_read_info(PORT_4);

        MyPower.Check_TIMER = 2; // 200ms一次
    }
}

/// @brief 功率自动分配任务函数
/// @param NULL
void PowerAllocation_task(void *parm)
{
    cpc_init();

    xTaskCreate(user_timer_task, "user_timer", 1024, NULL, 7, NULL);

    while (1)
    {
        period_check();



        sw3566_irq_handler();


        
        vTaskDelay(pdMS_TO_TICKS(10)); 
        // vTaskDelay(10 / portTICK_PERIOD_MS); // 10ms/loop
    }
}



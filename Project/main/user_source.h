#ifndef __USER_SOURCE_H__
#define __USER_SOURCE_H__

#include "main.h"

#define POWER_STEEP 5        // 功率调节步进

typedef enum
{
    SRC_QC2 = 1,
    SRC_QC3,
    SRC_FCP,
    SRC_SCP,
    SRC_FIX,
    SRC_PPS,
    SRC_PE1,
    SRC_PE2,
    SRC_SFCP,
    SRC_AFC,
} PROTOCOL;

typedef enum
{
    IDLE = 1,
    SING_MODE,					// 单口模式
    MULT_MODE,					// 多口模式
    TURN_MODE,					// 分配功率
} PORT_STA;

typedef struct
{
    // FLAG
    uint8_t State;                 // 状态机状态
    uint8_t NexState;              // 状态机下一状态
    uint8_t PortChange;            // 端口状态变化标志位
    uint8_t currPortSta;           // 当前端口状态      1:C1 2:C2 3:C1C2
    uint8_t OldPortSta;            // 上一次端口状态
    uint8_t ToMultSta;             // 切换到多端口状态
    uint8_t NotifySta;             // 蓝牙上报标志位
    uint8_t NTCSta;                // NTC状态
    uint8_t OldNTCSta;             // 上一次NTC状态
    uint8_t FirstIn;               // 第一次进入功率分配标志位
    uint8_t LowSpeedCharg;         // 低速充电标志位
    uint8_t NewNoteBookSta;        // 最新20V设备状态 0:无 1:C1有 2:C2有 3:C1C2都有
    uint8_t NoteBookSta;           // 20V设备状态 0:无 1:C1有 2:C2有 3:C1C2都有
    uint8_t NoteBookStaChange;     // 20V设备状态变化标志位

    // TIMER
    uint32_t Check_TIMER;          // 充电信息读取定时器
    uint32_t PortCheck_TIMER;      // 端口变化检测定时器
    uint32_t PortCheckDelay_COUNT; // 端口变化消抖计数器
    uint32_t PowerCheck_TIMER;     // 功率变化检测定时器
    uint32_t Notify_TIMER;         // 数据蓝牙上传定时器
    uint32_t ADCCheck_TIMER;       // ADC_NTC检测定时器
    uint32_t NTCLowPower_TIMER;    // NTC降功率定时器
    uint32_t SystemI2CCheck_TIMER; // IIC总线异常检测定时器
    uint32_t FirstIn_TIMER;        // 第一次进入功率分配定时器
    uint32_t NoteBookCheck_TIMER;  // 20V设备状态检测定时器
} StateMachine_struct;

typedef struct
{
    uint8_t change_flag; // 调整目标值 标志位,为1时需要调整

    uint8_t port_state;         // C口开关状态
    uint8_t curr_protocol[3];   // [0]当前快充状态,[1]快充电压状态,[2]当前协议
    uint16_t curr_vin;
    uint16_t curr_vout;
    uint16_t curr_iout;
    uint16_t curr_power_pd;
    float curr_power;

    uint8_t targ_protocol; // 目标协议
    uint16_t targ_iout;
    uint16_t targ_vout;
    uint16_t targ_power_pd;
    uint16_t temp_targ_power_pd;
} ChargingInformation_struct;

extern uint8_t ReportedData[20];// 蓝牙上报
extern uint8_t ComdData[3];// 蓝牙接收
extern StateMachine_struct MyPower;
extern ChargingInformation_struct PORT[2];

extern void PowerAllocation_task(void *parm);

#endif

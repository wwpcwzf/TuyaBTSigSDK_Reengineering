/****************************************************************************************
> File Name: lutec_config.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_CONFIG_H
#define _LUTEC_CONFIG_H

#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"

#include "lutec_tick.h"







//--------------------调试数据串口打印------------------------------------------
#define BT_DATA_DEBUG                       1   //打印接收发出蓝牙数据
#define FLASH_DEBUG                         1   //打印存储数据
#define PIR_BROADCAST_DEBUG                 0   //广播联动

//--------------------主灯闪烁使能----------------------------------------------
#define LIGHT_BLINK_ENABLE                  0  //主灯闪烁使能

//--------------------PIR------------------------------------------------------
#define PIR_ENABLE                          1
#define PIR_BROADCAST_SIG_CMD_ENABLE        1  //联动使用SIG命令

//--------------------按键-----------------------------------------------------
#define USER_KEY_ENABLR                     0  //按键功能有效
#define USER_KEY_ACTIVE_LOW                 1  //按键低电平有效

//--------------------EEPROM存储-----------------------------------------------
#define FLASH_SAVE_ENABLE                   1 //数据存储使能

#define EEPROM_START_ADDRESS                0x7E000 //eeprom存储地址


//--------------------默认系统数据----------------------------------------------
#define SWITCH_ONOFF_DEF                     0x02   //开关默认--AUTO
#define PIR_UPDATA_APP_DEF                   0xF1   //感应上报--使能
#define PIR_UPDATA_WIFI_DEF                  0x01   //感应通报--使能
#define PIR_SENSITIVITY_DEF                  0x64   //感应灵敏度--100%
#define LUX_THRESHOLD_DEF                    0x64   //照度阈值--100% - LUX_THRESHOLD_DEF
#define SOMEONE_LIGHT_ON_TIME                10     //有人亮灯时间--10s
#define NO_ONE_LIGHT_ON_TIME                 0     //无人低亮时间--60s




 //------------------设备识别码--------------------------------------------------
//设备名称14bytes // "Camera-Light-0"//摄像灯 "OrdinaryLamp-0"//普通灯  "Battery-Lamp-0"//电池灯   "Heat-Sensor-00"//感应器  "Multi-Socket-0"//插座  "RemoteControl-"//遥控器 
#define DEFICE_NAME                      "Camera-Light-0"
//设备类别: "1"(0x31)普通灯;"2"(0x32)电池灯;"3"(0x33)感应器;"4"(0x34)摄像灯;"5"(0x35)网关;"6"(0x36)插座;"7"(0x37)遥控器
#define DEVICE_CLASSES                        0x34//摄像灯
//硬件版本
#define HARD_VERSION                          0x01
//软件版本   
#define SOFT_VERSION                          0x06
//产品序列号
#define SERIAL_NUMBER                         0x0000000B

/*设备功能
bit31     bit30     bit29    bit28    bit27    bit26    bit25     bit24  
保留      保留      保留      保留     保留     保留     保留      保留 
bit23     bit22     bit21    bit20    bit19    bit18    bit17     bit16  
保留      保留      保留    token使能 亮度调光    摄像   设备温度   环境温度
bit15     bit14     bit13    bit12    bit11    bit10    bit9      bit8   
环境湿度  环境照度    时钟     电池     云台      PIR     WiFi      HSV调光
bit7      bit6      bit5     bit4     bit3     bit2     bit1      bit0
RGB调光   CW调光    情景      开关     重启      报警     更新      地址分配  */
//摄像灯 0x001C4658---0b0000 0000 0001 1100 0100 0110 0101 1000 ---觅睿摄像头灯SOC-CW
//#define      DEVICE_FUNTICON                  0x001C4658 
//摄像灯 0x001C4658---0b0000 0000 0001 1100 0100 0110 0001 1000 ---觅睿摄像头灯SOC-C
#define      DEVICE_FUNTICON                  0x001C4618 




//---------------------------------------------------------------功能定义
#define      RESET_KEY_PRESS_TIME              500  //重置长按时间5s--单位10ms

#define      MESH_CONFIG_TIME                  10   //重置后允许蓝牙配网时间10min--单位分钟



//---------------------------------------------------------------wifi
#define      WIFI_CONNECT_INQUIRE_TIME         1000  //wifi连接查询周期10s--单位10ms
#define      WIFI_CMD_SEND_RETRY_TIME          50    //指令重发周期500ms--单位10ms
#define      WIFI_RESEND_MAX_NUM               3     //重发次数

#define      PIR_SOMEBODY_UPATA_TIME           900   //PIR感应有人上报间隔5s--单位10ms  
#define      PIR_LIGHT_CONTROL_TIME            550   //PIR感应灯控组播间隔5.5s--单位10ms  







typedef struct {
    u16 f_save_flag;
    u8 f_devicce_state;
    u8 f_onoff;
    u8 f_pir_up_en;
    u8 f_pir_inform_en;
    u8 f_pir_sensitivity;
    u8 f_lux_threshold;
    u32 f_noone_time;
    u8 f_noone_dim_p[11];
    u32 f_someone_time;
    u8 f_someone_dim_p[11];
}LUTEC_DATA_FLASH_T;





//--------------------------------------------------------------常用功能
u8 lutec_check_sum(u8* data, u8 len);

u8 lutec_string_compare(u8* str1_ptr, u8* str2_ptr, u8 str_len);

void lutec_string_copy(u8* str_to, u8* str_from, u8 str_l);



#endif

/***************************************File end********************************************/




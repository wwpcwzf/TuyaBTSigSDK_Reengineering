/****************************************************************************************
> File Name: lutec_bt_dp.c
> Description: 
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/

#include "lutec_bt_dp.h"

#include "lutec_main.h"
#include "lutec_wifi.h"
#include "lutec_tick.h"

#include "hal_uart.h"
#include "app_common.h"
#include "app_light_cmd.h"

/*-------------------------------------------------------------------------
*简  介: 自定义蓝牙数据点处理
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_bluetooth_dp_data(u16 s_addr, u16 d_addr, u8 *par, int par_len)
{
#if 0
    hal_uart_send(par, par_len);
#endif
    u8 len_buf = (u8)par_len;
    if(len_buf <= 7)//数据包长度不正确
    {
        return;
    }
    //0x01远程下行---解析+转发
    //0x02远程上行---WiFi推送
    //0x03本地下行---解析+转发
    //0x04本地上行---不应收到此类数据帧
    //0x06转发---解析
    
    
    if(par[4] == 0x04)//不应收到此类数据帧
    {
        return;
    }

    if(((par[4] == 0x01) || (par[4] == 0x03)) && (d_addr >= 0xc000))
    {   
        lutec_ack_by_bt(par[4], par[1], s_addr);
        par[4] = 0x06;//转发
        app_light_vendor_data_publish(d_addr, par, len_buf);
    }

    if(par[4] == 0x02)//WiFi推送远程回复
    {
        lutec_updata_by_wifi(par, len_buf);
    }
	else //解析0x01、0x03、0x06
    { 
        uint8_t ack_len = 0;
        ack_len = lutec_protocol_dp_analysis(&par[1]);//指令解析
        if(ack_len > 6)//回复 （dpID*1 数据类型*1 长度*1 指向*1 地址*2）== 6
        {
            par[0] = 0x01;
            app_light_vendor_data_publish(s_addr, par, ack_len + 1);
        }
    }	
}


/*-------------------------------------------------------------------------
*简  介: 自定义蓝牙数据点指令解析
*参  数: 参数指针[dpID | 数据类型 | 指令长度 | 指向 | 地址 | 指令参数]
         下标     0        1         2       3      4 5   6.
*返回值: 要回复的数据存储到参数指针指向处；
*       返回回复参数的长度 > 6，反之不需要回复。
-------------------------------------------------------------------------*/
u8 lutec_protocol_dp_analysis(u8 *par)
{
    u8 return_num = 0;
    u16 addr_buf = ((uint16_t)par[4] << 8) + par[5];

    switch(par[0])
    {
        case 0x65://开关
            lutec_light_switch(par[6]);
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                par[6] = lutec_get_switch_flag();
                return_num = 7;
            }
            break;
        case 0x66://灯光调节
            lutec_light_dimmer(&par[6]);
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_dimming_para(&par[6]) + 6;
                par[2] = return_num - 3;
            }    
            break;
        case 0x6A://延时调光
            hal_uart_send(par, par[2] + 3);
            lutec_light_delay_dimmer(&par[6]);
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_delay_dimming_para(&par[6]) + 6;
                par[2] = return_num - 3;
            }            
            hal_uart_send(par, par[2] + 3);
            break;
        case 0x6B://wifi配网
            //sys_execution_0x6B();
            break;
        case 0x6C://重启
            //sys_execution_0x6C();
            break;
        case 0x6D://感应器配置
            //sys_execution_0x6D();
            break;
        case 0x6E://感应灯控设置
            //sys_execution_0x6E();
            break;
        case 0x6F://感应查询、上报及设置
            //sys_execution_0x6F();
            break;
        case 0x70://云台控制
            //sys_ack_0x7C(0x03);//指令未实现
            break;
        case 0x71://时钟同步
            //sys_execution_0x71();
            break;
        case 0x72://电源信息   
            //sys_execution_0x72();
            break;
        case 0x73://环境照度    
            //sys_execution_0x73();
            break;
        case 0x74://环境湿度
            //sys_ack_0x7C(0x03);//指令未实现
            break;
        case 0x75://环境温度
            //sys_ack_0x7C(0x03);//指令未实现
            break;
        case 0x76://设备温度
            //sys_ack_0x7C(0x03);//指令未实现
            break;
        case 0x77://情景模式
            //sys_execution_0x77();
            break;
        case 0x78://设备状态
            //sys_execution_0x78();
            break;
        case 0x79://报警
            //sys_execution_0x79();
            break;
        case 0x7A://产测
            //sys_execution_0x7A();
            break;
        case 0x7B://更新固件
            //sys_execution_0x7B();
            break;
        //case 0x7C://应答
        //break;
        case 0x7D://注册指令（设置地址，分组）
            //sys_execution_0x7D();
            break;
        case 0x7E://设备识别号
            //sys_execution_0x7E();
            break;
        case 0x7F://wifi状态
            //sys_execution_0x7F();
            break;
        case 0x80: //wifi模块信息
            //sys_execution_0x80();
            break;
        default:
        break;
    }

    return return_num;
}





/*-------------------------------------------------------------------------
*简  介: 本地[通过蓝牙]简单回复
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_ack_by_bt(uint8_t point_to, uint8_t ack_id, uint16_t sent_to_addr)
{
    uint8_t data_buffer[9] = {0};

    data_buffer[0]  = 0x01;
    data_buffer[1]  = 0x7C;
    data_buffer[2]  = 0x00;
    data_buffer[3]  = 0x05;
    data_buffer[4]  = point_to + 1; 
    u16 addrbuf = lutec_get_address();
    data_buffer[5] = (addrbuf >> 8) & 0xFF;
    data_buffer[6] = (addrbuf >> 0) & 0xFF;
    data_buffer[7]  = ack_id;
    data_buffer[8]  = 0x06;

    app_light_vendor_data_publish(sent_to_addr, data_buffer, 9);
}

static u8 switch_flag = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_switch(u8 switch_v)
{
    switch(switch_v) 
    {
    case 0:
        app_light_ctrl_data_switch_set(0);
        app_light_ctrl_data_countdown_set(0);
        switch_flag = 0;
        break;
    case 1:
        app_light_ctrl_data_switch_set(1);
        app_light_ctrl_data_countdown_set(0);
        switch_flag = 1;
        break;
    //case 2:
        //app_light_ctrl_data_switch_set(1);
        //app_light_ctrl_data_countdown_set(3000);
        //break;
    default:
        app_light_ctrl_data_switch_set(1);
        app_light_ctrl_data_countdown_set(3);
        switch_flag = 2;
        break;
    }
    //app_light_ctrl_data_auto_save_start(5000);
    app_light_ctrl_proc();
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_switch_flag(void)
{
    return switch_flag;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_dimmer(u8* para_ptr)
{
    u16 buffer16_v = 0;

    switch(para_ptr[0] > 0x40 ? (para_ptr[0] & 0x0F) + ((para_ptr[3] & 0xF0) >> 2) : para_ptr[0])
    {
    case 0x11://色温调光
        buffer16_v = ((u16)para_ptr[1] << 8) + para_ptr[2];
        if(buffer16_v == 0)
        {
            app_light_ctrl_data_switch_set(0);
            app_light_ctrl_data_countdown_set(0);
        }
        else
        {
            if(buffer16_v > 1000)  buffer16_v = 1000;
            app_light_ctrl_data_switch_set(1);
            app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_temperature_set(buffer16_v);
        }
        break;
    case 0x12://亮度调光    
        buffer16_v = ((u16)para_ptr[1] << 8) + para_ptr[2];
        if(buffer16_v == 0)
        {
            app_light_ctrl_data_switch_set(0);
            app_light_ctrl_data_countdown_set(0);
        }
        else
        {
            if(buffer16_v > 1000)  buffer16_v = 1000;
            app_light_ctrl_data_switch_set(1);
            app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_bright_set(buffer16_v);
        }    
        break;
    case 0x13://亮度色温调光
        //app_light_ctrl_data_mode_set(WHITE_MODE);
        buffer16_v = ((u16)para_ptr[1] << 8) + para_ptr[2];
        if(buffer16_v == 0)
        {
            app_light_ctrl_data_switch_set(0);
            app_light_ctrl_data_countdown_set(0);
        }
        else
        {
            if(buffer16_v > 1000)  buffer16_v = 1000;
            app_light_ctrl_data_switch_set(1);
            app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_bright_set(buffer16_v);
            buffer16_v = ((u16)para_ptr[3] << 8) + para_ptr[4];
            if(buffer16_v > 1000) buffer16_v = 1000;
            app_light_ctrl_data_temperature_set(buffer16_v);
        }        
        break;
    case 0x1A://HSV调光
        break;
    case 0x1B://RGB调光                
        break;    
    case 0x1C://关灯
        app_light_ctrl_data_switch_set(0);
        app_light_ctrl_data_countdown_set(0);
    break;
    case 0x1D://开灯   
        app_light_ctrl_data_switch_set(0);
        app_light_ctrl_data_countdown_set(0);        
    break;
    case 0x2A://亮度色温+HSV调光
    break;                
    case 0x2B://亮度色温+RGB调光
    break;
    default:
    break;
    }
    app_light_ctrl_proc();
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_dimming_para(u8* get_ptr)
{
    u16 buffer16_v = 0;

    switch(get_ptr[0])
    {
    case 0x01://色温调光
        buffer16_v = app_light_ctrl_data_temperature_get();
        get_ptr[1] = (u8)(buffer16_v >> 8);    
        get_ptr[2] = (u8)(buffer16_v >> 0);
        return 3;
    case 0x02://亮度调光
        buffer16_v = app_light_ctrl_data_bright_get();
        get_ptr[1] = (u8)(buffer16_v >> 8);    
        get_ptr[2] = (u8)(buffer16_v >> 0);
        return 3;
    case 0x13://亮度色温调光    
        buffer16_v = app_light_ctrl_data_bright_get();
        get_ptr[1] = (u8)(buffer16_v >> 8);    
        get_ptr[2] = (u8)(buffer16_v >> 0);
        buffer16_v = app_light_ctrl_data_temperature_get();
        get_ptr[3] = (u8)(buffer16_v >> 8);    
        get_ptr[4] = (u8)(buffer16_v >> 0);
        return 5;
    case 0x0C://关灯
        return 0;
    case 0x0D://开灯
        return 0;
    //case 0x0A://HSV调光
        //return 0;
    //case 0x0B://RGB调光
        //return 0;
    //case 0x0A://亮度色温+HSV调光
        //return 0;
    //case 0x0B://亮度色温+RGB调光
        //return 0;
    default:   
        get_ptr[0] = 0x13;
        buffer16_v = app_light_ctrl_data_bright_get();
        get_ptr[1] = (u8)(buffer16_v >> 8);    
        get_ptr[2] = (u8)(buffer16_v >> 0);
        buffer16_v = app_light_ctrl_data_temperature_get();
        get_ptr[3] = (u8)(buffer16_v >> 8);    
        get_ptr[4] = (u8)(buffer16_v >> 0);
        return 5;
    }
}

static u32 delay_time = 0;
static u8 delay_target_para[11] = {0};
static u32 delay_time_base = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_delay_dim_loop(void)
{


}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_dim_para_len(u8 dim_flag)
{
    switch(dim_flag)
    {
        case 0x11:
        case 0x12:
        case 0x41:
        case 0x42:
            return 3;
        case 0x13:
        case 0x43:
            return 5;
        case 0x1A:
        case 0x1B:
        case 0x4A:
        case 0x4B:
            return 7;
        case 0x2A:
        case 0x2B:
        case 0x8A:
        case 0x8B:
            return 11;
        case 0x1C:
        case 0x1D:
        case 0x4C:
        case 0x4D:
        default:
            return 1;
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_delay_dimmer(u8* para_p)
{
    //延时时间
    delay_time = ((u32)para_p[0] << 16) + ((u32)para_p[1] << 8) + para_p[2];
    if(delay_time > 86400)
    {
        delay_time = 0;
    }
    
    //保存参数
    if(para_p[3] > 0x40)//先等待再调光
    {
        u8 para_l = lutec_get_dim_para_len(para_p[3]);
        for(u8 i = 0; i < para_l; i++)
        {
            delay_target_para[i] = para_p[3 + i];
        }
    }
    else //先调光在等待关灯
    {
        lutec_light_dimmer(&para_p[3]);
        delay_target_para[0] = 0x1C;//关灯
    }

    //延时时基
    if((para_p[3] == 0x4C) || (para_p[3] == 0x4D))
    {
        app_light_ctrl_data_countdown_set(delay_time);
        delay_time_base = 0;
    } 
    else
    {
        delay_time_base = lutec_get_tick_10ms();
    }   
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_delay_dimming_para(u8* get_p)
{

    return lutec_get_dim_para_len(get_p[3]);
}


















/***************************************File end********************************************/


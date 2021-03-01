/****************************************************************************************
> File Name: lutec_bt_dp.c
> Description: 
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/

#include "lutec_bt_dp.h"

#include "lutec_config.h"

#include "lutec_main.h"
#include "lutec_wifi.h"
#include "lutec_tick.h"

#include "lutec_pir.h"
#include "lutec_lux.h"

#include "hal_uart.h"
#include "app_common.h"
#include "app_light_cmd.h"
#include "app_light_control.h"

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
            if(par[2] == 4)
            {
                lutec_light_switch(par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                par[6] = lutec_get_switch_flag();
                return_num = 7;
            }
            break;
        case 0x66://灯光调节            
            if(par[2] >= 6)
            {
                lutec_light_dimmer(&par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_dimming_para(&par[6]) + 6;//[dpID | 数据类型 | 指令长度 | 指向 | 地址]6Bytes
                par[2] = return_num - 3;
            }    
            break;
        case 0x6A://延时调光
            if(par[2] >= 7)
            {
                lutec_light_delay_dimmer(&par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_delay_dimming_para(&par[6]) + 9;//时间3bytes
                par[2] = return_num - 3;
            }           
            break;
        case 0x6B://wifi配网            
            if((par[6] < 33) && (par[6] > 0) && (par[2] > 11))//33 > ssid长度(1~32) > 0,密码 > 8; 8+3=11
            {
                lutec_light_config_wifi(&par[6], par[2] - 3);
                lutec_set_wifi_config_flag(2);
            }
            else
            {
                lutec_set_wifi_config_flag(1);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_wifi_para(&par[6]) + 6;
                par[2] = return_num - 3;
            }  
            break;
        case 0x6C://重启control
            if(par[2] == 4)
            {
                lutec_light_reset_control(par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_reset_state(&par[6]) + 6;
                par[2] = return_num - 3;
            }        
            break;
        case 0x6D://感应器配置         
            if(par[2] == 6)
            {
                lutec_pir_config(&par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_pir_para(&par[6]) + 6;
                par[2] = return_num - 3;
            }           
            break;
        case 0x6E://感应灯控设置            
            if(par[2] == 32) //
            {
                lutec_pir_light_control_set(&par[6]);
            }
            else
            {
                if(par[6] == 0x00)//关PIR
                {
                    //未处理 ？
                }    
            }            
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_pir_light_control_para(&par[6]) + 6;
                par[2] = return_num - 3;
            } 
            break;
        case 0x6F://感应查询、上报及设置
            if(par[2] == 4)
            {
                lutec_pir_updata_set(par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_pir_updata_conf(&par[6]) + 6;
                par[2] = return_num - 3;
            }                 
            break;
        case 0x70://云台控制
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x70;//云台控制
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x71://时钟同步            
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x71;//时钟同步 
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x72://电源信息                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x72;//电源信息   
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x73://环境照度 
            hal_uart_send(par, par[2] + 3); 
            if(par[2] == 5)
            {
                lutec_env_illum_set(&par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_env_illum_conf(&par[6]) + 6;
                par[2] = return_num - 3;
            }                 
            hal_uart_send(par, par[2] + 3);
            break;
        case 0x74://环境湿度                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x74;// 
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x75://环境温度                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x75;//
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x76://设备温度                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x76;
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x77://情景模式                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x77;// 
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x78://设备状态                      
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x78;// 
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x79://报警                       
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x79;//  
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
            break;
        case 0x7A://产测                      
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x7A;// 
                par[7]  = 0x03;//未实现A
                return_num = 8; 
            }
            break;
        case 0x7B://更新固件                      
            if(addr_buf < 0xC000)
            {
                par[0]  = 0x7C;//ACK回复
                par[2]  = 5;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = 0x7B;//  
                par[7]  = 0x03;//未实现
                return_num = 8; 
            }
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

    switch(para_ptr[0] > 0x40 ? (para_ptr[0] & 0x0F) + ((para_ptr[0] & 0xF0) >> 2) : para_ptr[0])
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
        app_light_ctrl_data_switch_set(1);
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
    case 0x11://色温调光
        buffer16_v = app_light_ctrl_data_temperature_get();
        get_ptr[1] = (u8)(buffer16_v >> 8);    
        get_ptr[2] = (u8)(buffer16_v >> 0);
        return 3;
    case 0x12://亮度调光
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
    if(delay_time_base == 0)
        return;
    if(lutec_get_interval_tick_10ms(delay_time_base) > (delay_time * 100))
    {
        lutec_light_dimmer(delay_target_para);
        delay_time_base = 0;
    }
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_dim_para_len(u8 dim_flag)
{
    hal_uart_send(&dim_flag, 1);
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
    //------------获取参数--未处理？

    return lutec_get_dim_para_len(get_p[3]);
}

static u8 wifi_ssid_psw[99] = {0}; //总长度[1]|ssid长度[1]|ssid[1~32]|password[8~64]|和校验[1]
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_config_wifi(u8* para_p, u8 para_l)
{
    if((para_p[0] == wifi_ssid_psw[1]) && (para_l == wifi_ssid_psw[0]))//长度相同
    {
        if(lutec_string_compare(&para_p[1], &wifi_ssid_psw[2], para_l))//信息相同
        {
            return;
        }
    }
    if(para_l < (para_p[0] + 8))//密码长度大于8
    {
        return;
    }
    // 更新wifi信息
    for(u8 sub_i = 0; sub_i < para_l; sub_i++)
    {
        wifi_ssid_psw[1 + sub_i] = para_p[sub_i];
    }
    wifi_ssid_psw[0] = para_l;//总长
    wifi_ssid_psw[para_l + 1] = lutec_check_sum(wifi_ssid_psw, wifi_ssid_psw[0]);
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_wifi_para(u8* para_g)
{
    if((wifi_ssid_psw[0] < (wifi_ssid_psw[1] + 8)) || (wifi_ssid_psw[wifi_ssid_psw[0] + 1] != lutec_check_sum(wifi_ssid_psw, wifi_ssid_psw[0])))
    {
        para_g[0] = 0xFE;//暂无数据
        return 1;
    }

    for(u8 sub_i = 0; sub_i <= wifi_ssid_psw[0]; sub_i++)
    {
        para_g[sub_i] = wifi_ssid_psw[1 + sub_i];
    }

    return wifi_ssid_psw[0];
}
/*
0xF1: MCU重启
0xF2: 恢复出厂设置
0xF3: 蓝牙重置
0xF4: 摄像模块重置
//--------------------------
0xF7: MCU重启完成
0xF8: 恢复出厂设置成功
0xF9: 蓝牙重置中
0xFA: 蓝牙重置成功
0xFB: 摄像模块重置中
0xFC: 摄像模块重置成功
*/
static u8 reset_state = 0;//
static u32 reset_time_base = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_reset_control(u8 rset_cm)
{
    switch(rset_cm)
    {
        case 0xF1://MCU重启 细节未处理？
        //break;
        case 0xF2://恢复出厂设置
        // kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;  
        // lutec_reset_wifi_module();//重置WiFi模块
        // break;
        case 0xF3://蓝牙重置
        //kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;  
        reset_time_base = lutec_get_tick_10ms;
        reset_state = 0xF2;
        break;
        case 0xF4://重置WiFi模块
        lutec_reset_wifi_module();
        break;
        default:
        break;
    }
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_device_reset(void)
{
    if(reset_time_base == 0)
    {
        return;
    }
    if(lutec_get_interval_tick_10ms(reset_time_base) > 300)
    { 
        reset_time_base = 0;
        kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT; 
    }
}
 /*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/ 
 u8 lutec_get_reset_state(u8* sv_ptr)
 {
     sv_ptr[0] = reset_state;
     return 1;
 }
 
 
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_reset_state(u8 rset_s)
{

}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_config(u8* para_p)
{
    u16 buf16 = ((u16)para_p[0] << 8) + para_p[1];
    if(buf16 > 100)
    {
        return;
    }
    lutec_pir_set_sensitivity((u8)buf16);
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_pir_para(u8* g_ptr)
{
    g_ptr[0] = 0;
    g_ptr[1] = lutec_pir_get_sensitivity();
    g_ptr[2] = 0;
    return 3;
}

static u32 pir_noone_delay = 0;
static u8 noone_pir_light_par[11] = {0x1C,0,0,0,0,0,0,0,0,0,0};
static u32 pir_anyone_delay = 10000;
static u8 anyone_pir_light_par[11] = {0x1D,0,0,0,0,0,0,0,0,0,0};
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_light_control_set(u8* para_p)
{
    u32 buffer32_v = 0;
    u8 buf8_v = 0;
    u8 sub8_i = 0;
    //pir无人灯控参数
	buffer32_v = (uint32_t)(((u32)para_p[1] << 16) + ((u16)para_p[2] << 8) + para_p[3]);//时间
	if((buffer32_v <= 86400) && ((para_p[0] & 0x80) == 0x80))
	{
		pir_noone_delay = buffer32_v;
    }
    else
    {
        pir_noone_delay = 0; 
    }
    buf8_v = 0;
    sub8_i = 1;
    buffer32_v = ((u16)para_p[4] << 8) + para_p[5];
    if((buffer32_v <= 1000) && ((para_p[0] & 0x40) == 0x40))//pir无人亮度
    {
        noone_pir_light_par[1] = para_p[4];
        noone_pir_light_par[2] = para_p[5];
        buf8_v |= 0x12;
    }
    else
    {
        noone_pir_light_par[1] = 0;
        noone_pir_light_par[2] = 0;
    }
    buffer32_v = ((u16)para_p[6] << 8) + para_p[7];
    if((buffer32_v <= 1000) && ((para_p[0] & 0x20) == 0x20))//pir无人色温
    {
        noone_pir_light_par[3] = para_p[6];
        noone_pir_light_par[4] = para_p[7];
        buf8_v |= 0x11;
    }
    else
    {
        noone_pir_light_par[3] = 0;
        noone_pir_light_par[4] = 0;
    }
    if(((para_p[0] & 0x10) == 0x10) && ((para_p[8] == 0x1A) || (para_p[8] == 0x1B)))//pir无人采光 未处理？
    {
        if(buf8_v == 0x13)
        {
            buf8_v = 0x20 + (para_p[8] & 0x0F);
            noone_pir_light_par[5] = para_p[9];
            noone_pir_light_par[6] = para_p[10];
            noone_pir_light_par[7] = para_p[11];
            noone_pir_light_par[8] = para_p[12];
            noone_pir_light_par[9] = para_p[13];
            noone_pir_light_par[10] = para_p[14];
        } 
        else
        {
            buf8_v = para_p[8];
            noone_pir_light_par[1] = 0;
            noone_pir_light_par[2] = 0;
            noone_pir_light_par[3] = 0;
            noone_pir_light_par[4] = 0;
            noone_pir_light_par[5] = 0;
            noone_pir_light_par[6] = 0;
        }
    }    
    else
    {
        noone_pir_light_par[5] = 0;
        noone_pir_light_par[6] = 0;
        noone_pir_light_par[7] = 0;
        noone_pir_light_par[8] = 0;
        noone_pir_light_par[9] = 0;
        noone_pir_light_par[10] = 0;
    }
    noone_pir_light_par[0] = buf8_v == 0 ? 0x1C : buf8_v;//无人调光标记

    //有人灯控参数
	buffer32_v = (uint32_t)(((u32)para_p[15] << 16) + ((u16)para_p[16] << 8) + para_p[17]);//时间
	if((buffer32_v <= 86400) && ((para_p[0] & 0x08) == 0x08))
	{
		pir_anyone_delay = buffer32_v;
    }
    else
    {
        pir_anyone_delay = 0; 
    }
    buf8_v = 0;
    sub8_i = 1;
    buffer32_v = ((u16)para_p[4] << 8) + para_p[5];
    if((buffer32_v <= 1000) && ((para_p[0] & 0x04) == 0x04))//pir无人亮度
    {
        anyone_pir_light_par[1] = para_p[18];
        anyone_pir_light_par[2] = para_p[19];
        buf8_v |= 0x12;
    }
    else
    {
        anyone_pir_light_par[1] = 0;
        noone_pir_light_par[2] = 0;
    }
    buffer32_v = ((u16)para_p[6] << 8) + para_p[7];
    if((buffer32_v <= 1000) && ((para_p[0] & 0x02) == 0x02))//pir无人色温
    {
        anyone_pir_light_par[3] = para_p[20];
        anyone_pir_light_par[4] = para_p[21];
        buf8_v |= 0x11;
    }
    else
    {
        anyone_pir_light_par[3] = 0;
        anyone_pir_light_par[4] = 0;
    }
    if(((para_p[0] & 0x01) == 0x01) && ((para_p[22] == 0x1A) || (para_p[22] == 0x1B)))//pir无人采光 未处理？
    {
        if(buf8_v == 0x13)
        {
            buf8_v = 0x20 + (para_p[8] & 0x0F);
            anyone_pir_light_par[5] = para_p[23];
            anyone_pir_light_par[6] = para_p[24];
            anyone_pir_light_par[7] = para_p[25];
            anyone_pir_light_par[8] = para_p[26];
            anyone_pir_light_par[9] = para_p[27];
            anyone_pir_light_par[10] = para_p[28];
        } 
        else
        {
            buf8_v = para_p[8];
            anyone_pir_light_par[1] = 0;
            anyone_pir_light_par[2] = 0;
            anyone_pir_light_par[3] = 0;
            anyone_pir_light_par[4] = 0;
            anyone_pir_light_par[5] = 0;
            anyone_pir_light_par[6] = 0;
        }
    }    
    else
    {
        anyone_pir_light_par[5] = 0;
        anyone_pir_light_par[6] = 0;
        anyone_pir_light_par[7] = 0;
        anyone_pir_light_par[8] = 0;
        anyone_pir_light_par[9] = 0;
        anyone_pir_light_par[10] = 0;
    }
    anyone_pir_light_par[0] = buf8_v == 0 ? 0x1C : buf8_v;//调光标记
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_pir_light_control_para(u8* s_ptr)
{
    u8 falg_v = 0;
    u8 sub_i = 0;
    //无人参数    
	if(pir_noone_delay != 0)
	{
		falg_v |= 0x80;
		s_ptr[1] = (uint8_t)(pir_noone_delay >> 16);
		s_ptr[2] = (uint8_t)(pir_noone_delay >> 8);
		s_ptr[3] = (uint8_t)(pir_noone_delay >> 0);
	}
	else
	{
		s_ptr[1] = 0;
		s_ptr[2] = 0;
		s_ptr[3] = 0;
	}
	switch(noone_pir_light_par[0])
	{
		case 0x11://色温
			falg_v |= 0x20;
			s_ptr[4] = 0;			
			s_ptr[5] = 0;
			s_ptr[6] = noone_pir_light_par[1];			
			s_ptr[7] = noone_pir_light_par[2];	
			for(sub_i = 8; sub_i < 15; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x12://亮度
			falg_v |= 0x40;
			s_ptr[4] = noone_pir_light_par[1];			
			s_ptr[5] = noone_pir_light_par[2];			
			for(sub_i = 6; sub_i < 15; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x13://亮度、色温
			falg_v |= 0x60;		
			s_ptr[4] = noone_pir_light_par[1];			
			s_ptr[5] = noone_pir_light_par[2];	
			s_ptr[6] = noone_pir_light_par[3];			
			s_ptr[7] = noone_pir_light_par[4];		
			for(sub_i = 8; sub_i < 15; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x1A://HSV彩光
			falg_v |= 0x10;		
			for(sub_i = 4; sub_i < 8; sub_i++)
				s_ptr[sub_i] = 0;		
			s_ptr[8] = 0x1A;			
			s_ptr[9] = noone_pir_light_par[1];			
			s_ptr[10] = noone_pir_light_par[2];	
			s_ptr[11] = noone_pir_light_par[3];			
			s_ptr[12] = noone_pir_light_par[4];		
			s_ptr[13] = noone_pir_light_par[5];			
			s_ptr[14] = noone_pir_light_par[6];	
			break;
		case 0x1B://RGB彩光			
			falg_v |= 0x10;		
			for(sub_i = 4; sub_i < 8; sub_i++)
				s_ptr[sub_i] = 0;		
			s_ptr[8] = 0x1B;			
			s_ptr[9] = noone_pir_light_par[1];			
			s_ptr[10] = noone_pir_light_par[2];	
			s_ptr[11] = noone_pir_light_par[3];			
			s_ptr[12] = noone_pir_light_par[4];		
			s_ptr[13] = noone_pir_light_par[5];			
			s_ptr[14] = noone_pir_light_par[6];	
			break;
//		case 0x1C://关灯

//			break;
//		case 0x1D://开灯

//			break;
		case 0x2A: //亮度、色温、HSV彩光
			falg_v |= 0x70;		
			s_ptr[4] = noone_pir_light_par[1];			
			s_ptr[5] = noone_pir_light_par[2];	
			s_ptr[6] = noone_pir_light_par[3];			
			s_ptr[7] = noone_pir_light_par[4];	
			s_ptr[8] = 0x1A;			
			s_ptr[9] = noone_pir_light_par[5];			
			s_ptr[10] = noone_pir_light_par[6];	
			s_ptr[11] = noone_pir_light_par[7];			
			s_ptr[12] = noone_pir_light_par[8];		
			s_ptr[13] = noone_pir_light_par[9];			
			s_ptr[14] = noone_pir_light_par[10];
			break;
		case 0x2B://亮度、色温、RGB彩光
			falg_v |= 0x70;		
			s_ptr[4] = noone_pir_light_par[1];			
			s_ptr[5] = noone_pir_light_par[2];	
			s_ptr[6] = noone_pir_light_par[3];			
			s_ptr[7] = noone_pir_light_par[4];	
			s_ptr[8] = 0x1B;			
			s_ptr[9] = noone_pir_light_par[5];			
			s_ptr[10] = noone_pir_light_par[6];	
			s_ptr[11] = noone_pir_light_par[7];			
			s_ptr[12] = noone_pir_light_par[8];		
			s_ptr[13] = noone_pir_light_par[9];			
			s_ptr[14] = noone_pir_light_par[10];
			break;
		default:
			for(sub_i = 3; sub_i < 29; sub_i++)
				s_ptr[sub_i] = 0;	
			break;	
	}
    //有人参数
    if(pir_anyone_delay != 0)
	{
		falg_v |= 0x08;
		s_ptr[15] = (u8)(pir_anyone_delay >> 16);
		s_ptr[16] = (u8)(pir_anyone_delay >> 8);
		s_ptr[17] = (u8)(pir_anyone_delay >> 0);
	}
	else
	{
		s_ptr[15] = 0;
		s_ptr[16] = 0;
		s_ptr[17] = 0;
	}
	switch(anyone_pir_light_par[0])
	{
		case 0x11://色温
			falg_v |= 0x02;
			s_ptr[18] = 0;			
			s_ptr[19] = 0;
			s_ptr[20] = anyone_pir_light_par[1];			
			s_ptr[21] = anyone_pir_light_par[2];	
			for(sub_i = 22; sub_i < 29; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x12://亮度
			falg_v |= 0x04;
			s_ptr[18] = anyone_pir_light_par[1];			
			s_ptr[19] = anyone_pir_light_par[2];			
			for(sub_i = 20; sub_i < 29; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x13://亮度、色温
			falg_v |= 0x06;		
			s_ptr[18] = anyone_pir_light_par[1];			
			s_ptr[19] = anyone_pir_light_par[2];	
			s_ptr[20] = anyone_pir_light_par[3];			
			s_ptr[21] = anyone_pir_light_par[4];		
			for(sub_i = 22; sub_i < 29; sub_i++)
				s_ptr[sub_i] = 0;
			break;
		case 0x1A://HSV彩光
			falg_v |= 0x01;		
			for(sub_i = 18; sub_i < 22; sub_i++)
				s_ptr[sub_i] = 0;		
			s_ptr[22] = 0x1A;			
			s_ptr[23] = anyone_pir_light_par[1];			
			s_ptr[24] = anyone_pir_light_par[2];	
			s_ptr[25] = anyone_pir_light_par[3];			
			s_ptr[26] = anyone_pir_light_par[4];		
			s_ptr[27] = anyone_pir_light_par[5];			
			s_ptr[28] = anyone_pir_light_par[6];	
			break;
		case 0x1B://RGB彩光			
			falg_v |= 0x01;		
			for(sub_i = 18; sub_i < 22; sub_i++)
				s_ptr[sub_i] = 0;		
			s_ptr[22] = 0x1B;			
			s_ptr[23] = anyone_pir_light_par[1];			
			s_ptr[24] = anyone_pir_light_par[2];	
			s_ptr[25] = anyone_pir_light_par[3];			
			s_ptr[26] = anyone_pir_light_par[4];		
			s_ptr[27] = anyone_pir_light_par[5];			
			s_ptr[28] = anyone_pir_light_par[6];	
			break;
//		case 0x1C://关灯

//			break;
//		case 0x1D://开灯

//			break;
		case 0x2A: //亮度、色温、HSV彩光
			falg_v |= 0x07;		
			s_ptr[18] = anyone_pir_light_par[1];			
			s_ptr[19] = anyone_pir_light_par[2];	
			s_ptr[20] = anyone_pir_light_par[3];			
			s_ptr[21] = anyone_pir_light_par[4];	
			s_ptr[22] = 0x1A;			
			s_ptr[23] = anyone_pir_light_par[5];			
			s_ptr[24] = anyone_pir_light_par[6];	
			s_ptr[25] = anyone_pir_light_par[7];			
			s_ptr[26] = anyone_pir_light_par[8];		
			s_ptr[27] = anyone_pir_light_par[9];			
			s_ptr[28] = anyone_pir_light_par[10];
			break;
		case 0x2B://亮度、色温、RGB彩光
			falg_v |= 0x07;		
			s_ptr[18] = anyone_pir_light_par[1];			
			s_ptr[19] = anyone_pir_light_par[2];	
			s_ptr[20] = anyone_pir_light_par[3];			
			s_ptr[21] = anyone_pir_light_par[4];	
			s_ptr[22] = 0x1B;			
			s_ptr[23] = anyone_pir_light_par[5];			
			s_ptr[24] = anyone_pir_light_par[6];	
			s_ptr[25] = anyone_pir_light_par[7];			
			s_ptr[26] = anyone_pir_light_par[8];		
			s_ptr[27] = anyone_pir_light_par[9];			
			s_ptr[28] = anyone_pir_light_par[10];
			break;
		default:
			for(sub_i = 3; sub_i < 29; sub_i++)
				s_ptr[sub_i] = 0;		
			break;	
	}

	s_ptr[0] = falg_v;
    
    return 29;

}



static u8 pir_updata_falg  = 0xF2;//0xF1:上报APP；0xF2：不上报；
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_updata_set(u8 conf_v)
{
    if((conf_v == 0xF1) || (conf_v  == 0xF2))
    {
        pir_updata_falg = conf_v;
    }
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_pir_updata_conf(u8* s_conf)
{
    s_conf[0] = pir_updata_falg;
    return 1;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_pir_updata_is(void)
{
    return pir_updata_falg == 0xF1 ? 1 : 0;
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_env_illum_set(u8* para_p)
{
    u16 buf_va = ((u16)para_p[0] << 8) + para_p[1];
    if(buf_va <= 100)
    {
        lutec_set_lux_threshold((u8)buf_va);
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_env_illum_conf(u8* gs_p) 
{
    gs_p[0] = 0;
    gs_p[1] = lutec_get_lux_threshold();
    return 2;
}





















/***************************************File end********************************************/


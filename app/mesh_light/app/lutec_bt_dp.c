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

#include "lutec_pir.h"
#include "lutec_lux.h"

#include "hal_uart.h"
#include "app_common.h"
#include "app_light_cmd.h"
//#include "app_light_cmd.h"
#include "app_light_control.h"

/*-------------------------------------------------------------------------
*简  介: 自定义蓝牙数据点处理
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_bluetooth_dp_data(u16 s_addr, u16 d_addr, u8 *par, int par_len)
{
// #if BT_DATA_DEBUG
//     hal_uart_send(par, par_len);
// #endif
    u8 len_buf = (u8)par_len;
    if(len_buf <= 7)//数据包长度不正确
    {
        return;
    }
    //0x01远程下行---解析+转发
    //0x02远程上行---WiFi推送
    //0x03本地下行---解析+转发
    //0x04本地上行---不应收到此类数据帧
    //0x05转发---解析
    //0x06联动---解析    
    if(par[4] == 0x04)//不应收到此类数据帧
    {
        return;
    }

    if(((par[4] == 0x01) || (par[4] == 0x03)) && (d_addr >= 0xc000))
    {   
        lutec_ack_by_bt(par[4], par[1], s_addr);
        par[4] = 0x05;//转发
        app_light_vendor_data_send(d_addr, par, len_buf);
    }

    if(par[4] == 0x02)//WiFi推送远程回复
    {
        lutec_updata_by_wifi(par, len_buf);
    }
	else //解析0x01、0x03、0x05、0x06
    { 
        uint8_t ack_len = 0;
        ack_len = lutec_protocol_dp_analysis(&par[1]);//指令解析
        if(ack_len > 6)//回复 （dpID*1 数据类型*1 长度*1 指向*1 地址*2）== 6
        {
            par[0] = 0x01;            
            par[5] = (u8)(lutec_get_address() >> 8);
            par[6] = (u8)(lutec_get_address());
            app_light_vendor_data_send(s_addr, par, ack_len + 1);

        #if BT_DATA_DEBUG    
            hal_uart_send(par, ack_len + 1);
            //hal_uart_send(&d_addr, 2);
        #endif
        }

        //hal_uart_send(&s_addr, 2);
    }	
}
/*-------------------------------------------------------------------------
*简  介: sig mesh数据点指令解析
*参  数: 参数指针 dpID[1] | 数据类型[1] | 功能长度[2] | 功能指令[n]
         下标     0        1             2       3     4.......
*返回值: 要回复的数据存储到参数指针指向处；
*       返回回复参数的长度 > 3，反之不需要回复。
-------------------------------------------------------------------------*/
u8 lutec_sigmesh_dp_analysis(u8* cmd_ptr, u8 return_flag)
{
    u8 return_num = 0;
    u32 buffer32_v = 0;

    switch(cmd_ptr[0])
    {
    case 0x01://开关
        app_light_ctrl_data_switch_set(cmd_ptr[4]);
        app_light_ctrl_data_countdown_set(0);
        if(return_flag)
        {
            cmd_ptr[4] = app_light_ctrl_data_switch_get();
            return_num = 5;
        }
        break;
    case 0x02://模式
        app_light_ctrl_data_mode_set(cmd_ptr[4]);
        if(return_flag)
        {
            return_num = 5;
        }
        break;
    case 0x03://亮度    
        buffer32_v = cmd_ptr[4];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[5];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[6];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[7];
        if(buffer32_v > 1000)
        {
            buffer32_v = 1000;
        }
        app_light_ctrl_data_bright_set(buffer32_v);
        if(return_flag)
        {
            return_num = 8;
        }
        break;
    case 0x04://冷暖值 
        if(WHITE_MODE!=app_light_ctrl_data_mode_get_value())
        {
            app_light_ctrl_data_mode_set(WHITE_MODE);
        }
        buffer32_v = cmd_ptr[4];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[5];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[6];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[7];
        if(buffer32_v > 1000)
        {
            buffer32_v = 1000;
        }
        app_light_ctrl_data_temperature_set(buffer32_v);
        if(return_flag)
        {
            return_num = 8;
        }
        break;
    case 0x05://彩光  
    {      
        u16 hsv_h = 0, hsv_s = 0, hsv_v = 0;
        u8 buf_ar[2] = {0};
        if(COLOR_MODE!=app_light_ctrl_data_mode_get_value())
        {
            app_light_ctrl_data_mode_set(COLOR_MODE);
        }
        ty_string_op_hexstr2hex(&cmd_ptr[4], 4, buf_ar);
        hsv_h = ((u16)buf_ar[0] << 8) + buf_ar[1];
        
        ty_string_op_hexstr2hex(&cmd_ptr[8], 4, buf_ar);
        hsv_s = ((u16)buf_ar[0] << 8) + buf_ar[1];
        
        ty_string_op_hexstr2hex(&cmd_ptr[12], 4, buf_ar);
        hsv_v = ((u16)buf_ar[0] << 8) + buf_ar[1];

        app_light_ctrl_data_hsv_set(hsv_h, hsv_s, hsv_v);
        if(return_flag)
        {
            return_num = cmd_ptr[4] + 4;
        }
    }
        break;
    case 0x06://情景  dpID[1] | 数据类型[1] | 功能长度[2] | 功能指令[n]
        if(SCENE_MODE!=app_light_ctrl_data_mode_get_value())
        {
            app_light_ctrl_data_mode_set(SCENE_MODE);
        }
        //hal_uart_send(cmd_ptr, cmd_ptr[3] + 4);
        if(cmd_ptr[1] == 0x03)//string类型
        {
            u8 control_data[255] = {0};
            if(cmd_ptr[3] == 0x02) //01 06 03 02 70 00 
            {
                control_data[0] = 2;//参数长度
                control_data[1] = cmd_ptr[5] << 4;//场景号
            }
            else
            {     
                u32 scene_len = 0;
                u8 par_temp[LIGHT_SCENE_MAX_LENGTH] = {0};
                memcpy(par_temp, &cmd_ptr[4], cmd_ptr[3]);
                ty_light_basis_tools_scene_data_compress(par_temp, &control_data[1], &scene_len);
               control_data[0] = (u8)scene_len; 
            }
            ty_light_scene_cmd_data_set(&control_data[1],control_data[0]);
            //hal_uart_send(&control_data[1],control_data[0]);
        }
        if(return_flag)
        {
            return_num = cmd_ptr[4] + 4;
        }
    break;
    case 0x07://倒计时
        buffer32_v = cmd_ptr[4];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[5];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[6];
        buffer32_v = (buffer32_v<<8) + cmd_ptr[7];
        app_light_ctrl_data_countdown_set(buffer32_v);
        if(return_flag)
        {
            return_num = 8;
        }
        break;
    default:
        break;
    }
    app_light_ctrl_data_auto_save_start(APP_DATA_AUTO_SAVE_DELAY_TIME);
    app_light_ctrl_proc();

    return return_num;
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
                lutec_set_save_data_flag();
                if(par[6] < 0x02)//常亮or长灭
                {
                    if(addr_buf >= 0xC000)//组播
                    {
                        lutec_pir_enable_set(0);//关闭PIR功能
                        lutec_self_pir_dimmer_onoff(1);
                    }
                    else//点播
                    {
                        lutec_pir_enable_set(1);
                        lutec_self_pir_dimmer_onoff(0);//关闭自身PIR调光
                    }
                }
                else
                {
                    lutec_pir_enable_set(1);
                    lutec_self_pir_dimmer_onoff(1);
                }
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
                if((((u16)par[7] << 8) + par[8]) != 0xFFFF)//非查询
                {
                    //hal_uart_send(par, par[2] + 3);      
                    lutec_light_dimmer(&par[6]);
                }
                
                if(par[3] != 0x06)//非联动
                {   
                    lutec_set_save_data_flag();
                }
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_dimming_para(&par[6]) + 6;//[dpID | 数据类型 | 指令长度 | 指向 | 地址]6Bytes
                par[2] = return_num - 3;
            }    
            break;
        case 0x6A://延时调光
            if((lutec_get_switch_flag() != 2) && (par[3] == 0x06))//常亮or长灭时，不执行PIR联动指令
            {
                break;
            }
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
            //hal_uart_send(par, par[2] + 3);          
            if((par[7] < 33) && (par[7] > 0) && (par[6] < 97) && (par[6] > 8))//ssid长度(1~32); 密码(8~64);
            {
                lutec_light_config_wifi(&par[6], par[2] - 3);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_wifi_para(&par[6]) + 6;
                par[2] = return_num - 3;
            }  
            //hal_uart_send(par, par[2] + 3); 
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
                lutec_set_save_data_flag(); 
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
                par[2] = 4;
                par[3]  = par[3] == 0x01 ? 0x02 : 0x04;
                par[6]  = lutec_get_device_state();
                return_num = 7; 
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
        case 0x7A://产测  dpID | 数据类型 | 指令长度 | 指向 | 地址 | 指令参数
             if(par[2] == 4)
            {
                lutec_production_test_on(par[6]);
            }
            if(addr_buf < 0xC000)
            {
                par[2] = 4;
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                par[6] = lutec_get_production_test_state();
                return_num = 7;
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
            if(par[2] >= 6)
            {
                lutec_device_addr_control(&par[6], addr_buf, par[2] - 3);
            }
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_ack_addr(&par[6]) + 6;
                par[2] = return_num - 3;
            }              
            break;
        case 0x7E://设备识别号
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_device_id(&par[6]) + 6;
                par[2] = return_num - 3;
            }        
            break;
        case 0x7F://wifi状态
            //hal_uart_send(par, par[2] + 3);
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                par[6] = lutec_get_wifi_state();
                return_num = 7;
                par[2] = return_num - 3;
            }                 
            //hal_uart_send(par, par[2] + 3);
            break;
        case 0x80: //wifi模块信息
            //hal_uart_send(par, par[2] + 3); 
            if(addr_buf < 0xC000)
            {
                par[3] = par[3] == 0x01 ? 0x02 : 0x04;
                return_num = lutec_get_wifi_id(&par[6]) + 6;
                par[2] = return_num - 3;
            }                 
            //hal_uart_send(par, par[2] + 3);
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
    //u16 addrbuf = lutec_get_address();
    data_buffer[5] = (u8)(lutec_get_address() >> 8);
    data_buffer[6] = (u8)(lutec_get_address());
    data_buffer[7]  = ack_id;
    data_buffer[8]  = 0x06;

    app_light_vendor_data_send(sent_to_addr, data_buffer, 9);
}

/*-------------------------------------------------------------------------
*简  介:
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_updata_by_bt(u8 point_to, u8 dp_id, u8* data_ptr, u8 data_len, u16 sent_to_addr)
{  
    if(sent_to_addr == 0x0000)
    {
        return;
    }
    uint8_t data_buffer[255] = {0};

    data_buffer[0]  = 0x01;
    data_buffer[1]  = dp_id;
    data_buffer[2]  = 0x00;
    data_buffer[3]  = data_len + 3;
    data_buffer[4]  = point_to; 
    u16 addrbuf = lutec_get_address();
    data_buffer[5] = (addrbuf >> 8) & 0xFF;
    data_buffer[6] = (addrbuf >> 0) & 0xFF;
    for(addrbuf = 0; addrbuf < data_len; addrbuf++)
    {
        data_buffer[7 + addrbuf] = data_ptr[addrbuf];
    }
    //hal_uart_send(data_buffer, 7 + data_len);
    app_light_vendor_data_send(sent_to_addr, data_buffer, 7 + data_len);
    //hal_uart_send(&sent_to_addr, 2);
}

static u8 switch_flag = SWITCH_ONOFF_DEF;
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
        app_light_ctrl_data_bright_set(0x03E8);
        app_light_ctrl_data_countdown_set(0);
        switch_flag = 1;
        break;
    //case 2:
        //app_light_ctrl_data_switch_set(1);
        //app_light_ctrl_data_countdown_set(3000);
        //break;
    default:
        app_light_ctrl_data_switch_set(1);
        app_light_ctrl_data_bright_set(0x03E8);
        //app_light_ctrl_data_countdown_set(3);
        switch_flag = 2;
        lutec_set_pir_someone();
        break;
    }
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
void lutec_set_switch_flag(u8 flag_value)
{
    switch_flag = flag_value;
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
        // if(buffer16_v == 0)
        // {
        //     app_light_ctrl_data_switch_set(0);
        //     app_light_ctrl_data_countdown_set(0);
        // }
        // else
        // {
            if(buffer16_v > 1000)  buffer16_v = 1000;
            //app_light_ctrl_data_switch_set(1);
            //app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_temperature_set(buffer16_v);
            //pir色温
            //lutec_set_pir_dim_temperature(buffer16_v);
        // }
        break;
    case 0x12://亮度调光    
        buffer16_v = ((u16)para_ptr[1] << 8) + para_ptr[2];
        // if(buffer16_v == 0)
        // {
        //     app_light_ctrl_data_switch_set(0);
        //     app_light_ctrl_data_countdown_set(0);
        // }
        // else
        // {
            if(buffer16_v > 1000)  buffer16_v = 1000;
            app_light_ctrl_data_switch_set(1);
            app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_bright_set(buffer16_v);
        // }    
        break;
    case 0x13://亮度色温调光
        //app_light_ctrl_data_mode_set(WHITE_MODE);
        buffer16_v = ((u16)para_ptr[1] << 8) + para_ptr[2];
        // if(buffer16_v == 0)
        // {
        //     app_light_ctrl_data_switch_set(0);
        //     app_light_ctrl_data_countdown_set(0);
        // }
        // else
        // {
            if(buffer16_v > 1000) 
            {
                buffer16_v = 1000;
            }
            app_light_ctrl_data_switch_set(1);
            app_light_ctrl_data_countdown_set(0);
            app_light_ctrl_data_bright_set(buffer16_v);
            buffer16_v = ((u16)para_ptr[3] << 8) + para_ptr[4];
            if(buffer16_v > 1000) 
            {
                buffer16_v = 1000;
            }
            app_light_ctrl_data_temperature_set(buffer16_v);            
            //pir色温
            //lutec_set_pir_dim_temperature(buffer16_v);
        // }        
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
    //hal_uart_send(&dim_flag, 1);
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
            return 1;
        default:
            return 0;
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
        if(para_l > 0)
        {
            for(u8 i = 0; i < para_l; i++)
            {
                delay_target_para[i] = para_p[3 + i];
            }
        }
        else  //参数不正确---未处理
        {

        }        
    }
    else //先调光在等待关灯
    {
        lutec_light_dimmer(&para_p[3]);
        delay_target_para[0] = 0x1C;//关灯       
    }

    //延时时基
    // if((para_p[3] == 0x4C) || (para_p[3] == 0x4D))
    // {
    //     app_light_ctrl_data_countdown_set(delay_time);
    //     delay_time_base = 0;
    // } 
    // else
    //
        delay_time_base = lutec_get_tick_10ms();
    //}   
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

//static u8 wifi_config_flg = 0;
static u8 wifi_ssid_psw[132] = {0}; //总长度[1]|ssid+密码长度[1]|ssid长度[1]|ssid[1~32]|password[8~64]|token[1~32]和校验[1]
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_config_wifi(u8* para_p, u8 para_l)
{
    //hal_uart_send(para_p, para_l);
    if((para_l > (para_p[0] + 2)) && (para_l < 129))//长度
    {        
        // 更新wifi信息
        wifi_ssid_psw[0] = para_l;//总长
        for(u8 sub_i = 0; sub_i < para_l; sub_i++)//ssid+密码长度[1]|ssid长度[1]|ssid[1~32]|password[8~64]|token[1~32]
        {
            wifi_ssid_psw[sub_i + 1] = para_p[sub_i];
        }
        wifi_ssid_psw[para_l + 1] = lutec_check_sum(wifi_ssid_psw, wifi_ssid_psw[0] + 1);//

        lutec_start_wifi_connect();
        //hal_uart_send(wifi_ssid_psw, wifi_ssid_psw[0] + 2);
    }
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_wifi_para(u8* para_g)
{
    if((wifi_ssid_psw[1] < (wifi_ssid_psw[2] + 8)) || (wifi_ssid_psw[wifi_ssid_psw[0] + 1] != lutec_check_sum(wifi_ssid_psw, wifi_ssid_psw[0] + 1)))
    {
        //hal_uart_send(wifi_ssid_psw, wifi_ssid_psw[0] + 2);
        para_g[0] = 0xFE;//暂无数据
        return 1;
    }

    for(u8 sub_i = 0; sub_i <= wifi_ssid_psw[0]; sub_i++)
    {
        para_g[sub_i] = wifi_ssid_psw[1 + sub_i];
    }
    //hal_uart_send(wifi_ssid_psw, wifi_ssid_psw[0] + 2);
    return wifi_ssid_psw[0];
}
/*
0xF1: MCU重启
0xF2: 恢复出厂设置
0xF3: 蓝牙重置
0xF4: 摄像模块重置
*/
static u8 reset_cmd = 0;//
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
        reset_time_base = lutec_get_tick_10ms();//reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;
        reset_cmd = 0xF1;
        break;
    case 0xF2://恢复出厂设置
        lutec_device_reset();
        reset_cmd = 0xF2;
        if(lutec_get_device_state() > 0x53)
        {
            lutec_set_device_state(0x53);//出场状态
        }
        break;
    case 0xF3://蓝牙重置
        //kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;  
        reset_time_base = lutec_get_tick_10ms();
        reset_cmd = 0xF3;
        break;
    case 0xF4://重置WiFi模块
        lutec_reset_wifi_module();
        reset_cmd = 0xF4;
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
void lutec_bt_module_reset_control(void)
{
    if(reset_time_base == 0)
    {
        return;
    }
    if(lutec_get_interval_tick_10ms(reset_time_base) > 200)
    { 
        switch(reset_cmd)
        {
        case 0xF1:
            reset_cmd = 0;
            reset_time_base = 0;
            reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT; 
            break;
        case 0xF3:
            reset_cmd = 0;
            reset_time_base = 0;
            kick_out();
            break;
        default:
            reset_cmd = 0;
            reset_time_base = 0;
            break;
        }
    }
}
 /*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/ 
 u8 lutec_get_reset_state(u8* sv_ptr)
 {
     sv_ptr[0] = reset_cmd;
     return 1;
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
    lutec_set_save_data_flag();
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


//----------------------------若修改在lutec_eeprom.c的lutec_eeprom_save_data_init()中同步修改
static u32 pir_noone_delay = NO_ONE_LIGHT_ON_TIME;
static u8 noone_pir_light_par[11] = {0x13,0x00,0x32,0x01,0xF4,0,0,0,0,0,0};
static u32 pir_anyone_delay = SOMEONE_LIGHT_ON_TIME;//
static u8 anyone_pir_light_par[11] = {0x13,0x03,0xE8,0x03,0xE8,0,0,0,0,0,0};

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_pir_dim_temperature(u16 temp_v)
{
    u8 buf8_v = 0;
    buf8_v = (u8)(temp_v >> 8);
    noone_pir_light_par[3] = buf8_v;
    anyone_pir_light_par[3] = buf8_v;
    
    buf8_v = (u8)temp_v;
    noone_pir_light_par[4] = buf8_v;
    anyone_pir_light_par[4] = buf8_v;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u16 lutec_get_pir_dim_temperature(void)
{
    u16 buf16_v = 0;
    buf16_v = anyone_pir_light_par[3];    
    buf16_v = (buf16_v << 8) + anyone_pir_light_par[4];
    return buf16_v;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_get_pir_dim_para(u8* noone_p,  u8* someone_p)
{
    for(u8 p_sub = 0; p_sub < 11; p_sub++)
    {
        noone_p[p_sub] = noone_pir_light_par[p_sub];
        someone_p[p_sub] = anyone_pir_light_par[p_sub];
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_pir_dim_para(u32 noone_t, u8* noone_p, u32 someone_t, u8* someone_p)
{
    pir_noone_delay = noone_t;
    pir_anyone_delay = someone_t; 
    for(u8 p_sub = 0; p_sub < 11; p_sub++)
    {
        noone_pir_light_par[p_sub] = noone_p[p_sub];
        anyone_pir_light_par[p_sub] = someone_p[p_sub];
    }
}

/*-------------------------------------------------------------------------
*简  介: 获取设置的亮灯时间
*参  数: u8 any_one（0-无人；1-有人）
*返回值: 单位1s
-------------------------------------------------------------------------*/
u32 lutec_get_pir_delay_time_s(u8 any_one)
{
    if(any_one == 0)
    {
        return pir_noone_delay;
    }
    else
    {
        return pir_anyone_delay < SOMEONE_LIGHT_ON_TIME ? SOMEONE_LIGHT_ON_TIME : pir_anyone_delay;
    }    
}

/*-------------------------------------------------------------------------
*简  介: 获取亮灯时间
*参  数: u8 any_one（0-无人；1-有人）
*返回值: 单位10ms
-------------------------------------------------------------------------*/
u32 lutec_get_pir_delay_time_10ms(u8 any_one)
{
    if(any_one == 0)
    {
        //return pir_noone_delay < 10 ? 0 : (u32)(pir_noone_delay / 10);
        if(pir_noone_delay == 0)
        {
            return 0;
        }
        else
        {
            return (pir_noone_delay * 100) - 50;
        }
    }
    else
    {
        //return pir_anyone_delay < 10 ? 0 : (u32)(pir_anyone_delay / 10);
        if(pir_anyone_delay == 0)
        {
            return 450;
        }
        else
        {
            return (pir_anyone_delay * 100) - 50;
        }
    }    
}

/*-------------------------------------------------------------------------
*简  介: 感应灯控
*参  数: u8 any_one（0-结束；1-无人；2-有人）
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_dimmer(u8 any_one)
{
    // hal_uart_send(&any_one, 1);//--------测试
    // hal_uart_send(&pir_noone_delay, 4);
    // hal_uart_send(noone_pir_light_par, 11);
    // hal_uart_send(&pir_anyone_delay, 4);
    // hal_uart_send(anyone_pir_light_par, 11);

    switch(any_one)
    {
    case 0x00:
        {
        u8 cmdbuf = 0x1C;
        lutec_light_dimmer(&cmdbuf);
        }
        break;
    case 0x01:  
        lutec_light_dimmer(noone_pir_light_par);          
        break;
    case 0x02:    
        lutec_light_dimmer(anyone_pir_light_par);  
        break;
    default:
        return;
    }
}

/*-------------------------------------------------------------------------
*简  介: 组播感应灯控
*参  数: u8 any_one（0-结束；1-无人；2-有人），组播地址
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_multicast(u8 any_one, u16 addr_v)
{
	u8 buffer_data[28]= {0};
    u8 mult_data_len = 0;

    buffer_data[0]  = 0x01;
    buffer_data[1]  = 0x6A;
    buffer_data[2]  = 0x00;
    //buffer_data[3]  = ;//长度
    buffer_data[4]  = 0x06;//联动
    buffer_data[5] = (u8)(addr_v >> 8);//目标地址
    buffer_data[6] = (u8)(addr_v);
    switch(any_one)
    {
    case 0x00:
        buffer_data[7] = 0x00;//时间
        buffer_data[8] = 0x00;
        buffer_data[9] = 0x00;
        buffer_data[10] = 0x1C;//灯控参数--关灯
        mult_data_len = 11;
        break;
    case 0x01:        
        buffer_data[7] = (u8)(pir_noone_delay >> 16);//时间
        buffer_data[8] = (u8)(pir_noone_delay >> 8);
        buffer_data[9] = (u8)(pir_noone_delay >> 0);
        mult_data_len = lutec_get_dim_para_len(noone_pir_light_par[0]);
        if(mult_data_len == 0)
        {            
            buffer_data[10] = 0x1C;//灯控参数--关灯
            mult_data_len = 11;
        }
        else
        {
            for(u8 sub_ii = 0; sub_ii < mult_data_len; sub_ii++)
            {
                buffer_data[10 + sub_ii] = noone_pir_light_par[sub_ii];
            }
            mult_data_len += 10;
        }
        break;
    case 0x02:
        if(pir_anyone_delay == 0)
        {
            pir_anyone_delay = SOMEONE_LIGHT_ON_TIME;
        }
        buffer_data[7] = (u8)(pir_anyone_delay >> 16);//时间
        buffer_data[8] = (u8)(pir_anyone_delay >> 8);
        buffer_data[9] = (u8)(pir_anyone_delay >> 0);
        mult_data_len = lutec_get_dim_para_len(anyone_pir_light_par[0]);
        if(mult_data_len == 0)
        {            
            buffer_data[10] = 0x1D;//灯控参数--关灯
            mult_data_len = 11;
        }
        else
        {
            for(u8 sub_ii = 0; sub_ii < mult_data_len; sub_ii++)
            {
                buffer_data[10 + sub_ii] = anyone_pir_light_par[sub_ii];
            }
            mult_data_len += 10;
        }
        break;
    default:
        return;
    }
    buffer_data[3]  = mult_data_len - 4;//长度

    app_light_vendor_data_send(addr_v, buffer_data, mult_data_len);
}
/*-------------------------------------------------------------------------
*简  介: 组播感应灯控
*参  数: u8 any_one（0-结束；1-无人；2-有人），组播地址
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_sig_cmd_multicast(u8 any_one, u16 addr_v)
{
    u16 buffer16_v = 0;
    switch(any_one)
    {
    case 0x00:
        lutec_send_onoff_sig_cmd(addr_v, 0x00);//关灯
        break;
    case 0x01:  
        if(pir_noone_delay == 0)
        {
            lutec_send_onoff_sig_cmd(addr_v, 0x00);//关灯
            break;
        }
        switch(noone_pir_light_par[0])
        {
        case 0x11://色温
            buffer16_v = ((u16)noone_pir_light_par[1] << 8) + noone_pir_light_par[2];
            lutec_send_temperature_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x12://亮度        
            buffer16_v = ((u16)noone_pir_light_par[1] << 8) + noone_pir_light_par[2];
            lutec_send_bright_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x13://亮度色温        
            buffer16_v = ((u16)noone_pir_light_par[1] << 8) + noone_pir_light_par[2];  
            lutec_send_bright_sig_cmd(addr_v, buffer16_v);
            buffer16_v = ((u16)noone_pir_light_par[3] << 8) + noone_pir_light_par[4];
            lutec_send_temperature_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x1A://HSV
            break;
        case 0x1B://RGB
            break;
        case 0x2A://亮度色温+HSV
            break;
        case 0x2B://亮度色温+RGB
            break;
        default:
            lutec_send_onoff_sig_cmd(addr_v, 0x00);//关灯
            break;
        }
        break;
    case 0x02:
        if(pir_anyone_delay == 0)
        {
            pir_anyone_delay = SOMEONE_LIGHT_ON_TIME;
            lutec_send_onoff_sig_cmd(addr_v, 0x01);//开灯       
            buffer16_v = 0x03E8;
            lutec_send_bright_sig_cmd(addr_v, buffer16_v);
            //lutec_send_temperature_sig_cmd(addr_v, buffer16_v);
            break;
        }
        switch(anyone_pir_light_par[0])
        {
        case 0x11://色温
            buffer16_v = ((u16)anyone_pir_light_par[1] << 8) + anyone_pir_light_par[2];
            lutec_send_temperature_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x12://亮度
            buffer16_v = ((u16)anyone_pir_light_par[1] << 8) + anyone_pir_light_par[2];
            lutec_send_bright_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x13://亮度色温               
            lutec_send_onoff_sig_cmd(addr_v, 0x01);//开灯       
            buffer16_v = ((u16)anyone_pir_light_par[1] << 8) + anyone_pir_light_par[2];
            lutec_send_bright_sig_cmd(addr_v, buffer16_v);
            buffer16_v = ((u16)anyone_pir_light_par[3] << 8) + anyone_pir_light_par[4];
            lutec_send_temperature_sig_cmd(addr_v, buffer16_v);
            break;
        case 0x1A://HSV
            break;
        case 0x1B://RGB
            break;
        case 0x2A://亮度色温+HSV
            break;
        case 0x2B://亮度色温+RGB
            break;
        default:
            lutec_send_onoff_sig_cmd(addr_v, 0x00);//关灯
            break;
        }
        break;
    default:
        return;
    }
}

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
        if(buffer32_v == 0)//无人亮度为0时，亮灯延时设置为0.
        {
            pir_noone_delay = 0; 
        }
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
        //noone_pir_light_par[3] = 0;
        //noone_pir_light_par[4] = 0;
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
    
    //hal_uart_send(&para_p[0], 1);

    //hal_uart_send(&pir_anyone_delay, 4);


    buf8_v = 0;
    sub8_i = 1;
    buffer32_v = ((u16)para_p[4] << 8) + para_p[5];
    if((buffer32_v <= 1000) && ((para_p[0] & 0x04) == 0x04))//pir人亮度
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
    if((buffer32_v <= 1000) && ((para_p[0] & 0x02) == 0x02))//pir人色温
    {
        anyone_pir_light_par[3] = para_p[20];
        anyone_pir_light_par[4] = para_p[21];
        buf8_v |= 0x11;
    }
    else
    {
        //anyone_pir_light_par[3] = 0;
        //anyone_pir_light_par[4] = 0;
    }
    if(((para_p[0] & 0x01) == 0x01) && ((para_p[22] == 0x1A) || (para_p[22] == 0x1B)))//pir人采光 未处理？
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
    lutec_set_save_data_flag();
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
	if(pir_noone_delay > 0)
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
    if(pir_anyone_delay > 0)
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

	s_ptr[0] = falg_v == 0x08 ? 0x00 : falg_v;
    
    return 29;

}



static u8 pir_updata_falg  = PIR_UPDATA_APP_DEF;//0xF1:上报APP；0xF2：不上报；
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
        lutec_set_save_data_flag();
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



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_device_addr_control(u8* para_p, u16 s_addr, u8 para_l)
{
    u16 buf16_v = 0;
    switch(para_p[0])
    {
        case 1://注册
            if(para_l == 3)//长度
            {
                buf16_v = ((u16)para_p[1] << 8) + para_p[2];
                if((s_addr < 0xC000) && (buf16_v < 0xC000))//点播&&节点地址
                {
                    lutec_set_address(buf16_v);
                }
            }
        break;
        case 2://加入组
            if((para_l >= 3) && (((para_l - 1) % 2) == 0))
            {
                u8 group_num = (para_l - 1) / 2;
                for(u8 sub_n = 0; sub_n < group_num; sub_n++)
                {
                    buf16_v = ((u16)para_p[1 + (sub_n << 1)] << 8) + para_p[2 + (sub_n << 1)];
                    if(buf16_v >= 0xC000)
                    {
                        lutec_join_group(buf16_v);
                    }
                }               
            }
        break;
        case 3://退出组
            if((para_l >= 3) && (((para_l - 1) % 2) == 0))
            {
                u8 group_num = (para_l - 1) / 2;
                if(group_num > 8) 
                {
                    return;
                }
                // hal_uart_send(&para_l, 1);
                // hal_uart_send(&group_num, 1);
                for(u8 sub_m = 0; sub_m < group_num; sub_m++)
                {
                    buf16_v = ((u16)para_p[1 + (sub_m << 1)] << 8) + para_p[2 + (sub_m << 1)];
                    if((group_num == 1) && (buf16_v == 0xFFFF))//推出所有组
                    {
                        lutec_init_group();
                        break;
                    }
                    else
                    {
                        if(lutec_is_own_group(buf16_v) == 1)
                        {
                            lutec_exit_group(buf16_v);
                        }
                    }
                    // hal_uart_send(&sub_m, 1);
                    // hal_uart_send(&buf16_v, 2);
                }               
            }
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
u8 lutec_get_ack_addr(u8* save_ptr)
{
    u8 buf_cm = save_ptr[0];
    //u16 buf_addr = lutec_get_address();

    save_ptr[0] = 0x0F;
    save_ptr[1] = (u8)(lutec_get_address() >> 8);
    save_ptr[2] = (u8)(lutec_get_address() >> 0);
    if(buf_cm == 1)//注册
    {
        return 3;
    }

    return (lutec_get_all_group_addr(&save_ptr[3]) + 3);
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_device_id(u8* ip_ptr)
{
    u8 buf_name[14] = DEFICE_NAME;
    
    lutec_string_copy(ip_ptr, buf_name, 14);
    ip_ptr[14] = DEVICE_CLASSES;
    ip_ptr[15] = HARD_VERSION;
    ip_ptr[16] = SOFT_VERSION;
    u32 buf32_v = SERIAL_NUMBER;
    ip_ptr[17] = (u8)(buf32_v >> 24);
    ip_ptr[18] = (u8)(buf32_v >> 16);
    ip_ptr[19] = (u8)(buf32_v >> 8);
    ip_ptr[20] = (u8)(buf32_v >> 0);
    buf32_v = DEVICE_FUNTICON;
    ip_ptr[21] = (u8)(buf32_v >> 24);
    ip_ptr[22] = (u8)(buf32_v >> 16);
    ip_ptr[23] = (u8)(buf32_v >> 8);
    ip_ptr[24] = (u8)(buf32_v >> 0);
    return 25;
}


static u16 lutec_reply_addr = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u16 lutec_get_reply_addr(void)
{
    return lutec_reply_addr;
}


/*-------------------------------------------------------------------------
*简  介: app连上后需要上报,显示给用户
*参  数: 手机地址
*返回值: 
-------------------------------------------------------------------------*/
void lutec_updata_app_callback(u16 d_addr)
{
    //-----------------------缓存app端地址
    if(lutec_reply_addr != d_addr)//app端地址
    {
        lutec_reply_addr = d_addr;
    }

    //hal_uart_send(&lutec_reply_addr, 2);
    //------------------------推送设备识别码
    u8 par_data[32] = {0};
    par_data[0] = 0x01;
    par_data[1] = 0x7E;
    par_data[2] = 0x00;  
    par_data[3] = 28;//0x1C
    par_data[4] = 0x04;
    par_data[5] = (u8)(lutec_get_address() >> 8);
    par_data[6] = (u8)lutec_get_address();    
    u8 buf_name[14] = DEFICE_NAME;
    lutec_string_copy(&par_data[7], buf_name, 14);//name
    par_data[21] = DEVICE_CLASSES;
    par_data[22] = HARD_VERSION;
    par_data[23] = SOFT_VERSION;
    u32 buf32_v = SERIAL_NUMBER;
    par_data[24] = (u8)(buf32_v >> 24);
    par_data[25] = (u8)(buf32_v >> 16);
    par_data[26] = (u8)(buf32_v >> 8);
    par_data[27] = (u8)(buf32_v >> 0);
    buf32_v = DEVICE_FUNTICON;
    par_data[28] = (u8)(buf32_v >> 24);
    par_data[29] = (u8)(buf32_v >> 16);
    par_data[30] = (u8)(buf32_v >> 8);
    par_data[31] = (u8)(buf32_v >> 0);
    app_light_vendor_data_send(d_addr, par_data, 32);
    
    hal_uart_send(par_data, 32);
}





/***************************************File end********************************************/


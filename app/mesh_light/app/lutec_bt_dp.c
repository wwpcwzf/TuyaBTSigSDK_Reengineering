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
*返回值: 要回复的数据存储到参数指针指向处；
*       返回回复参数的长度 > 6，反之不需要回复。
-------------------------------------------------------------------------*/
u8 lutec_protocol_dp_analysis(u8 *par)
{
    u8 return_num = 0;

    switch(par[0])
    {
        case 0x65://开关
        #if 0
            switch (par[6])
            {
                case 0:
                    app_light_ctrl_data_switch_set(0);
                    app_light_ctrl_data_countdown_set(0);
                    lutec_onoff_flag = 0;
                    break;
                case 1:
                    app_light_ctrl_data_switch_set(1);
                    app_light_ctrl_data_countdown_set(0);
                    lutec_onoff_flag = 1;
                    break;
                case 2:
                    app_light_ctrl_data_switch_set(1);
                    app_light_ctrl_data_countdown_set(1000);
                    lutec_onoff_flag = 2;
                    break;
                default:
                    break;
            }
            app_light_ctrl_data_auto_save_start(5000);
            app_light_ctrl_proc();

            if((((uint16_t)par[4] << 8) + par[5]) < 0xC000)//点播---回复
            {
                par[1] = 0x00;              
                par[2] = 0x04;
                par[6] = lutec_onoff_flag;
                if((par[3] == 0x01) || (par[3] == 0x03))
                {
                    par[3] = par[3] + 1; 
                    return 7;
                }
                else
                {
                    return 0;
                } 
            }
        #endif
            break;
        case 0x66://灯光调节
            //sys_execution_0x66();
            break;
        case 0x6A://延时调光
            //sys_execution_0x6A();
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






/***************************************File end********************************************/


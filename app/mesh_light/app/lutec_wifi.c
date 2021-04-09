/****************************************************************************************
> File Name: lutec_wifi.c
> Description:
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_wifi.h"

#include "lutec_config.h"

#include "lutec_main.h"
#include "lutec_bt_dp.h"

#include "app_common.h"
#include "hal_uart.h"
#include "ty_fifo.h"

#include "lutec_led.h"



static u8 wifi_module_connect_flag = 0;//模块连接标志

static u8 wifi_state = 0x00;

static u8 wifi_id[32] = {0};

static u8 send_retry_flag = 0;//bit0配网重发；bit1重启重发; bit2感应通报重发
static u8 wifi_config_order_num = 0;//配网z指令序号
static u8 net_config_counter = 0;
static u8 reset_order_num = 0;//重启指令序号
static u8 reset_retry_counter = 0;
static u8 pir_someone_order_num = 0;
static u8 pir_someone_retry_counter = 0;

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_start_wifi_connect(void)
{
    net_config_counter = 0;
    send_retry_flag |= 0x01;
    wifi_config_order_num = lutec_get_order_number();
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_stop_wifi_connect(void)
{
    net_config_counter = 0;
    send_retry_flag &= ~0x01;
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_wifi_control_loop(void)
{
    static u32 send_retry_time_base = 0; 
    static u32 inquire_time_base = 0;
    //------------------------wifi模块未连接
    if(wifi_module_connect_flag == 0)
    {
        return;
    }
    //-------------------------PIR感应上报控制
    lutec_pir_updata_control();

    //-------------------------wifi网络状态查询
    if(lutec_get_interval_tick_10ms(inquire_time_base) > WIFI_CONNECT_INQUIRE_TIME)
    {
        u8 data_buffer[2] = {0x7F,0xFF};
        lutec_updata_wifi_module(data_buffer, 2, lutec_get_order_number(), 0x00);
        inquire_time_base = lutec_get_tick_10ms();
    }
    //-------------------------重发控制
    if(send_retry_flag > 0)
    {
        //hal_uart_send(&send_retry_flag, 1);
        if(lutec_get_interval_tick_10ms(send_retry_time_base) > WIFI_CMD_SEND_RETRY_TIME)
        {
            if(send_retry_flag & 0x01)//配网重发
            {                
                lutec_wifi_net_config(wifi_config_order_num);
                if(net_config_counter++ >= WIFI_RESEND_MAX_NUM)
                {
                    net_config_counter = 0;
                    send_retry_flag &= ~0x01;
                }
            }
            if(send_retry_flag & 0x02)//重启重发
            {                
                u8 data_buffer[2] = {0x6C,0xF4};
                lutec_updata_wifi_module(data_buffer, 2, reset_order_num, 0x00);
                if(reset_retry_counter++ >= WIFI_RESEND_MAX_NUM)
                {
                    reset_retry_counter = 0;
                    send_retry_flag &= ~0x02;
                }
            }
            if(send_retry_flag & 0x04)//PIR有人重发
            {           
                u8 data_buffer[2] = {0x6F,0x01};
                if(lutec_pir_updata_is() > 0)   //上报
                {  
                    if(wifi_state == 0x74)
                    {
                        lutec_bt_data_up_by_wifi(0x6F, &data_buffer[1], 1);
                    }
                    else 
                    {
                        lutec_updata_by_bt(0x04, 0x6F, &data_buffer[1], 1, lutec_get_reply_addr());
                    }
                    //hal_uart_send(&data_buffer, 2);
                }
                lutec_updata_wifi_module(data_buffer, 2, pir_someone_order_num, 0x01);//通告
                if(pir_someone_retry_counter++ >= 1)//WIFI_RESEND_MAX_NUM)//总发两次
                {
                    pir_someone_retry_counter = 0;
                    send_retry_flag &= ~0x04;
                }
            }
            if(send_retry_flag > 7)
            {
                send_retry_flag &= 0x07;
            }
            send_retry_time_base = lutec_get_tick_10ms();
        }
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_updata_control(void)
{

    static u32 control_time_base = 0;

    if((lutec_pir_someone() == 0) || (lutec_pir_report_is_enable() == 0))//没感应||不上报
    {
        return;
    }

    if(lutec_get_interval_tick_10ms(control_time_base) > PIR_SOMEBODY_UPATA_TIME)
    {
        send_retry_flag |= 0x04;
        pir_someone_order_num = lutec_get_order_number();

        control_time_base = lutec_get_tick_10ms();
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_wifi_module_reset_state(void)
{
    return (send_retry_flag &= 0x02) == 0x02 ? 0xFB : 0xFC;
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_wifi_net_config(u8 cmd_order_number)
{
    u8 config_info[132] = {0};

    config_info[0] = lutec_get_wifi_para(&config_info[2]);
    if(config_info[0] > 8)
    {
        config_info[1] = 0x6B;
        lutec_updata_wifi_module(&config_info[1], config_info[0] + 1, cmd_order_number, 0x00);
    }
}




/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_uart_server_run(void)
{
	u8 num = ty_fifo_get_size();

	if(num < BT_MIN_PACKET_LEN) // BT_MIN_PACKET_LEN = 7
	{
		return;
	} 
	//1?judge head
	u8 data_buf[255] = {0};
	ty_fifo_read(data_buf, BT_MIN_PACKET_LEN);
    //识别帧头版本：WiFi(觅睿) = 0x5656 01；蓝牙(涂鸦)  0x55aa 00；串口(lutec) 0x5AA5 00
	if(((data_buf[0] != 0x56) || (data_buf[1] != 0x56) || (data_buf[4] != 0x01)) 
    && ((data_buf[0] != 0x55) || (data_buf[1] != 0xAA) || (data_buf[2] != 0x00)) 
    && ((data_buf[0] != 0x5A) || (data_buf[1] != 0xA5) || (data_buf[2] != 0x00)))
	{
		ty_fifo_pop(1);
		return;
	}
    
    //hal_uart_send(data_buf, BT_MIN_PACKET_LEN);

	//2.judge if it's a whole frame
	u16 data_len = 0;
    if((data_buf[0] == 0x56) && (data_buf[1] == 0x56))//wifi帧
    {
        data_len = ((u16)data_buf[2] << 8) + data_buf[3] + 4;
        if((data_len > MAX_PACKET_LEN) || (data_len < WIFI_MIN_PACKET_LEN))
        {
            ty_fifo_pop(2);
            return;
        }
    }
    else if((data_buf[0] == 0x55) && (data_buf[1] == 0xAA))//蓝牙帧
    {
        data_len = ((u16)data_buf[4] << 8) + data_buf[5] + 7;
        if((data_len < BT_MIN_PACKET_LEN) || (data_len > MAX_PACKET_LEN))
        {
            ty_fifo_pop(2);
            return;
        }
    }
    else //自定义串口指令: 帧头[2]||版本[1]||命令[1]||长度(参数+校验)[2]||参数[n]|校验(帧头~参数)[1]
    {
        data_len = ((u16)data_buf[4] << 8) + data_buf[5] + 6;
        if((data_len > MAX_PACKET_LEN) || (data_len < 7))
        {
            ty_fifo_pop(2);
            return;
        }
    }
    if(data_len > num)//未接收完数据包
    {
        return;
    }

	
	//3.judge check sum 
	ty_fifo_read(data_buf, data_len);
	u8 ck_sum = 0;
	ck_sum = lutec_check_sum(data_buf, data_len - 1);
	if(ck_sum != data_buf[data_len - 1])
	{
		ty_fifo_pop(2);
		return;
	}
	ty_fifo_pop(data_len); //释放缓存

	//4.数据处理
	switch(data_buf[1])
	{
    case 0x56: //wifi模组数据
        wifi_module_connect_flag = 1;
        //hal_uart_send(data_buf, data_len);  
        lutec_wifi_module_data(&data_buf[5], data_len - 6);//序号|类别|命令|数据，长度            
		break;
    case 0xA5: //串口产测指令

		break;
    case 0xAA: //蓝牙远程下行数据     
        //hal_uart_send(data_buf, data_len);         //-------------测试
        lutec_remote_bluetooth_data(&data_buf[6], data_len - 7);//dpID|数据类型|参数长度|参数，           
		break;
    default:
		break;
	}
}


static u8 order_number = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_order_number(void)
{
    return order_number++;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 序号|类别|命令|数据，长度   
*返回值: 
-------------------------------------------------------------------------*/
void lutec_wifi_module_data(u8* cmd_ptr, u8 para_l)
{
    u8 return_count = 0;
	switch(cmd_ptr[2])
	{
        case 0x00: //心跳
            //cmd_ptr[0] = ;//序号
            cmd_ptr[1] = 0x02;//类别--应答包
            //cmd_ptr[2] = ;//命令
            cmd_ptr[3] = 0x00;//回复状态0x00--成功
            return_count = 4;
		break;
        case 0x01: //单个数据点设置
            if(para_l < 4)
            {
                break;
            }
            if(cmd_ptr[3] == 0x00) //觅睿串口协议数据点
            {
                return_count = lutec_mr_wifi_dp(cmd_ptr, para_l);
            }
            else //lutec自定义数据点
            {
                return_count = lutec_wifi_dp(cmd_ptr, para_l);
            }           
        break;
        case 0x03: //主芯片事件:0-启动成功；1-复位；3-云端删除设备； 56 56 00 06 01 1F 01 03 01 D7        
            if(para_l < 4)
            {
                break;
            }
            switch(cmd_ptr[3])
            {
                case 0x00:
                    if(send_retry_flag & 0x02)
                    {
                        send_retry_flag &= ~0x02;
                    }
                    break;
                case 0x01:
                    wifi_module_connect_flag = 0;
                    kick_out();
                    break;
                case 0x03:

                    break;
                case 0xFF:
                    if(send_retry_flag & 0x02)
                    {
                        send_retry_flag &= ~0x02;
                    }
                    break;
                default:
                    break;
            }
            return_count = 0;//不回复
        break;
        case 0x04: //获取MCU信息
            //cmd_ptr[0] = ;//序号
            cmd_ptr[1] = 0x02;//类别--应答包
            //cmd_ptr[2] = ;//命令
            cmd_ptr[3] = 0x01;//协议版本
            cmd_ptr[4] = 0x00;//MCU固件版本
            cmd_ptr[5] = 0x00;
            cmd_ptr[6] = SOFT_VERSION;
            cmd_ptr[7] = 0x4C;//制造商--'L'=0x4C LUTEC
            cmd_ptr[8] = 0x00;//OTA长度--0：不支持OTA
            cmd_ptr[9] = 0x00;//
            return_count = 10;
        break;
        case 0x05: //OTA固件下发
            return_count = 0;//不回复
        break;
        default:
        break;
	}

    if(return_count >= 3)
    {  
        //hal_uart_send(cmd_ptr, return_count);
        //序号+类别+指令+功能ID+功能参数
        lutec_reply_wifi_module(cmd_ptr, return_count);
    }
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 序号|类别|命令[2]|数据，长度  
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_mr_wifi_dp(u8* para_ptr, u8 par_l)
{
    switch(para_ptr[4])
    {
    case 0x01://开关灯1
        break;
    case 0x02://亮度[2]
        break;
    case 0x03://色温[2]
        break;
    case 0x04://PIR高亮时间[2]s
        break;
    case 0x05://PIR低亮亮度[2]10-350
        break;
    case 0x06://低亮开关[1]0关；1晚上亮3h；2晚上亮6h
        break;
    case 0x07://PIR灵敏度[1]0低；1中；2高；3关
        break;
    case 0x08://PIR上报[1]0白天有感应；1晚上有感应
        break;
    case 0x09://PIR亮灯下发[1]
        break;
    case 0x0A://光敏上报[2]
        break;
    case 0x0B://亮度上报[2]
        break;
    case 0x0C://实时看视频[1]1看；0不看
        break;
    case 0x0D://夜视开关[1]1开；0关
        break;
    default:
        break;
    }
    return 0;
}


//通报开关
static u8 someone_updata_enable = PIR_UPDATA_WIFI_DEF;//1-使能；0-失能
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_pir_report_is_enable(void)
{
    return someone_updata_enable > 0 ? 1 : 0;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_report_set(u8 en_v)
{
    someone_updata_enable = en_v;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 序号|类别|命令[1]|数据，长度  
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_wifi_dp(u8* para_ptr, u8 par_l)
{
    u8 reply_len = 0;
    switch(para_ptr[3])
    {
        case 0x6B: //107 WiFi配网  94 02 01 6B FE B4
            if((par_l == 6) && (para_ptr[4] == 0xFE))
            {
                lutec_stop_wifi_connect();//停止配网重发
            }
            else
            {
                //--------未处理
            }
        break;
        case 0x6c: //108 重置WiFi模组
            wifi_module_connect_flag = 0;
            send_retry_flag &= ~0x02;
            break;
        case 0x6F: //111 PIR感应
            switch(para_ptr[4])
            {
            case 0xF1://开
                someone_updata_enable = 0x01;//上报
                break;
            case 0xF2://关
                someone_updata_enable = 0x00;//不上报
                break;
            case 0xFF://查询
                para_ptr[4] = lutec_pir_someone();
                break;
            default:
                break;
            }
            if(para_ptr[1] = 0x00)
            {
                //para_ptr[0] = 0x;
                para_ptr[1] = 0x02;
                para_ptr[2] = 0x02;
                //para_ptr[3] = 0x;
                //para_ptr[4] = 0x;
                reply_len = 5;
            }
            break;
        case 0x71: //113 时钟同步

            break;
        case 0x7F: //127 WiFi网络状态
            if(wifi_state != para_ptr[4])
            {
                wifi_state = para_ptr[4];
                lutec_updata_by_bt(0x04, 0x7F, &wifi_state, 1, lutec_get_reply_addr());
            }
            break;
        case 0x80: //128 WiFi模组识别

            break;
        default:
            break;
    }
    return reply_len;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_wifi_state(void)
{
    return wifi_state;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_wifi_id(u8* get_ptr)
{
    u8 sub_y = 0;
    if(wifi_id[0] == 0)
    {
        get_ptr[0] = 0xFE;
        return 1;
    }
    
    for(sub_y = 0; sub_y < wifi_id[0]; sub_y++)
    {
       get_ptr[sub_y] = wifi_id[sub_y + 1];
    }
    return wifi_id[0];
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_reset_wifi_module(void)
{
    //static u8 send_retry_flag = 0;//bit0配网重发；bit1重启重发
    reset_order_num = lutec_get_order_number();
    send_retry_flag |= 0x02;
}


/*-------------------------------------------------------------------------
*简  介: 蓝牙远程下行数据处理：解析自己的点播指令，其余转发。
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_remote_bluetooth_data(u8* data_ptr, u8 data_l)
{
	if((data_ptr[0] > 100) && (data_ptr[0] < 129))//自定义数据点
	{
        if((data_l <= 7) || (data_ptr[2] != 0x00)  || (data_ptr[3] != (data_l - 4)) || (data_ptr[4] != 0x01))//数据包不正确
        {
            return;
        }
        //格式整理
        data_ptr[2] = data_ptr[1];
        data_ptr[1] = data_ptr[0];
        data_ptr[0] = 0x01;
        u16 addrbuf = ((u16)data_ptr[5] << 8) + data_ptr[6];//地址
        if(lutec_get_address() == addrbuf)//自己的数据
        {
            lutec_protocol_dp_analysis(&data_ptr[1]);//dp解析
        }
        else //组播或者别人的指令
        {
            if((addrbuf == 0xFFFF) || (lutec_is_own_group(addrbuf) != 0))//自己的组播数据
            {
                lutec_protocol_dp_analysis(&data_ptr[1]);//dp解析
            }
            if(addrbuf >= 0xC000)//组播广播
            {
                lutec_ack_by_wifi(data_ptr[1]);//简单回复
                data_ptr[4] = 0x05;//转发
            }
            app_light_vendor_data_publish(addrbuf, data_ptr, data_l);//发出
        }		
	}
    else //其他涂鸦数据点---？
    {

    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_ack_by_wifi(u8 ackid)
{
    uint8_t data_buffer[16] = {0};

    data_buffer[0]  = 0x55;
    data_buffer[1]  = 0xAA;
    data_buffer[2]  = 0x00;
    data_buffer[3]  = 0x07;
    data_buffer[4]  = 0x00;
    data_buffer[5]  = 0x09;
    data_buffer[6]  = 0x7C;  
    data_buffer[7]  = 0x00;     
    data_buffer[8]  = 0x00; 
    data_buffer[9]  = 0x05; 
    data_buffer[10] = 0x02; 
    //u16 addrbuf = lutec_get_address();
    data_buffer[11] = (u8)(lutec_get_address() >> 8);
    data_buffer[12] = (u8)(lutec_get_address());
    data_buffer[13] = ackid;
    data_buffer[14] = 0x06;
    data_buffer[15] = lutec_check_sum(data_buffer, 15);        

    hal_uart_send(data_buffer, 16);
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: dpID, 指向|地址|参数，长度
*返回值: 
-------------------------------------------------------------------------*/
void lutec_updata_by_wifi(u8* data_ptr, u8 data_l)
{
    uint8_t data_buffer[255] = {0};
    u8 sub_j = 0;

    data_buffer[0]  = 0x55;
    data_buffer[1]  = 0xAA;
    data_buffer[2]  = 0x00;
    data_buffer[3]  = 0x07; //命令字
    data_buffer[4]  = 0x00; //数据长度高8位
    data_buffer[5]  = data_l; //数据长度低8位
    data_buffer[6]  = data_ptr[1];  //dpID
    data_buffer[7]  = 0x00; //数据类型
    data_buffer[8]  = 0x00; //功能长度高8位
    for(sub_j = 3; sub_j < data_l; sub_j++)
    {
        data_buffer[sub_j + 6] = data_ptr[sub_j];
    }
    data_buffer[sub_j + 6] = lutec_check_sum(data_buffer, sub_j + 6);        

    hal_uart_send(data_buffer, sub_j + 7);
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: dpID，参数，长度
*返回值: 
-------------------------------------------------------------------------*/
void lutec_bt_data_up_by_wifi(u8 dpID_v, u8* data_ptr, u8 data_l)
{
    uint8_t data_buffer[255] = {0};

    data_buffer[0]  = 0x55;
    data_buffer[1]  = 0xAA;
    data_buffer[2]  = 0x00;
    data_buffer[3]  = 0x07; //命令字
    data_buffer[4]  = 0x00; //数据长度
    data_buffer[5]  = data_l + 7; 
    data_buffer[6]  = dpID_v;  //dpID
    data_buffer[7]  = 0x00; //数据类型
    data_buffer[8]  = 0x00; //功能长度    
    data_buffer[9]  = data_l + 3; 
    data_buffer[10]  = 0x02; //指向--远程上行
    //u16 addrbuf = lutec_get_address();
    data_buffer[11] = (u8)(lutec_get_address() >> 8);
    data_buffer[12] = (u8)(lutec_get_address());
    u8 addrbuf = 0;
    for(addrbuf = 0; addrbuf < data_l; addrbuf++)
    {
        data_buffer[addrbuf + 13] = data_ptr[addrbuf];
    }
    data_buffer[addrbuf + 13] = lutec_check_sum(data_buffer, data_l + 13);        

    hal_uart_send(data_buffer, data_l + 14);
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 序号+类别+指令+功能ID+功能参数，长度
*返回值: 
-------------------------------------------------------------------------*/
void lutec_reply_wifi_module(u8* data_ptr, u8 data_l)
{
    uint8_t data_buffer[255] = {0};
    u8 sub_j = 0;

    data_buffer[0]  = 0x56;//头
    data_buffer[1]  = 0x56;
    data_buffer[2]  = 0x00;//长度
    data_buffer[3]  = data_l + 2; 
    data_buffer[4]  = 0x01; //版本
    for(sub_j = 0; sub_j < data_l; sub_j++)//序号+类别+指令+功能ID+功能参数
    {
        data_buffer[sub_j + 5] = data_ptr[sub_j];
    }
    data_buffer[sub_j + 5] = lutec_check_sum(data_buffer, data_l + 5);        

    hal_uart_send(data_buffer, data_l + 6);
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 功能ID + 功能参数，长度，序号，指令类别
*返回值: 
-------------------------------------------------------------------------*/
void lutec_updata_wifi_module(u8* data_ptr, u8 data_l,u8 od_num, u8 cmd_type)
{
    uint8_t data_buffer[255] = {0};
    u8 sub_j = 0;

    data_buffer[0]  = 0x56;//头
    data_buffer[1]  = 0x56;
    data_buffer[2]  = 0x00;//长度
    data_buffer[3]  = data_l + 5; 
    data_buffer[4]  = 0x01; //版本
    data_buffer[5]  = od_num;//序号
    data_buffer[6]  = cmd_type; //类别
    data_buffer[7]  = 0x02; //指令--MCU数据上报
    for(sub_j = 0; sub_j < data_l; sub_j++)//功能ID + 功能参数
    {
        data_buffer[sub_j + 8] = data_ptr[sub_j];
    }
    data_buffer[sub_j + 8] = lutec_check_sum(data_buffer, data_l + 8);//校验   

    hal_uart_send(data_buffer, data_l + 9);
}




















/***************************************File end********************************************/












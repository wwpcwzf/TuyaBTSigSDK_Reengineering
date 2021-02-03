/****************************************************************************************
> File Name: lutec_wifi.c
> Description:
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_wifi.h"

#include "lutec_main.h"
#include "lutec_bt_dp.h"

#include "app_common.h"
#include "hal_uart.h"
#include "ty_fifo.h"

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_check_sum(u8* data, u8 len)
{
	u8 i, sum = 0;
	for(i = 0; i < len; i++)
	{
		sum += data[i];
	}
	return sum;
}




/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_uart_server_run(void)
{
	u8 num = ty_fifo_get_size();

	if(num < MIN_PACKET_LEN) // MIN_PACKET_LEN = 7
	{
		return;
	}
	
	//1?judge head
	u8 data_buf[255] = {0};
	ty_fifo_read(data_buf, MIN_PACKET_LEN);
	if((data_buf[0] != 0x55) || ((data_buf[1] != 0xCC) && (data_buf[1] != 0xAA)) || (data_buf[2] != 0x00)) //识别0x55cc00和0x55aa00
	{
		ty_fifo_pop(1);
		return;
	}

	//2.judge if it's a whole frame
	u16 data_len = 0;
	data_len = ((u16)data_buf[4] << 8) + data_buf[5];
	if(data_len > MAX_DATA_LEN)
	{
		ty_fifo_pop(3);
		return;
	}
	if((data_len + 7) > num)//未接收完数据包
	{
		return;
	}
	
	//3.judge check sum 
	ty_fifo_read(data_buf, data_len + 7);
	u8 ck_sum = 0;
	ck_sum = lutec_check_sum(data_buf, data_len + 6);
	if(ck_sum != data_buf[data_len + 6])
	{
		ty_fifo_pop(3);
		return;
	}
	ty_fifo_pop(data_len + 7); //释放缓存

	//4.数据处理
	switch(data_buf[1])
	{
		case 0xCC: //wifi模组数据
		    lutec_wifi_module_data(&data_buf[6], data_len);
		break;
		case 0xBB: //串口产测指令
		break;
		case 0xAA: //蓝牙远程下行数据            
            lutec_remote_bluetooth_data(&data_buf[6], data_len);            
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
void lutec_wifi_module_data(u8* cmd_ptr, u8 para_l)
{
	switch(cmd_ptr[0])
	{
		case 0x6B: //107 WiFi配网

		break;
		case 0x6c: //108 重置WiFi模组

		break;
		case 0x6F: //111 PIR感应

		break;
		case 0x71: //113 时钟同步

		break;
		case 0x7F: //127 WiFi网络状态

		break;
		case 0x80: //128 WiFi模组识别

		break;
		default:
		break;
	}
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
        if(lutec_get_address() == addrbuf)
        {
            lutec_protocol_dp_analysis(&data_ptr[1]);//dp解析
        }
        else
        {
            if((addrbuf == 0xFFFF) || (lutec_is_own_group(addrbuf) != 0))
            {
                lutec_protocol_dp_analysis(&data_ptr[1]);//dp解析
            }
            if(addrbuf >= 0xC000)//组播广播
            {
                lutec_ack_by_wifi(data_ptr[1]);//简单回复
                data_ptr[4] = 0x06;//转发
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
    u16 addrbuf = lutec_get_address();
    data_buffer[11] = (addrbuf >> 8) & 0xFF;
    data_buffer[12] = (addrbuf >> 0) & 0xFF;
    data_buffer[13] = ackid;
    data_buffer[14] = 0x06;
    data_buffer[15] = lutec_check_sum(data_buffer, 15);        

    hal_uart_send(data_buffer, 16);
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
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


/***************************************File end********************************************/


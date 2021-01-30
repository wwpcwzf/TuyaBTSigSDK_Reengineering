/****************************************************************************************
> File Name: lutec_wifi.c
> Description£º
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_wifi.h"





static u8 lutec_check_sum(u8* data, u8 len)
{
  u8 i, sum = 0;
  for(i = 0; i < len; i++)
  {
    sum += data[i];
  }
  return sum;
}





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
  if((data_buf[0] != 0x55) || ((data_buf[1] != 0xCC) && (data_buf[1] != 0xAA)) || (data_buf[2] != 0x00)) //??0x55cc00?0x55aa00
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
  if((data_len + 7) > num)//?????????????
  {
    return;
  }
  
  //3.judge check sum 
  ty_fifo_read(data_buf, data_len + 7);
  u8 ck_sum = 0;
  ck_sum = lutec_check_sum(data_buf, data_len + 6);
  if(ck_sum != buf[data_len + 6])
  {
    ty_fifo_pop(3);
    return;
  }
  ty_fifo_pop(data_len + 7); //????

  //4?????
  switch(data_buf[1])
  {
    case 0xCC: //wifi????
      lutec_wifi_module_data(&data_buf[6], data_len);
    break;
    case 0xBB: //????
    break;
    case 0xAA: //????
      lutec_remote_bluetooth_data(&data_buf[6], data_len);
    break;
    default:
    break;
  }
}


void lutec_wifi_module_data(u8* cmd_ptr, u8 para_l)
{
  switch(cmd_ptr[0])
  {
    case 0x6B: //107 WiFi??

    break;
    case 0x6c: //108 ??WiFi??

    break;
    case 0x6F: //111 PIR??

    break;
    case 0x71: //113 ????

    break;
    case 0x7F: //127 WiFi????

    break;
    case 0x80: //128 WiFi??????

    break;
    default:
    break;
  }
}


void lutec_remote_bluetooth_data(u8* data_ptr, u8 data_l)
{
  if((data_ptr[0] > 100) && (data_ptr[0] < 129))//
  {

    
  }
}






/***************************************File end********************************************/


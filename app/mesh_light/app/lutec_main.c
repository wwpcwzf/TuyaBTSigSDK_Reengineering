/****************************************************************************************
> File Name: lutec_main.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_main.h"




#include "hal_uart.h"
#include "ty_timer_event.h"



#include "lutec_lux.h"
#include "lutec_pir.h"
#include "lutec_key.h"
#include "lutec_led.h"
#include "lutec_tick.h"
#include "lutec_wifi.h"


static u16 self_address = 0;
static u16 self_group_address_list[9] = {0};




/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_address(u16 addr)
{
    self_address = addr;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u16 lutec_get_address(void)
{
    return self_address;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_is_own_group(u16 g_addr)
{
    u8 i = 0;
    for(i = 0; i < self_group_address_list[0]; i++)
    {
        if(self_group_address_list[i] == g_addr)
        {
            return 1;
        }
    }
    return 0;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_join_group(u16 new_group)
{
    u8 i = 0;
    for(i = 0; i < self_group_address_list[0]; i++)
    {
        if(self_group_address_list[i] == new_group)
        {
            return;
        }
    }
    self_group_address_list[self_group_address_list[0]] = new_group;
    self_group_address_list[0] += 1;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_exit_group(u16 the_group)
{
    u8 i = 0;
    for(i = 0; i < self_group_address_list[0]; i++)
    {
        if(self_group_address_list[i] == the_group)
        {
            for( ; i < self_group_address_list[0]; i++)
            {
                self_group_address_list[i] = self_group_address_list[i + 1];
            }
        }
    }
    self_group_address_list[0] -= 1;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_init_group(void)
{
    u8 i = 0;
    for(i = 0; i < 8; i++)
    {
        self_group_address_list[i] == 0;
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_init(void)
{
    lutec_tick_start();
	lutec_pir_init();
    lutec_lux_init();
    lutec_key_init();
    lutec_led_init();
    
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_loop(void)
{
    lutec_lux_loop();
    lutec_key_loop();
    lutec_pir_loop();
    lutec_led_loop();

    lutec_device_loop();    
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_device_loop(void)
{
    //---------------------------------------按键
    switch(lutec_get_key_action())
    {
        case 1://短按
            lutec_led_trigger();
            break;
        case 2://长按
            kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;            
            break;
        default:
            break;
    }
    //---------------------------------------重置
    lutec_device_reset();
    //---------------------------------------照度


    //---------------------------------------延时
    lutec_delay_dim_loop();

    //---------------------------------------WiFi
    
    // lutec_pir_set_sensitivity(duty_f++);
    // hal_uart_send(&duty_f, 1);
    
}

/*-------------------------------------------------------------------------
*简  介: 蓝牙不配网使用---长灭
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_config_close_callback(void)
{
    lutec_led_flash_set(0);//不闪
    lutec_led_onoff(0);//关灯
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_mesh_state_callback(mesh_state_t m_state)
{
  switch(m_state) 
  {
    case NODE_POWER_ON_UNPROVISION: //上电未配网
        lutec_led_flash_set(50);//快闪0.5s
        break;
    case NODE_POWER_ON_IN_MESH://上电已配网    
        //break;
    case NODE_PROVISION_SUCCESS://配网成功    
        if(lutec_get_wifi_state() == 0x74)//wifi联网成功
        {   //常亮         
            lutec_led_flash_set(0);
            lutec_led_onoff(1);
        }
        else
        {
            lutec_led_flash_set(100);//慢闪1s
        }
        break;
    case NODE_KICK_OUT://重置完成
        lutec_reset_wifi_module();//重置WiFi模块
        lutec_led_flash_set(50);//快闪0.5s
    break;
    case NODE_MESH_RESET://mesh网络重置 --？
        lutec_led_flash_set(50);//快闪0.5s   
    break;
    case NODE_RECOVER_IN_MESH://恢复网络
    
    break;
    case TY_OTA_START:    
    break;
    case TY_OTA_SUCCESS:
    case TY_OTA_FAIL:    
    break;
    case TY_GROUP_SUB_ADD://加入组
    
    break;
    case TY_GROUP_SUB_DEL://退出组
    
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
void lutec_save_data_set_variable_callback(LIGHT_CUST_DATA_FLASH_T *Data_Ptr)
{


}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_read_saved_data_callback(int isready, LIGHT_CUST_DATA_FLASH_T R_Data)
{
  if(isready != 0)//LIGHT_OK = 0 ????
    return;



}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_scenes_control_start(void)
{

  
}
/***************************************File end********************************************/


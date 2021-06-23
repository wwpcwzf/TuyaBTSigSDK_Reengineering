/****************************************************************************************
> File Name: lutec_main.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_main.h"




#include "hal_gpio.h"
#include "hal_uart.h"
#include "hal_pwm.h"
#include "ty_timer_event.h"
#include "app_light_cmd.h"
#include "ty_light_save_soc_flash.h"


#include "lutec_bt_dp.h"
#include "lutec_lux.h"
#include "lutec_pir.h"
#include "lutec_key.h"
#include "lutec_led.h"
#include "lutec_tick.h"
#include "lutec_wifi.h"


#include "ty_string_op.h"


static u8 device_state = 0x51;


static u16 self_address = 0;
static u8 self_group_num = 0;
static u16 self_group_address_list[8] = {0};

static u8 mesh_connect_flag = 0;

static u8 self_pir_dimmer_enable_flag = 1;

u8 lutec_get_mesh_connect_lfag(void)
{
    //hal_uart_send(&mesh_connect_flag, 1); 
    return mesh_connect_flag;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_light_control_by_lux(void)
{
    

}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_self_pir_dimmer_onoff(u8 set_val)
{
    self_pir_dimmer_enable_flag = set_val > 0 ? 1 : 0;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: u8 any_one（0-结束；1-无人；2-有人）
*返回值: 
-------------------------------------------------------------------------*/
void lutec_someone_control(u8 any_one)
{
    //自身感应亮灯     
    if(self_pir_dimmer_enable_flag > 0)    
    {      
        lutec_pir_dimmer(any_one);//有人
    }
    //发送联动指令    
#if PIR_BROADCAST_DEBUG 
    //hal_uart_send(&any_one, 1); 
    lutec_pir_sig_cmd_multicast(any_one, 0xFFFF);    
#else
    if(self_group_num > 0)//有组---组播
    {
    #if PIR_BROADCAST_SIG_CMD_ENABLE
        lutec_pir_sig_cmd_multicast(any_one, self_group_address_list[0]);
    #else
        lutec_pir_multicast(any_one, self_group_address_list[0]);
    #endif
    }
#endif
}

    static u8 pir_control_flag = 0;
    static u32 multicast_time_base = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_pir_someone(void)
{
    lutec_someone_control(0x02);
    //更新状态，开始计时
    pir_control_flag = 0x01;
    multicast_time_base = lutec_get_tick_10ms();
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_stop_pir_someone_flow(void)
{
    pir_control_flag = 0;
    multicast_time_base = 0;
}


/*-------------------------------------------------------------------------
*简  介: 联动功能-----上报功能在lutec_wifi.c中
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_light_control(void)
{

    u8 some_one_buf = 0;

    // if(lutec_get_switch_flag() != 0x02)//常亮或者长灭
    // {
    //     //更新状态，开始计时
    //     if(pir_control_flag > 0)
    //     {
    //         pir_control_flag = 0x00;
    //         multicast_time_base = 0x00;
    //     }
    //     return;
    // }
    // static u32 timebase = 0;
    //  if(lutec_get_interval_tick_10ms(timebase) > 200)
    //  {
    //      hal_uart_send(&pir_control_flag, 1);      
    //      timebase = lutec_get_tick_10ms();
    //  }
    some_one_buf = lutec_pir_someone();
    // if(some_one_buf > 0)//PIR有人
    // {                
    //             hal_uart_send(&pir_control_flag, 1);    
    // }

    switch(pir_control_flag)
    {
    case 0x00://空闲
        if(lutec_get_lux_flag() == 0x00)//照度未达到限值
        {
            break;
        }
        if(some_one_buf > 0)//PIR有人
        {
            lutec_someone_control(0x02);
            //更新状态，开始计时
            pir_control_flag = 0x01;
            multicast_time_base = lutec_get_tick_10ms();
        }
        break;
    case 0x01://有人亮灯
        if(some_one_buf > 0)//PIR有人
        {
            if(lutec_get_interval_tick_10ms(multicast_time_base) > PIR_LIGHT_CONTROL_TIME)//在次组播时间到
            {        
                lutec_someone_control(0x02);
                //更新状态，开始计时
                //pir_control_flag = 0x01;
                multicast_time_base = lutec_get_tick_10ms();
            }
        }
        else//PIR无人
        {
            if(lutec_get_interval_tick_10ms(multicast_time_base) > lutec_get_pir_delay_time_10ms(1))//有人亮灯时间到
            {                       
                lutec_someone_control(0x01);
                //更新状态，开始计时
                pir_control_flag = 0x02;
                multicast_time_base = lutec_get_tick_10ms();
            }
        }
        break;
    case 0x02://无人亮灯        
        if(some_one_buf > 0)//PIR有人
        {
            lutec_someone_control(0x02);
            //更新状态，开始计时
            pir_control_flag = 0x01;
            multicast_time_base = lutec_get_tick_10ms();
        }
        else//PIR无人
        {
            u32 buffer_time = lutec_get_interval_tick_10ms(multicast_time_base);
            //无人亮灯时间到 || 10分钟后照度超过阈值
            if(((buffer_time > 60000) && (lutec_get_lux_flag() == 0x00)) || (buffer_time > lutec_get_pir_delay_time_10ms(0)))
            {
                lutec_someone_control(0x00);
                //更新状态，开始计时
                pir_control_flag = 0x00;
                multicast_time_base = 0x00;
            }
        }
    break;
    default:
        pir_control_flag = 0x00;
        multicast_time_base = 0x00;
        break;

    }
    some_one_buf = 0;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_address(u16 addr)
{
    if(self_address != addr)
    {
        self_address = addr;
    }
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
    for(i = 0; i < self_group_num; i++)
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
    if((lutec_is_own_group(new_group) == 1) || (self_group_num > 7))
    {
        return;
    }
    self_group_address_list[self_group_num] = new_group;
    self_group_num += 1;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_exit_group(u16 the_group)
{
    u8 i = 0;
    for(i = 0; i < self_group_num; i++)
    {
        if(self_group_address_list[i] == the_group)
        {
            for( ; i < (self_group_num - 1); i++)
            {
                self_group_address_list[i] = self_group_address_list[i + 1];
            }
            self_group_num -= 1;
            //return;
        }
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_all_group_addr(u8* save_group)
{
    u8 i = 0;
    for(i = 0; i < self_group_num; i++)
    {
        save_group[i << 1] = (u8)(self_group_address_list[i] >> 8);
        save_group[(i << 1) + 1] = (u8)(self_group_address_list[i]);
    }
    //hal_uart_send(&self_group_num, 1);
    return (self_group_num << 1);
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
        self_group_address_list[i] = 0;
    }
    self_group_num = 0;
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_init_all_addr(void)
{
    lutec_init_group();
    self_address = 0x0000;
}

/*-------------------------------------------------------------------------
*简  介: 回复
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_regain_all_addr(void)
{
    u16* addr_ptr = 0;
    u8 sub_t = 0;

    self_address = get_primary_ele_addr();
    //---------恢复组地址
    addr_ptr = tuya_group_addr_sub_list_get();
    self_group_num = 0;
    for(sub_t = 0; sub_t < 8; sub_t++)
    {
        if((addr_ptr[sub_t] > 0xBFFF) && (addr_ptr[sub_t] < 0xFF00))//可分配组地址
        {
            self_group_address_list[self_group_num] = addr_ptr[sub_t];
            self_group_num += 1;
        }
    }
}

/*-------------------------------------------------------------------------
*简  介: 测试输出所有组地址
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_send_group_sddr(void)
{
    u16 *addr_buf = 0;
    u8 send_buf[16] = {0};
    u8 sub_i = 0;

    addr_buf = tuya_group_addr_sub_list_get();

    for(sub_i = 0; sub_i < 8; sub_i++)
    {
        send_buf[sub_i << 1] = (uint8_t)(addr_buf[sub_i] >> 8);
        send_buf[(sub_i << 1) + 1] = (uint8_t)(addr_buf[sub_i]);
    }
    hal_uart_send(send_buf, 16);

    hal_uart_send(self_group_address_list, 18);
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_init(void)
{
//     u8 buffer[4] = {0xfe,0xfe,0xfe,0xfe};
//     hal_uart_send(buffer, 4);
    lutec_tick_start();
#if PIR_ENABLE
	lutec_pir_init();
#endif
    lutec_lux_init();
#if USER_KEY_ENABLE
    lutec_key_init();
#endif
#if USER_LED_ENABLE
    lutec_led_init(); 
#endif
    lutec_regain_all_addr();
    lutec_saved_data_init();    
#if LIGHT_DIM_CCT
    lutec_cct_temp_pin_init();
#endif
#if 0 //测试转换函数
    u8 hex_string[4] = {'a','b','9','F'};
    u8 result = 0;
    u8 hex_arr[2] = {0};
    hal_uart_send(hex_string, 4);
    hal_uart_send(&result, 1);
    hal_uart_send(hex_arr, 2);
    result = ty_string_op_hexstr2hex(hex_string, 4, hex_arr);
    
    hal_uart_send(hex_string, 4);
    hal_uart_send(&result, 1);
    hal_uart_send(hex_arr, 2);
    hex_arr[0] = 0xF5;
    hex_arr[1] = 0xa8;

    result = ty_string_op_hexarry2hexstr(hex_string, 4, hex_arr);        

    hal_uart_send(hex_string, 4);
    hal_uart_send(&result, 1);
    hal_uart_send(hex_arr, 2);

    u16 buf16_v = 0x55aa;

    result = ty_string_op_hex2hexstr(hex_string, 4, buf16_v);     

    hal_uart_send(hex_string, 4);
    hal_uart_send(&result, 1);
    hal_uart_send(hex_arr, 2);
#endif

}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_loop(void)
{
    lutec_lux_loop();
#if USER_KEY_ENABLE
    lutec_key_loop();
#endif
#if PIR_ENABLE
    lutec_pir_loop();
#endif
#if USER_LED_ENABLE
    lutec_led_loop();
#endif

    lutec_device_loop();    
    
    lutec_production_test_loop();
}

static u32 device_reset_time_base = 0;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_device_reset(void)
{
    device_reset_time_base = lutec_get_tick_10ms();
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
            device_reset_time_base = lutec_get_tick_10ms();
            break;
        default:
            break;
    }
    //---------------------------------------重置
    lutec_device_reset_control();

    //---------------------------------------照度
    lutec_light_control_by_lux();

    //---------------------------------------延时
    lutec_delay_dim_loop();

    //---------------------------------------PIR
    lutec_pir_light_control();

    //---------------------------------------WiFi
    lutec_wifi_control_loop();

    
    //---------------------------------------休眠
    //hal_cpu_sleep_wakeup(HAL_SUSPEND_MODE, HAL_PM_WAKEUP_PAD|HAL_PM_WAKEUP_TIMER, hal_clock_get_system_tick() + 250*1000*HAL_CLOCK_1US_TICKS);
    //void tuya_mesh_rf_power_set(TUYA_RF_Power level);
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_device_reset_control(void)
{
    lutec_bt_module_reset_control();
    if(device_reset_time_base == 0)
    {
        return;
    }
    lutec_reset_wifi_module();

    if(lutec_get_interval_tick_10ms(device_reset_time_base) >= 200)
    {
        device_reset_time_base = 0;
        kick_out();//蓝牙重置reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT; 
    }
}

/*-------------------------------------------------------------------------
*简  介: 蓝牙停止配网回调函数---长灭
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

    #if BT_STATUE_DEBUG
    u8 flag_date[2] = {0xEE, 0x00};
    #endif

    switch(m_state) 
    {
    case NODE_PROVISION_SUCCESS://配网成功 
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE5;
            hal_uart_send(flag_date, 2);
        #endif

        lutec_led_flash_set(100);//慢闪1s
        lutec_regain_all_addr();
        //reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT; //重启MCU
        if(device_state < 0x54)
        {
            device_state = 0x54;//运行状态                
            #if FLASH_SAVE_ENABLE
            //app_light_ctrl_data_auto_save_start(60000);
            lutec_save_data();
            #endif
        }
        mesh_connect_flag = 1;
    break;   
    case NODE_POWER_ON_IN_MESH://上电已配网 
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE4;
            hal_uart_send(flag_date, 2);
        #endif
        lutec_regain_all_addr();
        if(lutec_get_wifi_state() == 0x74)//wifi联网成功
        {   //常亮         
            lutec_led_flash_set(0);
            lutec_led_onoff(1);
        }
        else
        {
            lutec_led_flash_set(100);//慢闪1s
        }
        mesh_connect_flag = 1;
    break;
    case NODE_POWER_ON_UNPROVISION: //上电未配网
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE3;
            hal_uart_send(flag_date, 2);
        #endif
        lutec_led_flash_set(50);//快闪0.5s
        lutec_init_all_addr();
        lutec_init_saved_data();
        mesh_connect_flag = 0;
    break;
    case NODE_KICK_OUT://重置完成
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE2;
            hal_uart_send(flag_date, 2);
        #endif
        lutec_led_flash_set(50);//快闪0.5s
        lutec_init_all_addr();
        lutec_init_saved_data();        
        //reg_pwdn_ctrl |= FLD_PWDN_CTRL_REBOOT;
        mesh_connect_flag = 0;
    break;
    case NODE_MESH_RESET://mesh网络重置 
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE1;
            hal_uart_send(flag_date, 2);
        #endif
        lutec_led_flash_set(50);//快闪0.5s  
        lutec_init_all_addr();
        mesh_connect_flag = 0;
    break;
    case NODE_RECOVER_IN_MESH://恢复网络
        #if BT_STATUE_DEBUG   
            flag_date[1] = 0xE0;
            hal_uart_send(flag_date, 2);
        #endif
        lutec_regain_all_addr();
        lutec_led_flash_set(100);
        mesh_connect_flag = 1;
    break;
    case TY_OTA_START:    
    break;
    case TY_OTA_SUCCESS:
    case TY_OTA_FAIL:    
    break;
    case TY_GROUP_SUB_ADD://加入组 --测试
        lutec_regain_all_addr();
        //lutec_send_group_sddr();
    break;
    case TY_GROUP_SUB_DEL://退出组 --测试
        lutec_regain_all_addr();
        //lutec_send_group_sddr();
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
void lutec_scenes_control_start(void)
{

  
}



static u8 production_test_flag = 0;
/*-------------------------------------------------------------------------
*简  介: 产测流程
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_production_test_loop(void)
{
    if(production_test_flag)
    {
        if(device_state != 0x52)
        {
            device_state = 0x52;                
            #if FLASH_SAVE_ENABLE
            app_light_ctrl_data_auto_save_start(5000);
            #endif
        }
        //--------------------------------产测


        
    }  
    if((production_test_flag == 0) && (device_state == 0x52))
    {
        device_state = 0x53;                
        #if FLASH_SAVE_ENABLE
        app_light_ctrl_data_auto_save_start(5000);
        #endif
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_production_test_on(u8 en_v)
{
    production_test_flag = en_v > 0 ? 1 : 0;
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_production_test_state(void)
{
    return production_test_flag;
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_device_state(void)
{
    return device_state;
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_device_state(u8 state_v)
{
    device_state = state_v;
}

//========================================================================================= FLASH存储相关

static u8 save_data_flag = 0;//3--5s后存储；1--自定义存储数据；0--不存储
/*-------------------------------------------------------------------------
*简  介: dp点数据处理后，设置标记值，用于5s后自动存储数据
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_save_data_flag(void)
{
    save_data_flag = 3;
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 0-不存储，>0-存储
-------------------------------------------------------------------------*/
u8 lutec_get_save_data_flag(void)
{
    if(save_data_flag == 3)
    {
        save_data_flag = 1;
        #if FLASH_SAVE_ENABLE
        return 1;
        #endif
    }
    return 0;
}

// typedef struct {
//     u16 f_save_flag;//存储标记
//     u8 f_devicce_state;//设备状态
//     u8 f_onoff;//开关AUTO
//     u8 f_pir_up_en;//上报开关
//     u8 f_pir_inform_en;//通报开关
//     u8 f_pir_sensitivity;//pir灵敏度
//     u8 f_lux_threshold;//照度阈值
//     u32 f_noone_time;//无人亮灯时间
//     u8 f_noone_dim_p[11];//无人亮灯参数
//     u32 f_someone_time;//有人亮灯时间
//     u8 f_someone_dim_p[11];//有人亮灯参数
// }LUTEC_DATA_FLASH_T;
/*-------------------------------------------------------------------------
*简  介: 系统数据初始化（读取）
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_saved_data_init(void)
{
#if FLASH_SAVE_ENABLE
    LUTEC_DATA_FLASH_T data_buffer;

    if(ty_light_save_soc_flash_read(EEPROM_START_ADDRESS, sizeof(LUTEC_DATA_FLASH_T), &data_buffer) != 0)//LIGHT_OK = 0  读取错误
    {
        return;
    } 
    
    #if FLASH_DEBUG
    hal_uart_send(&data_buffer, sizeof(LUTEC_DATA_FLASH_T));
    #endif

    if(data_buffer.f_save_flag != 0xA55A)//非有效值
    {
        return;
    }
    device_state = data_buffer.f_devicce_state;
    lutec_light_switch(data_buffer.f_onoff);
    lutec_pir_updata_set(data_buffer.f_pir_up_en);
    lutec_pir_report_set(data_buffer.f_pir_inform_en);
    lutec_pir_set_sensitivity(data_buffer.f_pir_sensitivity);
    lutec_set_lux_threshold(data_buffer.f_lux_threshold);
    lutec_set_pir_dim_para(data_buffer.f_noone_time, data_buffer.f_noone_dim_p, data_buffer.f_someone_time, data_buffer.f_someone_dim_p);

    // #if FLASH_DEBUG
    // hal_uart_send(&data_buffer, sizeof(LUTEC_DATA_FLASH_T));
    // #endif

#endif
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_save_data(void)
{
    if(save_data_flag != 1)
    {
        return;
    }
    save_data_flag = 0;

#if FLASH_SAVE_ENABLE

    LUTEC_DATA_FLASH_T save_data_buf;


    memset(&save_data_buf, 0, sizeof(LUTEC_DATA_FLASH_T));     
    save_data_buf.f_save_flag = 0xA55A;
    save_data_buf.f_devicce_state = device_state;
    save_data_buf.f_onoff = lutec_get_switch_flag();
    lutec_get_pir_updata_conf(&save_data_buf.f_pir_up_en);
    save_data_buf.f_pir_inform_en = lutec_pir_report_is_enable();
    save_data_buf.f_pir_sensitivity = lutec_pir_get_sensitivity();
    save_data_buf.f_lux_threshold = lutec_get_lux_threshold();
    save_data_buf.f_someone_time = lutec_get_pir_delay_time_s(1);
    save_data_buf.f_noone_time = lutec_get_pir_delay_time_s(0);
    lutec_get_pir_dim_para(save_data_buf.f_noone_dim_p, save_data_buf.f_someone_dim_p);    

    #if FLASH_DEBUG    
    hal_uart_send(&save_data_buf, sizeof(LUTEC_DATA_FLASH_T));
    #endif
    ty_light_save_soc_flash_erase_sector(EEPROM_START_ADDRESS);
    ty_light_save_soc_flash_write(EEPROM_START_ADDRESS, &save_data_buf, sizeof(LUTEC_DATA_FLASH_T));
    
    #if FLASH_DEBUG    
    memset(&save_data_buf, 0, sizeof(LUTEC_DATA_FLASH_T));  
    ty_light_save_soc_flash_read(EEPROM_START_ADDRESS, sizeof(LUTEC_DATA_FLASH_T), &save_data_buf);    
    hal_uart_send(&save_data_buf, sizeof(LUTEC_DATA_FLASH_T));
    #endif

#endif
}

/*-------------------------------------------------------------------------
*简  介: 初始化系统数据（写入）
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_init_saved_data(void)
{
#if FLASH_SAVE_ENABLE

    LUTEC_DATA_FLASH_T tLutecData;

    memset(&tLutecData, 0, sizeof(LUTEC_DATA_FLASH_T));
    tLutecData.f_save_flag = 0xA55A;
    tLutecData.f_devicce_state = 0x53;
    tLutecData.f_onoff = SWITCH_ONOFF_DEF;
    tLutecData.f_pir_up_en = PIR_UPDATA_APP_DEF;
    tLutecData.f_pir_inform_en = PIR_UPDATA_WIFI_DEF;
    tLutecData.f_pir_sensitivity = PIR_SENSITIVITY_DEF;
    tLutecData.f_lux_threshold = LUX_THRESHOLD_DEF;

    tLutecData.f_noone_time = NO_ONE_LIGHT_ON_TIME;
    // static u8 noone_pir_light_par[11] = {0x13,0x00,0x32,0x01,0xF4,0,0,0,0,0,0};
    tLutecData.f_noone_dim_p[0] = 0x13;
    tLutecData.f_noone_dim_p[2] = 0x32;
    tLutecData.f_noone_dim_p[3] = 0x01;
    tLutecData.f_noone_dim_p[4] = 0xF4;

    tLutecData.f_someone_time = SOMEONE_LIGHT_ON_TIME;
    // static u8 anyone_pir_light_par[11] = {0x13,0x03,0xE8,0x03,0xE8,0,0,0,0,0,0};
    tLutecData.f_someone_dim_p[0] = 0x13;
    tLutecData.f_someone_dim_p[1] = 0x03;
    tLutecData.f_someone_dim_p[2] = 0xE8;
    tLutecData.f_someone_dim_p[3] = 0x03;
    tLutecData.f_someone_dim_p[4] = 0xE8;

    ty_light_save_soc_flash_erase_sector(EEPROM_START_ADDRESS);
    ty_light_save_soc_flash_write(EEPROM_START_ADDRESS, &tLutecData, sizeof(LUTEC_DATA_FLASH_T));

    lutec_saved_data_init();
    
    // #if FLASH_DEBUG
    // hal_uart_send(&tLutecData, sizeof(LUTEC_DATA_FLASH_T));
    // memset(&tLutecData, 0, sizeof(LUTEC_DATA_FLASH_T));
    // ty_light_save_soc_flash_read(EEPROM_START_ADDRESS, sizeof(LUTEC_DATA_FLASH_T), &tLutecData);    
    // hal_uart_send(&tLutecData, sizeof(LUTEC_DATA_FLASH_T));
    // #endif

#endif
}





//=======================================================================================sig mesh通用命令

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_send_onoff_sig_cmd(u16 dst_addr, u8 onoff)
{
    u8 buf_v = onoff;
    //                     源地址             目标地址          操作码            参数  参数长度
    tuya_mesh_data_send(lutec_get_address(), dst_addr, TUYA_G_ONOFF_SET_NOACK, &buf_v,   1,     0,    0);//开关
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_send_bright_sig_cmd(u16 dst_addr, u16 brightness_buffer)
{
    u32 temp_buf = 0;
    u8 para_buf[2] = {0x00,0x00};
    
    temp_buf = (u16)((u32)brightness_buffer * 13107 / 200);
           // hal_uart_send(&temp_buf, 2);
    para_buf[0] = temp_buf;
    para_buf[1] = temp_buf >> 8;
    //                     源地址            目标地址          操作码              参数      参数长度
    tuya_mesh_data_send(lutec_get_address(), dst_addr, TUYA_LIGHTNESS_SET_NOACK, para_buf,    2,    0,    0);//亮度
}
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_send_temperature_sig_cmd(u16 dst_addr, u16 temp_para)
{
    u16 temp_buf = 0;
    u8 para[4] = {0x00,0x00,0x00,0x00};
    temp_buf = (u16)((u32)temp_para * 96 / 5 + 800);
            //hal_uart_send(&temp_buf, 2);
    para[0] = temp_buf;
    para[1] = temp_buf >> 8;
    //                     源地址            目标地址          操作码              参数      参数长度
   tuya_mesh_data_send(lutec_get_address(), 0xFFFF, TUYA_LIGHT_CTL_TEMP_SET_NOACK, para, 4, 0, 0);//色温
}




//cct调光的色温PWM输出引脚初始化
#if LIGHT_DIM_CCT
void lutec_cct_temp_pin_init(void)
{
    hal_gpio_set_func(GPIO_PC3, GPIO_FUNC_AS_PWM);
    //PWM时钟源24mHz，24mHz = 12000 * 2kHz, 占空比7%计数 = 7 * 12000 /100 = 84
    //PWM通道，计数周期，比较值
	hal_pwm_set(PWM1_ID, 12000, 840);
	hal_pwm_start(PWM1_ID);

}


//cct调光的色温PWM输出函数
void lutec_set_cct_temperature(u16 ct_temp)
{
    hal_pwm_set_cmp(PWM1_ID, ct_temp * 12);
}
#endif















/***************************************File end********************************************/


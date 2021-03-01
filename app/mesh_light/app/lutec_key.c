/****************************************************************************************
> File Name: lutec_key.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_key.h"
#include "gpio.h "

#include "lutec_config.h"
#include "lutec_tick.h"
//#include "hal_uart.h"  //hal_uart_send(&duty_f, 1);
//#include "lutec_led.h"

static u8 key_action = 0;//0：没动作；1：短按；2：长按；
static u32 key_press_time_base = 0;


void lutec_key_init(void)
{
    gpio_set_func(GPIO_PD7, AS_GPIO);
    gpio_set_input_en(GPIO_PD7, 1);    
    gpio_set_output_en(GPIO_PD7, 0);
}


void lutec_key_loop(void)
{
    static u8 key_state = 0;
    u8 pin_state = gpio_read(GPIO_PD7) >> 7;
    
    switch(key_state)
    {
        case 0: //空闲
            if(pin_state == 1)//按下
            {
                if(key_press_time_base == 0)//按下计时
                {
                    key_press_time_base = lutec_get_tick_10ms();
                }
                else //消抖
                {
                    if(lutec_get_interval_tick_10ms(key_press_time_base) >= 2)//20ms
                    {
                        key_state = 1;
                    }
                }  
            }
            else
            {
                if(key_press_time_base != 0)
                {
                    key_press_time_base = 0;
                }
            }
        break;
        case 1: //按下
            if(pin_state == 0)//松开
            {
               key_action = 1;
               key_press_time_base = 0;
               key_state = 0;
            }
            else //长按判断
            {
                if(lutec_get_interval_tick_10ms(key_press_time_base) >= RESET_KEY_PRESS_TIME)//长按
                {
                    key_action = 2;
                    key_press_time_base = 0;
                    key_state = 2;
                }
            }
        break;
        case 2: //长按等待松开
            if(pin_state == 0)//松开
            {
                if(lutec_get_interval_tick_10ms(key_press_time_base) >= 1)//10ms
                {
                    key_action = 0;
                    key_press_time_base = 0;
                    key_state = 0;
                }
            }
        break;
        default:
            key_action = 0;
            key_press_time_base = 0;
            key_state = 0;
        break;        
    }
}

u8 lutec_get_key_action(void)
{
    if( key_action == 0) 
        return 0;

    u8 buf_v = key_action;
    key_action = 0;
    
    return buf_v;
}












/***************************************File end********************************************/


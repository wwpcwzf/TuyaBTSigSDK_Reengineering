/****************************************************************************************
> File Name: lutec_key.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_key.h"
#include "gpio.h "

#include "lutec_led.h"
//#include "hal_uart.h"

static u8 key_action = 0;


void lutec_key_init(void)
{
    gpio_set_func(GPIO_PD7, AS_GPIO);
    gpio_set_input_en(GPIO_PD7, 1);    
    gpio_set_output_en(GPIO_PD7, 0);
}


void lutec_key_loop(void)
{
    static u8 key_state_counter = 0;
    u8 pin_state = gpio_read(GPIO_PD7) >> 7;
    
    switch(key_action)
    {
        case 0: //空闲
            if(pin_state == 1)//按下
            {
                if(key_state_counter > 50)
                {
                    key_action = 1;
                    key_state_counter = 0;       
                }  
                else
                    key_state_counter++;              
            }
            else
            {
                key_state_counter = 0;
            }
        break;
        case 1: //按下
            if(pin_state == 0)//松开
            {
                key_state_counter++;
                if(key_state_counter > 5)
                {
                    key_action = 2;
                    key_state_counter = 0;
                }
            }
        break;
        case 2: //松开
        break;
        default:
            key_action = 0;
            key_state_counter = 0;
        break;        
    }
}

u8 lutec_get_key_action(void)
{
    if(key_action == 2)  
    {
        key_action = 0;
        return 2;
    } 
    return key_action;
}












/***************************************File end********************************************/


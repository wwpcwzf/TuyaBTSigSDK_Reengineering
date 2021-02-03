/****************************************************************************************
> File Name: lutec_led.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_led.h"
#include "lutec_tick.h"

#include "gpio.h "


#include "hal_uart.h"



void lutec_led_init(void)
{
    gpio_set_func(GPIO_PC0, AS_GPIO);
    gpio_set_output_en(GPIO_PC0, 1);    
    gpio_set_input_en(GPIO_PC0, 0);    
}


void lutec_led_onoff(u8 onoff)
{
    gpio_write(GPIO_PC0, onoff == 0 ? 0 : 1);
}


void lutec_led_trigger(void)
{
    gpio_write(GPIO_PC0, (REG_ADDR8(0x593) & 0x01) ^ 0x01);
}


void lutec_led_flash(u8 enbale_v)
{
    if(enbale_v == 0)
    {
        return;
    }
    static u32 flash_tick_base = 0;

    if(lutec_get_interval_tick_10ms(flash_tick_base) > 99)
    {
        flash_tick_base = lutec_get_tick_10ms();
        gpio_write(GPIO_PC0, (REG_ADDR8(0x593) & 0x01) ^ 0x01);
    }
}







/***************************************File end********************************************/


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

static u8 led_flash_time = 0;

void lutec_led_init(void)
{
    gpio_set_func(GPIO_PC0, AS_GPIO);
    gpio_set_output_en(GPIO_PC0, 1);    
    gpio_set_input_en(GPIO_PC0, 0);    
}


void lutec_led_onoff(u8 onoff)
{
    gpio_write(GPIO_PC0, onoff == 0 ? 1 : 0);
}


void lutec_led_trigger(void)
{
    gpio_write(GPIO_PC0, (REG_ADDR8(0x593) & 0x01) ^ 0x01);
}

/*-------------------------------------------------------------------------
*简  介: 设置指示灯闪烁时间
*参  数: 亮灭改变的时间间隔，单位10ms
*返回值: 无
-------------------------------------------------------------------------*/
void lutec_led_loop(void)
{
    static u32 flash_tick_base = 0;

    if(led_flash_time == 0)
    {
        return;
    }

    if(lutec_get_interval_tick_10ms(flash_tick_base) >= led_flash_time)
    {
        flash_tick_base = lutec_get_tick_10ms();
        gpio_write(GPIO_PC0, (REG_ADDR8(0x593) & 0x01) ^ 0x01);
    }
}

/*-------------------------------------------------------------------------
*简  介: 设置指示灯闪烁时间
*参  数: 亮灭改变的时间间隔，单位10ms
*返回值: 无
-------------------------------------------------------------------------*/
void lutec_led_flash_set(u8 interval_time)
{
    led_flash_time = interval_time;
}




/***************************************File end********************************************/


/****************************************************************************************
> File Name: lutec_pir.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_pir.h"



#include "hal_gpio.h"
#include "hal_pwm.h"


#include "hal_uart.h"

//有人标记
static u8 someone_flag = 0;

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_init(void)
{
    lutec_pir_set_pin_init();
    lutec_pir_signal_pin_init();
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_signal_pin_init(void)
{
    gpio_set_func(GPIO_PA0, AS_GPIO);
    gpio_set_input_en(GPIO_PA0, 1);    
    gpio_set_output_en(GPIO_PA0, 0);
    //pin_state = gpio_read(GPIO_PA0) & 0x01;
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_loop(void)
{
    static u16 counter  = 1;

    //someone_flag = gpio_read(GPIO_PA0) & 0x01;
    if((gpio_read(GPIO_PA0) & 0x01) == 0)//有人时间
    {
        if((counter++ > 10000) && (someone_flag == 0x01))
        {
            counter = 0;
            someone_flag = 0x00;
        }
    }
    else
    {
        counter = 0;
        someone_flag = 0x01;
        //hal_uart_send(&someone_flag, 1);
    }
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_pir_someone(void)
{
    return someone_flag;
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_set_pin_init(void)
{	
	hal_gpio_set_func(GPIO_PD2, GPIO_FUNC_AS_PWM);
	hal_pwm_set(PWM3_ID, 1200, 0);
	hal_pwm_start(PWM3_ID);
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_set_sensitivity(u8 percent)
{
    u16 buf16_v = percent * 12;

    if(buf16_v > 1200) 
    {
        buf16_v = 1200;
    }
    hal_pwm_set_cmp(PWM3_ID, buf16_v);
    //hal_pwm_set(PWM3_ID, 834, buf16_v);
}

















/***************************************File end********************************************/


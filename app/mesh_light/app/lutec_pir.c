/****************************************************************************************
> File Name: lutec_pir.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_pir.h"


#include "lutec_config.h"

#include "hal_gpio.h"
#include "hal_pwm.h"


#include "hal_uart.h"

#include "lutec_wifi.h"

//有人标记
static u8 anyone_flag = 0;//1-有人；0-无人




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

    // if((lutec_pir_report_enable() == 0) && (lutec_get_switch_flag () != 2))
    // {
    //     return;
    // }

    //someone_flag = gpio_read(GPIO_PA0) & 0x01;
    if((gpio_read(GPIO_PA0) & 0x01) == 0)//无人
    {
        if((counter++ > 10000) && (anyone_flag == 0x01))//8s
        {
            counter = 0;
            anyone_flag = 0x00;
        }
    }
    else //感应有人
    {
        counter = 0;
        if(anyone_flag == 0)
        {
            anyone_flag = 0x01;
            //lutec_pir_somebody();
            //hal_uart_send(&anyone_flag, 1);
        }
        
    }
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_pir_someone(void)
{
#if PIR_ENABLE
    return anyone_flag > 0 ? 1 : 0;
#else
    return 0;
#endif
}



/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_set_pin_init(void)
{	
	hal_gpio_set_func(GPIO_PD2, GPIO_FUNC_AS_PWM);
    //PWM时钟源24mHz，24mHz = 12000 * 2kHz, 占空比7%计数 = 7 * 12000 /100 = 84
    //PWM通道，计数周期，比较值
	hal_pwm_set(PWM3_ID, 12000, 840);
	hal_pwm_start(PWM3_ID);
}


static u8 pir_sensitivity = PIR_SENSITIVITY_DEF;
/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_pir_set_sensitivity(u8 percent)
{
    if(percent <= 100)
    {
        pir_sensitivity = percent;//
        hal_pwm_set_cmp(PWM3_ID, pir_sensitivity * 120);
        //hal_pwm_set(PWM3_ID, 834, buf16_v);
    }
}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_pir_get_sensitivity(void)
{
    return pir_sensitivity;
}















/***************************************File end********************************************/


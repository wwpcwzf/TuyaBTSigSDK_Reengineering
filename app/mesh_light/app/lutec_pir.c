/****************************************************************************************
> File Name: lutec_pir.c
> Description��
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_pir.h"



#include "hal_gpio.h"
#include "hal_pwm.h"


void lutec_pir_set_pin_init(void)
{
  hal_gpio_set_func(GPIO_PD2, GPIO_FUNC_AS_PWM);
  hal_pwm_set(PWM3_ID, 1000, 100);
  hal_pwm_start(PWM3_ID);
}





















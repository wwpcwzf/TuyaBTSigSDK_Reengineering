/****************************************************************************************
> File Name: lutec_main.c
> Description£º
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_main.h"

#include "lutec_lux.h"
#include "lutec_pir.h"



void lutec_main_init(void)
{

  lutec_pir_set_pin_init();
  lux_adc_init();

}










void lutec_main_loop(void)
{

  
}












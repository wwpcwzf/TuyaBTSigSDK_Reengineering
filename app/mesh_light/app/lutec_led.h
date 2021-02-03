/****************************************************************************************
> File Name: lutec_led.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_LED_H
#define _LUTEC_LED_H

#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"


void lutec_led_init(void);

void lutec_led_onoff(u8 onoff);

void lutec_led_trigger(void);

void lutec_led_flash(u8 enbale_v);

#endif

/***************************************File end********************************************/




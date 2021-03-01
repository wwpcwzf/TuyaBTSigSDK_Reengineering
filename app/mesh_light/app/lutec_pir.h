/****************************************************************************************
> File Name: lutec_pir.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_PIR_H
#define _LUTEC_PIR_H

    #include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"















void lutec_pir_init(void);

void lutec_pir_signal_pin_init(void);

void lutec_pir_loop(void);

u8 lutec_pir_someone(void);

void lutec_pir_set_pin_init(void);

void lutec_pir_set_sensitivity(u8 percent);

u8 lutec_pir_get_sensitivity(void);

#endif









/***************************************File end********************************************/

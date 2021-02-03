/****************************************************************************************
> File Name: lutec_tick.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/2/3        Create
        |
******************************************************************************************/
#ifndef _LUTEC_TICK_H
#define _LUTEC_TICK_H

	#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"




    



    void lutec_tick_start(void);
    int lutec_tick_callback(void);
    u32 lutec_get_tick_10ms(void);
    u32 lutec_get_interval_tick_10ms(u32 base_tick);









#endif

/***************************************File end********************************************/

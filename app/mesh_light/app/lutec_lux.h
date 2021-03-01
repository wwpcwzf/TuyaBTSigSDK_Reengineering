/****************************************************************************************
> File Name: lutec_lux.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_LUX_H
#define _LUTEC_LUX_H

    #include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"

	void adc_gpio_ain_init(void);

	void lutec_lux_init(void);

    void lutec_lux_loop(void);
    
    u8 lutec_get_lux_flag(void);

    void lutec_set_lux_threshold(u8 thsd_v);

    u8 lutec_get_lux_threshold(void);

#endif

/***************************************File end********************************************/




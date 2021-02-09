/****************************************************************************************
> File Name: lutec_bt_dp.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_BT_DP_H
#define _LUTEC_BT_DP_H

	#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"
















	void lutec_bluetooth_dp_data(u16 s_addr, u16 d_addr, u8 *par, int par_len);

    u8 lutec_protocol_dp_analysis(u8 *par);

    void lutec_ack_by_bt(uint8_t point_to, uint8_t ack_id, uint16_t sent_to_addr);

    void lutec_light_switch(u8 switch_v);

    u8 lutec_get_switch_flag(void);

    void lutec_light_dimmer(u8* para_ptr);
    
    u8 lutec_get_dimming_para(u8* get_ptr);

    u8 lutec_get_dim_para_len(u8 dim_flag);

    void lutec_light_delay_dimmer(u8* para_p);
    
    u8 lutec_get_delay_dimming_para(u8* get_p);

#endif
/***************************************File end********************************************/




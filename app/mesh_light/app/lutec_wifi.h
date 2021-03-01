/****************************************************************************************
> File Name: lutec_wifi.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_WIFI_H
#define _LUTEC_WIFI_H

	#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"








	#define MIN_PACKET_LEN       7
	#define MAX_PACKET_LEN       255
	#define MAX_DATA_LEN         248   //255-7










    void lutec_set_wifi_config_flag(u8 f_v);

	void lutec_uart_server_run(void);


	void lutec_wifi_module_data(u8* cmd_ptr, u8 para_l);

    u8 lutec_get_wifi_state(void);

    void lutec_reset_wifi_module(void);

	void lutec_remote_bluetooth_data(u8* data_ptr, u8 data_l);

    void lutec_ack_by_wifi(u8 ackid);

    void lutec_updata_by_wifi(u8* data_ptr, u8 data_l);

#endif

/***************************************File end********************************************/



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






    //蓝牙55AA00数据帧最小7字节，WiFi5656数据帧最小9字节
    #define WIFI_MIN_PACKET_LEN     9
	#define BT_MIN_PACKET_LEN       7

	#define MAX_PACKET_LEN          255


    void lutec_start_wifi_connect(void);

    void lutec_stop_wifi_connect(void);

    void lutec_wifi_control_loop(void);

    void lutec_pir_updata_control(void);

    u8 lutec_get_wifi_module_reset_state(void);

    void lutec_wifi_net_config(u8 cmd_order_number);

	void lutec_uart_server_run(void);

    u8 lutec_get_order_number(void);

	void lutec_wifi_module_data(u8* cmd_ptr, u8 para_l);

    u8 lutec_mr_wifi_dp(u8* para_ptr, u8 par_l);

    u8 lutec_pir_report_is_enable(void);

    void lutec_pir_report_set(u8 en_v);
    
    u8 lutec_wifi_dp(u8* para_ptr, u8 par_l);

    u8 lutec_get_wifi_state(void);

    u8 lutec_get_wifi_id(u8* get_ptr);

    void lutec_reset_wifi_module(void);

	void lutec_remote_bluetooth_data(u8* data_ptr, u8 data_l);

    void lutec_ack_by_wifi(u8 ackid);

    void lutec_updata_by_wifi(u8* data_ptr, u8 data_l);

    void lutec_bt_data_up_by_wifi(u8 dpID_v, u8* data_ptr, u8 data_l);

    void lutec_reply_wifi_module(u8* data_ptr, u8 data_l);

    void lutec_updata_wifi_module(u8* data_ptr, u8 data_l,u8 od_num, u8 cmd_type);

#endif

/***************************************File end********************************************/



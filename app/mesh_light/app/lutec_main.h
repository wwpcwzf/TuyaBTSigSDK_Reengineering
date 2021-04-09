/****************************************************************************************
> File Name: lutec_main.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_MAIN_H
#define _LUTEC_MAIN_H

	#include "tuya_sigmesh_hal.h"
	#include "ty_light_save_user_flash.h"

    #include "lutec_config.h"


    void lutec_light_control_by_lux(void);

    void lutec_someone_control(u8 any_one);

    void lutec_set_pir_someone(void);
    
    void lutec_pir_light_control(void);

    void lutec_set_address(u16 addr);

    u16 lutec_get_address(void);

    u8 lutec_is_own_group(u16 g_addr);

    void lutec_join_group(u16 new_group);

    void lutec_exit_group(u16 the_group);

    void lutec_init_all_addr(void);

    u8 lutec_get_all_group_addr(u8* save_group);

    void lutec_init_group(void);
    
    void lutec_regain_all_addr(void);

    void lutec_send_group_sddr(void);

	void lutec_main_init(void);

	void lutec_main_loop(void);

    void lutec_light_blink_control(void);

    void lutec_device_reset(void);

    void lutec_device_loop(void);
    
    void lutec_device_reset_control(void);
    
    void lutec_config_close_callback(void);

	void lutec_mesh_state_callback(mesh_state_t m_state);

	void lutec_scenes_control_start(void);

    void lutec_production_test_loop(void);
    
    void lutec_production_test_on(u8 en_v);

    u8 lutec_get_production_test_state(void);

    u8 lutec_get_device_state(void);

    void lutec_set_device_state(u8 state_v);

    void lutec_set_save_data_flag(void);
    
    u8 lutec_get_save_data_flag(void);

    void lutec_saved_data_init(void);

    void lutec_save_data(void);

    void lutec_init_saved_data(void);

    void lutec_send_onoff_sig_cmd(u16 dst_addr, u8 onoff);
    
    void lutec_send_bright_sig_cmd(u16 dst_addr, u16 brightness_buffer);
    
    void lutec_send_temperature_sig_cmd(u16 dst_addr, u16 temp_para);


#endif

/***************************************File end********************************************/

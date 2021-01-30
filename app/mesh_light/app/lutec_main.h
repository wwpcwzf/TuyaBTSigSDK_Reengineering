/****************************************************************************************
> File Name: lutec_main.h
> Description£º
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_MAIN_H
#define _LUTEC_MAIN_H

#include "tuya_sigmesh_hal.h"
#include "ty_light_save_user_flash.h"















void lutec_main_init(void);

void lutec_main_loop(void);

void lutec_mesh_state_callback(mesh_state_t m_state);

void lutec_save_data_set_variable_callback(LIGHT_CUST_DATA_FLASH_T *Data_Ptr);
void lutec_read_saved_data_callback(int isready, LIGHT_CUST_DATA_FLASH_T R_Data);

void lutec_scenes_control_start(void);














#endif

/***************************************File end********************************************/

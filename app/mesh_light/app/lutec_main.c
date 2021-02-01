/****************************************************************************************
> File Name: lutec_main.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/


#include "lutec_main.h"

#include "lutec_lux.h"
#include "lutec_pir.h"




/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_init(void)
{

	lutec_pir_set_pin_init();
	lux_adc_init();

}









/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_main_loop(void)
{

  
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_mesh_state_callback(mesh_state_t m_state)
{
  switch(m_state) 
  {
    case NODE_POWER_ON_UNPROVISION: //
    
    break;
    case NODE_POWER_ON_IN_MESH:
    
    break;
    case NODE_PROVISION_SUCCESS:
    
    break;
    case NODE_KICK_OUT:
    
    break;
    case NODE_MESH_RESET:
    
    break;
    case NODE_RECOVER_IN_MESH:
    
    break;
    case TY_OTA_START:
    
    break;
    case TY_OTA_SUCCESS:
    case TY_OTA_FAIL:
    
    break;
    case TY_GROUP_SUB_ADD:
    
    break;
    case TY_GROUP_SUB_DEL:
    
    break;
    default:

    break;
  }

}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_save_data_set_variable_callback(LIGHT_CUST_DATA_FLASH_T *Data_Ptr)
{


}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_read_saved_data_callback(int isready, LIGHT_CUST_DATA_FLASH_T R_Data)
{
  if(isready != 0)//LIGHT_OK = 0 ????
    return;



}

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_scenes_control_start(void)
{

  
}
/***************************************File end********************************************/


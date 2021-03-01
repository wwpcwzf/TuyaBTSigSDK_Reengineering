/****************************************************************************************
> File Name: lutec_config.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_CONFIG_H
#define _LUTEC_CONFIG_H

#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"





//---------------------------------------------------------------功能定义
#define      RESET_KEY_PRESS_TIME              500  //重置长按时间5s--单位10ms

#define      MESH_CONFIG_TIME                  10   //重置后允许蓝牙配网时间10min--单位分钟









//--------------------------------------------------------------常用功能
u8 lutec_check_sum(u8* data, u8 len);

u8 lutec_string_compare(u8* str1_ptr, u8* str2_ptr, u8 str_len);



#endif

/***************************************File end********************************************/




/****************************************************************************************
> File Name: lutec_key.h
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/
#ifndef _LUTEC_KEY_H
#define _LUTEC_KEY_H

#include "../../../includes/board/chip/telink_sig_mesh_sdk/sdk/proj/common/types.h"
        


void lutec_key_init(void);

void lutec_key_loop(void);

u8 lutec_get_key_action(void);



#endif

/***************************************File end********************************************/




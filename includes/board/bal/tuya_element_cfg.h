/*
 * tuya_element_cfg.h
 *
 *  Created on: 2020-05-28
 *      Author: Joey
 */

#pragma once

#include "../../proj/tl_common.h"

//   user config---------------//
#define ELE_CNT    (1)

#define GENERIC_ONOFF_SERVER_EN               1  //must 1
#define LIGHT_LIGHTNESS_SERVER_EN             1
#define LIGHT_CTL_SERVER_EN                   1
#define LIGHT_HSL_SERVER_EN                   1

#define GENERIC_ONOFF_CLIENT_EN               0
#define LIGHT_LIGHTNESS_CLIENT_EN             0
#define LIGHT_CTL_CLIENT_EN                   0
#define LIGHT_HSL_CLIENT_EN                   0
#define TUYA_VENDOR_CLIENT_EN                 0

#define SCENE_SERVER_EN                       1
#define SCENE_CLIENT_EN                       0

#ifndef TELINK_PM_DEEPSLEEP_RETENTION_ENABLE //---------------wwpc 20210127 消除警告重复定义
  #define TELINK_PM_DEEPSLEEP_RETENTION_ENABLE  0
#endif

#define USED_NV1

//---------------------------------------------------------------------//
#define VENDOR_MD_TUYA_LIGHT_S          VENDOR_MD_TUYA_S
#define VENDOR_MD_TUYA_LIGHT_C          VENDOR_MD_TUYA_C

#define MD_ID_ARRAY_CFG_CLIENT  
#define MD_ID_ARRAY_CFG         SIG_MD_CFG_SERVER, MD_ID_ARRAY_CFG_CLIENT       \
                                SIG_MD_HEALTH_SERVER, SIG_MD_HEALTH_CLIENT,


#if SCENE_SERVER_EN
#define MD_ID_ARRAY_SCENE_SERVER        SIG_MD_SCENE_S, SIG_MD_SCENE_SETUP_S,
#else
#define MD_ID_ARRAY_SCENE_SERVER
#endif
#if SCENE_CLIENT_EN
#define MD_ID_ARRAY_SCENE_CLIENT        SIG_MD_SCENE_C,
#else
#define MD_ID_ARRAY_SCENE_CLIENT
#endif

#if GENERIC_ONOFF_SERVER_EN
#define MD_ID_ARRAY_ONOFF_SERVER        SIG_MD_G_ONOFF_S,
#else
#define MD_ID_ARRAY_ONOFF_SERVER
#endif
#if LIGHT_LIGHTNESS_SERVER_EN
#define MD_ID_ARRAY_LIGHTNESS_SERVER    SIG_MD_LIGHTNESS_S, SIG_MD_LIGHTNESS_SETUP_S,
#else
#define MD_ID_ARRAY_LIGHTNESS_SERVER
#endif
#if LIGHT_CTL_SERVER_EN
#define MD_ID_ARRAY_LIGHT_CTL_SERVER    SIG_MD_LIGHT_CTL_S, SIG_MD_LIGHT_CTL_SETUP_S, SIG_MD_LIGHT_CTL_TEMP_S,
#else
#define MD_ID_ARRAY_LIGHT_CTL_SERVER
#endif
#if LIGHT_HSL_SERVER_EN
#define MD_ID_ARRAY_LIGHT_HSL_SERVER    SIG_MD_LIGHT_HSL_S, SIG_MD_LIGHT_HSL_SETUP_S, SIG_MD_LIGHT_HSL_HUE_S, SIG_MD_LIGHT_HSL_SAT_S,
#else
#define MD_ID_ARRAY_LIGHT_HSL_SERVER
#endif


#if GENERIC_ONOFF_CLIENT_EN
#define MD_ID_ARRAY_ONOFF_CLIENT        SIG_MD_G_ONOFF_C,
#else
#define MD_ID_ARRAY_ONOFF_CLIENT
#endif
#if LIGHT_LIGHTNESS_CLIENT_EN
#define MD_ID_ARRAY_LIGHTNESS_CLIENT    SIG_MD_LIGHTNESS_C,
#else
#define MD_ID_ARRAY_LIGHTNESS_CLIENT
#endif
#if LIGHT_CTL_CLIENT_EN
#define MD_ID_ARRAY_LIGHT_CTL_CLIENT    SIG_MD_LIGHT_CTL_C,
#else
#define MD_ID_ARRAY_LIGHT_CTL_CLIENT
#endif
#if LIGHT_HSL_CLIENT_EN
#define MD_ID_ARRAY_LIGHT_HSL_CLIENT    SIG_MD_LIGHT_HSL_C,
#else
#define MD_ID_ARRAY_LIGHT_HSL_CLIENT
#endif


#define MD_ID_ARRAY_VENDOR_SERVER       VENDOR_MD_TUYA_S,
#if TUYA_VENDOR_CLIENT_EN
#define MD_ID_ARRAY_VENDOR_CLIENT       VENDOR_MD_TUYA_C,
#else
#define MD_ID_ARRAY_VENDOR_CLIENT
#endif


#define MD_ID_ARRAY_ONOFF               MD_ID_ARRAY_ONOFF_SERVER      MD_ID_ARRAY_ONOFF_CLIENT
#define MD_ID_ARRAY_LIGHTNESS           MD_ID_ARRAY_LIGHTNESS_SERVER  MD_ID_ARRAY_LIGHTNESS_CLIENT
#define MD_ID_ARRAY_LIGHT_CTL           MD_ID_ARRAY_LIGHT_CTL_SERVER  MD_ID_ARRAY_LIGHT_CTL_CLIENT
#define MD_ID_ARRAY_LIGHT_HSL           MD_ID_ARRAY_LIGHT_HSL_SERVER  MD_ID_ARRAY_LIGHT_HSL_CLIENT

#define MD_ID_ARRAY_PRIMARY     MD_ID_ARRAY_CFG  MD_ID_ARRAY_SCENE_SERVER  MD_ID_ARRAY_SCENE_CLIENT\
                                MD_ID_ARRAY_ONOFF  MD_ID_ARRAY_LIGHTNESS  MD_ID_ARRAY_LIGHT_CTL  MD_ID_ARRAY_LIGHT_HSL
#define MD_ID_ARRAY_SECOND      MD_ID_ARRAY_ONOFF

#define MD_ID_ARRAY_PRIMARY_VD  MD_ID_ARRAY_VENDOR_SERVER  MD_ID_ARRAY_VENDOR_CLIENT
#define MD_ID_ARRAY_SECOND_VD  




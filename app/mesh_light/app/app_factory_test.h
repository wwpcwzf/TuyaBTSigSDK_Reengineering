/*************************************************************************
	> File Name: app_factory_test.h
	> Author: 
	> Mail: 
	> Created Time: Wed 12 Aug 2020 11:57:13 CST
 ************************************************************************/

#ifndef _APP_FACTORY_TEST_H
#define _APP_FACTORY_TEST_H

#include "ty_light_basis_types.h"


void app_factory_test_enter_cb(void);
u8 app_factory_test_rssi_test_start_cb(u8 *para, u8 len);
u8 app_factory_test_rssi_test_cb(void);
void app_factory_test_reset_cb(void);
void app_factory_test_init(void);
int app_factory_test_cmd(u8 cmd, u8 *data, u8 len);
int app_factory_test_run(void);
int app_factory_test_if_enter(void);






















#endif

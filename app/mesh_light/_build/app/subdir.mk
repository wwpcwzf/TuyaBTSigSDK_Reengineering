C_SRC = ../app/app_common.c ../app/app_factory_test.c ../app/app_light_cmd.c ../app/app_light_control.c ../app/app_light_prompt.c ../app/app_light_uart.c ../app/app_rssi.c ../app/app_uart.c ../app/lutec_bt_dp.c ../app/lutec_key.c ../app/lutec_led.c ../app/lutec_lux.c ../app/lutec_main.c ../app/lutec_pir.c ../app/lutec_wifi.c ../app/tuya_node_init.c
USER_OBJ = ./app/app_common.o ./app/app_factory_test.o ./app/app_light_cmd.o ./app/app_light_control.o ./app/app_light_prompt.o ./app/app_light_uart.o ./app/app_rssi.o ./app/app_uart.o ./app/lutec_bt_dp.o ./app/lutec_key.o ./app/lutec_led.o ./app/lutec_lux.o ./app/lutec_main.o ./app/lutec_pir.o ./app/lutec_wifi.o ./app/tuya_node_init.o
INC = -I../_build -I../app
 
C_SRCS += ${C_SRC}
OBJS += ${USER_OBJ}
INCS += ${INC}
 
./app/%.o: ../app/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: TC32 Compiler'
	$(CC) $(CFLAGS) ${INCS} -o $@ -c $< ${CMACROS}
	@echo 'Finished building: $<'
	@echo ' '

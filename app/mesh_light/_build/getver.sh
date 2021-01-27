VER_FILE=version.in
echo -n " .equ BUILD_VERSION, " > $VER_FILE
echo 0x3031 >> $VER_FILE
 
CONFIG_FILE=base_oem_config.h
echo "//-------------------------------------------"> $CONFIG_FILE
echo "//common define">> $CONFIG_FILE
echo "//-------------------------------------------">> $CONFIG_FILE
echo "#define     FW_VERSION                \"1.0\"" >> $CONFIG_FILE
echo "#define     FW_VERSION_HEX            0x3031" >> $CONFIG_FILE
echo "#define     BUILD_TIME                \"2021-01-27-16:29:00\"" >> $CONFIG_FILE
echo "#define     BUILD_FIRMNAME            \"oem_2cw_tlsr8250_light2_pwm_user\"" >> $CONFIG_FILE
echo "#define     MESH_PID                  \"xl2yyswh\"" >> $CONFIG_FILE
echo "#define     CONFIG_LOG_ON             0" >> $CONFIG_FILE
echo "#define     CONFIG_OEM_ON             0" >> $CONFIG_FILE
echo "#define     JSON_CONFIG_ON            0" >> $CONFIG_FILE
echo "#define     LOCAL_CONTROL_ON          0" >> $CONFIG_FILE
echo "#define     KIND_ID                   0x1012" >> $CONFIG_FILE
echo "#define     POWER_RESET_CNT           5" >> $CONFIG_FILE
echo "#define     POWER_RESET_TIME          5" >> $CONFIG_FILE
echo "#define     POWER_RESET_RECOVER_TIME  600" >> $CONFIG_FILE
echo "#define     TY_BT_MODULE              TYBT3L" >> $CONFIG_FILE
echo "            #define     TYBT3L        0" >> $CONFIG_FILE
echo "            #define     TYBT8C        1" >> $CONFIG_FILE
echo "#define     IC_TLSR825x" >> $CONFIG_FILE
echo "" >> $CONFIG_FILE
echo "//-------------------------------------------">> $CONFIG_FILE
echo "//module light define">> $CONFIG_FILE
echo "//-------------------------------------------">> $CONFIG_FILE
echo "" >> $CONFIG_FILE
echo "#define     LIGHT_CFG_INIT_PARAM_CHECK         	0" >> $CONFIG_FILE
echo "#define     LIGHT_CFG_PROD_DRIVER_NEED_INIT    	0" >> $CONFIG_FILE
echo "#define     LIGHT_CFG_3IN1_SAVE                	0" >> $CONFIG_FILE
echo "#define     LIGHT_CFG_REMOTE_ENABLE            	1" >> $CONFIG_FILE
echo "#define     LIGHT_CFG_UART_ENABLE          	0" >> $CONFIG_FILE
echo "" >> $CONFIG_FILE
echo "#define DEFAULT_CONFIG \"{Jsonver:1.1.8,category:1012,module:BT3L,cmod:cw,dmod:0,cwtype:0,\
onoffmode:0,pmemory:1,defcolor:c,defbright:100,deftemp:100,\
cwmaxp:100,brightmin:10,brightmax:100,colormin:10,colormax:100,\
wfcfg:spcl_auto,rstmode:0,rstnum:3,remdmode:0,rstcor:c,rstbr:50,\
rsttemp:100,pwmhz:1000,pairt:6,notdisturb:0,prodagain:0,cagt:20,\
wt:20,r_r_g_\
g_b_b_c_pin:2,c_lv:1,w_pin:7,w_lv:1,\
\
ctrl_pin:12,ctrl_lv:1,\
crc:111,}\" ">> $CONFIG_FILE
echo "" >> $CONFIG_FILE

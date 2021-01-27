//-------------------------------------------
//common define
//-------------------------------------------
#define     FW_VERSION                "1.0"
#define     FW_VERSION_HEX            0x3031
#define     BUILD_TIME                "2021-01-27-16:29:00"
#define     BUILD_FIRMNAME            "oem_2cw_tlsr8250_light2_pwm_user"
#define     MESH_PID                  "xl2yyswh"
#define     CONFIG_LOG_ON             0
#define     CONFIG_OEM_ON             0
#define     JSON_CONFIG_ON            0
#define     LOCAL_CONTROL_ON          0
#define     KIND_ID                   0x1012
#define     POWER_RESET_CNT           5
#define     POWER_RESET_TIME          5
#define     POWER_RESET_RECOVER_TIME  600
#define     TY_BT_MODULE              TYBT3L
            #define     TYBT3L        0
            #define     TYBT8C        1
#define     IC_TLSR825x

//-------------------------------------------
//module light define
//-------------------------------------------

#define     LIGHT_CFG_INIT_PARAM_CHECK         	0
#define     LIGHT_CFG_PROD_DRIVER_NEED_INIT    	0
#define     LIGHT_CFG_3IN1_SAVE                	0
#define     LIGHT_CFG_REMOTE_ENABLE            	1
#define     LIGHT_CFG_UART_ENABLE          	0

#define DEFAULT_CONFIG "{Jsonver:1.1.8,category:1012,module:BT3L,cmod:cw,dmod:0,cwtype:0,onoffmode:0,pmemory:1,defcolor:c,defbright:100,deftemp:100,cwmaxp:100,brightmin:10,brightmax:100,colormin:10,colormax:100,wfcfg:spcl_auto,rstmode:0,rstnum:3,remdmode:0,rstcor:c,rstbr:50,rsttemp:100,pwmhz:1000,pairt:6,notdisturb:0,prodagain:0,cagt:20,wt:20,r_r_g_g_b_b_c_pin:2,c_lv:1,w_pin:7,w_lv:1,ctrl_pin:12,ctrl_lv:1,crc:111,}" 


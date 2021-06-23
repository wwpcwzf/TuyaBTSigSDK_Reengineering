# tuya_smesh_sdk_tlsr825x_light
tuya_smesh_sdk_tlsr825x_light

readme: https://developer.tuya.com/cn/docs/iot/device-development/embedded-software-development/module-sdk-development-access/ble-mesh-chip-sdk/bluetooth-sigmesh-lighting-sdkdemo-instructions?id=Ka5e0524ewo4n


================================================================================================================================
模块介绍：

a、BT3L正面：                                                            
                         ——————————————————————                         | b、json中配置数字与引脚对应关系：
                         |                    |                         |
                         |                    |                         |
                         |        BT3L        |                         |                          14：PC1  x
                         | 1-RST       PB1-16 |----Tx[PWM4][AD1]        |            0：PB1        13：PC5  x
[AD8][PWM0_N][PWM2]()----| 2-PC4       PB7-15 |----Rx[AD7]              |            1：PB7        12：PB0
                 X-------| 3-NC        PB5-14 |----(C_PWM5)[AD5]        |            2：PB5        11：PC4
                   ()----| 4-PD7       PB4-13 |----(R_PWM4)[AD4]        |            3：PB4        10：PD7
             (B_PWM3)----| 5-PD2       PA0-12 |----()[PWM0_N][Rx]       |            4：PA0        9 ：PD2
[I2C_SCK][Rx](G_PWM1)----| 6-PC3       PA7-11 |----SWS                  |                          8 ：PC3
    [I2C_SDA](W_PWM0)----| 7-PC2       PC0-10 |----()[I2C_SDA][PWM4_N]  |            6：PC0        7 ：PC2
                 3.3V----| 8-VDD       GND-9  |----GND                  |
                         ——————————————————————                         |

c、json配置文件中默认引脚配置:    
            "r_pin":"3"---PB4  脚位-13
            "g_pin":"8"---PC3  脚位-6
            "b_pin":"9"---PD2  脚位-5
            "c_pin":"2"---PB5  脚位-14
            "w_pin":"7"---PC2  脚位-7





---------------------------------------------------------------------------------------------------------------------------------
基于涂鸦蓝牙SDK二次开发的LUTEC产品信息：============================================================================================
                                          调光方式       名称                 PID
------------------------------------------CWRGB调光  	 LUTEC CWRGB Light	 hhfd6tmx
1、编译烧写命令
        $ bash run.sh build 5rgbcw_bt3l_pwm_V1.0.json
        $ bash run.sh flash 5rgbcw_bt3l_pwm_V1.0.json
2、引脚  
             BT3L正面：                                                            
                         ——————————————————————                         | json中配置：
                         |                    |                         |          "version": "1.3"
                         |                    |                         |          "pid":"hhfd6tmx"
                         |        BT3L        |                         |  
                         | 1-RST       PB1-16 |----Tx                   | 
       (SET_PIR_PWM2)----| 2-PC4       PB7-15 |----(LUX_AD7)            |  
                 X-------| 3-NC        PB5-14 |----C_PWM5               |   
             (PIR_IN)----| 4-PD7       PB4-13 |----R_PWM4               |   
               B_PWM3----| 5-PD2       PA0-12 |----Rx                   |    
               G_PWM1----| 6-PC3       PA7-11 |----SWS(KEY)             |   
               W_PWM0----| 7-PC2       PC0-10 |----(LED)                |   
                 3.3V----| 8-VDD       GND-9  |----GND                  |
                         ——————————————————————                         |
3、lutec_config.h中设备配置：
        //摄像灯 0x001C4658---0b0000 0000 0001 1100 0100 0111 0111 1010 ---觅睿摄像头灯SOC-RGBCW
        #define      DEVICE_FUNTICON                  0x001C477A 






























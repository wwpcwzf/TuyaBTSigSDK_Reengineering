/****************************************************************************************
> File Name: lutec_lux.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/

#include "lutec_lux.h"
#include "lutec_config.h"
#include "adc.h"
#include "hal_uart.h" //hal_uart_send(databuf, 4);

/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void adc_gpio_ain_init(void)
{
	//set misc channel en,  and adc state machine state cnt 2( "set" stage and "capture" state for misc channel)
	adc_set_chn_enable_and_max_state_cnt(ADC_MISC_CHN, 2);//0xf2 //set total length for sampling state machine and channel
	//set "capture state" length for misc channel: 240
	//set "set state" length for misc channel: 10
	//adc state machine  period  = 24M/250 = 96K, T = 10.4 uS
	adc_set_state_length(240, 0, 10);  	//set R_max_mc,R_max_c,R_max_s
	//set misc channel use differential_mode (telink advice: only differential mode is available)
	//single mode adc source, PB4 for example: PB4 positive channel, GND negative channel
	gpio_set_func(GPIO_PC4, AS_GPIO);
	gpio_setup_up_down_resistor(GPIO_PC4, PM_PIN_UP_DOWN_FLOAT);
	gpio_set_input_en(GPIO_PC4, 0);
	gpio_set_output_en(GPIO_PC4, 0);
	gpio_write(GPIO_PC4, 0);
#if ((PIR_LIGHT_SEL != PIR_LIGHT_1WAY)&&(PIR_LIGHT_SEL != PIR_LIGHT_2WAY))
	adc_set_ain_channel_differential_mode(ADC_MISC_CHN, B1P, GND);
#else
	adc_set_ain_channel_differential_mode(ADC_MISC_CHN, C4P, GND);
#endif
	//set misc channel resolution 14 bit
	//notice that: in differential_mode MSB is sign bit, rest are data,  here BIT(13) is sign bit
	adc_set_resolution(ADC_MISC_CHN, RES14);  //set resolution
	//set misc channel vref 1.2V
	adc_set_ref_voltage(ADC_MISC_CHN, ADC_VREF_1P2V);  		//set channel Vref
	//set misc t_sample 6 cycle of adc clock:  6 * 1/4M
	adc_set_tsample_cycle(ADC_MISC_CHN, SAMPLING_CYCLES_6); //Number of ADC clock cycles in sampling phase

	//set Analog input pre-scaling 1/8
	adc_set_ain_pre_scaler(ADC_PRESCALER_1F8);
}


/*-------------------------------------------------------------------------
*简  介: 
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_lux_init(void)
{
////Step 1: power off sar adc/////////////////////////////////////////////////////////
	/******power off sar adc********/
	adc_power_on_sar_adc(0);//0xfc<5>
////Step 2: Config some common adc settings(user can not change these)/////////////////
	/******enable signal of 24M clock to sar adc********/
	adc_enable_clk_24m_to_sar_adc(1);//0x82
	/******set adc sample clk as 4MHz******/
	adc_set_sample_clk(5); //0xf4<2:0> adc sample clk= 24M/(1+5)=4M
	/******set adc L R channel Gain Stage bias current trimming******/
	adc_set_left_gain_bias(GAIN_STAGE_BIAS_PER100); //0xfc<1:0>
	adc_set_right_gain_bias(GAIN_STAGE_BIAS_PER100); //0xfc<3:2>
////Step 3: Config adc settings  as needed /////////////////////////////////////////////
#if 1//(TEST_ADC_SELECT == TEST_ADC_GPIO)
	adc_gpio_ain_init();
#else// (TEST_ADC_SELECT == TEST_ADC_VBAT)
	adc_vbat_detect_init();
#endif
////Step 4: power on sar adc/////////////////////////////////////////////////////////
	/******power on sar adc********/
	adc_power_on_sar_adc(1);//0xfc<5>
////////////////////////////////////////////////////////////////////////////////////////
}


//-------------------------------------------------------------------------
static u8 lux_flag = 0;
static u16 lux_threshold = LUX_THRESHOLD_DEF;//30% -- 30*29 = 870  

/*-------------------------------------------------------------------------
*简  介: 获取AD采样值--滤波--判断--修改标志
*参  数: 
*返回值: 
-------------------------------------------------------------------------*/
void lutec_lux_loop(void)
{
    static u16 now_lux_ad = 0;
    static u32 sum_lux_ad = 0;
    static u8 average_counter = 0;
    static u8 buffeting_counter = 0;
    u16 databuf = 0;

    databuf = (analog_read(areg_adc_misc_h) << 8) + analog_read(areg_adc_misc_l);

    if(databuf & 0x8000)//转换完成（最高位为标记位）
    {
        databuf &= 0x7FFF;
        //平均值滤波
        if(average_counter < 64)
        {
            sum_lux_ad += databuf;
            average_counter++;
            return;
        }
        else
        {
            average_counter = 0;
            databuf = (u16)(sum_lux_ad >> 6);
            sum_lux_ad = 0;
        }
        //消抖滤波
        if(now_lux_ad == databuf)
        {
            buffeting_counter = 0;
        }
        else
        {
            if(buffeting_counter++ > 3)
            {
                now_lux_ad = databuf;
                buffeting_counter = 0;
            }
        }
        //阈值判断  now_lux_ad = {46--2611}
        if(lux_flag == 0)//未使能
        {
            if(now_lux_ad > (lux_threshold + 5))//暗了
            {
                lux_flag = 1;//使能
            }
        }
        else //已使能
        {
            if(now_lux_ad < lux_threshold)//亮了
            {
                lux_flag = 0;//失能
            }
        }
        //hal_uart_send(&now_lux_ad, 2);
    }    
}



/*-------------------------------------------------------------------------
*简  介: 获取lux判断标志
*参  数: 
*返回值: 0：失能；1：使能；
-------------------------------------------------------------------------*/
u8 lutec_get_lux_flag(void)
{
    return lux_flag;
}



/*-------------------------------------------------------------------------
*简  介:  设置阈值
*         lux AD： 11.5位 = 2896.3
*         实际AD： 最亮 34---最暗2640
*         阈值AD： 使能  0---失能2900
*参  数:  thsd_v：使能100%---失能0%
*返回值: 
-------------------------------------------------------------------------*/
void lutec_set_lux_threshold(u8 thsd_v)
{
    lux_threshold = thsd_v > 100 ? 0 : (100 - thsd_v) * 29;
}

/*-------------------------------------------------------------------------
*简  介:  获取lux阈值(AD)
*参  数:
*返回值: 
-------------------------------------------------------------------------*/
u8 lutec_get_lux_threshold(void)
{
    return (100 - (lux_threshold / 29));
}





/***************************************File end********************************************/


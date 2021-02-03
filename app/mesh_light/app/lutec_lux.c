/****************************************************************************************
> File Name: lutec_lux.c
> Description：
> Log:    Author     Time            modification
        | wwpc       2021/1/26        Create
        |
******************************************************************************************/

#include "lutec_lux.h"
#include "adc.h"
#include "hal_uart.h"

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

/*
#define    areg_adc_misc_l      0xf7
#define    areg_adc_misc_h      0xf8
enum{
	FLD_ADC_MISC_DATA   	= 	BIT_RNG(0,6),
	FLD_ADC_MISC_VLD    	= 	BIT(7),
};*/
void lutec_lux_loop(void)
{
    u8 databuf[4] = {0};

    databuf[0] = analog_read(areg_adc_misc_h);
    databuf[1] = analog_read(areg_adc_misc_l);

    if(databuf[0] & 0xE0)
    {
        databuf[2] = 0x0D;
        databuf[3] = 0x0A;
        //hal_uart_send(databuf, 4);
        databuf[0] = 0x00;
    }
    
}
















/***************************************File end********************************************/


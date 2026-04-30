
#include "system.h"

#include "main.h"

#include "log.h"

#define SYSTEM_DEBUG 1

#ifdef CMAKE_CROSSCOMPILING
#include "stm32f1xx.h"
extern TIM_HandleTypeDef htim1;
#else 

#include <sys/time.h>
#include "stdio.h"
uint32_t HAL_GetTick(void){
    static struct timeval start_tv = {.tv_sec=0};
    struct timeval end_tv;
	if (start_tv.tv_sec == 0) gettimeofday(&start_tv, NULL);
	gettimeofday(&end_tv, NULL);

    uint64_t t1 = start_tv.tv_sec*1000 + start_tv.tv_usec/1000;
    uint64_t t2 = end_tv.tv_sec*1000 + end_tv.tv_usec/1000;
    // uint32_t diff_sec = (end_tv.tv_sec - start_tv.tv_sec)%(60*60);
	// if ((end_tv.tv_sec > start_tv.tv_sec))
    // uint32_t diff_usec = (end_tv.tv_usec - start_tv.tv_usec);

	return (int) t2-t1;//(diff_sec*1000 + diff_usec/1000);
}
#define __HAL_TIM_GET_COUNTER(x) 0
void NVIC_SystemReset(void){Printf("reset\n\r");}
#endif
// My threads
struct ThreadFlyStruct {			// This is time settings for some processes
	int enabled;		// enabled = 1 => on; else off
	uint64_t t_interval;		// interval to run
	uint64_t t_previous_run;	// previous launch time
} 		ThreadFly[THREAD_ALL] = {	
	[THREAD_IMU_LOOP] = 			{.enabled = 1, 		.t_interval = 2, 		.t_previous_run = 0},
	[THREAD_MAV_SEND_ATTITUDE] = 	{.enabled = 1, 		.t_interval = 500, 		.t_previous_run = 0},
	[THREAD_MAV_SEND_STATUS] = 		{.enabled = 1, 		.t_interval = 500, 	.t_previous_run = 0},
	[THREAD_ADC] = 					{.enabled = 1, 		.t_interval = 100, 		.t_previous_run = 0},	// to read battery voltage
	[THREAD_MODEMCONTROL] = 		{.enabled = 1, 		.t_interval = 1, 		.t_previous_run = 0},	// ModemControl loop
	[THREAD_TEST] = 				{.enabled = 1, 		.t_interval = 500, 		.t_previous_run = 200},
};


void system_changeThread(int name, int param, int value){
	int * ptrToStruct = (int *)&ThreadFly[name];
	ptrToStruct[param] = value;
}

uint64_t system_getTime_us(void){
	//uint64_t local_time_us = 0;
	uint64_t local_time_us = HAL_GetTick()*1000;
	int addition_us = __HAL_TIM_GET_COUNTER(&htim1);
	local_time_us += addition_us;
	return local_time_us;
}

uint32_t system_getTime_ms(void){
	return HAL_GetTick();
}

void system_Delay_us(unsigned int us){
	uint64_t stime = system_getTime_us(), t=0;
	do {
		t = system_getTime_us() - stime;
	}while(t <  us);
}

void system_Delay_ms(unsigned int  ms){
	uint32_t stime = system_getTime_ms(), t=0;
	do {
		t = system_getTime_ms() - stime;
	}while(t <  ms);
}

void system_reboot(void){
	NVIC_SystemReset();
}

int Thread_Cycle(void)
{
	uint32_t c_time = (uint32_t)system_getTime_ms();
	static uint32_t t_launch = 0;	// launch time

	if (t_launch == 0) t_launch = c_time;	// if it's the first launch, then set the launch time
	c_time -= t_launch;

	// MY_DEBUGF(SYSTEM_DEBUG, ("sys: cycle ct:%d, lt:%d, tick: %d\n\r", (int)c_time, (int)t_launch, (int) system_getTime_ms()));
	static int i=0;
	for (; i<THREAD_ALL; i++){
		if ((ThreadFly[i].enabled == 1) &&
					((c_time - ThreadFly[i].t_previous_run) > ThreadFly[i].t_interval)){
			ThreadFly[i].t_previous_run = c_time;
			return i;
		}
	}
	i = 0;
	return -1;
}


#ifdef CMAKE_CROSSCOMPILING

extern ADC_HandleTypeDef hadc1;
/* ADC init function */
static void ADC_Change_Channel(int Channel)
{
	  ADC_ChannelConfTypeDef sConfig;

	    /**Common config
	    */
	  hadc1.Instance = ADC1;
	  hadc1.Init.ScanConvMode = ADC_SCAN_DISABLE;
	  hadc1.Init.ContinuousConvMode = DISABLE;
	  hadc1.Init.DiscontinuousConvMode = DISABLE;
	  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	  hadc1.Init.NbrOfConversion = 1;
	  if (HAL_ADC_Init(&hadc1) != HAL_OK)
	  {
		    Error_Handler();
	  }

	  sConfig.Channel = Channel;//ADC_CHANNEL_1;
	  sConfig.Rank = 1;
	  sConfig.SamplingTime = ADC_SAMPLETIME_1CYCLE_5;
	  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
	  {
		    Error_Handler();
	  }

}



int Read_ADC_Channel(int channel)
{
	//static int ch = -1;
	//if (ch != channel)
	int adcResult=0, i=0,Count = 10;	// if Count = 1 => Time read = 50us; if Count = 10 => Time read = 63.6us
	  HAL_ADC_DeInit(&hadc1);
	  ADC_Change_Channel(channel);


	  HAL_ADC_Start(&hadc1);
	  HAL_ADC_PollForConversion(&hadc1, 100);
	  for (i=0; i<Count; i++)
		  {
		  adcResult += HAL_ADC_GetValue(&hadc1);
		  }
	  HAL_ADC_Stop(&hadc1);
	  return (adcResult/Count);
}
#else

static void ADC_Change_Channel(int Channel){}
int Read_ADC_Channel(int channel){}

#endif
static int voltage = 0;
static int batPercent = 0;


int Battery_getVoltage(void){
	return voltage;
}
int Battery_getBatPercent(void){
	return batPercent;
}


void Battery_Read(void){
	//static float f_voltage = 0;
	const int undervoltage = 3000;
	const float multiplier = 12.2;//11.0;	//res divider

	int adc = Read_ADC_Channel(0);
	int calc_mV=((adc*3000)/0xFFF) * multiplier;
	//if (voltage == 0) voltage = calc_mV;
	//else
		voltage += (calc_mV - voltage)*(0.2f);	//LPF
	batPercent = (voltage-undervoltage) * 100 / (4200-undervoltage);
	if (batPercent > 100) batPercent = 100;
}




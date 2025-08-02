/*

The MIT License (MIT)

Copyright (c) 2016 Hubert Denkmair

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.

*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include "board.h"
#include "can.h"
#include "can_common.h"
#include "config.h"
#include "device.h"
#include "dfu.h"
#include "gpio.h"
#include "gs_usb.h"
#include "hal_include.h"
#include "led.h"
#include "timer.h"
#include "usbd_conf.h"
#include "usbd_core.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include "usbd_gs_can.h"
#include "util.h"
#include "stm32g0xx_hal_tim.h"

void HAL_MspInit(void);
static void SystemClock_Config(void);

static USBD_GS_CAN_HandleTypeDef hGS_CAN;
static USBD_HandleTypeDef hUSB = {0};

TIM_HandleTypeDef htim15;

//note frequencies
const float note_G5 = 783.99;				//hz
const float note_Fsharp5 = 739.99;
const float note_E5 = 659.26;
const float note_D5 = 587.33;
const float note_B4 = 493.88;

//note lengths
const int half = 1192;					//ms
const int dotted_4th = 894;
const int quarter = 596;
const int dotted_8th = 447;
const int eighth = 298;
const int sixteenth = 149;
const int delay = 10;

uint16_t freqToPeriod(float freq) {
	float temp = 1/freq;
	return (uint16_t) (temp * 1000000);
}

void setPWM(TIM_HandleTypeDef timer, uint32_t channel, float freq)
{
	HAL_TIM_PWM_Stop(&timer, channel); // stop generation of pwm
	TIM_OC_InitTypeDef sConfigOC;
	uint16_t per = freqToPeriod(freq);
	uint16_t pulse = per / 2;
	timer.Init.Period = per; // set the period duration
	HAL_TIM_PWM_Init(&timer); // reinititialise with new period value
	sConfigOC.OCMode = TIM_OCMODE_PWM1;
	sConfigOC.Pulse = pulse; // set the pulse duration
	sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
	sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
	HAL_TIM_PWM_ConfigChannel(&timer, &sConfigOC, channel);
	HAL_TIM_PWM_Start(&timer, channel); // start pwm generation
}


// static void playNote(uint16_t period, uint16_t pulse, uint16_t duration) {
// 	setPWM(htim15, TIM_CHANNEL_1, period, pulse);
	
// 	HAL_Delay(duration);
// 	setPWM(htim15, TIM_CHANNEL_1, 0, 0);
// }

// static void playTune(void) {

// 	playNote(740, 370, 297);
// 	// HAL_Delay(50);
// 	playNote(660, 330, 297);
// }


void kendrick(void)
{
	//intro (may want to revisit)
	setPWM(htim15, TIM_CHANNEL_1, note_E5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_D5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_E5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_D5);
	HAL_Delay(sixteenth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_E5);
	HAL_Delay(dotted_8th - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_D5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_B4);
	HAL_Delay(quarter - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_D5);
	HAL_Delay(dotted_4th - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_B4);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
	HAL_Delay(quarter - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay); 

	setPWM(htim15, TIM_CHANNEL_1, note_E5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	setPWM(htim15, TIM_CHANNEL_1, note_D5);
	HAL_Delay(eighth - delay);
	setPWM(htim15, TIM_CHANNEL_1, 0.0);
	HAL_Delay(delay);

	for (uint8_t i = 0; i < 2; i++)
	{
		//main melody 1st time thru (short final note)
		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_G5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(eighth);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(half);

		//main melody 2nd time thru (long final note)
		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_G5);
		HAL_Delay(eighth - delay);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(delay);

		setPWM(htim15, TIM_CHANNEL_1, note_Fsharp5);
		HAL_Delay(dotted_8th);
		setPWM(htim15, TIM_CHANNEL_1, 0.0);
		HAL_Delay(half - sixteenth);
	}

	HAL_TIM_PWM_Stop(&htim15, TIM_CHANNEL_1); 
}

int main(void)
{
	HAL_Init();
	SystemClock_Config();

	config.setup(&hGS_CAN);
	timer_init();

	MX_TIM15_Init();
	if(HAL_TIM_PWM_Start(&htim15, TIM_CHANNEL_1) != HAL_OK){
		Error_Handler();
	}

	INIT_LIST_HEAD(&hGS_CAN.list_frame_pool);
	INIT_LIST_HEAD(&hGS_CAN.list_to_host);

	for (unsigned i = 0; i < ARRAY_SIZE(hGS_CAN.msgbuf); i++) {
		list_add_tail(&hGS_CAN.msgbuf[i].list, &hGS_CAN.list_frame_pool);
	}
		
	for (unsigned int i = 0; i < ARRAY_SIZE(hGS_CAN.channels); i++) {
		const struct BoardChannelConfig *channel_config = &config.channels[i];
		const struct LEDConfig *led_config = channel_config->leds;
		can_data_t *channel = &hGS_CAN.channels[i];

		channel->nr = i;

		INIT_LIST_HEAD(&channel->list_from_host);

		led_init(&channel->leds,
				 led_config[LED_RX].port, led_config[LED_RX].pin, led_config[LED_RX].active_high,
				 led_config[LED_TX].port, led_config[LED_TX].pin, led_config[LED_TX].active_high);

		/* nice wake-up pattern */
		for (uint8_t j = 0; j < 10; j++) {
			HAL_GPIO_TogglePin(led_config[LED_RX].port, led_config[LED_RX].pin);
			HAL_Delay(50);
			HAL_GPIO_TogglePin(led_config[LED_TX].port, led_config[LED_TX].pin);
		}
		
		led_set_mode(&channel->leds, LED_MODE_OFF);

		can_init(channel, config.channels[i].interface);
		can_disable(channel);
	}

	kendrick();

	USBD_Init(&hUSB, (USBD_DescriptorsTypeDef*)&FS_Desc, DEVICE_FS);
	USBD_RegisterClass(&hUSB, &USBD_GS_CAN);
	USBD_GS_CAN_Init(&hGS_CAN, &hUSB);
	USBD_Start(&hUSB);

	//kendrick();

	while (1) {
		for (unsigned int i = 0; i < ARRAY_SIZE(hGS_CAN.channels); i++) {
			can_data_t *channel = &hGS_CAN.channels[i];

			CAN_SendFrame(&hGS_CAN, channel);
		}

		USBD_GS_CAN_ReceiveFromHost(&hUSB);
		USBD_GS_CAN_SendToHost(&hUSB);

		for (unsigned int i = 0; i < ARRAY_SIZE(hGS_CAN.channels); i++) {
			can_data_t *channel = &hGS_CAN.channels[i];

			CAN_ReceiveFrame(&hGS_CAN, channel);
			CAN_HandleError(&hGS_CAN, channel);

			led_update(&channel->leds);
		}

		if (USBD_GS_CAN_DfuDetachRequested(&hUSB)) {
			dfu_run_bootloader();
		}
	}
}

void HAL_MspInit(void)
{
	__HAL_RCC_SYSCFG_CLK_ENABLE();
#if defined(STM32F4)
	__HAL_RCC_PWR_CLK_ENABLE();
	__HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);
#elif defined(STM32G0)
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1);
#endif
	HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void SystemClock_Config(void)
{
	device_sysclock_config();
}

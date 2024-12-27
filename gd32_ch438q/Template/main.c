/*!
	\file    main.c
	\brief   GPIO running led demo

	\version 2014-12-26, V1.0.0, demo for GD32F10x
	\version 2017-06-30, V2.0.0, demo for GD32F10x
	\version 2021-04-30, V2.1.0, demo for GD32F10x
*/

/*
	Copyright (c) 2021, GigaDevice Semiconductor Inc.

	Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

	1. Redistributions of source code must retain the above copyright notice, this
	   list of conditions and the following disclaimer.
	2. Redistributions in binary form must reproduce the above copyright notice,
	   this list of conditions and the following disclaimer in the documentation
	   and/or other materials provided with the distribution.
	3. Neither the name of the copyright holder nor the names of its contributors
	   may be used to endorse or promote products derived from this software without
	   specific prior written permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
OF SUCH DAMAGE.
*/

#include "gd32f10x.h"
#include "systick.h"
#include "gd32f10x_rcu.h"
#include <stdio.h>
#include "string.h"
#include "usart.h"
#include "params_manage.h"
#include "CH438INC.H"
#include "CH438_driver.h"
#include "multi_add.h"
#include "VL_modbus.h"
#include "gd32_timer.h"

//#define SYS_CLK_TEST
#define SerilNUM 	3	//串口号

extern uint8_t rxbuffer[256];
extern uint8_t rx_count;
extern uint8_t tx_count;
extern uint8_t receive_flag;


void led_config(void);


void my_Config() {

#ifdef SYS_CLK_TEST
	static int sys_clk_freq;
	static int apb1_clk_freq;
	static int sys_clk_source;
	static int source_type;

	sys_clk_freq = rcu_clock_freq_get(CK_SYS);
	apb1_clk_freq = rcu_clock_freq_get(CK_APB1);
	sys_clk_source = rcu_system_clock_source_get();
	if (sys_clk_source == RCU_SCSS_IRC8M)
	{
		source_type = 1;//系统时钟为高速内部时钟
	}
	else if (sys_clk_source == RCU_SCSS_HXTAL)
	{
		source_type = 2;//系统时钟为高速外部时钟
	}
	else  if (sys_clk_source == RCU_SCSS_PLL)
	{
		source_type = 3;//系统时钟为高速外部时钟倍频
	}
#endif

	led_config();

	systick_config();
	
#if 0
	dma0_config();
	usart0_config();
	nvic_config();
#endif
//	TIM_Init();
	/* CH438接口配置 */
	CH438PortConfig();
	DELAY(250);

}

void led_config(void) {
	/* enable the led clock */
	rcu_periph_clock_enable(RCU_GPIOC);
	/* configure led GPIO port */
	#if 0
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_0);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);
	gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);

	gpio_bit_reset(GPIOA, GPIO_PIN_0);
	gpio_bit_reset(GPIOA, GPIO_PIN_1);
	gpio_bit_reset(GPIOA, GPIO_PIN_2);
	#endif
	
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_11);
	gpio_bit_reset(GPIOC, GPIO_PIN_10);
	gpio_bit_set(GPIOC, GPIO_PIN_11);
	gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8);


}

/*!
	\brief      main function
	\param[in]  none
	\param[out] none
	\retval     none
*/

unsigned char test[] = "string";
int main(void)
{
#if 0
	my_Config();

	CH438_ResetSeril(SerilNUM);				/* CH438串口复位 */
	CH438_UARTInit(SerilNUM);				/* CH438串口初始化 */
	CH438_INTConfig(SerilNUM);				/* CH438串口中断使能 */
	CH438_RegTEST(SerilNUM);
	gpio_bit_reset(GPIOC, GPIO_PIN_8);
	
	while (1)
	{
		VL_main();
	}
#endif
}

/* retarget the C library printf function to the USART */
int fputc(int ch, FILE* f)
{
	usart_data_transmit(USART0, (uint8_t)ch);
	while (RESET == usart_flag_get(USART0, USART_FLAG_TBE));
	return ch;
}

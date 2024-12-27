/********************************** (C) COPYRIGHT ******************************
* File Name          : interface.c
* Author             : WCH
* Version            : 
* Date               : 2014/9/11
* Description        : ch438与stm32连接接口配置及寄存器读写
*******************************************************************************/

#include "gd32f10x.h"
#include <stdio.h>
#include "multi_add.h"

/*硬件接口*/
/*
       CH438          DIR        STM32
        D0-D7        <==>       PB_8~PB_15
         WR          <==>        PD_10
         RD          <==>        PD_12
				 CS          <==>        PD_13
				INT          <==>        PA_9
				ALE          <==>        PD_11
*/		

/* 定义CH438地址数据复用端口单片机IO端口模拟总线时序接口 */

#define CH438_DATA_RCC     RCU_GPIOD		//数据地址时钟
#define CH438_DATA_PORT    GPIOD
//((uint16_t)0xFF00)	    //数据地址引脚  PD0-7
#define CH438_DATA_PIN     GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7

#define MODBUS_UART0_PORT 	GPIOA
#define MODBUS_UART0_EN    GPIO_PIN_12
#define MODBUS_UART1_PORT 	GPIOA
#define MODBUS_UART1_EN    GPIO_PIN_8
#define MODBUS_UART2_PORT 	GPIOC
#define MODBUS_UART2_EN    GPIO_PIN_9
#define MODBUS_UART3_PORT 	GPIOC
#define MODBUS_UART3_EN    GPIO_PIN_8
#define MODBUS_UART4_PORT 	GPIOC
#define MODBUS_UART4_EN    GPIO_PIN_7
#define MODBUS_UART5_PORT 	GPIOC
#define MODBUS_UART5_EN    GPIO_PIN_6
#define MODBUS_UART6_PORT 	GPIOD
#define MODBUS_UART6_EN    GPIO_PIN_15
#define MODBUS_UART7_PORT 	GPIOD
#define MODBUS_UART7_EN    GPIO_PIN_14

#define CH438_CONL_RCC     RCU_GPIOD		//控制管脚时钟
#define CH438_CONL_PORT    GPIOD
#define CH438_WR_PIN       GPIO_PIN_10    // WR引脚 	PD_10
#define CH438_ALE_PIN      GPIO_PIN_11    // ALE引脚  PD_11
#define CH438_RD_PIN       GPIO_PIN_12    // RD引脚   PD_12
#define CH438_CS_PIN       GPIO_PIN_13    // CS引脚   PD_13

#define CH438Q_RCU_GPIO		 RCU_GPIOA
#define CH438_INT_PORT     GPIOA
#define CH438_INT_PIN      GPIO_PIN_11    // 中断引脚 PA_11


static void SetInputMode(uint16_t GPIO_Pin);		/* 设置管脚为输入方式 */
static void SetOupputMode(uint16_t GPIO_Pin);		/* 设置管脚为输出方式 */
static void GD32_GPIO_Config(void);					/* 设置GPIO方式 */
static void GD32_NVIC_Config(void);


/**
  * Function Name  : CH438InterfaceConfig()
  * Description    : CH438接口配置
  * Input          : None
  * Return         : None
  */
void CH438PortConfig(void)
{
	GD32_GPIO_Config();
	GD32_NVIC_Config();
}

void CH438Q_DELAY(int time)
{
		for (uint16_t i = 0; i < time; i++) {}
}

/**
  * Function Name  : CH438WriteReg()
  * Description    : 写CH438寄存器
  * Input          : 寄存器地址; 写入数值
  * Return         : None
  */
void CH438WriteReg(unsigned char add,unsigned char data)	/* 写CH438 */
{
	uint16_t value;
	
	__disable_irq();
	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 0打开片选
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  				//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  				//RD = 1
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	CH438Q_DELAY(10);
	SetOupputMode(CH438_DATA_PIN);								//设为输出模式

	value = (uint8_t)add | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, (value));	   	//写地址
	
	gpio_bit_write(CH438_CONL_PORT, CH438_CS_PIN, RESET);    // CS = 0
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, SET);  	//锁存信号 ALE = 1	
	//CH438Q_DELAY(10);
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	value = (uint8_t)data | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, (value));	   	//写数据
	gpio_bit_reset(CH438_CONL_PORT, CH438_WR_PIN);  			//WR = 0
	//CH438Q_DELAY(5);
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  				//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 1
	__enable_irq();
}

/**
  * Function Name  : CH438ReadReg()
  * Description    : 读CH438寄存器
  * Input          : 寄存器地址; 
  * Return         : 读出数值
  */
unsigned char CH438ReadReg(unsigned char add)	/* 读CH438 */
{
	uint16_t value;
	__disable_irq();
	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);						//CS = 0
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  						//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  						//RD = 1
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	CH438Q_DELAY(10);
	SetOupputMode(CH438_DATA_PIN);								//设为输出模式
	value = add | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, ((uint16_t)value));	   	//写地址add

	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);              //打开片选 CS = 0
	gpio_bit_set(CH438_CONL_PORT, CH438_ALE_PIN);           	//锁存信号 ALE = 1
	//CH438Q_DELAY(10);
	gpio_bit_reset(CH438_CONL_PORT, CH438_ALE_PIN);  	        //ALE = 0
	SetInputMode(CH438_DATA_PIN);								//设为输入模式
	gpio_bit_reset(CH438_CONL_PORT, CH438_RD_PIN);  			//RD = 0
	//CH438Q_DELAY(5);
	value = (uint8_t)(gpio_input_port_get(CH438_DATA_PORT));	//读数据
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  				//RD = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 1	
	__enable_irq();
	return value;
}

/**
  * Function Name  : XINT_Config()
  * Description    : stm32外部中断配置
  * Input          : None 
  * Return         : None
  */
void GD32_XINT_Config(void)
{
	#ifdef CH438_INT_PIN
	// 设置GPIO时钟
	rcu_periph_clock_enable(CH438Q_RCU_GPIO);
	// 配置GPIO
	gpio_init(CH438_INT_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, CH438_INT_PIN);
	
	/*使能EXTI时钟*/
	rcu_periph_clock_enable(RCU_AF);
	/*IO连接到EXTI线*/
	gpio_exti_source_select (GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_11);
	/*配置上升沿还是下降沿*/
	exti_init(EXTI_11, EXTI_INTERRUPT, EXTI_TRIG_FALLING);//下降沿触发
	/*清除标志位*/
	exti_flag_clear(EXTI_11);
	
	#endif
}

////////  以下是内部函数  ///////////////////////////////////////////
static void SetInputMode(uint16_t GPIO_Pin)
{
	gpio_init(CH438_DATA_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_Pin);//上拉输入/* 数据地址管脚 */
	//gpio_init(CH438_DATA_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_Pin);//悬浮输入/* 数据地址管脚 */
}

static void SetOupputMode(uint16_t GPIO_Pin)
{
	gpio_init(CH438_DATA_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_Pin);//推挽输出/* 数据地址管脚 */
}

static void GD32_GPIO_Config(void)
{
	//时钟使能
	rcu_periph_clock_enable(CH438_DATA_RCC);
	rcu_periph_clock_enable(CH438_CONL_RCC);
	//PB4配置成输出
	gpio_init(CH438_DATA_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CH438_DATA_PIN);//悬浮输入/* 数据地址管脚 */
	gpio_init(CH438_DATA_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,  CH438_WR_PIN|CH438_RD_PIN|CH438_CS_PIN|CH438_ALE_PIN);//推挽输出
	
#ifdef CH438_INT_PIN
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(CH438_INT_PORT, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, CH438_INT_PIN);//上拉输入,外部中断管脚
#endif
	
	gpio_bit_reset(CH438_CONL_PORT, CH438_ALE_PIN);    //ALE信号默认为低
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);       //cs片选信号为高
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);       //WR片选信号为高
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);       //RD片选信号为高
}

static void GD32_NVIC_Config(void)
{
	#ifdef CH438_INT_PIN
	
	GD32_XINT_Config();
		/*使能中断*/
	nvic_irq_enable(EXTI10_15_IRQn, 1, 0);
	#endif
}




/********************************** (C) COPYRIGHT ******************************
* File Name          : interface.c
* Author             : WCH
* Version            : 
* Date               : 2014/9/11
* Description        : ch438��stm32���ӽӿ����ü��Ĵ�����д
*******************************************************************************/

#include "gd32f10x.h"
#include <stdio.h>
#include "multi_add.h"

/*Ӳ���ӿ�*/
/*
       CH438          DIR        STM32
        D0-D7        <==>       PB_8~PB_15
         WR          <==>        PD_10
         RD          <==>        PD_12
				 CS          <==>        PD_13
				INT          <==>        PA_9
				ALE          <==>        PD_11
*/		

/* ����CH438��ַ���ݸ��ö˿ڵ�Ƭ��IO�˿�ģ������ʱ��ӿ� */

#define CH438_DATA_RCC     RCU_GPIOD		//���ݵ�ַʱ��
#define CH438_DATA_PORT    GPIOD
//((uint16_t)0xFF00)	    //���ݵ�ַ����  PD0-7
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

#define CH438_CONL_RCC     RCU_GPIOD		//���ƹܽ�ʱ��
#define CH438_CONL_PORT    GPIOD
#define CH438_WR_PIN       GPIO_PIN_10    // WR���� 	PD_10
#define CH438_ALE_PIN      GPIO_PIN_11    // ALE����  PD_11
#define CH438_RD_PIN       GPIO_PIN_12    // RD����   PD_12
#define CH438_CS_PIN       GPIO_PIN_13    // CS����   PD_13

#define CH438Q_RCU_GPIO		 RCU_GPIOA
#define CH438_INT_PORT     GPIOA
#define CH438_INT_PIN      GPIO_PIN_11    // �ж����� PA_11


static void SetInputMode(uint16_t GPIO_Pin);		/* ���ùܽ�Ϊ���뷽ʽ */
static void SetOupputMode(uint16_t GPIO_Pin);		/* ���ùܽ�Ϊ�����ʽ */
static void GD32_GPIO_Config(void);					/* ����GPIO��ʽ */
static void GD32_NVIC_Config(void);


/**
  * Function Name  : CH438InterfaceConfig()
  * Description    : CH438�ӿ�����
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
  * Description    : дCH438�Ĵ���
  * Input          : �Ĵ�����ַ; д����ֵ
  * Return         : None
  */
void CH438WriteReg(unsigned char add,unsigned char data)	/* дCH438 */
{
	uint16_t value;
	
	__disable_irq();
	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 0��Ƭѡ
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  				//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  				//RD = 1
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	CH438Q_DELAY(10);
	SetOupputMode(CH438_DATA_PIN);								//��Ϊ���ģʽ

	value = (uint8_t)add | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, (value));	   	//д��ַ
	
	gpio_bit_write(CH438_CONL_PORT, CH438_CS_PIN, RESET);    // CS = 0
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, SET);  	//�����ź� ALE = 1	
	//CH438Q_DELAY(10);
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	value = (uint8_t)data | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, (value));	   	//д����
	gpio_bit_reset(CH438_CONL_PORT, CH438_WR_PIN);  			//WR = 0
	//CH438Q_DELAY(5);
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  				//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 1
	__enable_irq();
}

/**
  * Function Name  : CH438ReadReg()
  * Description    : ��CH438�Ĵ���
  * Input          : �Ĵ�����ַ; 
  * Return         : ������ֵ
  */
unsigned char CH438ReadReg(unsigned char add)	/* ��CH438 */
{
	uint16_t value;
	__disable_irq();
	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);						//CS = 0
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);  						//WR = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  						//RD = 1
	gpio_bit_write(CH438_CONL_PORT, CH438_ALE_PIN, RESET);  	//ALE = 0
	CH438Q_DELAY(10);
	SetOupputMode(CH438_DATA_PIN);								//��Ϊ���ģʽ
	value = add | (gpio_output_port_get(CH438_DATA_PORT) & 0xff00);
	gpio_port_write(CH438_DATA_PORT, ((uint16_t)value));	   	//д��ַadd

	gpio_bit_reset(CH438_CONL_PORT, CH438_CS_PIN);              //��Ƭѡ CS = 0
	gpio_bit_set(CH438_CONL_PORT, CH438_ALE_PIN);           	//�����ź� ALE = 1
	//CH438Q_DELAY(10);
	gpio_bit_reset(CH438_CONL_PORT, CH438_ALE_PIN);  	        //ALE = 0
	SetInputMode(CH438_DATA_PIN);								//��Ϊ����ģʽ
	gpio_bit_reset(CH438_CONL_PORT, CH438_RD_PIN);  			//RD = 0
	//CH438Q_DELAY(5);
	value = (uint8_t)(gpio_input_port_get(CH438_DATA_PORT));	//������
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);  				//RD = 1
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);				//CS = 1	
	__enable_irq();
	return value;
}

/**
  * Function Name  : XINT_Config()
  * Description    : stm32�ⲿ�ж�����
  * Input          : None 
  * Return         : None
  */
void GD32_XINT_Config(void)
{
	#ifdef CH438_INT_PIN
	// ����GPIOʱ��
	rcu_periph_clock_enable(CH438Q_RCU_GPIO);
	// ����GPIO
	gpio_init(CH438_INT_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, CH438_INT_PIN);
	
	/*ʹ��EXTIʱ��*/
	rcu_periph_clock_enable(RCU_AF);
	/*IO���ӵ�EXTI��*/
	gpio_exti_source_select (GPIO_PORT_SOURCE_GPIOA, GPIO_PIN_SOURCE_11);
	/*���������ػ����½���*/
	exti_init(EXTI_11, EXTI_INTERRUPT, EXTI_TRIG_FALLING);//�½��ش���
	/*�����־λ*/
	exti_flag_clear(EXTI_11);
	
	#endif
}

////////  �������ڲ�����  ///////////////////////////////////////////
static void SetInputMode(uint16_t GPIO_Pin)
{
	gpio_init(CH438_DATA_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_Pin);//��������/* ���ݵ�ַ�ܽ� */
	//gpio_init(CH438_DATA_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_Pin);//��������/* ���ݵ�ַ�ܽ� */
}

static void SetOupputMode(uint16_t GPIO_Pin)
{
	gpio_init(CH438_DATA_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_Pin);//�������/* ���ݵ�ַ�ܽ� */
}

static void GD32_GPIO_Config(void)
{
	//ʱ��ʹ��
	rcu_periph_clock_enable(CH438_DATA_RCC);
	rcu_periph_clock_enable(CH438_CONL_RCC);
	//PB4���ó����
	gpio_init(CH438_DATA_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, CH438_DATA_PIN);//��������/* ���ݵ�ַ�ܽ� */
	gpio_init(CH438_DATA_PORT, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ,  CH438_WR_PIN|CH438_RD_PIN|CH438_CS_PIN|CH438_ALE_PIN);//�������
	
#ifdef CH438_INT_PIN
	rcu_periph_clock_enable(RCU_GPIOA);
	gpio_init(CH438_INT_PORT, GPIO_MODE_IPU, GPIO_OSPEED_2MHZ, CH438_INT_PIN);//��������,�ⲿ�жϹܽ�
#endif
	
	gpio_bit_reset(CH438_CONL_PORT, CH438_ALE_PIN);    //ALE�ź�Ĭ��Ϊ��
	gpio_bit_set(CH438_CONL_PORT, CH438_CS_PIN);       //csƬѡ�ź�Ϊ��
	gpio_bit_set(CH438_CONL_PORT, CH438_WR_PIN);       //WRƬѡ�ź�Ϊ��
	gpio_bit_set(CH438_CONL_PORT, CH438_RD_PIN);       //RDƬѡ�ź�Ϊ��
}

static void GD32_NVIC_Config(void)
{
	#ifdef CH438_INT_PIN
	
	GD32_XINT_Config();
		/*ʹ���ж�*/
	nvic_irq_enable(EXTI10_15_IRQn, 1, 0);
	#endif
}




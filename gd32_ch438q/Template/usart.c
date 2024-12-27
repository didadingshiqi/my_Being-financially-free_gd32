#include "usart.h"
#include "gd32f10x.h"
#include <stdio.h>

//#define  USART0_REMAP 

uint8_t rxbuffer[256];
uint8_t rx_count = 0;
uint8_t tx_count = 0;
uint8_t receive_flag = 0;

void dma0_config(void)
{
	//����һ����Ϊdma_init_struct�Ľṹ����������ڴ洢DMAͨ�������ò�����
	dma_parameter_struct dma_init_struct;
	//ʹ��DMA0ʱ�� 
	rcu_periph_clock_enable(RCU_DMA0);

	/* deinitialize DMA channel4 (USART0 rx) */
	  //��DMA0�ĵ�4ͨ������ȥ��ʼ������ͨ�������������û�λͨ��ʱִ�еĲ�����
	dma_deinit(DMA0, DMA_CH4);
	dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY; //DMAͨ�����ݴ��䷽��,�����赽�ڴ�
	dma_init_struct.memory_addr = (uint32_t)rxbuffer; // ָ���������ݵ�Ŀ���ڴ��ַ��������rxbuffer����ʼ��ַ��
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; //�洢����ַ�����㷨ģʽ �����ڴ��ַ����ģʽ��ÿ��DMA������ڴ��ַ�Զ����ӡ�
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;//�����ڴ����ݿ��Ϊ8λ��
	dma_init_struct.number = 256;//���õ���DMA�������������������Ϊ256�ֽ�
	dma_init_struct.periph_addr = USART0_RDATA_ADDRESS;//�������ַ ָ�������Դ��ַ��������USART0�Ľ������ݼĴ�����ַ��
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;//�����ַ�����㷨ģʽ ���������ַ����������ÿ��DMA���䶼����ͬ�������ַ��ȡ���ݡ�
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;//�����������ݿ��Ϊ8λ��
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;//�����������ݿ��Ϊ8λ��
	dma_init(DMA0, DMA_CH4, &dma_init_struct);//��ʼ��DMA0�ĵ�4ͨ����
	/* configure DMA mode *///����ѭ��ģʽ������ζ��DMA������ɺ󲻻��Զ���ͷ��ʼ���䡣
	dma_circulation_disable(DMA0, DMA_CH4);
	/* enable DMA channel4 *///ʹ��DMA0�ĵ�4ͨ��
	dma_channel_enable(DMA0, DMA_CH4);
}

void usart0_config(void)
{
    /* enable USART clock */
    rcu_periph_clock_enable(RCU_USART0);
    
    #if defined USART0_REMAP
        /* enable GPIO clock */
        rcu_periph_clock_enable(RCU_GPIOB);
        rcu_periph_clock_enable(RCU_AF);
        /* USART0 remapping */
        gpio_pin_remap_config(GPIO_USART0_REMAP, ENABLE);
        /* connect port to USARTx_Tx */
        gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_6);
        /* connect port to USARTx_Rx */
        gpio_init(GPIOB, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_7);
    #else
        /* enable GPIO clock */
        rcu_periph_clock_enable(RCU_GPIOA);
        
        /* connect port to USARTx_Tx */
        gpio_init(GPIOA, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);
        /* connect port to USARTx_Rx */
        gpio_init(GPIOA, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    #endif
    
    /* USART configure */
    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200U);// ������
    usart_word_length_set(USART0, USART_WL_8BIT);// ֡�����ֳ�
    usart_stop_bit_set(USART0, USART_STB_1BIT);// ֹͣλ
    usart_parity_config(USART0, USART_PM_NONE);// ��żУ��λ
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);// Ӳ��������RTS
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE); // Ӳ��������CTS
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);// ʹ�ܽ���
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);// ʹ�ܷ���
		usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);	//����USART DMA����
		usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE); //����USART DMA����
	
    usart_enable(USART0);
}
/*!
	\brief      configure NVIC
*/
void usart0_nvic_config(void)
{
	
	//����NVIC�ж�����
	nvic_irq_enable(USART0_IRQn, 0, 0);
	// ʹ�ܴ��ڽ����ж�
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	/* ʹ��USART2�����ж� */
	usart_interrupt_enable(USART0, USART_INT_IDLE);
}


/**
 @brief ����д������
 @param pData д��һ�ֽ�����
 @return ��
*/
void UART_Write(uint8_t pData)
{

	usart_data_transmit(USART0, pData);                  // ����һ���ֽ�����
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // ��������ж�
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET)//�ȴ����ݷ������
	{}

}

////RS232����len���ֽ�
////buf:�������׵�ַ
////len:���͵��ֽ���
void usart0_send_data(uint8_t* buf, uint8_t len)
{
	uint8_t t;
	for (t = 0; t < len; t++)		//ѭ����������
	{
		while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
		UART_Write(buf[t]);
	}
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // ��������ж�
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET);//�ȴ����ݷ������
}

////RS485����len���ֽ�
////buf:�������׵�ַ
////len:���͵��ֽ���
void rs485_send_data(uint8_t* buf, uint8_t len)
{
	uint8_t t;
	gpio_bit_set(GPIOA, GPIO_PIN_8);//��������ģʽ���رս���ģʽ
	//usart_receive_config(USART0, USART_RECEIVE_DISABLE); 		// ʧ�ܽ���
	for(int i = 100; i<=0;i--){}
	for (t = 0; t < len; t++)		//ѭ����������
	{
		while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
		UART_Write(buf[t]);
	}
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // ��������ж�
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET);//�ȴ����ݷ������
	for(int i = 100; i<=0;i--){}
	//usart_receive_config(USART0, USART_RECEIVE_ENABLE); 		// ʹ�ܽ���
	gpio_bit_reset(GPIOA, GPIO_PIN_8); //��������ģʽ���رշ���ģʽ
}


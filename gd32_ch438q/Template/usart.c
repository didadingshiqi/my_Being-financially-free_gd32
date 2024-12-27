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
	//声明一个名为dma_init_struct的结构体变量，用于存储DMA通道的配置参数。
	dma_parameter_struct dma_init_struct;
	//使能DMA0时钟 
	rcu_periph_clock_enable(RCU_DMA0);

	/* deinitialize DMA channel4 (USART0 rx) */
	  //对DMA0的第4通道进行去初始化，这通常是在重新配置或复位通道时执行的操作。
	dma_deinit(DMA0, DMA_CH4);
	dma_init_struct.direction = DMA_PERIPHERAL_TO_MEMORY; //DMA通道数据传输方向,从外设到内存
	dma_init_struct.memory_addr = (uint32_t)rxbuffer; // 指定接收数据的目标内存地址，这里是rxbuffer的起始地址。
	dma_init_struct.memory_inc = DMA_MEMORY_INCREASE_ENABLE; //存储器地址生成算法模式 设置内存地址递增模式，每次DMA传输后，内存地址自动增加。
	dma_init_struct.memory_width = DMA_MEMORY_WIDTH_8BIT;//设置内存数据宽度为8位。
	dma_init_struct.number = 256;//设置单次DMA传输的数据数量，这里为256字节
	dma_init_struct.periph_addr = USART0_RDATA_ADDRESS;//外设基地址 指定外设的源地址，这里是USART0的接收数据寄存器地址。
	dma_init_struct.periph_inc = DMA_PERIPH_INCREASE_DISABLE;//外设地址生成算法模式 设置外设地址不递增，即每次DMA传输都从相同的外设地址读取数据。
	dma_init_struct.periph_width = DMA_PERIPHERAL_WIDTH_8BIT;//设置外设数据宽度为8位。
	dma_init_struct.priority = DMA_PRIORITY_ULTRA_HIGH;//设置外设数据宽度为8位。
	dma_init(DMA0, DMA_CH4, &dma_init_struct);//初始化DMA0的第4通道。
	/* configure DMA mode *///禁用循环模式，这意味着DMA传输完成后不会自动从头开始传输。
	dma_circulation_disable(DMA0, DMA_CH4);
	/* enable DMA channel4 *///使能DMA0的第4通道
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
    usart_baudrate_set(USART0, 115200U);// 波特率
    usart_word_length_set(USART0, USART_WL_8BIT);// 帧数据字长
    usart_stop_bit_set(USART0, USART_STB_1BIT);// 停止位
    usart_parity_config(USART0, USART_PM_NONE);// 奇偶校验位
    usart_hardware_flow_rts_config(USART0, USART_RTS_DISABLE);// 硬件流控制RTS
    usart_hardware_flow_cts_config(USART0, USART_CTS_DISABLE); // 硬件流控制CTS
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);// 使能接收
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);// 使能发送
		usart_dma_receive_config(USART0, USART_RECEIVE_DMA_ENABLE);	//配置USART DMA接收
		usart_dma_transmit_config(USART0, USART_TRANSMIT_DMA_ENABLE); //配置USART DMA传输
	
    usart_enable(USART0);
}
/*!
	\brief      configure NVIC
*/
void usart0_nvic_config(void)
{
	
	//启用NVIC中断请求
	nvic_irq_enable(USART0_IRQn, 0, 0);
	// 使能串口接收中断
	usart_interrupt_enable(USART0, USART_INT_RBNE);
	/* 使能USART2空闲中断 */
	usart_interrupt_enable(USART0, USART_INT_IDLE);
}


/**
 @brief 串口写入数据
 @param pData 写入一字节数据
 @return 无
*/
void UART_Write(uint8_t pData)
{

	usart_data_transmit(USART0, pData);                  // 发送一个字节数据
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // 发送完成判断
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET)//等待数据发送完成
	{}

}

////RS232发送len个字节
////buf:发送区首地址
////len:发送的字节数
void usart0_send_data(uint8_t* buf, uint8_t len)
{
	uint8_t t;
	for (t = 0; t < len; t++)		//循环发送数据
	{
		while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
		UART_Write(buf[t]);
	}
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // 发送完成判断
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET);//等待数据发送完成
}

////RS485发送len个字节
////buf:发送区首地址
////len:发送的字节数
void rs485_send_data(uint8_t* buf, uint8_t len)
{
	uint8_t t;
	gpio_bit_set(GPIOA, GPIO_PIN_8);//开启发送模式，关闭接收模式
	//usart_receive_config(USART0, USART_RECEIVE_DISABLE); 		// 失能接收
	for(int i = 100; i<=0;i--){}
	for (t = 0; t < len; t++)		//循环发送数据
	{
		while (usart_flag_get(USART0, USART_FLAG_TC) == RESET) {}
		UART_Write(buf[t]);
	}
	while(RESET == usart_flag_get(USART0, USART_FLAG_TBE)); // 发送完成判断
	while (usart_flag_get(USART0, USART_FLAG_TC) == RESET);//等待数据发送完成
	for(int i = 100; i<=0;i--){}
	//usart_receive_config(USART0, USART_RECEIVE_ENABLE); 		// 使能接收
	gpio_bit_reset(GPIOA, GPIO_PIN_8); //开启接收模式，关闭发送模式
}


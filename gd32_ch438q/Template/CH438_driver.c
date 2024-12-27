/********************************** (C) COPYRIGHT ******************************
* File Name          : CH438_driver.c
* Author             : WCH
* Version            : 
* Date               : 2014/9/11
* Description        :ch438控制驱动
*******************************************************************************/

#include <stdio.h>
#include "gd32f10x.h"
#include "systick.h"
#include "multi_add.h"
#include "CH438INC.H"
#include "CH438_driver.h"
#include "string.h"

//#define Fpclk    	  1843200         /* 定义内部时钟频率,默认外部晶振的12分频    */
#define Fpclk    	  666666 


const unsigned char offsetadd[] = {0x00,0x10,0x20,0x30,0x08,0x18,0x28,0x38,};		/* 串口号的偏移地址 */
const unsigned char Interruptnum[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,};	/* SSR寄存器中断号对应值 */

unsigned char Revbuff[MaxRecvLen];		/* 接收缓存区 */
unsigned char RevLen;									/* 接收计数 */
unsigned char RevFlag;								/* 接收标志 */
unsigned char ch438q_com;							/* 接收串口 */

static void CH438_TranConfig(unsigned char num);								/* 串口数据格式配置及FIFO */
static void CH438_SetBandrate(unsigned char num, unsigned long value);			/* 设置串口波特率 */

/**
  * Function Name  : CH438_CheckIIR()
  * Description    : 读IIR寄存器
  * Input          : 串口号（0-7）
  * Return         : IIR寄存器值
  */
unsigned char CH438_CheckIIR(unsigned char num)
{
	unsigned char value;
	value = CH438ReadReg( offsetadd[num] | REG_IIR_ADDR );
	return value;
}

/**
  * Function Name  : CH438_CloseSeril()
  * Description    : 关闭串口
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_CloseSeril(unsigned char num)
{
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_LOWPOWER);
}

/**
  * Function Name  : CH438_CloseALLSeril()
  * Description    : 关闭所有串口
  * Input          : None
  * Return         : None
  */
void CH438_CloseALLSeril(void)
{
	CH438WriteReg(offsetadd[0]|REG_IER_ADDR, BIT_IER_LOWPOWER|BIT_IER_SLP);
}

/**
  * Function Name  : CH438_ResetSeril()
  * Description    : 复位串口
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_ResetSeril(unsigned char num)
{
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_RESET);
}

/**
  * Function Name  : CH438_UARTInit()
  * Description    : 串口初始化
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_UARTInit(unsigned char num)
{
	CH438_SetBandrate(num, 2400);	/* CH438串口1波特率设置 */
	CH438_TranConfig(num); 			/* CH438串口1数据格式配置及FIFO大小 */	
}

/**
  * Function Name  : CH438_SetBandrate()
  * Description    : 设置串口波特率
  * Input          : 串口号（0-7）;波特率值
  * Return         : None
  */
void CH438_SetBandrate(unsigned char num, unsigned long value)
{
	uint8_t dlab=0;
	uint16_t bandspeed;
	
	dlab = CH438ReadReg(offsetadd[num]|REG_LCR_ADDR);
	dlab |= 0x80;		//置LCR寄存器DLAB位为1
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, dlab);
	
	bandspeed = Fpclk/16/value;
	CH438WriteReg(offsetadd[num]|REG_DLL_ADDR, (uint8_t)bandspeed);
	CH438WriteReg(offsetadd[num]|REG_DLM_ADDR, (uint8_t)(bandspeed>>8));
	
#if	DEBUG_EN	
	printf("bandrate: %x\n", bandspeed);
	printf("DLM: %x\n", CH438ReadReg(offsetadd[num]|REG_DLM_ADDR));
	printf("DLL: %x\n", CH438ReadReg(offsetadd[num]|REG_DLL_ADDR));
#endif	
	
	dlab &= 0x7F;		//置IIR寄存器DLAB位为0
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, dlab);
}

/**
  * Function Name  : CH438_SendDatas()
  * Description    : 查询方式:发送数据
  * Input          : 串口号（0-7）; 数据首地址; 数量
  * Return         : None
  */
void CH438_SendDatas(unsigned char num, unsigned char* sendbuff,unsigned char len)
{

	MAX_485_WR
	while(len)
	{		
		if((CH438ReadReg(offsetadd[num]|REG_LSR_ADDR)&BIT_LSR_TEMT))	    //LSR->THRE==1  保持寄存器空
		{
			CH438WriteReg(offsetadd[num]|REG_THR_ADDR, *sendbuff++);
			len--;
		}
	}
	delay_1ms(5);
	MAX_485_RD
}

/**
  * Function Name  : CH438_RecvDatas()
  * Description    : 查询方式：接受数据
  * Input          : 串口号（0-7）; 存储首地址
  * Return         : 接受数量
  */
unsigned char CH438_RecvDatas(unsigned char num, unsigned char* revbuff)
{
	uint8_t len=0;
	uint8_t *p_rev;
	p_rev = revbuff;

	while( ( CH438ReadReg( offsetadd[num]|REG_LSR_ADDR ) & BIT_LSR_DATARDY ) == 0 );    /*等待数据准备好 */
	while((CH438ReadReg(offsetadd[num]|REG_LSR_ADDR)&BIT_LSR_DATARDY))	//LSR->DATARDY==1
	{
		*p_rev = CH438ReadReg(offsetadd[num]|REG_RBR_ADDR);
		p_rev++;
		len++;
	}
	return len;
}

/**
  * Function Name  : CH438_TranConfig()
  * Description    : 串口数据格式配置及FIFO
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_TranConfig(unsigned char num)
{	
	/* 发送数据格式:8位数据，无校验，1个停止位  */
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, BIT_LCR_WORDSZ1|BIT_LCR_WORDSZ0);
	/* 设置FIFO模式，触发点为112字节 */
	CH438WriteReg(offsetadd[num]|REG_FCR_ADDR, BIT_FCR_RECVTG1|BIT_FCR_RECVTG0|BIT_FCR_FIFOEN|BIT_FCR_TFIFORST|BIT_FCR_RFIFORST);
	//CH438WriteReg(offsetadd[num]|REG_FCR_ADDR, BIT_FCR_RECVTG0|BIT_FCR_FIFOEN|BIT_FCR_TFIFORST|BIT_FCR_RFIFORST);	
}

/**
  * Function Name  : CH438_IntConfig()
  * Description    : 串口中断开启
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_INTConfig(unsigned char num)
{	
	/* 注意: CH438打开BIT_IER_IETHRE中断(0->1),会产生一个发生空中断 */	
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_IELINES|BIT_IER_IETHRE|BIT_IER_IERECV);
	CH438_CheckIIR(num);
	CH438WriteReg(offsetadd[num]|REG_MCR_ADDR, BIT_MCR_OUT2);//可以产生一个实际的中断	
}

/**
  * Function Name  : CH438_AutoHFCtrl()
  * Description    : 硬件自动流开启
  * Input          : 串口号（0-7）
  * Return         : None
  */
void CH438_AutoHFCtrl(unsigned char num)
{
    CH438WriteReg( offsetadd[num]|REG_MCR_ADDR, BIT_MCR_AFE | BIT_MCR_OUT2 | BIT_MCR_RTS );/* 设置MCR寄存器的AFE和RTS为1 */
}

/**
  * Function Name  : DELAY()
  * Description    : 延时函数
  * Input          : None
  * Return         : None
  */
void DELAY(unsigned char time)
{
	unsigned char	i, j, c;
	for ( i = time; i != 0; i -- ) for ( j = 200; j != 0; j -- ) c+=3;
}

/**
  * Function Name  : CH438REG_RWTEST()
  * Description    : 测试438寄存器读写
  * Input          : 串口号（0-7）
  * Return         : None
  */
int CH438_RegTEST(unsigned char num)
{
#if  DEBUG_EN	
	printf("current test serilnum: %02x \n",(unsigned short)offsetadd[num]);
	printf("IER: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_IER_ADDR));//读IER
	printf("IIR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_IIR_ADDR));//读IIR
	printf("LCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_LCR_ADDR));//读LCR
	printf("MCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_MCR_ADDR));//读MCR
	printf("LSR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_LSR_ADDR));//读LSR
	printf("MSR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_MSR_ADDR));//读MSR
	CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, 0x78);
	printf("SCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR));//读SCR
#else
			unsigned short  ch348qnum = (unsigned short)offsetadd[num];
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_IER_ADDR);//读IER
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_IIR_ADDR);//读IIR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_LCR_ADDR);//读LCR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_MCR_ADDR);//读MCR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_LSR_ADDR);//读LSR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_MSR_ADDR);//读SCR
			#if 1
			CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, 0x78);
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//读SCR
			if(ch348qnum == 0x78){
				return 0;
			}else{
				return 1;
			}
			#endif
			DELAY(ch348qnum);
#endif
}

/**
  * Function Name  : CH438_save_data()
  * Description    : 438寄存器写入SCR保留寄存器
  * Input          : num 串口号（0-7） data 数据
  * Return         : 保存的data数据 
  */
unsigned char CH438_save_data(unsigned char num,unsigned char data){
	CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, data);
	return (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//读SCR
}
/**
  * Function Name  : CH438_save_data()
  * Description    : 438寄存器读取SCR保留寄存器
  * Input          : num 串口号
  * Return         : 保存的data数据 
  */
unsigned char CH438_read_data(unsigned char num){
	return (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//读SCR
}



/**
  * Function Name  : CH438InterruptFun()
  * Description    : ch438中断方式处理
  * Input          : None
  * Return         : None
  */
void CH438InterruptFun (void)
{
	uint8_t gInterruptStatus;		/* 全局中断状态 */
	uint8_t InterruptStatus;		/* 独立串口中断状态 */	
	uint8_t i;	
	
//	do
//	{
//		i = CH438ReadReg(0x10 | REG_LSR_ADDR);
//		printf("LSR: %x\n",(unsigned short)i);
//	}while(!(i&BIT_LSR_TEMT));
////	CH438WriteReg(offsetadd[1] | REG_SCR_ADDR, 0x24);		
//	printf("IIR: %x\n",(unsigned short)CH438ReadReg(0x10 | REG_IIR_ADDR));	

	
	gInterruptStatus = CH438ReadReg( REG_SSR_ADDR );

	if(!gInterruptStatus)
		return ;
    
	for(i=0; i<8; i++)
	{
		if( gInterruptStatus & Interruptnum[i] )    /* 检测哪个串口发生中断 */
		{
			InterruptStatus = CH438ReadReg( offsetadd[i] | REG_IIR_ADDR ) & 0x0f;    /* 读串口的中断状态 */	

			switch( InterruptStatus )
			{
				case INT_NOINT:			/* 没有中断 */					
					break;
				case INT_THR_EMPTY:		/* THR空中断 */						
					break;
				case INT_RCV_OVERTIME:	/* 接收超时中断 */
								// 重置缓冲区
					RevLen = 0;
					memset(Revbuff,0,sizeof(Revbuff));
					RevLen = CH438_RecvDatas(i, Revbuff);
					RevFlag = 1;
					break;
				case INT_RCV_SUCCESS:	/* 接收数据可用中断 */
								// 重置缓冲区
					RevLen = 0;
					memset(Revbuff,0,sizeof(Revbuff));
					RevLen = CH438_RecvDatas(i, Revbuff);
					RevFlag = 2;
					break;
				case INT_RCV_LINES:		/* 接收线路状态中断 */
					CH438ReadReg( offsetadd[i] | REG_LSR_ADDR );
					break;
				case INT_MODEM_CHANGE:	/* MODEM输入变化中断 */
					CH438ReadReg( offsetadd[i] | REG_MSR_ADDR );
					break;
				default:
					break;

			}
		}
	}
}


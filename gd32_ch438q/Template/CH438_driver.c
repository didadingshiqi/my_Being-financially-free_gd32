/********************************** (C) COPYRIGHT ******************************
* File Name          : CH438_driver.c
* Author             : WCH
* Version            : 
* Date               : 2014/9/11
* Description        :ch438��������
*******************************************************************************/

#include <stdio.h>
#include "gd32f10x.h"
#include "systick.h"
#include "multi_add.h"
#include "CH438INC.H"
#include "CH438_driver.h"
#include "string.h"

//#define Fpclk    	  1843200         /* �����ڲ�ʱ��Ƶ��,Ĭ���ⲿ�����12��Ƶ    */
#define Fpclk    	  666666 


const unsigned char offsetadd[] = {0x00,0x10,0x20,0x30,0x08,0x18,0x28,0x38,};		/* ���ںŵ�ƫ�Ƶ�ַ */
const unsigned char Interruptnum[] = {0x01,0x02,0x04,0x08,0x10,0x20,0x40,0x80,};	/* SSR�Ĵ����жϺŶ�Ӧֵ */

unsigned char Revbuff[MaxRecvLen];		/* ���ջ����� */
unsigned char RevLen;									/* ���ռ��� */
unsigned char RevFlag;								/* ���ձ�־ */
unsigned char ch438q_com;							/* ���մ��� */

static void CH438_TranConfig(unsigned char num);								/* �������ݸ�ʽ���ü�FIFO */
static void CH438_SetBandrate(unsigned char num, unsigned long value);			/* ���ô��ڲ����� */

/**
  * Function Name  : CH438_CheckIIR()
  * Description    : ��IIR�Ĵ���
  * Input          : ���ںţ�0-7��
  * Return         : IIR�Ĵ���ֵ
  */
unsigned char CH438_CheckIIR(unsigned char num)
{
	unsigned char value;
	value = CH438ReadReg( offsetadd[num] | REG_IIR_ADDR );
	return value;
}

/**
  * Function Name  : CH438_CloseSeril()
  * Description    : �رմ���
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_CloseSeril(unsigned char num)
{
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_LOWPOWER);
}

/**
  * Function Name  : CH438_CloseALLSeril()
  * Description    : �ر����д���
  * Input          : None
  * Return         : None
  */
void CH438_CloseALLSeril(void)
{
	CH438WriteReg(offsetadd[0]|REG_IER_ADDR, BIT_IER_LOWPOWER|BIT_IER_SLP);
}

/**
  * Function Name  : CH438_ResetSeril()
  * Description    : ��λ����
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_ResetSeril(unsigned char num)
{
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_RESET);
}

/**
  * Function Name  : CH438_UARTInit()
  * Description    : ���ڳ�ʼ��
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_UARTInit(unsigned char num)
{
	CH438_SetBandrate(num, 2400);	/* CH438����1���������� */
	CH438_TranConfig(num); 			/* CH438����1���ݸ�ʽ���ü�FIFO��С */	
}

/**
  * Function Name  : CH438_SetBandrate()
  * Description    : ���ô��ڲ�����
  * Input          : ���ںţ�0-7��;������ֵ
  * Return         : None
  */
void CH438_SetBandrate(unsigned char num, unsigned long value)
{
	uint8_t dlab=0;
	uint16_t bandspeed;
	
	dlab = CH438ReadReg(offsetadd[num]|REG_LCR_ADDR);
	dlab |= 0x80;		//��LCR�Ĵ���DLABλΪ1
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, dlab);
	
	bandspeed = Fpclk/16/value;
	CH438WriteReg(offsetadd[num]|REG_DLL_ADDR, (uint8_t)bandspeed);
	CH438WriteReg(offsetadd[num]|REG_DLM_ADDR, (uint8_t)(bandspeed>>8));
	
#if	DEBUG_EN	
	printf("bandrate: %x\n", bandspeed);
	printf("DLM: %x\n", CH438ReadReg(offsetadd[num]|REG_DLM_ADDR));
	printf("DLL: %x\n", CH438ReadReg(offsetadd[num]|REG_DLL_ADDR));
#endif	
	
	dlab &= 0x7F;		//��IIR�Ĵ���DLABλΪ0
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, dlab);
}

/**
  * Function Name  : CH438_SendDatas()
  * Description    : ��ѯ��ʽ:��������
  * Input          : ���ںţ�0-7��; �����׵�ַ; ����
  * Return         : None
  */
void CH438_SendDatas(unsigned char num, unsigned char* sendbuff,unsigned char len)
{

	MAX_485_WR
	while(len)
	{		
		if((CH438ReadReg(offsetadd[num]|REG_LSR_ADDR)&BIT_LSR_TEMT))	    //LSR->THRE==1  ���ּĴ�����
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
  * Description    : ��ѯ��ʽ����������
  * Input          : ���ںţ�0-7��; �洢�׵�ַ
  * Return         : ��������
  */
unsigned char CH438_RecvDatas(unsigned char num, unsigned char* revbuff)
{
	uint8_t len=0;
	uint8_t *p_rev;
	p_rev = revbuff;

	while( ( CH438ReadReg( offsetadd[num]|REG_LSR_ADDR ) & BIT_LSR_DATARDY ) == 0 );    /*�ȴ�����׼���� */
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
  * Description    : �������ݸ�ʽ���ü�FIFO
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_TranConfig(unsigned char num)
{	
	/* �������ݸ�ʽ:8λ���ݣ���У�飬1��ֹͣλ  */
	CH438WriteReg(offsetadd[num]|REG_LCR_ADDR, BIT_LCR_WORDSZ1|BIT_LCR_WORDSZ0);
	/* ����FIFOģʽ��������Ϊ112�ֽ� */
	CH438WriteReg(offsetadd[num]|REG_FCR_ADDR, BIT_FCR_RECVTG1|BIT_FCR_RECVTG0|BIT_FCR_FIFOEN|BIT_FCR_TFIFORST|BIT_FCR_RFIFORST);
	//CH438WriteReg(offsetadd[num]|REG_FCR_ADDR, BIT_FCR_RECVTG0|BIT_FCR_FIFOEN|BIT_FCR_TFIFORST|BIT_FCR_RFIFORST);	
}

/**
  * Function Name  : CH438_IntConfig()
  * Description    : �����жϿ���
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_INTConfig(unsigned char num)
{	
	/* ע��: CH438��BIT_IER_IETHRE�ж�(0->1),�����һ���������ж� */	
	CH438WriteReg(offsetadd[num]|REG_IER_ADDR, BIT_IER_IELINES|BIT_IER_IETHRE|BIT_IER_IERECV);
	CH438_CheckIIR(num);
	CH438WriteReg(offsetadd[num]|REG_MCR_ADDR, BIT_MCR_OUT2);//���Բ���һ��ʵ�ʵ��ж�	
}

/**
  * Function Name  : CH438_AutoHFCtrl()
  * Description    : Ӳ���Զ�������
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
void CH438_AutoHFCtrl(unsigned char num)
{
    CH438WriteReg( offsetadd[num]|REG_MCR_ADDR, BIT_MCR_AFE | BIT_MCR_OUT2 | BIT_MCR_RTS );/* ����MCR�Ĵ�����AFE��RTSΪ1 */
}

/**
  * Function Name  : DELAY()
  * Description    : ��ʱ����
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
  * Description    : ����438�Ĵ�����д
  * Input          : ���ںţ�0-7��
  * Return         : None
  */
int CH438_RegTEST(unsigned char num)
{
#if  DEBUG_EN	
	printf("current test serilnum: %02x \n",(unsigned short)offsetadd[num]);
	printf("IER: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_IER_ADDR));//��IER
	printf("IIR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_IIR_ADDR));//��IIR
	printf("LCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_LCR_ADDR));//��LCR
	printf("MCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_MCR_ADDR));//��MCR
	printf("LSR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_LSR_ADDR));//��LSR
	printf("MSR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_MSR_ADDR));//��MSR
	CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, 0x78);
	printf("SCR: %02x\n",(unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR));//��SCR
#else
			unsigned short  ch348qnum = (unsigned short)offsetadd[num];
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_IER_ADDR);//��IER
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_IIR_ADDR);//��IIR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_LCR_ADDR);//��LCR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_MCR_ADDR);//��MCR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_LSR_ADDR);//��LSR
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_MSR_ADDR);//��SCR
			#if 1
			CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, 0x78);
			ch348qnum = (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//��SCR
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
  * Description    : 438�Ĵ���д��SCR�����Ĵ���
  * Input          : num ���ںţ�0-7�� data ����
  * Return         : �����data���� 
  */
unsigned char CH438_save_data(unsigned char num,unsigned char data){
	CH438WriteReg(offsetadd[num] | REG_SCR_ADDR, data);
	return (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//��SCR
}
/**
  * Function Name  : CH438_save_data()
  * Description    : 438�Ĵ�����ȡSCR�����Ĵ���
  * Input          : num ���ں�
  * Return         : �����data���� 
  */
unsigned char CH438_read_data(unsigned char num){
	return (unsigned short)CH438ReadReg(offsetadd[num] | REG_SCR_ADDR);//��SCR
}



/**
  * Function Name  : CH438InterruptFun()
  * Description    : ch438�жϷ�ʽ����
  * Input          : None
  * Return         : None
  */
void CH438InterruptFun (void)
{
	uint8_t gInterruptStatus;		/* ȫ���ж�״̬ */
	uint8_t InterruptStatus;		/* ���������ж�״̬ */	
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
		if( gInterruptStatus & Interruptnum[i] )    /* ����ĸ����ڷ����ж� */
		{
			InterruptStatus = CH438ReadReg( offsetadd[i] | REG_IIR_ADDR ) & 0x0f;    /* �����ڵ��ж�״̬ */	

			switch( InterruptStatus )
			{
				case INT_NOINT:			/* û���ж� */					
					break;
				case INT_THR_EMPTY:		/* THR���ж� */						
					break;
				case INT_RCV_OVERTIME:	/* ���ճ�ʱ�ж� */
								// ���û�����
					RevLen = 0;
					memset(Revbuff,0,sizeof(Revbuff));
					RevLen = CH438_RecvDatas(i, Revbuff);
					RevFlag = 1;
					break;
				case INT_RCV_SUCCESS:	/* �������ݿ����ж� */
								// ���û�����
					RevLen = 0;
					memset(Revbuff,0,sizeof(Revbuff));
					RevLen = CH438_RecvDatas(i, Revbuff);
					RevFlag = 2;
					break;
				case INT_RCV_LINES:		/* ������·״̬�ж� */
					CH438ReadReg( offsetadd[i] | REG_LSR_ADDR );
					break;
				case INT_MODEM_CHANGE:	/* MODEM����仯�ж� */
					CH438ReadReg( offsetadd[i] | REG_MSR_ADDR );
					break;
				default:
					break;

			}
		}
	}
}


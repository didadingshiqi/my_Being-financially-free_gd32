/********************************** (C) COPYRIGHT ******************************
* File Name          : CH438_driver.h
* Author             : WCH
* Version            : 
* Date               : 2014/9/11
* Description        : ch438��������ͷ�ļ�
*******************************************************************************/

#ifndef _CH438_DRIVER
#define _CH438_DRIVER

#define MaxRecvLen    50         	  /* ���ջ�������С    */

extern unsigned char Revbuff[MaxRecvLen];		/* ���ջ����� */
extern unsigned char RevLen;					/* ���ռ��� */
extern unsigned char RevFlag;					/* ���ձ�־ */
extern unsigned char ch438q_com;							/* ���մ��� */

unsigned char CH438_CheckIIR(unsigned char num);		/* ��IIR�Ĵ��� */
void CH438_CloseSeril(unsigned char num);				/* �رմ��� */
void CH438_CloseALLSeril(void);							/* �ر����д���,����͹��� */
void CH438_ResetSeril(unsigned char num);				/* ��λ���� */

void CH438_UARTInit(unsigned char num);					/* ���ڳ�ʼ�� */

void CH438_INTConfig(unsigned char num);				/* �����жϿ��� */
void CH438_AutoHFCtrl(unsigned char num);				/* Ӳ���Զ������� */

void CH438_SendDatas(unsigned char num, unsigned char* sendbuff,unsigned char len);		/* ��ѯ��ʽ:�������� */
unsigned char CH438_RecvDatas(unsigned char num, unsigned char* revbuff);				/* ��ѯ��ʽ���������� */

void CH438InterruptFun (void);				/* �����жϺ��� */

int CH438_RegTEST(unsigned char num);	/* ���Դ��ڼĴ���ͨѶ */
void DELAY(unsigned char time);


#endif

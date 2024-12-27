/********************************** (C) COPYRIGHT ******************************
* File Name          : port.h
* Author             : WCH
* Version            : 
* Date               : 2014/9/1
* Description        :ch438��stm32���ӽӿ�����ͷ�ļ�
*******************************************************************************/

#ifndef _MULTI_ADD_H
#define _MULTI_ADD_H

#define MAX_485_WR 	gpio_bit_set(GPIOC, GPIO_PIN_8);
#define MAX_485_RD  gpio_bit_reset(GPIOC, GPIO_PIN_8);

void CH438PortConfig(void);										/* CH438�ӿ����� */
void CH438WriteReg(unsigned char add,unsigned char data);				/* дCH438 */
unsigned char CH438ReadReg(unsigned char add);	        				/* ��CH438 */

void GD32_XINT_Config(void);													/* ��Ƭ���ж����� */

#endif

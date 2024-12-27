#ifndef __PARAMS_MANAGE_H__
#define __PARAMS_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "gd32f10x.h"

//�ϵ���ó�ʼ��������Ȼ��Ϳ��Լ���֮ǰ�洢��flash�е�����
void store_Init (void);
//���ñ��溯���ͻὫ����д��flash�洢����
void store_save (void);
	
void store_clear (void);

extern uint16_t store_Data [1024];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

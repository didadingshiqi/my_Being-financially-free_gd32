#ifndef __PARAMS_MANAGE_H__
#define __PARAMS_MANAGE_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdint.h>
#include "gd32f10x.h"

//上电调用初始化函数，然后就可以加载之前存储在flash中的数据
void store_Init (void);
//调用保存函数就会将数据写入flash存储器中
void store_save (void);
	
void store_clear (void);

extern uint16_t store_Data [1024];

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif

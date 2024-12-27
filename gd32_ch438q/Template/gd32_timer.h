#ifndef _GD32_TIMER_H
#define _GD32_TIMER_H

/* 包含头文件 ---------------------------------------------------------------*/
#include "gd32f10x.h"
#include <stdio.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
/* 类型定义 ------------------------------------------------------------------*/

/* 宏定义 --------------------------------------------------------------------*/
void TIM_Init(void);
void TIM_DelayMs(__IO uint32_t ms);
void TIM_ValDecrese(void);

#endif 

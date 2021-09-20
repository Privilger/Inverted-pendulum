#ifndef _TIMER1_H
#define _TIMER1_H
#include "sys.h"

//extern TIM_HandleTypeDef TIM3_Handler;      //¶¨Ê±Æ÷¾ä±ú 

//void TIM3_Init(u16 arr,u16 psc);
//void TIM3_PWM_Init(u16 arr,u16 psc);
//void TIM_SetTIM3Compare2(u32 compare);
extern TIM_HandleTypeDef htim1;
void MX_TIM1_Init(void);
void USR_TIM_PWM_SetCompare(uint16_t duty);
#endif


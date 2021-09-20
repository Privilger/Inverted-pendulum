#ifndef _EXTI_H
#define _EXTI_H
#include "sys.h"

extern u8 limitAflag;
extern u8 limitAflaglock; 	
extern u16 limitBflag;
extern u16 limitBcnt;


void EXTI_Init(void);
u8 LimitB_Read(void);
void LimitB_Task(void);
void LimitA_Task(void);
#endif

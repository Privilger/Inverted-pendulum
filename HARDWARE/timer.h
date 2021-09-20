#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
#include "main.h"
extern TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 

void TIM3_Init(u16 arr,u16 psc);


extern u32 positionend;//�յ� ���ξ���
extern u32 positionmid;//�е�
extern u32 position;	 	//��ǰʵʱλ��
extern u32 positionlast;

extern u32 encoderdiff;
extern u32 encodercntlast;
extern u32 encodercnt;
extern u16 encodertime;


extern u8 DirectionB;
extern	u8 debugcmd;

extern u8 resetflag;
extern u8 resetstep;		//��λ���� 0�տ�ʼ ��0��λ ȷ�����  1���յ�λ ȷ���յ� 2 �ƶ����м��
extern u32 resettime;
extern u32 resetendcnt;
extern u32 positionendavg;

extern u8 findmidflag;
extern u16 speed;


extern u8 findmidstep;


void PositionReset_Task(void);
void PositionFindMid_Task(void);

//#define	dTimeTickCntMax		2000000UL	//��λ5us 1s
#define	dTimeTickCntMax		36000000UL	//��λ1ms 10h


typedef struct TimeTickStruct{
	uint32_t TimeTickCnt;			
	uint32_t TimeTickNow;
	uint32_t TimeTickLast;
	uint32_t TimeTickDiff;
}TimeTickStruct;
extern volatile TimeTickStruct sTimeTick;

void TimeTick_Init(void);
uint32_t TimeTick_GetNowTime(void);
uint32_t TimeTick_GetCntTimeDiff(void); 
uint32_t TimeTick_GetNowTimeDiff(void);
uint32_t TimeTick_GetTimeDiff(uint32_t time1,uint32_t time2) ;
#endif


#ifndef _TIMER_H
#define _TIMER_H
#include "sys.h"
#include "main.h"
extern TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 

void TIM3_Init(u16 arr,u16 psc);


extern u32 positionend;//终点 整段距离
extern u32 positionmid;//中点
extern u32 position;	 	//当前实时位置
extern u32 positionlast;

extern u32 encoderdiff;
extern u32 encodercntlast;
extern u32 encodercnt;
extern u16 encodertime;


extern u8 DirectionB;
extern	u8 debugcmd;

extern u8 resetflag;
extern u8 resetstep;		//复位步骤 0刚开始 往0点位 确认起点  1往终点位 确认终点 2 移动到中间点
extern u32 resettime;
extern u32 resetendcnt;
extern u32 positionendavg;

extern u8 findmidflag;
extern u16 speed;


extern u8 findmidstep;


void PositionReset_Task(void);
void PositionFindMid_Task(void);

//#define	dTimeTickCntMax		2000000UL	//单位5us 1s
#define	dTimeTickCntMax		36000000UL	//单位1ms 10h


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


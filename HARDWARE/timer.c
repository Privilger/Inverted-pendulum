#include "timer.h"


TIM_HandleTypeDef TIM3_Handler;      //定时器句柄 

//通用定时器3中断初始化
//arr：自动重装值。
//psc：时钟预分频数
//定时器溢出时间计算方法:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=定时器工作频率,单位:Mhz
//这里使用的是定时器3!
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //通用定时器3
    TIM3_Handler.Init.Prescaler=psc;                     //分频系数
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //向上计数器
    TIM3_Handler.Init.Period=arr;                        //自动装载值
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//时钟分频因子
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //使能定时器3和定时器3更新中断：TIM_IT_UPDATE   
}

//定时器底册驱动，开启时钟，设置中断优先级
//此函数会被HAL_TIM_Base_Init()函数调用
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //使能TIM3时钟
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //设置中断优先级，抢占优先级1，子优先级3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //开启ITM3中断   
	}
}

//定时器3中断服务函数
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

u32 positionend=0;//终点 整段距离
u32 positionmid=0;//中点
u32 position=0;	 	//当前实时位置
u32 positionlast=0;

u32 encoderdiff=0;
u32 encodercntlast=0;
u32 encodercnt=0;
u16 encodertime=0;

u8 DirectionB=0;
u8 debugcmd=0;		//0 速度  1复位校准

u8 resetflag=0;		//上电检测复位标志 是否校准过 1校准过
u8 resetstep=0;		//复位步骤 0刚开始 往0点位 确认起点  1往终点位 确认终点 2 移动到中间点
u32 resettime=0;
u32 resetendcnt=0;
u32 positionendavg=0;


u8 findmidstep=0;	
u8 findmidflag=0;

u16 speed=0;

char dir=0;
int Read_Encoder(u8 TIMX)
{
  int Encoder_TIM;
	Encoder_TIM = __HAL_TIM_GET_COUNTER(&htim4);	
		
	__HAL_TIM_SET_COUNTER(&htim4,0);
	return Encoder_TIM;
}

u32 Encoder_PositionGetDiff(void)
{
	#if 0
	//反转 远离0点位置
	if(dir==1)//dir==1  DirectionB==2
	{
		if(encodercnt<5000)
		{
			return 0;
		}
		else			
			return (65535-encodercnt);
		
//		if(encodercntlast>encodercnt)
//		{
//			return (encodercntlast-encodercnt);
//		}
//		else if(encodercntlast<encodercnt)
//		{
//			return (65535-encodercnt+encodercntlast);
//		}
//		else
//		{
//			return 0;
//		}		
	}
	////正转 -->0点位置
	else if(dir==0)//dir==0  DirectionB==1
	{
		if(encodercnt<5000)
		{
				return encodercnt;
		}
		else
			return 0;
		
//		if(encodercnt>encodercntlast)
//		{
//			return (encodercnt-encodercntlast);
//		}
//		else if(encodercnt<encodercntlast)
//		{
//			return (65535-encodercntlast+encodercnt);
//		}
//		else
//		{
//			return 0;
//		}
	}
	return 0;
	
	#endif
	u32 diff=0;
	//反转 远离0点位置
	if(dir==1)//dir==1  DirectionB==2
	{
		if(encodercntlast>encodercnt)
		{
			diff=(encodercntlast-encodercnt);
			//return (encodercntlast-encodercnt);
		}
		else if(encodercntlast<encodercnt)
		{
			if(encodercnt>60000)
			{
				diff=(65535-encodercnt+encodercntlast);
				//return (65535-encodercnt+encodercntlast);
			}
			else
			{
				diff=0;
				//return 0;
			}
		}
		else
		{
			return 0;
		}		
	}
	////正转 -->0点位置
	else if(dir==0)//dir==0  DirectionB==1
	{		
		if(encodercnt>encodercntlast)
		{
			diff=(encodercnt-encodercntlast);
			//return (encodercnt-encodercntlast);
		}
		else if(encodercnt<encodercntlast)
		{
			if(encodercntlast>60000)
				diff=(65535-encodercntlast+encodercnt);
				//return (65535-encodercntlast+encodercnt);
			else
				diff=0;
				//return 0;
		}
		else
		{
			return 0;
		}
	}
	if(diff>10000)diff=0;//防抖动
	return diff;
}

void PositionFindMid_Task(void)
{
	if(debugcmd==1)
	{		
		if(findmidstep==1)
		{
			//-->起点
			DirectionB=1;	
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
			USR_TIM_PWM_SetCompare(1500);
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);	
			findmidstep=2;		
		}	
		else if(findmidstep==3)
		{
			//起点-->找中点
			DirectionB=2;							
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//AIN2					
			USR_TIM_PWM_SetCompare(5500);
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
			findmidstep=4;	
		}
		else if(findmidstep==4)
		{
			if(position+2500>positionmid)//电机转一圈5000左右
			{
				DirectionB=0;
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
				HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
				
				//复位校准			
				debugcmd=0;
				findmidstep=0;				
				
				
				findmidflag=1;
				Debug_SetData();
				Debug_SendData();			
			}
		}
	}
}

void PositionReset_Task(void)
{
	if(debugcmd==1)
	{
		
	if(resetstep==1)
	{
		//-->起点
		DirectionB=1;	
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
		USR_TIM_PWM_SetCompare(1500);
		HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);	
		resetstep=2;		
	}
	else if(resetstep==3)//完成到起点-->找终点
	{
		DirectionB=2;							
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//AIN2					
		USR_TIM_PWM_SetCompare(5500);
		HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
		resetstep=4;
	}
	else if(resetstep==4)//检测是否到终点
	{
		if(DirectionB==2)
		{
			if(TimeTick_GetTimeDiff(sTimeTick.TimeTickCnt,resettime) > 100)//100ms
			{
				resettime=TimeTick_GetNowTime();
				if(position > positionlast)
				{
					if((position - positionlast)<50)
					{
						resetendcnt++;
						positionendavg+=position;
					}
				}
				else 
				{
					resetendcnt++;	
					positionendavg+=position;					
				}
				
				if(resetendcnt>=5)
				{
					
					resetstep=5;	
					positionend=positionendavg/resetendcnt;//终点 整段距离
					positionmid=positionend/2;//中点
					resetendcnt=0;
					positionendavg=0;
					
					//停止电机运动防止堵转
					DirectionB=0;//停止状态
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
					HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);	
	
					
					//-->向起点方向移动到中点
					DirectionB=1;	
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
					USR_TIM_PWM_SetCompare(1500);
					HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);					
					
				}					
			}				
		}
	}
	else if(resetstep==5)//检测是否到中点附近
	{
		if(position<positionmid+2500)//电机转一圈5000左右
		{
			DirectionB=0;
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
			
			//复位校准			
			debugcmd=0;
			resetstep=0;
			
			Position_SaveEnd();
			findmidflag=1;
			Debug_SetData();	
			Debug_SendData();
						
		}
	}
	}
}



//回调函数，定时器中断服务函数调用 1ms
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if(htim==(&TIM3_Handler))
	{    
		//time		
		sTimeTick.TimeTickCnt++;			
		if(sTimeTick.TimeTickCnt>=dTimeTickCntMax)
		{
			sTimeTick.TimeTickCnt=0;	
		}
		
		//if(DirectionB!=0)//运动才记位置
		{
			encodertime++;
			if(encodertime>=5)
			{
				encodertime=0;					
				encodercntlast=encodercnt;	
				dir = __HAL_TIM_IS_TIM_COUNTING_DOWN(&htim4);					
				encodercnt=__HAL_TIM_GET_COUNTER(&htim4);		
			//	__HAL_TIM_SET_COUNTER(&htim4,0);
				positionlast=position;
				
				if(dir==0)//dir==0  DirectionB==1
				//if(DirectionB==1)
				{
					encoderdiff=Encoder_PositionGetDiff();
					if(position>=encoderdiff)
					{
						position-=encoderdiff;	
					}						
					else
					{
						position=0;
					}
				}
				else if(dir==1)//dir==1  DirectionB==2
				//else if(DirectionB==2)
				{
					encoderdiff=Encoder_PositionGetDiff();	
					position+=encoderdiff;	
				}
								
			}
		}
		
		//限位开关检测
		if(LimitB_Read())
		{			
			limitBflag=0;
			limitBcnt=0;
		}
		else
		{
			limitBcnt++;
			if(limitBcnt>500)
	 		{
				limitBcnt=0;
				limitBflag=1;
			}
		}
		
		//UART
		if(USARTHMIRxTimeOutFlag==1)
		{
			if(USARTHMIRxTimeOutCnt<dUSARTHMI_TimeOutMax)  
			{
				USARTHMIRxTimeOutCnt++;
			}
			else
			{
				USARTHMIRxTimeOutCnt=0;
				USARTHMIRxTimeOutFlag=0;  
			}			
		}
		
	}
}

volatile TimeTickStruct sTimeTick={0};

void TimeTick_Init(void) 
{
	sTimeTick.TimeTickCnt = 0;
	sTimeTick.TimeTickNow = 0;
	sTimeTick.TimeTickLast = 0;
	sTimeTick.TimeTickDiff = 0;
}

uint32_t TimeTick_GetNowTime(void) 
{
	sTimeTick.TimeTickLast = sTimeTick.TimeTickNow;//更新
	sTimeTick.TimeTickNow = sTimeTick.TimeTickCnt;
	return sTimeTick.TimeTickNow;
}

//cnt now
uint32_t TimeTick_GetCntTimeDiff(void) 
{
	if(sTimeTick.TimeTickCnt > sTimeTick.TimeTickNow)
	{
		sTimeTick.TimeTickDiff=sTimeTick.TimeTickCnt - sTimeTick.TimeTickNow;
	}
	else
	{
		sTimeTick.TimeTickDiff=dTimeTickCntMax - (sTimeTick.TimeTickNow - sTimeTick.TimeTickCnt);
	}
	return sTimeTick.TimeTickDiff;
}

//now last
uint32_t TimeTick_GetNowTimeDiff(void) 
{
	if(sTimeTick.TimeTickNow > sTimeTick.TimeTickLast)
	{
		sTimeTick.TimeTickDiff=sTimeTick.TimeTickNow - sTimeTick.TimeTickLast;
	}
	else
	{
		sTimeTick.TimeTickDiff=dTimeTickCntMax - (sTimeTick.TimeTickLast - sTimeTick.TimeTickNow);
	}
	return sTimeTick.TimeTickDiff;
}
//time1-time2 时间差
uint32_t TimeTick_GetTimeDiff(uint32_t time1,uint32_t time2) 
{
	uint32_t diff;
	if(time1 > time2)
	{
		diff=time1-time2;
	}
	else
	{
		diff=dTimeTickCntMax - (time2 - time1);
	}
	return diff;
}

#include "timer.h"


TIM_HandleTypeDef TIM3_Handler;      //��ʱ����� 

//ͨ�ö�ʱ��3�жϳ�ʼ��
//arr���Զ���װֵ��
//psc��ʱ��Ԥ��Ƶ��
//��ʱ�����ʱ����㷽��:Tout=((arr+1)*(psc+1))/Ft us.
//Ft=��ʱ������Ƶ��,��λ:Mhz
//����ʹ�õ��Ƕ�ʱ��3!
void TIM3_Init(u16 arr,u16 psc)
{  
    TIM3_Handler.Instance=TIM3;                          //ͨ�ö�ʱ��3
    TIM3_Handler.Init.Prescaler=psc;                     //��Ƶϵ��
    TIM3_Handler.Init.CounterMode=TIM_COUNTERMODE_UP;    //���ϼ�����
    TIM3_Handler.Init.Period=arr;                        //�Զ�װ��ֵ
    TIM3_Handler.Init.ClockDivision=TIM_CLOCKDIVISION_DIV1;//ʱ�ӷ�Ƶ����
    HAL_TIM_Base_Init(&TIM3_Handler);
    
    HAL_TIM_Base_Start_IT(&TIM3_Handler); //ʹ�ܶ�ʱ��3�Ͷ�ʱ��3�����жϣ�TIM_IT_UPDATE   
}

//��ʱ���ײ�����������ʱ�ӣ������ж����ȼ�
//�˺����ᱻHAL_TIM_Base_Init()��������
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  if(htim->Instance==TIM3)
	{
		__HAL_RCC_TIM3_CLK_ENABLE();            //ʹ��TIM3ʱ��
		HAL_NVIC_SetPriority(TIM3_IRQn,1,3);    //�����ж����ȼ�����ռ���ȼ�1�������ȼ�3
		HAL_NVIC_EnableIRQ(TIM3_IRQn);          //����ITM3�ж�   
	}
}

//��ʱ��3�жϷ�����
void TIM3_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&TIM3_Handler);
}

u32 positionend=0;//�յ� ���ξ���
u32 positionmid=0;//�е�
u32 position=0;	 	//��ǰʵʱλ��
u32 positionlast=0;

u32 encoderdiff=0;
u32 encodercntlast=0;
u32 encodercnt=0;
u16 encodertime=0;

u8 DirectionB=0;
u8 debugcmd=0;		//0 �ٶ�  1��λУ׼

u8 resetflag=0;		//�ϵ��⸴λ��־ �Ƿ�У׼�� 1У׼��
u8 resetstep=0;		//��λ���� 0�տ�ʼ ��0��λ ȷ�����  1���յ�λ ȷ���յ� 2 �ƶ����м��
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
	//��ת Զ��0��λ��
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
	////��ת -->0��λ��
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
	//��ת Զ��0��λ��
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
	////��ת -->0��λ��
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
	if(diff>10000)diff=0;//������
	return diff;
}

void PositionFindMid_Task(void)
{
	if(debugcmd==1)
	{		
		if(findmidstep==1)
		{
			//-->���
			DirectionB=1;	
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
			USR_TIM_PWM_SetCompare(1500);
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);	
			findmidstep=2;		
		}	
		else if(findmidstep==3)
		{
			//���-->���е�
			DirectionB=2;							
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//AIN2					
			USR_TIM_PWM_SetCompare(5500);
			HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
			findmidstep=4;	
		}
		else if(findmidstep==4)
		{
			if(position+2500>positionmid)//���תһȦ5000����
			{
				DirectionB=0;
				HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
				HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
				
				//��λУ׼			
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
		//-->���
		DirectionB=1;	
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
		USR_TIM_PWM_SetCompare(1500);
		HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);	
		resetstep=2;		
	}
	else if(resetstep==3)//��ɵ����-->���յ�
	{
		DirectionB=2;							
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//AIN2					
		USR_TIM_PWM_SetCompare(5500);
		HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
		resetstep=4;
	}
	else if(resetstep==4)//����Ƿ��յ�
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
					positionend=positionendavg/resetendcnt;//�յ� ���ξ���
					positionmid=positionend/2;//�е�
					resetendcnt=0;
					positionendavg=0;
					
					//ֹͣ����˶���ֹ��ת
					DirectionB=0;//ֹͣ״̬
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
					HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);	
	
					
					//-->����㷽���ƶ����е�
					DirectionB=1;	
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
					USR_TIM_PWM_SetCompare(1500);
					HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);					
					
				}					
			}				
		}
	}
	else if(resetstep==5)//����Ƿ��е㸽��
	{
		if(position<positionmid+2500)//���תһȦ5000����
		{
			DirectionB=0;
			HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
			HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
			
			//��λУ׼			
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



//�ص���������ʱ���жϷ��������� 1ms
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
		
		//if(DirectionB!=0)//�˶��ż�λ��
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
		
		//��λ���ؼ��
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
	sTimeTick.TimeTickLast = sTimeTick.TimeTickNow;//����
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
//time1-time2 ʱ���
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

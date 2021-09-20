#include "main.h"

void LimitB_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */

  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void EXTI_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */

  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin : PB9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void)
{
  /* USER CODE BEGIN EXTI9_5_IRQn 0 */
	
  /* USER CODE END EXTI9_5_IRQn 0 */
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_9);
  /* USER CODE BEGIN EXTI9_5_IRQn 1 */

  /* USER CODE END EXTI9_5_IRQn 1 */
}

u8 limitAflag=0;
u8 limitAflaglock=0;
u16 limitBflag=0;
u16 limitBcnt=0;

#define dLimitAProtectDistance			10000//到达终点附近多远 刹车
#define dLimitAClearProtectDistance	15000//离终点多远后清除标志位


u8 LimitB_Read(void)
{
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_9))
	{
		return 1;
	}
	else
		return 0;
}

//终点 防撞
void LimitA_Task(void)
{
	if(position+dLimitAProtectDistance>positionend)//到达终点附近 刹车
	{
		limitAflag=1;		
	}
	else if(position+dLimitAClearProtectDistance<positionend)//离开终点附近 清除标志位
	{
		limitAflag=0;		
		limitAflaglock=0;		
	}
	
	if(limitAflag==1 && limitAflaglock==0)
	{
		limitAflaglock=1;
		
		//停止电机运动防止堵转
		DirectionB=0;//停止状态
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
		HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);	
		
		limitAflaglock=1;
	}		
}

//起点 限位开关
void LimitB_Task(void)
{
	if(limitBflag==1)
	{
		if(resetstep==2)
		{
			resetstep=3;
		}
		if(findmidstep==2)
		{
			findmidstep=3;
		}		
		
		limitBflag=0;
				
		//停止电机运动防止堵转
		DirectionB=0;//停止状态
		HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
		HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);	
			
		position=0;//0点位	
		
	//	HAL_TIM_Encoder_Stop(&htim4, TIM_CHANNEL_ALL);
	//	__HAL_TIM_SET_COUNTER(&htim4,0);
	//	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);	
		
	}
}


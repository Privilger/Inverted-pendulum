#include "main.h"



void SystemClock_Config(void);


void GPIO_Init(void)
{
	GPIO_InitTypeDef GPIO_Initure;

	__HAL_RCC_GPIOB_CLK_ENABLE();           	

	GPIO_Initure.Pin=GPIO_PIN_12|GPIO_PIN_13; 				//PB12 PB13
	GPIO_Initure.Mode=GPIO_MODE_OUTPUT_PP;  	//推挽输出
	GPIO_Initure.Pull=GPIO_PULLUP;          	//上拉
	GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;    //高速
	HAL_GPIO_Init(GPIOB,&GPIO_Initure);

	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12|GPIO_PIN_13,GPIO_PIN_RESET);	
}

#define	dFlashStoreLen  4	//ABCD 	
#define	dFlashDataLen  2	// 	
u16 flashdata[dFlashDataLen]={0};

void Position_ReadEnd(void)
{
	u8 i;
	u16 temp=0;
	u16 buff[dFlashStoreLen]={0};
  
	STMFLASH_Read(FLASH_SAVE_ADDR,(u16*)buff,dFlashStoreLen);
	
	if(buff[0]==0xABCD)
	{
		for(i=0;i<dFlashDataLen;i++)
		{
			temp^=buff[i+1];
		}
		if(temp==buff[dFlashStoreLen-1])
		{
			for(i=0;i<dFlashDataLen;i++)
			{
				flashdata[i]=buff[i+1];
			}
			
			resetflag=1;
			positionend=0;			
			positionend=flashdata[0];
			positionend=(positionend<<16)|(flashdata[1]);
			positionend=160000;	
			positionmid=positionend/2;
		}
		else	//数据不对
		{
			resetflag=0;	
			positionend=160000;	
			positionmid=positionend/2;			
		}
	}
	else
	{
		//刚出厂 没设置过最大值
		resetflag=0;		
		positionend=160000;	
		positionmid=positionend/2;	
	}
}

void Position_SaveEnd(void)	
{
	u8 i;
	u16 buff[dFlashStoreLen]={0};
	for(i=0;i<dFlashStoreLen;i++)
	{
		buff[i]=0x0000;
	}
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)buff,dFlashStoreLen);
	
	delay_ms(100);
	
	buff[0]=0xABCD;	
	buff[1]=(positionend>>16)&0x0000ffff;//high 2byte
	buff[2]=(positionend)&0x0000ffff;			//low
	buff[3]	=buff[1]^buff[2];		
	STMFLASH_Write(FLASH_SAVE_ADDR,(u16*)buff,dFlashStoreLen);
	resetflag=1;
}
int main(void)
{
   HAL_Init();

	Stm32_Clock_Init(RCC_PLL_MUL9); 
	delay_init(72); 

	GPIO_Init();
	MX_TIM1_Init();	
	HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
	
	TimeTick_Init(); 
	TIM3_Init(10-1,8400-1);//1ms  
	UART_Init();
	ADC_Init();
	delay_ms(1000);
	
	MX_TIM4_Init();
	HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);	
	
	Position_ReadEnd();
	
	delay_ms(2000);
	
	HAL_ADC_Start_DMA(&hadc1,(uint32_t*)ADC_DMA_ConvertedValue,2);	
	hadc1.Instance->CR2 |= (uint32_t)ADC_CR2_SWSTART;
	delay_ms(1000);
  while (1)
  {	
		delay_ms(1);
		Debug_SendTask();
		Debug_ReceiveTask();
		LimitA_Task();
		LimitB_Task();
		PositionReset_Task();
		PositionFindMid_Task();
  } 
}


void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}







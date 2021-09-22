#include "main.h"	
#include "sys.h"
#include "usart.h"	

//�������´���,֧��printf����,������Ҫѡ��use MicroLIB	  
#if 1
#pragma import(__use_no_semihosting)             
//��׼����Ҫ��֧�ֺ���                 
struct __FILE 
{ 
	int handle; 

}; 

FILE __stdout;       
//����_sys_exit()�Ա���ʹ�ð�����ģʽ    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//�ض���fputc���� 
int fputc(int ch, FILE *f)
{      
	while((USART1->SR&0X40)==0);//ѭ������,ֱ���������   
    USART1->DR = (u8) ch;      
	return ch;
}
#endif 



UART_HandleTypeDef UART1_Handler,UART2_Handler; //UART���
 

DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;
DMA_HandleTypeDef hdma_usart2_tx;
DMA_HandleTypeDef hdma_usart2_rx;








#if 1

volatile u8 debugtxbusy=0;
volatile u8 debugtxnum=0;
volatile u8 debugtxbegin=0;
volatile u8 debugtxend=0;
volatile u8 debugTxData[dDebugDataLenMax]={0};					//�ټ����ݷ�������֡
volatile u8 debugTxDataBuff[dDebugDataLenMax]={0};
volatile u8 debugTxRing[dDebugTxBuffLenMax]={0};				//�ټ����ݷ������ݻ�����	
volatile u8 debugTxRingBuff[dDebugTxBuffLenMax]={0};

void Debug_WriteRingBuff(u8 data)
{		
	debugTxRingBuff[debugtxend]=data;
	debugtxend++;
	if(debugtxend >= dDebugTxBuffLenMax)
	{
		debugtxend=0;
	}

	//end׷��begin begin�ڵ�ƫ�ƣ�������������
	if(debugtxbegin == debugtxend)
	{
		debugtxbegin++;
		if(debugtxbegin >= dDebugTxBuffLenMax)
		{
			debugtxbegin=0;
		}
	}
}

u16 Debug_ReadRingBuff(u8* pdata)
{
	u16 length=0;	//��ǰͷβ�ڵ�ɶ����ݳ���
	u16 offset=0;
	memset((char*)pdata,0,dDebugTxBuffLenMax);
	
	//��������ݳ��ȣ�����ֱ��forѭ��end����һ���ֻؾʹ���
	if(debugtxbegin < debugtxend)
	{
		length = debugtxend - debugtxbegin;
	}
	else if(debugtxbegin > debugtxend)
	{
		length = dDebugTxBuffLenMax - debugtxbegin + debugtxend;
	}
	else
	{
		return length;
	}
	for(offset=0;offset<length;offset++)
	{		
		pdata[offset] = debugTxRingBuff[debugtxbegin];
		debugtxbegin++; 
		if(debugtxbegin >= dDebugTxBuffLenMax)  //����������ֵ ����ͷ��ʼ�������
		{
			debugtxbegin=0; 
		}
	}
	return length;	
}

void Debug_DmaTxIrqHandler(void)
{	
	debugtxnum=Debug_ReadRingBuff((uint8_t*)debugTxRing);//���������Ƿ�������
	if(debugtxnum)
	{
		HAL_UART_Transmit_IT(&UART1_Handler,(uint8_t*)debugTxRing,debugtxnum);
	}
	else
	{
		debugtxbusy=0;
	}
}

#define dDebugSendTime 				10	//��λms		�������ݼ������ϱ�һ��

//12byte

uint16_t pre_angular=0;
int pole_angular_velocity=0;
u32 pre_position=0;//��һʱ��λ��
int cart_speed=0;

void calc_cart_speed()
{
	// cart_speed = (int)(position - pre_position) / (dDebugSendTime * 0.001);
	cart_speed = (int)(position - pre_position);
	pre_position = position;
}

void calc_angle_speed()
{
	int diff = (int)(UserADCValue[0] - pre_angular);
	if (diff > 3000){
		diff -= 4000;
	}
	if (diff < -3000){
		diff += 4000;
	}
	// pole_angular_velocity = diff / (dDebugSendTime * 0.001);
	pole_angular_velocity = diff;
	pre_angular = UserADCValue[0];
}

void Debug_SetData(void)	
{
	calc_cart_speed();
	calc_angle_speed();
	// ��ͷ
	debugTxData[0]=0x5A;
	debugTxData[1]=0xA5;
	// cart λ��
	debugTxData[2]=(position>>24)&0x000000ff;		
	debugTxData[3]=(position>>16)&0x000000ff;
	debugTxData[4]=(position>>8)&0x000000ff;
	debugTxData[5]=(position)&0x000000ff;
	// cart �ٶ�
	debugTxData[6]=(cart_speed>>24)&0x000000ff;		
	debugTxData[7]=(cart_speed>>16)&0x000000ff;
	debugTxData[8]=(cart_speed>>8)&0x000000ff;
	debugTxData[9]=(cart_speed)&0x000000ff;
	// pole λ��
	debugTxData[10]=(UserADCValue[0]>>8)&0x00ff;	//ADC1 HIGH BYTE   youyong
	debugTxData[11]=(UserADCValue[0])&0x00ff;		//ADC1 Low BYTE
	// pole �ٶ�
	debugTxData[12]=(pole_angular_velocity>>24)&0x000000ff;		
	debugTxData[13]=(pole_angular_velocity>>16)&0x000000ff;
	debugTxData[14]=(pole_angular_velocity>>8)&0x000000ff;
	debugTxData[15]=(pole_angular_velocity)&0x000000ff;
	// reset flag
	debugTxData[16]=findmidflag;
	debugTxData[17]=findmidflag;
	// ��β
	debugTxData[18]=0x0d;
	debugTxData[19]=0x0a;
	// debugTxData[10]=0x6B;
	// debugTxData[11]=0xB6;
}

void Debug_SendData(void)
{
	u8 i;
	if(debugtxbusy)
	{
		for(i=0;i<dDebugDataLenMax;i++)
		{
			Debug_WriteRingBuff(debugTxData[i]);
		}				
	}
	else
	{			
		memcpy((u8*)debugTxDataBuff,(u8*)debugTxData,dDebugDataLenMax);//
		HAL_UART_Transmit_IT(&UART1_Handler,(u8*)debugTxDataBuff,dDebugDataLenMax);
		debugtxbusy=1;
	}
}


u32 debugtime=0;
extern u32 uarttime;

void Debug_SendTask(void)	
{
	
	if(uarttime >= dDebugSendTime)
	//if(TimeTick_GetTimeDiff(sTimeTick.TimeTickCnt,debugtime) > dDebugSendTime)
	{
		uarttime=0;
		//debugtime=TimeTick_GetNowTime();
		Debug_SetData();
		Debug_SendData();
	}
}

#endif


volatile uint16_t USARTHMIRxCnt=0;
volatile uint8_t USARTHMIRxTimeOutFlag=0;
volatile uint16_t USARTHMIRxTimeOutCnt=0;
volatile uint8_t USARTHMI_DMABUFF[dUSARTHMI_DMABuffLen]={0};
void Debug_RxHandler(void)
{
	uint8_t Res;
	
	if((__HAL_UART_GET_FLAG(&UART1_Handler,UART_FLAG_RXNE)!=RESET))  //�����ж�
	{
		HAL_UART_Receive(&UART1_Handler,&Res,1,0);   //timeout ��λms  
		
		
		USARTHMI_DMABUFF[USARTHMIRxCnt]=Res;
		USARTHMIRxCnt++;
		USARTHMIRxTimeOutFlag=1;
	}	
}
//5A A5 00 01 00 00 6B B6 //Ŀǰ 8Byte

void Debug_DataRxHandler(void)
{	
	if(USARTHMIRxCnt!=0)
	{
		if((USARTHMI_DMABUFF[0] == 0X5A) && (USARTHMI_DMABUFF[1] == 0XA5))
		{
			if((USARTHMI_DMABUFF[6] == 0X6B) && (USARTHMI_DMABUFF[7] == 0XB6))
			{				
				//user
				speed = USARTHMI_DMABUFF[3];
				speed = (speed<<8)|(USARTHMI_DMABUFF[4]);
		
				if(USARTHMI_DMABUFF[2]==0)//ֹͣ
				{
					debugcmd=0;
					DirectionB=0;
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2	
					HAL_TIM_PWM_Stop(&htim1,TIM_CHANNEL_1);
				}
				else if(USARTHMI_DMABUFF[2]==1)//��ת -->0��λ��
				{		
					debugcmd=0;
					DirectionB=1;					
					HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_RESET);//AIN2					
					USR_TIM_PWM_SetCompare(speed);
					HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
				}
				else if(USARTHMI_DMABUFF[2]==2)//��ת Զ��0��λ��
				{		
					debugcmd=0;		
					if(!limitAflag)//���յ㴦 �����������յ��ƶ� ��ײ
					{
						DirectionB=2;							
						HAL_GPIO_WritePin(GPIOB,GPIO_PIN_13,GPIO_PIN_SET);//AIN2					
						USR_TIM_PWM_SetCompare(speed);
						HAL_TIM_PWM_Start(&htim1,TIM_CHANNEL_1);
					}	
				}				
				else if(USARTHMI_DMABUFF[2]==3)//�����λ��  ��λ
				{
					debugcmd=1;				
					resetstep	=1;		
					findmidflag=0;					
				}
				else if(USARTHMI_DMABUFF[2]==4)//���е�λ��  ��λ
				{
					debugcmd=1;				
					findmidstep	=1;	
					findmidflag=0;					
				}		
				memset((uint8_t*)USARTHMI_DMABUFF,0,dUSARTHMI_DMABuffLen);
				
			}
		}	
		USARTHMIRxCnt=0;
	}
	else
	{
		USARTHMIRxCnt=0;
	}
}
void Debug_ReceiveTask(void)
{
	if(USARTHMIRxTimeOutFlag==0)
	{
		if(USARTHMIRxCnt!=0)
		{
			Debug_DataRxHandler();
		}		
	}
}

void UART_Init(void)
{	
	
	#if dUART1_ON
	UART1_Handler.Instance=USART1;					    //USART1
	UART1_Handler.Init.BaudRate=dUART1_BOUND;				    //������
	UART1_Handler.Init.WordLength=UART_WORDLENGTH_8B;   //�ֳ�Ϊ8λ���ݸ�ʽ
	UART1_Handler.Init.StopBits=UART_STOPBITS_1;	    //һ��ֹͣλ
	UART1_Handler.Init.Parity=UART_PARITY_NONE;		    //����żУ��λ
	UART1_Handler.Init.HwFlowCtl=UART_HWCONTROL_NONE;   //��Ӳ������
	UART1_Handler.Init.Mode=UART_MODE_TX_RX;		    //�շ�ģʽ
	HAL_UART_Init(&UART1_Handler);					    //HAL_UART_Init()��ʹ��UART1
  #endif
	
	#if dUART2_ON	
	UART2_Handler.Instance = USART2;
  UART2_Handler.Init.BaudRate = dUART2_BOUND;
  UART2_Handler.Init.WordLength = UART_WORDLENGTH_8B;
  UART2_Handler.Init.StopBits = UART_STOPBITS_1;
  UART2_Handler.Init.Parity = UART_PARITY_NONE;
  UART2_Handler.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	UART2_Handler.Init.Mode = UART_MODE_TX_RX;
  //UART2_Handler.Init.OverSampling = UART_OVERSAMPLING_16;
  HAL_UART_Init(&UART2_Handler);
  #endif
	
	DMA_NVIC_UART_Init();//DMA�жϳ�ʼ��һ��Ҫ����֮��  �����һ�δ��ڽ��ն�����
 
}

//UART�ײ��ʼ����ʱ��ʹ�ܣ��������ã��ж�����
//�˺����ᱻHAL_UART_Init()����
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
	GPIO_InitTypeDef GPIO_Initure;
	
	if(huart->Instance==USART1)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		__HAL_RCC_GPIOA_CLK_ENABLE();			//ʹ��GPIOAʱ��
		__HAL_RCC_USART1_CLK_ENABLE();			//ʹ��USART1ʱ��
		__HAL_RCC_AFIO_CLK_ENABLE();
	
		GPIO_Initure.Pin=GPIO_PIN_9;			//PA9
		GPIO_Initure.Mode=GPIO_MODE_AF_PP;		//�����������
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
		GPIO_Initure.Speed=GPIO_SPEED_FREQ_HIGH;//����
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA9

		GPIO_Initure.Pin=GPIO_PIN_10;			//PA10
		GPIO_Initure.Mode=GPIO_MODE_AF_INPUT;	//ģʽҪ����Ϊ��������ģʽ��	
		HAL_GPIO_Init(GPIOA,&GPIO_Initure);	   	//��ʼ��PA10
		
		__HAL_UART_ENABLE_IT(&UART1_Handler,UART_IT_TC); 
		__HAL_UART_ENABLE_IT(&UART1_Handler,UART_IT_RXNE);		//���������ж�
		HAL_NVIC_EnableIRQ(USART1_IRQn);				//ʹ��USART1�ж�ͨ��
		HAL_NVIC_SetPriority(USART1_IRQn,0,0);			//��ռ���ȼ�3�������ȼ�3
	 	
		#if 0 
		/* USART1_TX Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
		hdma_usart1_tx.Instance=DMA1_Channel4;                          //ͨ��ѡ��
    hdma_usart1_tx.Init.Direction=DMA_MEMORY_TO_PERIPH;             //�洢��������
    hdma_usart1_tx.Init.PeriphInc=DMA_PINC_DISABLE;                 //���������ģʽ
    hdma_usart1_tx.Init.MemInc=DMA_MINC_ENABLE;                     //�洢������ģʽ
    hdma_usart1_tx.Init.PeriphDataAlignment=DMA_PDATAALIGN_BYTE;    //�������ݳ���:8λ
    hdma_usart1_tx.Init.MemDataAlignment=DMA_MDATAALIGN_BYTE;       //�洢�����ݳ���:8λ
    hdma_usart1_tx.Init.Mode=DMA_NORMAL;                            //������ͨģʽ
    hdma_usart1_tx.Init.Priority=DMA_PRIORITY_MEDIUM;               //�е����ȼ�
		if (HAL_DMA_Init(&hdma_usart1_tx) != HAL_OK)
    {
      Error_Handler();
    }
		__HAL_LINKDMA(huart,hdmatx,hdma_usart1_tx);
		#endif
		
	}
	if(huart->Instance==USART2)//����Ǵ���1�����д���1 MSP��ʼ��
	{
		/* Peripheral clock enable */
    __HAL_RCC_USART2_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
		//__HAL_RCC_AFIO_CLK_ENABLE();
		
		/**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
		GPIO_Initure.Pin = GPIO_PIN_2;
    GPIO_Initure.Mode = GPIO_MODE_AF_PP;
		GPIO_Initure.Pull=GPIO_PULLUP;			//����
    GPIO_Initure.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);

    GPIO_Initure.Pin = GPIO_PIN_3;
    GPIO_Initure.Mode = GPIO_MODE_INPUT;
    GPIO_Initure.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_Initure);
		
		
		/* USART2 interrupt Init */
    HAL_NVIC_SetPriority(USART2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USART2_IRQn);

		/* USART2 DMA Init */
    /* USART2_TX Init */
		__HAL_RCC_DMA1_CLK_ENABLE();
    hdma_usart2_tx.Instance = DMA1_Channel7;
    hdma_usart2_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_usart2_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_usart2_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_usart2_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_usart2_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_usart2_tx.Init.Mode = DMA_NORMAL;
    hdma_usart2_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_usart2_tx) != HAL_OK)
    {
      Error_Handler();
    }
    __HAL_LINKDMA(huart,hdmatx,hdma_usart2_tx);


		
	}
}

/**
* @brief UART MSP De-Initialization
* This function freeze the hardware resources used in this example
* @param huart: UART handle pointer
* @retval None
*/
void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{
	if(huart->Instance==USART1)
  {
    __HAL_RCC_USART1_CLK_DISABLE();
    /**USART1 GPIO Configuration
    PA9     ------> USART1_TX
    PA10     ------> USART1_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9|GPIO_PIN_10);

    /* USART1 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART1 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  if(huart->Instance==USART2)
  {
    __HAL_RCC_USART2_CLK_DISABLE();
    /**USART2 GPIO Configuration
    PA2     ------> USART2_TX
    PA3     ------> USART2_RX
    */
    HAL_GPIO_DeInit(GPIOA, GPIO_PIN_2|GPIO_PIN_3);

    /* USART2 DMA DeInit */
    HAL_DMA_DeInit(huart->hdmatx);

    /* USART2 interrupt DeInit */
    HAL_NVIC_DisableIRQ(USART2_IRQn);

  }
}

void DMA_NVIC_UART_Init(void)
{
	#if dUART1_ON	
	
//	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
//  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
//	

//	__HAL_UART_CLEAR_IDLEFLAG(&UART2_Handler);
//	__HAL_UART_ENABLE_IT(&UART2_Handler, UART_IT_TC);
//	
//	HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);//����1 DMA TX
//  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
	#endif	
	
	#if dUART2_ON	

	HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
	

	__HAL_UART_CLEAR_IDLEFLAG(&UART2_Handler);
	__HAL_UART_ENABLE_IT(&UART2_Handler, UART_IT_TC);
	
	#endif	

}

void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
		Debug_DmaTxIrqHandler();
	}
	else if(huart->Instance == USART2)
	{

	}
}
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	if(huart->Instance == USART1)
	{
	
	}
	else if(huart->Instance == USART2)
	{

	}
}
 
void USART1_IRQHandler(void)                	
{ 
	Debug_RxHandler();
	HAL_UART_IRQHandler(&UART1_Handler);	
} 

void USART2_IRQHandler(void)
{
  HAL_UART_IRQHandler(&UART2_Handler);
}


void DMA1_Channel7_IRQHandler(void)//UART2 TX
{
  HAL_DMA_IRQHandler(&hdma_usart2_tx); 
}


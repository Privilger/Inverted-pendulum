#ifndef __USART_H
#define __USART_H
#include "stdio.h"	
#include "sys.h" 

#define dUART1_ON 		 1		//使能（1）/禁止（0）
#define dUART1_PORT    GPIOA
#define dUART1_TX_PIN  GPIO_PIN_9  //串口1 kooloo add 20181213
#define dUART1_RX_PIN  GPIO_PIN_10

#define dUART2_ON 		0		//使能（1）/禁止（0）
#define dUART2_PORT    GPIOA
#define dUART2_TX_PIN  GPIO_PIN_2  //串口2 
#define dUART2_RX_PIN  GPIO_PIN_3

#define dUART1_BOUND	115200 //115200
#define dUART2_BOUND	115200 //31250


extern UART_HandleTypeDef UART1_Handler,UART2_Handler; 
extern DMA_HandleTypeDef hdma_usart1_tx,hdma_usart1_rx;
extern DMA_HandleTypeDef hdma_usart2_tx,hdma_usart2_rx;


void UART_Init(void);
void DMA_NVIC_UART_Init(void);



#define dDebugDataLenMax 				12
#define dDebugTxBuffLenMax 			100
extern volatile u8 debugTxData[dDebugDataLenMax];					//琴键数据发送数据帧
extern volatile u8 debugTxDataBuff[dDebugDataLenMax];
extern volatile u8 debugTxRing[dDebugTxBuffLenMax];				//琴键数据发送数据缓冲区	
extern volatile u8 debugTxRingBuff[dDebugTxBuffLenMax];

void Debug_SetData(void);	
void Debug_SendData(void);
void Debug_SendTask(void);
void Debug_ReceiveTask(void);




#define dUSARTHMI_TimeOutMax	50
#define dUSARTHMI_DMABuffLen	8
extern volatile uint16_t USARTHMIRxCnt;
extern volatile uint8_t USARTHMIRxTimeOutFlag;
extern volatile uint16_t USARTHMIRxTimeOutCnt;
extern volatile uint8_t USARTHMI_DMABUFF[dUSARTHMI_DMABuffLen];
#endif



#ifndef __ADC_H__
#define __ADC_H__
#include "main.h"

#define dADC1_ON 		 1	
#define dADC2_ON 		 0

extern ADC_HandleTypeDef hadc1;
extern DMA_HandleTypeDef hdma_adc1;

void ADC_Init(void);
void DMA_NVIC_ADC_Init(void);

extern uint16_t UserADCValue[2];
extern uint16_t ADC_DMA_ConvertedValue[10];
#endif

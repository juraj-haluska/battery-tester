#include "stm32l0xx_hal.h"

#define ADCPRIORITY		2
#define TIMPRIORITY		1
#define UARTPRIORITY	3

void HAL_MspInit(void)
{
  __HAL_RCC_SYSCFG_CLK_ENABLE();
  HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(SysTick_IRQn);
}

void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOB_CLK_ENABLE();
	__HAL_RCC_ADC1_CLK_ENABLE();

	GPIO_InitTypeDef      	GPIO_InitStruct;

	//GPIOA
	GPIO_InitStruct.Pin = 	GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
							GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|
							GPIO_PIN_6|GPIO_PIN_7;

	GPIO_InitStruct.Mode = 	GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = 	GPIO_NOPULL;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

	//GPIOB
	GPIO_InitStruct.Pin = 	GPIO_PIN_0|GPIO_PIN_1;

	GPIO_InitStruct.Mode = 	GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = 	GPIO_NOPULL;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

	//GPIOC
	GPIO_InitStruct.Pin = 	GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
							GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;

	GPIO_InitStruct.Mode = 	GPIO_MODE_ANALOG;
	GPIO_InitStruct.Pull = 	GPIO_NOPULL;
	HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

	HAL_NVIC_SetPriority(ADC1_COMP_IRQn, ADCPRIORITY, 0);
	HAL_NVIC_EnableIRQ(ADC1_COMP_IRQn);
}

void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{

  if(hadc->Instance==ADC1)
  {
    __ADC1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOA,  GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
							GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|
							GPIO_PIN_6|GPIO_PIN_7);

    HAL_GPIO_DeInit(GPIOB,  GPIO_PIN_0|GPIO_PIN_1);

    HAL_GPIO_DeInit(GPIOC,  GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|
							GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);

  }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef* hspi)
{

  GPIO_InitTypeDef GPIO_InitStruct;
  if(hspi->Instance==SPI1)
  {
    __SPI1_CLK_ENABLE();
  
    /*
    PB3     ------> SPI1_SCK
    PB4     ------> SPI1_MISO
    PB5     ------> SPI1_MOSI 
    */

    GPIO_InitStruct.Pin = GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  }
  else if(hspi->Instance==SPI2)
  {
    __SPI2_CLK_ENABLE();
  
    /*
    PB13     ------> SPI2_SCK
    PB14     ------> SPI2_MISO
    PB15     ------> SPI2_MOSI 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_13|GPIO_PIN_15|GPIO_PIN_14;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_SPI2;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  }

}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef* hspi)
{

  if(hspi->Instance==SPI1)
  {
    __SPI1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5);
  
  }
  else if(hspi->Instance==SPI2)
  {
    __SPI2_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15);

  }

}

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM21)
  {
	  __TIM21_CLK_ENABLE();
	  HAL_NVIC_SetPriority(TIM21_IRQn, TIMPRIORITY, 0);
	  HAL_NVIC_EnableIRQ(TIM21_IRQn);
  }

}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef* htim_base)
{
  if(htim_base->Instance==TIM21)
   {
     __TIM21_CLK_DISABLE();
     HAL_NVIC_DisableIRQ(TIM21_IRQn);
   }

}

void HAL_UART_MspInit(UART_HandleTypeDef* huart)
{

  GPIO_InitTypeDef GPIO_InitStruct;

  if(huart->Instance==USART1)
  {
    __USART1_CLK_ENABLE();
  
    /*
    PB6     ------> USART1_TX
    PB7     ------> USART1_RX 
    */

    GPIO_InitStruct.Pin = GPIO_PIN_6|GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF0_USART1;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    HAL_NVIC_SetPriority(USART1_IRQn, UARTPRIORITY, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);

  }

}

void HAL_UART_MspDeInit(UART_HandleTypeDef* huart)
{

  if(huart->Instance==USART1)
  {
    __USART1_CLK_DISABLE();

    HAL_GPIO_DeInit(GPIOB, GPIO_PIN_6|GPIO_PIN_7);

    HAL_NVIC_DisableIRQ(USART1_IRQn);

  }

}

#include "stm32l0xx_hal.h"
#include "stm32l0xx.h"
#include "stm32l0xx_it.h"
#include "hw.h"

//extern UART_HandleTypeDef huart1;

/******************************************************************************/
/*            Cortex-M0+ Processor Interruption and Exception Handlers         */ 
/******************************************************************************/

void SysTick_Handler(void)
{
  HAL_IncTick();
  HAL_SYSTICK_IRQHandler();
}

void USART1_IRQHandler(void)
{
  HAL_UART_IRQHandler(&huart1);
}

void TIM21_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&htim21);
}

void ADC1_COMP_IRQHandler(void)
{
  HAL_ADC_IRQHandler(&AdcHandle);
}

void EXTI4_15_IRQHandler(void)
{
	HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_8|GPIO_PIN_9|GPIO_PIN_10|GPIO_PIN_11);
}



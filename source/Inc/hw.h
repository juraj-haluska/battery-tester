//HAL handlers
SPI_HandleTypeDef hspi1;
SPI_HandleTypeDef hspi2;
TIM_HandleTypeDef htim21;
UART_HandleTypeDef huart1;
ADC_HandleTypeDef AdcHandle;

//init functions
void SystemClock_Config(void);
void GPIO_Init(void);
void SPI1_Init(void);
void SPI2_Init(void);
void TIM21_Init(void);
void USART1_UART_Init(void);
void ADC_Init(void);
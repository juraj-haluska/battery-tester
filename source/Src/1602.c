#include "stm32l0xx_hal.h"
#include "1602.h"

GPIO_InitTypeDef DisplayGPIO;


// Send strobe to LCD via E line
void LCD_strobe(void){
	HAL_GPIO_WritePin(E_REG,E_PIN,SET);
	HAL_Delay(4); // Due to datasheet E cycle time is about ~500ns
	HAL_GPIO_WritePin(E_REG,E_PIN,RESET);
	HAL_Delay(4); // Due to datasheet E cycle time is about ~500ns
}

// Send low nibble of cmd to LCD via 4bit bus
void LCD_send_4bit(uint8_t cmd) {
	HAL_GPIO_WritePin(D1_REG,D1_PIN,cmd & (1<<0) ? SET : RESET);
	HAL_GPIO_WritePin(D2_REG,D2_PIN,cmd & (1<<1) ? SET : RESET);
	HAL_GPIO_WritePin(D3_REG,D3_PIN,cmd & (1<<2) ? SET : RESET);
	HAL_GPIO_WritePin(D4_REG,D4_PIN,cmd & (1<<3) ? SET : RESET);
	LCD_strobe();
}

// Send command to LCD via 4bit bus
void LCD_cmd_4bit(uint8_t cmd) {
	HAL_GPIO_WritePin(RS_REG,RS_PIN,RESET);
    LCD_send_4bit(cmd>>4); // send high nibble
    LCD_send_4bit(cmd); // send low nibble
    HAL_Delay(1); // typical command takes about 39us
}

// Send data to LCD via 4bit bus
void LCD_data_4bit(uint8_t data) {
	HAL_GPIO_WritePin(RS_REG,RS_PIN,SET);
    LCD_send_4bit(data>>4);                 // send high nibble
    LCD_send_4bit(data);                    // send low nibble
    HAL_GPIO_WritePin(RS_REG,RS_PIN,RESET);
    HAL_Delay(1);                           // write data to RAM takes about 43us
}

// Set cursor position on LCD
// column : Column position
// line   : Line position
void LCD_GotoXY(int column, int line) {
    LCD_cmd_4bit((column+(line<<6)) | 0x80);  // Set DDRAM address with coordinates
}

void LCD_Init(){
	//inicializacia GPIO
	__GPIOC_CLK_ENABLE();
	DisplayGPIO.Pin = 	GPIO_PIN_8|GPIO_PIN_9|
							GPIO_PIN_14|GPIO_PIN_15;
	DisplayGPIO.Mode = GPIO_MODE_OUTPUT_PP;
	DisplayGPIO.Pull = GPIO_PULLDOWN;
	DisplayGPIO.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOC, &DisplayGPIO);

	__GPIOB_CLK_ENABLE();
	DisplayGPIO.Pin = GPIO_PIN_8|GPIO_PIN_9;
	DisplayGPIO.Mode = GPIO_MODE_OUTPUT_PP;
	DisplayGPIO.Pull = GPIO_PULLDOWN;
	DisplayGPIO.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOB, &DisplayGPIO);

	__GPIOA_CLK_ENABLE();
	DisplayGPIO.Pin = GPIO_PIN_12;
	DisplayGPIO.Mode = GPIO_MODE_OUTPUT_PP;
	DisplayGPIO.Pull = GPIO_PULLDOWN;
	DisplayGPIO.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init(GPIOA, &DisplayGPIO);

	HAL_Delay(30);              // must wait >=30us after LCD Vdd rises to 4.5V
	LCD_send_4bit(0b00000011); // select 4-bit bus (still 8bit)
	HAL_Delay(5);               // must wait more than 4.1ms
	LCD_send_4bit(0b00000011); // select 4-bit bus (still 8bit)
	HAL_Delay(150);             // must wait more than 100us
	LCD_send_4bit(0b00000011); // select 4-bit bus (still 8bit)
	LCD_send_4bit(0b00000010); // Function set: 4-bit bus (gotcha!)

	LCD_cmd_4bit(0x28); // LCD Function: 2 Lines, 5x8 matrix
	LCD_cmd_4bit(0x0C); // Display control: Display: on, cursor: off
	LCD_cmd_4bit(0x06); // Entry mode: increment, shift disabled

	LCD_GotoXY(0,0);
	LCD_Cls();
}

// Clear LCD display and set cursor at first position
void LCD_Cls(void) {
	LCD_cmd_4bit(0x01); // Clear display command
	HAL_Delay(2); // Numb display does it at least 1.53ms
	LCD_cmd_4bit(0x02); // Return Home command
	HAL_Delay(2); // Numb display does it at least 1.53ms
}

// Send string to LCD
void LCD_Print(char *string) {
    while (*string) { LCD_data_4bit(*string++); }
}

// Send integer to LCD
void LCD_PrintI(uint32_t num) {
	char str[11]; // 10 chars max for UINT32_MAX
	int i = 0;
	do { str[i++] = num % 10 + '0'; } while ((num /= 10) > 0);
	for (i--; i>=0; i--) { LCD_data_4bit(str[i]); }
}

// Send HEX integer to LCD
void LCD_PrintH(uint32_t num) {
	char str[11]; // 10 chars max for UINT32_MAX in HEX
	int i = 0;
	do { str[i++] = "0123456789ABCDEF"[num % 0x10]; } while ((num /= 0x10) > 0);
	str[i++] = 'x';
	str[i++] = '0';
	for (i--; i>=0; i--) { LCD_data_4bit(str[i]); }
}

// Send BIN integer to LCD (max 8bit number)
void LCD_PrintB8(uint8_t num) {
	char str[8] = "00000000";
	int i = 0;
	do { str[i++] = num % 2 + '0'; } while ((num /= 2) > 0);
	for (i=7; i>=0; i--) { LCD_data_4bit(str[i]); }
}

// Send BIN integer to LCD (max 16bit number)
void LCD_PrintB16(uint16_t num) {
	char str[16] = "0000000000000000";
	int i = 0;
	do { str[i++] = num % 2 + '0'; } while ((num /= 2) > 0);
	for (i=15; i>=0; i--) { LCD_data_4bit(str[i]); }
}


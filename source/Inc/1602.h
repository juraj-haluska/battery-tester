#define E_PIN GPIO_PIN_15
#define E_REG GPIOC
#define RS_PIN GPIO_PIN_9
#define RS_REG GPIOB

#define D1_PIN GPIO_PIN_8
#define D1_REG GPIOC
#define D2_PIN GPIO_PIN_9
#define D2_REG GPIOC
#define D3_PIN GPIO_PIN_12
#define D3_REG GPIOA
#define D4_PIN GPIO_PIN_8
#define D4_REG GPIOB

void LCD_strobe(void);
void LCD_send_4bit(uint8_t cmd);
void LCD_cmd_4bit(uint8_t cmd);
void LCD_data_4bit(uint8_t data);
void LCD_GotoXY(int column, int line);
void LCD_Init(void);
void LCD_Cls(void);
void LCD_Print(char *string);
void LCD_PrintI(uint32_t num);
void LCD_PrintH(uint32_t num);
void LCD_PrintB8(uint8_t num);
void LCD_PrintB16(uint16_t num);
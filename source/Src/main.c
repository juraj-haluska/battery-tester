#include "stm32l0xx_hal.h"
#include "hw.h"
#include "stdint.h"
#include "stdlib.h"
#include "main.h"
#include "math.h"
#include "string.h"
#include "1602.h"

#define EEPROM_OFFSET 0x08080000

#define CHANNELS 16
#define CALIBCOUNT 20
uint16_t channels[CHANNELS];
uint16_t channels_i = 0;

char printStr[50];
char RxChar;

int count = 0;

uint8_t verbose = 0;

#define CMDSIZE 20
#define CMDCOUNT 10
char command[CMDCOUNT][CMDSIZE];

uint8_t commandNumber = 0;
uint8_t commandIndex = 0;

vetva_t vetvy[4];

#define MAXPROCESSES 20
static process_t FRONT[MAXPROCESSES];

#define MAXCOUNTERS 5
long COUNTERS[MAXCOUNTERS];

void initProcesses(void){
	int i;
	for(i=0;i<MAXPROCESSES;i++){
		FRONT[i].PID = -1;
		FRONT[i].Data = NULL;
	}
}
int addProcess(void * func,void * Data){
	int i;
	for(i = 0; i<MAXPROCESSES;i++){
		if(FRONT[i].PID == -1){
			FRONT[i].Data = Data;
			FRONT[i].func = func;
			FRONT[i].PID = i;
			return FRONT[i].PID;
		}
	}
	return -1;
}
void removeProcess(int PID){
	if(FRONT[PID].PID != -1){
		free(FRONT[PID].Data);
		FRONT[PID].Data = NULL;
		FRONT[PID].func = NULL;
		FRONT[PID].PID = -1;
	}
}

void initCounters(void){
	int i;
	for(i=0;i<MAXCOUNTERS;i++){
		COUNTERS[i] = -1;
	}
}
uint8_t addCounter(void){
	int i;
	for(i=0;i<MAXCOUNTERS;i++){
		if(COUNTERS[i] == -1){
			COUNTERS[i] = 0;
			return i;
		}
	}
	return -1;
}
void removeCounter(uint8_t i){
	COUNTERS[i] = -1;
}

uint8_t GPIO_FLAG = 0;
dialog_s DIALOG = INIT;
uint8_t option = 0;
uint8_t selection[2];

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  ADC_Init();
  GPIO_Init();
  SPI2_Init();
  USART1_UART_Init();
  TIM21_Init();
  LCD_Init();

  initProcesses();
  initCounters();

  HAL_UART_Receive_IT(&huart1,(uint8_t *)&RxChar,sizeof(RxChar));
  HAL_TIM_Base_Start_IT(&htim21);
  HAL_ADC_Start_IT(&AdcHandle);

  vetvyInit();
  LCD_GotoXY(0,0);
  LCD_Print("dobry den");
  sprintf(printStr,"\r\nwelcome!\r\n");
  HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
  help();
  while (1)
  {
  }

}


//******************UNIVERZALNE FUNKCIE**********************
int eepromFloatWrite(float * value, uint8_t addr){
	int retval = 1;
	if(addr >= 0x00 && addr <= 0xFF){
		HAL_FLASHEx_DATAEEPROM_Unlock();
		if(HAL_FLASHEx_DATAEEPROM_Erase(EEPROM_OFFSET + addr*4) != HAL_OK) retval = 0;
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD,EEPROM_OFFSET + addr*4,*(uint32_t *)value) != HAL_OK) retval = 0;
		HAL_FLASHEx_DATAEEPROM_Lock();
	} else retval = 0;
	return retval;
}

float * eepromFloatRead(uint8_t addr){
	return (float *)(EEPROM_OFFSET + addr*4);
}

int eepromIntWrite(int * value, uint8_t addr){
	int retval = 1;
	if(addr >= 0x00 && addr <= 0xFF){
		HAL_FLASHEx_DATAEEPROM_Unlock();
		if(HAL_FLASHEx_DATAEEPROM_Erase(EEPROM_OFFSET + addr*4) != HAL_OK) retval = 0;
		if(HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAMDATA_WORD,EEPROM_OFFSET + addr*4,*(uint32_t *)value) != HAL_OK) retval = 0;
		HAL_FLASHEx_DATAEEPROM_Lock();
	} else retval = 0;
	return retval;
}

int * eepromIntRead(uint8_t addr){
	return (int *)(EEPROM_OFFSET + addr*4);
}

float getTemp(int measured){
	float val = (measured*33000)/(float)(4095-measured);
	double l = log(val/100000);
	float temp = A1 + B1*l + C1*l*l + D1*l*l*l;
	temp = 1/temp - 273.15;
	return temp;
}

void led(led_cmd cmd,uint32_t pin){
	switch(cmd){
		case ON:HAL_GPIO_WritePin(LEDSPORT,pin,SET);break;
		case OFF:HAL_GPIO_WritePin(LEDSPORT,pin,RESET);break;
		case TGLE:HAL_GPIO_TogglePin(LEDSPORT,pin);break;
	}
}

void ErrorHandler(int error){
	switch(error){
		case 0:{
			sprintf(printStr,"Couldn't start process at selected channel! ERRNO = %d\r\n",error);
			LCD_GotoXY(0,1);
			LCD_Print("!used channel");
		} break;
		case 1:{
			sprintf(printStr,"There already is running process! ERRNO = %d\r\n",error);
		} break;
		case 2:{
			sprintf(printStr,"Couldn't kill process! ERRNO = %d\r\n",error);
		} break;
		case 3:{
			sprintf(printStr,"Error while writing to EEPROM! ERRNO = %d\r\n",error);
		} break;
		case 4:{
			sprintf(printStr,"Non existing process! ERRNO = %d\r\n",error);
		} break;
		case 5:{
			sprintf(printStr,"Selected channel is busy or not existing! ERRNO = %d\r\n",error);
		}
		default:{
			sprintf(printStr,"Sorry bro, error occurred!\r\n");
		}
	}
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);;
}

void help(){
	sprintf(printStr,"List of commands:\r\n\t-help\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-charge [channel(1-4)] [current(mA)] [cutoff(mV)] [time(s)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-chargeli [channel(1-4)] [current(mA)] [voltage(mV)] \r\n\t\t[cutoff(mV)] [cutoff(mA)] [verbose(0/1)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-discharge [channel(1-4)] [current(mA)] [cutoff(mV)] \r\n\t\t[verbose(0/1)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-resistance [channel(1-4)] [current1(mA)] [current2(mA)]\r\n\t\t [samples(1-255)] [tolerance(uint)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-getconst [index(0-255)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-setconst [index(0-255)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-calib [channel(1-4)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-ps [PID(0-19)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-kill [channel(1-4)]\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	sprintf(printStr,"\t-reset\r\n");
	HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
}

void addAlert(){
	DataAlert * DataA = (DataAlert *) malloc(sizeof(DataAlert));
	DataA->i=0;
	DataA->counter=-1;
	DataA->count=3;
	addProcess(&alert,DataA);
}
//*****************FUNKCIE PRE VETVY****************************
void vetvyInit(void){
	vetvy[0].vetvaOut.csPort = GPIOB;
	vetvy[0].vetvaOut.csPin = GPIO_PIN_2;
	vetvy[0].ID = 1;
	vetvy[0].state = IDLE;
	vetvy[1].vetvaOut.csPort = GPIOB;
	vetvy[1].vetvaOut.csPin = GPIO_PIN_10;
	vetvy[1].ID = 2;
	vetvy[1].state = IDLE;
	vetvy[2].vetvaOut.csPort = GPIOB;
	vetvy[2].vetvaOut.csPin = GPIO_PIN_11;
	vetvy[2].ID = 3;
	vetvy[2].state = IDLE;
	vetvy[3].vetvaOut.csPort = GPIOB;
	vetvy[3].vetvaOut.csPin = GPIO_PIN_12;
	vetvy[3].ID = 4;
	vetvy[3].state = IDLE;

	//plne vypnutie oboch tranzistorov na vsetkych vetvach
	int i,x;
	for(x=0;x<=2;x++){			//pre istotu viac krat
		for (i=0;i<=3;i++){
			vetvaOff(&vetvy[i]);
		}
	}
	//kalibracia prudovych senzorov
	int avg[4] = {0, 0, 0, 0};
	for(i = 0; i< CALIBCOUNT;i++){
		HAL_Delay(10);
		avg[0] += channels[11];
		avg[1] += channels[2];
		avg[2] += channels[5];
		avg[3] += channels[9];
		while(!ADC_FLAG_EOS);
	}
	vetvy[0].cCalibration = avg[0]/CALIBCOUNT;
	vetvy[1].cCalibration = avg[1]/CALIBCOUNT;
	vetvy[2].cCalibration = avg[2]/CALIBCOUNT;
	vetvy[3].cCalibration = avg[3]/CALIBCOUNT;
}

void vetvyUpdate(void){
	if(vetvy[0].state != DEACTIVE){
		vetvy[0].actVoltage = round(channels[10]*(float)*eepromFloatRead(0));
		vetvy[0].actCurrent = round((channels[11] - vetvy[0].cCalibration)**eepromFloatRead(1));
		vetvy[0].actTempHeat = getTemp(channels[12]);
		vetvy[0].actTempBatt = getTemp(channels[13]);
	}

	if(vetvy[1].state != DEACTIVE){
		vetvy[1].actVoltage = round(channels[0]*(float)*eepromFloatRead(4));
		vetvy[1].actCurrent = round((channels[2] - vetvy[1].cCalibration)**eepromFloatRead(5));
		vetvy[1].actTempHeat = getTemp(channels[3]);
		vetvy[1].actTempBatt = getTemp(channels[1]);
	}

	if(vetvy[2].state != DEACTIVE){
		vetvy[2].actVoltage = round(channels[6]*(float)*eepromFloatRead(8));
		vetvy[2].actCurrent = round((channels[5] - vetvy[2].cCalibration)**eepromFloatRead(9));
		vetvy[2].actTempHeat = getTemp(channels[4]);
		vetvy[2].actTempBatt = getTemp(channels[7]);
	}

	if(vetvy[3].state != DEACTIVE){
		vetvy[3].actVoltage = round(channels[8]*(float)*eepromFloatRead(12));;
		vetvy[3].actCurrent = round((channels[9] - vetvy[3].cCalibration)**eepromFloatRead(13));
		vetvy[3].actTempHeat = getTemp(channels[14]);
		vetvy[3].actTempBatt = getTemp(channels[15]);
	}
}

void vetvaSet(vetva_t * vetva, int setpoint){
	if(vetva->state == ACTIVE){
		uint16_t data;
		uint8_t out;
		if(vetva->vetvaOut.vetvaOutState == OUTA){
			out = 0;
		}
		else if(vetva->vetvaOut.vetvaOutState == OUTB){
			out = 1;
		} else {
			out = 0b11;
		}
		uint16_t setup = (out << 2 | 0b01) << 12;
		data = (setup)|(setpoint & 0xFFF);

		HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,0);
		HAL_SPI_Transmit(&hspi2,(uint8_t *)&data,sizeof(data),10);
		HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,1);
	}
}

void vetvaOff(vetva_t * vetva){
	uint16_t data = ((0 << 2 | 0b01) << 12)|(4095 & 0xFFF);
	HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,0);
	HAL_SPI_Transmit(&hspi2,(uint8_t *)&data,sizeof(data),10);
	HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,1);
	data = ((1 << 2 | 0b01) << 12)|(0 & 0xFFF);
	HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,0);
	HAL_SPI_Transmit(&hspi2,(uint8_t *)&data,sizeof(data),10);
	HAL_GPIO_WritePin(vetva->vetvaOut.csPort,vetva->vetvaOut.csPin,1);
}

//*******************INTERRUPTS CALLBACKS******************
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* AdcHandle)
{
	channels[channels_i++] = HAL_ADC_GetValue(AdcHandle);
	if (channels_i >= CHANNELS) {
		//conversion complete
		//aktualizuje namerane hodnoty v strukturach
		vetvyUpdate();
		int i;
		for(i=0;i<MAXPROCESSES;i++){
			if(FRONT[i].PID != -1){
				void (*func)(int PID) = FRONT[i].func;
				func(FRONT[i].PID);
			}
		}
		//display a tlacitka
		if(GPIO_FLAG)dialog();

		led(TGLE,LED2);
		channels_i = 0;
		HAL_ADC_Start_IT(AdcHandle);
	}
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim21){	//kazda sekunda
	int i;
	for(i=0;i<MAXCOUNTERS;i++){
		if(COUNTERS[i] != -1){
			COUNTERS[i]++;
		}
	}
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef* huart)
{
	HAL_UART_Transmit(&huart1,(uint8_t *)&RxChar,sizeof(RxChar),100);
	if(commandIndex < CMDSIZE - 1 && RxChar != '\r'){
		if (RxChar == 0x20 && commandNumber < CMDCOUNT){
			command[commandNumber][commandIndex] = '\0';
			commandIndex = 0;
			commandNumber++;
		} else {
			command[commandNumber][commandIndex++] = RxChar;
		}
	} else {
		addProcess(&uart,NULL);
	}
	HAL_UART_Receive_IT(&huart1,(uint8_t *)&RxChar,sizeof(RxChar));
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin){
	switch(~((GPIOA->IDR & GPIO_Pin) | ~GPIO_Pin)){
		case GPIO_PIN_8:{
			GPIO_FLAG |= (1 << 0);
		}break;
		case GPIO_PIN_9:{
			GPIO_FLAG |= (1 << 1);
		}break;
		case GPIO_PIN_10:{
			GPIO_FLAG |= (1 << 2);
		}break;
		case GPIO_PIN_11:{
			GPIO_FLAG |= (1 << 3);
		}break;
	}
}

//**************PROCESY*****************************

void discharge(int PID){
	DataDischarge * Data = (DataDischarge *)FRONT[PID].Data;

	switch(Data->state){
	case START:{
		if(verbose & (1 << (Data->vetva->ID - 1))){
			sprintf(printStr,"ID:%d[Time;Voltage;Current;T1;T2]\r\n",Data->vetva->ID);
			HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		}
		Data->set = 0;
		Data->vetva->state = ACTIVE;
		Data->vetva->vetvaOut.vetvaOutState = OUTB;
		Data->state = REGULATE;
		Data->counter = addCounter();
		Data->flag = 0;
	}
	case REGULATE:{
		if(verbose & (1 << (Data->vetva->ID - 1))){
			uint8_t mod = COUNTERS[Data->counter] % 2;
			if(Data->flag  != mod){
				sprintf(printStr,"ID:%d[%d;%d;%d;%f;%f]\r\n",Data->vetva->ID,(int)COUNTERS[Data->counter],Data->vetva->actVoltage,Data->vetva->actCurrent,Data->vetva->actTempBatt,Data->vetva->actTempHeat);
				HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
				Data->flag = mod;
			}
		}
		if(Data->vetva->actVoltage <= Data->cutoff){
			Data->state = STOP;
		}
		else if(Data->vetva->actTempBatt >= MAXTEMPLI || Data->vetva->actTempHeat >= MAXTEMPTR){
			sprintf(printStr,"ERROR! OVERTEMPERATURE. STOPPING PROCESS\r\n");
			HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
			Data->state = STOP;
		}
		else {
			int rate, error = Data->target + Data->vetva->actCurrent;

			if(error >= 100 || error <= -100){
				rate = 10;
			} else {
				rate = 1;
			}

			if(error > 0 + Data->tolerance && Data->set < 4095){
				Data->set += rate;
			}
			if(error < 0 - Data->tolerance && Data->set > 0){
				Data->set -= rate;
			}

			vetvaSet(Data->vetva,Data->set);
		}
	}break;
	case STOP:{
		int capacity = Data->target * COUNTERS[Data->counter] / 3600;
		sprintf(printStr,"capacity: %d,t = %d s\r\n", capacity,(int)COUNTERS[Data->counter]);
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		LCD_GotoXY(0,1);
		LCD_Print(printStr);
		removeCounter(Data->counter);
		Data->vetva->state = IDLE;
		vetvaOff(Data->vetva);
		if(verbose & (1 << (Data->vetva->ID - 1)))verbose &= ~(1 << (Data->vetva->ID -1));
		removeProcess(PID);
		addAlert();
	} break;
	}
}

void resistance(int PID){
	DataResistance * Data = (DataResistance *)FRONT[PID].Data;

	switch(Data->state){
	case START:{
		Data->set = 0;
		Data->vetva->state = ACTIVE;
		Data->vetva->vetvaOut.vetvaOutState = OUTB;
		Data->phase = 0;
		Data->samplesI = 0;
		Data->v[0] = 0;
		Data->v[1] = 0;
		//Data->samples = 20;
		Data->state = REGULATE;
	} break;
	case REGULATE:{
		int rate, error = Data->target[Data->phase] + Data->vetva->actCurrent;

		if(abs(error) <= Data->tolerance){
			Data->v[Data->phase] += Data->vetva->actVoltage;
			Data->samplesI++;
			if(Data->samplesI >= Data->samples){
				if(Data->phase == 0){
					Data->phase++;
					Data->samplesI = 0;
				}
				else Data->state = STOP;
			}
			return;
		}

		if(error >= 100 || error <= -100){
			rate = 10;
		} else {
			rate = 1;
		}

		if(error > 0 && Data->set < 4095){
			Data->set += rate;
		}
		if(error < 0  && Data->set > 0){
			Data->set -= rate;
		}

		vetvaSet(Data->vetva,Data->set);
	} break;
	case STOP:{
		float va = Data->v[1]/(float)Data->samples;
		float vb = Data->v[0]/(float)Data->samples;
		float resistance = fabsf((va-vb)/(Data->target[1]-Data->target[0]));
		sprintf(printStr,"Ri = %f ohm\r\n", resistance);
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		LCD_GotoXY(0,1);
		LCD_Print(printStr);
		Data->vetva->state = IDLE;
		vetvaOff(Data->vetva);
		removeProcess(PID);
		addAlert();
	} break;
	}

}

void chargeLithium(int PID){
	DataChargeLithium * Data = (DataChargeLithium *)FRONT[PID].Data;
	vetva_t * vetva = Data->vetva;

	switch(Data->state){
	case START:{
		if(verbose & (1 << (Data->vetva->ID - 1))){
			sprintf(printStr,"ID:%d[Time;Voltage;Current;T1;T2]\r\n",Data->vetva->ID);
			HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		}
		Data->set = 4095;
		Data->phase = 0;
		vetva->vetvaOut.vetvaOutState = OUTA;
		vetva->state = ACTIVE;
		Data->state = REGULATE;
		Data->counter = addCounter();
	} break;
	case REGULATE:{
		if(verbose & (1 << (Data->vetva->ID - 1))){
			uint8_t mod = COUNTERS[Data->counter] % 2;
			if(Data->flag  != mod){
				sprintf(printStr,"ID:%d[%d;%d;%d;%f;%f]\r\n",Data->vetva->ID,(int)COUNTERS[Data->counter],Data->vetva->actVoltage,Data->vetva->actCurrent,Data->vetva->actTempBatt,Data->vetva->actTempHeat);
				HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
				Data->flag = mod;
			}
		}
		if(Data->phase==0?(vetva->actVoltage >= Data->cutoff[Data->phase]):(abs(vetva->actCurrent) <= Data->cutoff[Data->phase])){
			if(Data->phase == 0){
				Data->phase++;
			} else {
				Data->state = STOP;
			}
		} else {
			int rate, error = Data->target[Data->phase] - (Data->phase==0?vetva->actCurrent:vetva->actVoltage);
			if(error >= 100 || error <= -100){
				rate = 10;
			} else {
				rate = 1;
			}

			if((error > (0 + Data->tolerance) ) && Data->set > 0){
				Data->set -= rate;
			}
			if((error < (0  - Data->tolerance) ) && Data->set < 4905){
				Data->set += rate;
			}
			vetvaSet(vetva,Data->set);
		}
	} break;
	case STOP:{
		sprintf(printStr,"charging done, time = %d s!\r\n",(int)COUNTERS[Data->counter]);
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		removeCounter(Data->counter);
		vetva->state = IDLE;
		vetvaOff(vetva);
		if(verbose & (1 << (Data->vetva->ID - 1)))verbose &= ~(1 << (Data->vetva->ID -1));
		removeProcess(PID);
		addAlert();
	} break;
	}
}

void charge(int PID){
	DataCharge * Data = (DataCharge *)FRONT[PID].Data;
	vetva_t * vetva = Data->vetva;

	switch(Data->state){
	case START:{
		Data->set = 4095;
		vetva->vetvaOut.vetvaOutState = OUTA;
		vetva->state = ACTIVE;
		Data->state = REGULATE;
		Data->counter = addCounter();
	} break;
	case REGULATE:{
		if(COUNTERS[Data->counter] >= Data->limit || vetva->actVoltage >= Data->cutoff){
			Data->state = STOP;
		} else {
			int rate, error = Data->target - vetva->actCurrent;
			if(error >= 100 || error <= -100){
				rate = 10;
			} else {
				rate = 1;
			}

			if((error > (0 + Data->tolerance) ) && Data->set > 0){
				Data->set -= rate;
			}
			if((error < (0  - Data->tolerance) ) && Data->set < 4905){
				Data->set += rate;
			}
			vetvaSet(vetva,Data->set);
		}
	} break;
	case STOP:{
		sprintf(printStr,"charging done, time = %d s!\r\n",(int)COUNTERS[Data->counter]);
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		removeCounter(Data->counter);
		vetva->state = IDLE;
		vetvaOff(vetva);
		removeProcess(PID);
		addAlert();
	} break;
	}
}

void uart(int PID){					//PROCES SPRACUVAJUCI PRIKAZY Z UART
	command[commandNumber][commandIndex] = '\0';
	commandIndex = 0;
	commandNumber = 0;
	HAL_UART_Transmit(&huart1,(uint8_t *)"\r\n",2,1000);

	if(strcmp(command[0],"help") == 0){
		help();
	}
	else if(strcmp(command[0],"discharge") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
			DataDischarge * Data = (DataDischarge *) malloc(sizeof(DataDischarge));
			Data->vetva = &vetvy[v - 1];
			Data->target = atoi(command[2]);
			Data->cutoff = atoi(command[3]);
			Data->tolerance = 0;
			Data->state = START;
			if(atoi(command[4]))verbose |= (1 << (v-1));
			addProcess(&discharge,Data);
		} else {
			ErrorHandler(0);
		}
	}
	else if(strcmp(command[0],"charge") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
			DataCharge * Data = (DataCharge *) malloc(sizeof(DataCharge));
			Data->vetva = &vetvy[v - 1];
			Data->target = atoi(command[2]);
			Data->cutoff = atoi(command[3]);
			Data->limit = atoi(command[4]);
			Data->tolerance = 0;
			Data->state = START;
			addProcess(&charge,Data);
		} else {
			ErrorHandler(0);
		}
	}
	else if(strcmp(command[0],"chargeli") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4){
			DataChargeLithium * Data = (DataChargeLithium *) malloc(sizeof(DataChargeLithium));
			Data->vetva = &vetvy[v - 1];
			Data->target[0] = atoi(command[2]);
			Data->target[1] = atoi(command[3]);
			Data->cutoff[0] = atoi(command[4]);
			Data->cutoff[1] = atoi(command[5]);
			Data->tolerance = 0;
			Data->state = START;
			if(atoi(command[6]))verbose |= (1 << (v-1));
			addProcess(&chargeLithium,Data);
		} else {
			ErrorHandler(0);
		}
	}
	else if(strcmp(command[0],"resistance") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
			DataResistance * Data = (DataResistance *) malloc(sizeof(DataResistance));
			Data->vetva = &vetvy[v - 1];
			Data->target[0] = atoi(command[2]);
			Data->target[1] = atoi(command[3]);
			Data->samples = atoi(command[4]);
			Data->tolerance = atoi(command[5]);
			Data->state = START;
			addProcess(&resistance,Data);
		} else {
			ErrorHandler(0);
		}
	}
	else if(strcmp(command[0],"setconst") == 0){
		uint8_t addr = atoi(command[1]);
		float val = atoff(command[2]);
		if(!eepromFloatWrite(&val,addr)){
			ErrorHandler(3);
		}
	}
	else if(strcmp(command[0],"setconsti") == 0){
		uint8_t addr = atoi(command[1]);
		int val = atoi(command[2]);
		if(!eepromIntWrite(&val,addr)){
			ErrorHandler(3);
		}
	}
	else if(strcmp(command[0],"getconst") == 0){
		uint8_t addr = atoi(command[1]);
		sprintf(printStr,"constant @ %d: %f\r\n",addr,*eepromFloatRead(addr));
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	}
	else if(strcmp(command[0],"getconsti") == 0){
		uint8_t addr = atoi(command[1]);
		sprintf(printStr,"constant @ %d: %d\r\n",addr,*eepromIntRead(addr));
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	}
	else if(strcmp(command[0],"calib") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
			sprintf(printStr,"v:\t %dmV\r\nc:\t %dmA\r\n",vetvy[v - 1].actVoltage,vetvy[v - 1].actCurrent);
			HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		} else {
			ErrorHandler(5);
		}
	}
	else if(strcmp(command[0],"gettemp") == 0){
		int v = atoi(command[1]);
		if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
			sprintf(printStr,"t1:\t%f\r\nt2:\t%f\r\n",getTemp(vetvy[v - 1].actTempBatt),getTemp(vetvy[v - 1].actTempHeat));
			HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
		} else {
			ErrorHandler(5);
		}
	}
	else if(strcmp(command[0],"ps") == 0){
		int i;
		for(i = 0; i<MAXPROCESSES;i++){
			if(FRONT[i].PID != -1){
				sprintf(printStr,"PID:\t%d\r\n",FRONT[i].PID);
				HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
			}
		}
	}
	else if(strcmp(command[0],"kill") == 0){
		int v = atoi(command[1]);
		if(v >= 0 && v < MAXPROCESSES && FRONT[v].PID != -1){
			removeProcess(v);
			vetvyInit();
		} else {
			ErrorHandler(4);
		}
	}
	else if(strcmp(command[0],"reset") == 0){
		HAL_NVIC_SystemReset();
	}
	else {
		sprintf(printStr,"unknow command\r\n");
		HAL_UART_Transmit(&huart1,(uint8_t *)printStr,strlen(printStr),1000);
	}
	removeProcess(PID);
}

void alert(int PID){
	DataAlert * Data = FRONT[PID].Data;
	if(Data->counter == -1){
		Data->counter = addCounter();
		Data->i = COUNTERS[Data->counter];
	} else {
		if(Data->i!=COUNTERS[Data->counter]){
			Data->i=COUNTERS[Data->counter];
			HAL_GPIO_TogglePin(GPIOA,GPIO_PIN_15);
			if(Data->i==Data->count*2) removeProcess(PID);
		}
	}
}

void dialog(void){
	switch(DIALOG){
	case INIT:{
		LCD_Cls();
		LCD_GotoXY(0,0);
		LCD_Print("action:");
		DIALOG = SELECT;
		option = 0;
		GPIO_FLAG = 0;
	}
	case SELECT:{
		if(GPIO_FLAG & 1){
			if(option < 3)option++;
		} else if(GPIO_FLAG & 2){
			if(option > 0)option--;
		} else if(GPIO_FLAG & 4){
			LCD_GotoXY(0,0);
			LCD_Print("channel:");
			selection[0]=option;
			option = 0;
			GPIO_FLAG = 0x10;
			DIALOG = CHANNEL;
			break;
		} else if(GPIO_FLAG & 8){
			DIALOG = INIT;
			GPIO_FLAG = 0x10;
			break;
		}
		GPIO_FLAG = 0;
		LCD_GotoXY(0,1);
		switch(option){
			case 0:{LCD_Print("resistance      ");}break;
			case 1:{LCD_Print("capacity        ");}break;
			case 2:{LCD_Print("charge          ");}break;
			case 3:{LCD_Print("charge li       ");}break;
		}
	} break;
	case CHANNEL:{
		if(GPIO_FLAG & 1){
			if(option < 3)option++;
		} else if(GPIO_FLAG & 2){
			if(option > 0)option--;
		} else if(GPIO_FLAG & 4){
			LCD_GotoXY(0,0);
			LCD_Print("start:  ");
			selection[1]=option;
			option = 0;
			GPIO_FLAG = 0x10;
			DIALOG = BEGIN;
			break;
		} else if(GPIO_FLAG & 8){
			DIALOG = INIT;
			GPIO_FLAG = 0x10;
			break;
		}
		GPIO_FLAG = 0;
		LCD_GotoXY(0,1);
		switch(option){
			case 0:{LCD_Print("channel 1 ");}break;
			case 1:{LCD_Print("channel 2 ");}break;
			case 2:{LCD_Print("channel 3 ");}break;
			case 3:{LCD_Print("channel 4 ");}break;
		}
	} break;
	case BEGIN:{
		if(GPIO_FLAG & 1){
			if(option < 3)option++;
		} else if(GPIO_FLAG & 2){
			if(option > 0)option--;
		} else if(GPIO_FLAG & 4){
			int v = selection[1]+1;
			switch(selection[0]){
			case 0:{	//pridaj do fronty meranie vnutorneho odporu
				if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
					DataResistance * Data = (DataResistance *) malloc(sizeof(DataResistance));
					Data->vetva = &vetvy[v - 1];
					Data->target[0] = *eepromIntRead(16);
					Data->target[1] = *eepromIntRead(17);
					Data->samples = *eepromIntRead(18);
					Data->tolerance = *eepromIntRead(19);
					Data->state = START;
					addProcess(&resistance,Data);
				} else {
					ErrorHandler(0);
				}
			} break;
			case 1:{
				if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
					DataDischarge * Data = (DataDischarge *) malloc(sizeof(DataDischarge));
					Data->vetva = &vetvy[v - 1];
					Data->target = *eepromIntRead(20);
					Data->cutoff = *eepromIntRead(21);
					Data->tolerance = 0;
					Data->state = START;
					if(atoi(command[4]))verbose |= (1 << (v-1));
					addProcess(&discharge,Data);
				} else {
					ErrorHandler(0);
				}
			} break;
			case 2:{
				if(v >= 1 && v <=4 && vetvy[v - 1].state == IDLE){
					DataCharge * Data = (DataCharge *) malloc(sizeof(DataCharge));
					Data->vetva = &vetvy[v - 1];
					Data->target = *eepromIntRead(22);
					Data->cutoff = *eepromIntRead(23);
					Data->limit = *eepromIntRead(24);
					Data->tolerance = 0;
					Data->state = START;
					addProcess(&charge,Data);
				} else {
					ErrorHandler(0);
				}
			} break;
			case 3:{
				if(v >= 1 && v <=4){
					DataChargeLithium * Data = (DataChargeLithium *) malloc(sizeof(DataChargeLithium));
					Data->vetva = &vetvy[v - 1];
					Data->target[0] = *eepromIntRead(25);
					Data->target[1] = *eepromIntRead(26);
					Data->cutoff[0] = *eepromIntRead(27);
					Data->cutoff[1] = *eepromIntRead(28);
					Data->tolerance = 0;
					Data->state = START;
					if(atoi(command[6]))verbose |= (1 << (v-1));
					addProcess(&chargeLithium,Data);
				} else {
					ErrorHandler(0);
				}
			} break;
			}
			DIALOG = INIT;
			GPIO_FLAG = 0x10;
			break;
		} else if(GPIO_FLAG & 8){
			DIALOG = INIT;
			GPIO_FLAG = 0x10;
			break;
		}
		GPIO_FLAG = 0;
	} break;
	}
}

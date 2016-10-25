//thermistors
#define A1 3.354016E-03
#define B1 2.460382E-04
#define C1 3.405377E-06
#define D1 1.034240E-07

#define MAXTEMPTR 130
#define MAXTEMPLI 60


//*************************************leds

#define LED1		GPIO_PIN_12
#define LED2		GPIO_PIN_11
#define LED3		GPIO_PIN_10
#define LEDSPORT	GPIOC

typedef enum {
	ON,
	OFF,
	TGLE
} led_cmd;

//*************************************vetvy

typedef enum {
	IDLE,
	DEACTIVE,
	ACTIVE
} vetvaState_t;

typedef enum {
	OUTA,
	OUTB
} vetvaOutState_t;

typedef struct {
	vetvaOutState_t vetvaOutState;
	GPIO_TypeDef * csPort;	//CS port dac prevodniku
	uint16_t csPin;			//CS pin dac prevodniku
} vetvaOut_t;

typedef struct
{
	uint8_t ID;				//identifikator vetvy 1 - 4
	vetvaState_t state;		//stav vetvy(aby procesy vedeli ci je volna atd..)
	vetvaOut_t vetvaOut;

	int actVoltage;			//aktualne napatie v mV
	int actCurrent;			//aktualny prud v mA
	float actTempHeat;		//aktualna teplota chladica
	float actTempBatt;		//aktualna teplota akumulatora

	int cCalibration;		//kalibracia prudoveho senzora, pocita sa pri inicializaci vetvy
} vetva_t;

//*************************************stavy
typedef enum{
	START,
	REGULATE,
	STOP
} states;
//************************************stavy dialogu

typedef enum{
	INIT,
	SELECT,
	CHANNEL,
	BEGIN
} dialog_s;

//*************************************datove struktury pre jednotlive procesy
typedef struct {
	uint8_t count;
	uint8_t i;
	int counter;
} DataAlert;

typedef struct {
	vetva_t * vetva;
	uint8_t counter;
	states state;
} DataUni;

typedef struct {
	vetva_t * vetva;
	int target;
	int cutoff;
	int tolerance;
	int set;
	states state;
	uint8_t counter;
	uint8_t flag;
} DataDischarge;

typedef struct {
	int set;
	int target[2];
	int v[2];
	int tolerance;
	uint8_t phase;
	uint8_t samples;
	uint8_t samplesI;
	vetva_t * vetva;
	states state;
} DataResistance;


typedef struct {
	int set;
	int tolerance;
	int target[2]; //0=current, 1=voltage
	int cutoff[2]; //0=voltage, 1=current
	uint8_t phase;
	vetva_t * vetva;
	states state;
	uint8_t counter;
	uint8_t flag;
} DataChargeLithium;

typedef struct {
	int set;
	int tolerance;
	int target;
	int cutoff;
	int limit;
	vetva_t * vetva;
	states state;
	uint8_t counter;
} DataCharge;

//************************************struktura procesu
typedef struct{
	void (*func)(int PID);
	void * Data;
	int PID;
} process_t;

void vetvyInit(void);
void vetvyUpdate(void);
void vetvaOff(vetva_t * vetva);
void vetvaSet(vetva_t * vetva, int setpoint);

int eepromFloatWrite(float * value, uint8_t addr);
float * eepromFloatRead(uint8_t addr);
float getTemp(int measured);
void led(led_cmd cmd,uint32_t pin);
void ErrorHandler(int error);
void help();
void addAlert();

void discharge(int PID);
void resistance(int PID);
void chargeLithium(int PID);
void charge(int PID);
void uart(int PID);
void alert(int PID);

void dialog(void);


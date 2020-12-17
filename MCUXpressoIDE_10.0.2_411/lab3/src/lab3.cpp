/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */


#include "board.h"
#include "ITM_write.h"
#include <cr_section_macros.h>
#include <atomic>
#include <string>
#include <cstdio>
#define TICKRATE_HZ1 (1000)	/* 1000 ticks  per second */
/* I2CM transfer record */
static I2CM_XFER_T  i2cmXferRec;
/* I2C clock is set to 1.8MHz */
#define I2C_CLK_DIVIDER         (40)
/* 50KHz I2C bit-rate */
#define I2C_BITRATE         (50000)
/* Standard I2C mode */
#define I2C_MODE    (0)
#define I2C_TEMP_ADDR_7BIT  (0x48)

#define TICKRATE_HZ         (1000)

static volatile int state;
/* Current state for LED control via I2C cases */
static volatile std::atomic_int counter;
extern "C"
{
void SysTick_Handler(void)
{
	static int ticks = 0;
	if(counter>0){
		counter--;
	}
	ticks++;
	if (ticks > TICKRATE_HZ) {
		ticks = 0;
		state = 1 - state;
	}
}
}

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}


class DigitalIOPin {
public:
	DigitalIOPin(int prt, int pn, bool inp = true, bool pllup = true, bool inv = false):port(prt),pin(pn),invert(inv) {
		unsigned int setting = 0;
		if(inp){
			setting |= (pllup ? IOCON_MODE_PULLUP : IOCON_MODE_PULLDOWN) | IOCON_DIGMODE_EN | (inv ? IOCON_INV_EN : 0);
			Chip_IOCON_PinMuxSet(LPC_IOCON, prt, pn, setting);
			Chip_GPIO_SetPinDIRInput(LPC_GPIO, prt, pn);
		}
		else {
			setting |= IOCON_MODE_INACT | IOCON_DIGMODE_EN;
			Chip_IOCON_PinMuxSet(LPC_IOCON, prt, pn, setting);
			Chip_GPIO_SetPinDIROutput(LPC_GPIO, prt, pn);
			Chip_GPIO_SetPinState(LPC_GPIO, prt, pn, inv ? true : false);
		}
	}
	bool Read() {
		return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
	}
	void Write(bool value){
		if(Chip_GPIO_GetPinDIR(LPC_GPIO, port, pin)) {	//if pin is output
			if(invert){
				Chip_GPIO_SetPinState(LPC_GPIO, port, pin, (!value));
				state = !value;
			}
			else{
				Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
				state = value;
			}
		}
	}
private:
	int port;
	int pin;
	bool invert;
};


/* Initializes pin muxing for I2C interface - note that SystemInit() may
   already setup your pin muxing at system startup */
static void Init_I2C_PinMux(void)
{
#if defined(BOARD_KEIL_MCB1500)||defined(BOARD_NXP_LPCXPRESSO_1549)
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 22, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 23, IOCON_DIGMODE_EN | I2C_MODE);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SCL);
	Chip_SWM_EnableFixedPin(SWM_FIXED_I2C0_SDA);
#else
#error "No I2C Pin Muxing defined for this example"
#endif
}

/* Setup I2C handle and parameters */
static void setupI2CMaster()
{
	/* Enable I2C clock and reset I2C peripheral - the boot ROM does not
	   do this */
	Chip_I2C_Init(LPC_I2C0);

	/* Setup clock rate for I2C */
	Chip_I2C_SetClockDiv(LPC_I2C0, I2C_CLK_DIVIDER);

	/* Setup I2CM transfer rate */
	Chip_I2CM_SetBusSpeed(LPC_I2C0, I2C_BITRATE);

	/* Enable Master Mode */
	Chip_I2CM_Enable(LPC_I2C0);
}

static void SetupXferRecAndExecute(uint8_t devAddr,
		uint8_t *txBuffPtr,
		uint16_t txSize,
		uint8_t *rxBuffPtr,
		uint16_t rxSize)
{
	/* Setup I2C transfer record */
	i2cmXferRec.slaveAddr = devAddr;
	i2cmXferRec.status = 0;
	i2cmXferRec.txSz = txSize;
	i2cmXferRec.rxSz = rxSize;
	i2cmXferRec.txBuff = txBuffPtr;
	i2cmXferRec.rxBuff = rxBuffPtr;

	Chip_I2CM_XferBlocking(LPC_I2C0, &i2cmXferRec);
}

/* Function to read TC74 I2C temperature sensor and output result */
static void ReadTemperatureI2CM()
{
	uint8_t temperature;
	uint8_t status = 0x01;
	uint8_t temp = 0x00;
	char hex[2];
	char message[100];

	SetupXferRecAndExecute(I2C_TEMP_ADDR_7BIT, &status, 1, &temperature, 1);		/* Read TC74 temperature sensor */
	if(temperature == 0x40){
		Sleep(1);
		SetupXferRecAndExecute(I2C_TEMP_ADDR_7BIT, &temp, 1, &temperature, 1);
		hex[0] = temperature & 0b00001111;
		hex[1] = temperature >> 4;
		if(hex[0] >= 10)
			hex[0] += 55;
		else hex[0] += '0';
		if(hex[1] >= 10)
			hex[1] += 55;
		else hex[1] += '0';

		if (i2cmXferRec.status == I2CM_STATUS_OK) {			//I2CM_STATUS_OK	/* Test for valid operation */
			if(temperature & 0b10000000){				//if negative		// Output temperature.
				temperature = ~temperature;
				temperature += 1;
				snprintf(message, 100,"I2C status: OK\nDev status: OK\nTemp. : -%d, 0x%c%c degr. C\n",
						(int) temperature, hex[1], hex[0]);
				ITM_write(message);
			}
			else{
				snprintf(message, 100,"I2C status: OK\nDev status: OK\nTemp. : %d, 0x%c%c degr. C\n",
						(int) temperature, hex[1], hex[0]);
				ITM_write(message);
			}
		}
		else {
			snprintf(message, 100, "Error %d reading temperature.\r\n", i2cmXferRec.status);
			ITM_write(message);
		}
	}

}


int main(void) {
	uint32_t sysTickRate;
	SystemCoreClockUpdate();
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ);
	Board_Init();

	Init_I2C_PinMux();							/* Setup I2C pin muxing */
	setupI2CMaster();							/* Allocate I2C handle, setup I2C rate, and initialize I2C clocking */
	NVIC_DisableIRQ(I2C0_IRQn);					/* Disable the interrupt for the I2C */

	DigitalIOPin sw3(1, 9, true, true, true);
	uint8_t timer = 0;
	uint8_t stb[2] = {0x01, 0x80};
	uint8_t wk[2] = {0x01, 0x00};
	uint8_t trash;
	bool justwk = false;

	while(1){


		if(sw3.Read()){
			ReadTemperatureI2CM();
			Sleep(200);
		}


		while (1) {
			if(sw3.Read()){
				if(justwk){
					ITM_write("Wake up\n");
					SetupXferRecAndExecute(I2C_TEMP_ADDR_7BIT, wk, 2, &trash, 0);
					justwk = false;
				}
				//Sleep(1000);
				ReadTemperatureI2CM();
				timer = 0;
			}
			Sleep(100);
			timer++;
			if(timer == 50) {
				timer = 0;
				if(!justwk){
					ITM_write("Entered standby mode\n");
					SetupXferRecAndExecute(I2C_TEMP_ADDR_7BIT, stb, 2, &trash, 0);
				}
				justwk = true;
			}
		}




	}
	return 0;
}

















/*
 *





 */

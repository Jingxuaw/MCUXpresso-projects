/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#if defined (__USE_LPCOPEN)
#if defined(NO_BOARD_LIB)
#include "chip.h"
#else
#include "board.h"
#endif
#endif
#include <string.h>
#include <stdlib.h>
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "ITM_write.h"
// TODO: insert other include files here
#ifndef DIGITALIOPIN_H_
#define DIGITALIOPIN_H_
class DigitalIoPin {
public:
	enum pinMode {
		output,
		input,
		pullup,
		pulldown
	};
	DigitalIoPin(int port, int pin, pinMode mode, bool invert = false);
	virtual ~DigitalIoPin();
	virtual bool read();
	void write(bool value);
private:
	int port;
	int pin;
};
#endif
DigitalIoPin::DigitalIoPin(int port_, int pin_, pinMode mode, bool invert) : port(port_), pin(pin_) {
	if(mode == output) {
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, port, pin);
	}
	else {
		uint32_t pm = IOCON_DIGMODE_EN;
		if(invert) pm |= IOCON_INV_EN;
		if(mode == pullup) {
			pm |= IOCON_MODE_PULLUP;
		}
		else if(mode == pulldown) {
			pm |= IOCON_MODE_PULLDOWN;
		}
		Chip_IOCON_PinMuxSet(LPC_IOCON, port, pin, pm);
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, port, pin);
	}
}
DigitalIoPin::~DigitalIoPin() {
	// TODO Auto-generated destructor stub
}
bool DigitalIoPin::read() {
	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
}
void DigitalIoPin::write(bool value) {
	return Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
}
// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


void SCT_Init(void)
{
	Chip_SCT_Init(LPC_SCTLARGE0);
	LPC_SCTLARGE0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCTLARGE0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCTLARGE0->MATCHREL[0].L = 1000-1; // set the max frequency to 1kHz
	LPC_SCTLARGE0->MATCHREL[1].L = 950; // match 1 used for duty cycle (in 10 steps)

	LPC_SCTLARGE0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCTLARGE0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O,0,3);//for green led

}

void PWM_set(int val) {
	LPC_SCTLARGE0->MATCHREL[1].L = val;
}

/* LED1 toggle thread */
static void vLEDTask1(void *pvParameters) {
	DigitalIoPin sw1(0,17,DigitalIoPin::pullup,true);
	DigitalIoPin sw2(1,11,DigitalIoPin::pullup,true);
	DigitalIoPin sw3(1,9,DigitalIoPin::pullup,true);
	int count=950;
	char buffer[80]={0};
	bool pressed=false;
	float percent=0;
	while(1){
		if(sw1.read()){
			if(count>0){
				pressed=true;
				count--;
				if(sw2.read()){
					count -= 10;
				}
				PWM_set(count);
			}
			else{
				count=0;
			}
			percent=((1000-(float)count)/1000)*100;
			sprintf(buffer,"Duty Cycle = %.f%%\r\n",percent);
			ITM_write(buffer);
		}
		else if(!sw1.read()&&pressed==true){
			pressed=false;
		}
		else if(sw3.read()){
			if(count<1000){
				if(sw2.read()){
					count += 10;
				}
				pressed=true;
				count++;
				PWM_set(count);
			}else{
				count=1000;
			}
			percent=((1000-(float)count)/1000)*100;
			sprintf(buffer,"Duty Cycle = %.f%%\r\n",percent);
			ITM_write(buffer);
		}
		else if(!sw3.read()&&pressed==true){
			pressed=false;
		}
		vTaskDelay(10);
	}
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/

/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void) {
	prvSetupHardware();
	ITM_init();

	SCT_Init();

	/* LED1 toggle thread */
	xTaskCreate(vLEDTask1, "vTaskLed1",
			configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

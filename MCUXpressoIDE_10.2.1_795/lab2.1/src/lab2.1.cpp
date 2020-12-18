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
#include <mutex>
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include "board.h"
#include "FreeRTOS.h"
#include "semphr.h"
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

class Syslog {
public:
	Syslog();
	virtual ~Syslog();
	void write(char *description);
private:
	SemaphoreHandle_t syslogMutex;
};
Syslog::Syslog() {
	syslogMutex = xSemaphoreCreateMutex();
}
Syslog::~Syslog() {
	vSemaphoreDelete(syslogMutex);
}
void Syslog::write(char *description) {
	if(xSemaphoreTake(syslogMutex, portMAX_DELAY) == pdTRUE) {
		Board_UARTPutSTR(description);
		xSemaphoreGive(syslogMutex);
	}
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


Syslog syslog;
static void sw1Task(void *pvParameters) {
	DigitalIoPin sw1(0,17,DigitalIoPin::pullup,true);
	while(1){
		if(sw1.read()){
			syslog.write("sw1 pressed \r\n");
		}
	}
}
static void sw2Task(void *pvParameters) {
	DigitalIoPin sw2(1,11,DigitalIoPin::pullup,true);
	while(1){
		if(sw2.read()){
			syslog.write("sw2 pressed \r\n");
		}
	}
}
static void sw3Task(void *pvParameters) {
	DigitalIoPin sw3(1,9,DigitalIoPin::pullup,true);
	while(1){
		if(sw3.read()){
			syslog.write("sw3 pressed \r\n");
		}
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

	xTaskCreate(sw1Task,"TaskSW1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(sw2Task,"TaskSW2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(sw3Task,"TaskSW3",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);




	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

	//#if defined (__USE_LPCOPEN)
	//    // Read clock settings and update SystemCoreClock variable
	//    SystemCoreClockUpdate();
	//#if !defined(NO_BOARD_LIB)
	//    // Set up and initialize all required blocks and
	//    // functions related to the board hardware
	//    Board_Init();
	//    // Set the LED to the state of "On"
	//    Board_LED_Set(0, true);
	//#endif
	//#endif
	//
	//    // TODO: insert code here
	//
	//    // Force the counter to be placed into memory
	//    volatile static int i = 0 ;
	//    // Enter an infinite loop, just incrementing a counter
	//    while(1) {
	//        i++ ;
	//    }
	//    return 0 ;
}

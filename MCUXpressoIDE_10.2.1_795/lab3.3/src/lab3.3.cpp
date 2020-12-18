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

#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <mutex>
#include "semphr.h"
#include <stdio.h>
#include <time.h>
#include <stdio.h>
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


QueueHandle_t que;
//SemaphoreHandle_t mutex;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

struct debugEvent {
	const char *format;
	uint32_t data[3];
};
void debug(const char *format, uint32_t d1, uint32_t d2, uint32_t d3){

	debugEvent e={format,d1,d2,d3};
	xQueueSendToBack(que,(void*)&e,(TickType_t)10);

}
void debugTask(void *pvParameters)
{
	char buffer[64];
	debugEvent e;

	while (1) {
		// read queue
		if(xQueueReceive(que,(void*)&e,(TickType_t)10) == pdPASS){
			snprintf(buffer, 64, e.format, e.data[0], e.data[1], e.data[2]);
			ITM_write(buffer);

		}
	}
}
static void vTask1(void *pvParameters) { //input to serial port
	int c;
	int size=0, temp=0;
	while(1){

		c=Board_UARTGetChar();
		if(c!=EOF) {
			if(c=='\r'|| c=='\n'||c=='\t'||c==' '){
				size=temp;
				debug("Received cmd: %d at %d\r\n",size, xTaskGetTickCount(), 0);
				temp=0;
				size=0;
			}
			else{
				temp++;
			}
			Board_UARTPutChar(c);
		}
		vTaskDelay(1);
	}
}


static void vTask2(void *pvParameters) { // button pressed time
	DigitalIoPin sw1(0,17,DigitalIoPin::input,true);
	int i = 0;
	bool pressed=false;
	while(1){
		if(sw1.read()){
			i++;
			pressed=true;
		}
		if(!sw1.read()&&pressed==true){

			debug("Hold the button for %d ms\n",i,0,0);
			pressed=false;
			i=0;

		}
		vTaskDelay(1);
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


	//	mutex=xSemaphoreCreateMutex();
	que=xQueueCreate(20,sizeof(debugEvent));

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 2UL),
			(TaskHandle_t *) NULL);


	xTaskCreate(debugTask,"TaskDebug",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

}

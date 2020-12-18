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


SemaphoreHandle_t syslogB = NULL;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


DigitalIoPin lim_sw1(0,27,DigitalIoPin::pullup,true);
DigitalIoPin lim_sw2(0,28,DigitalIoPin::pullup,true);


static void vTask1(void *pvParameters) { //input to serial port

	while(lim_sw1.read()||lim_sw2.read()){
		Board_LED_Set(2, true);
		vTaskDelay(500);
		Board_LED_Set(2, false);
		vTaskDelay(500);
	}

	while(1){

		xSemaphoreGive(syslogB);
		if(lim_sw1.read()){
			Board_LED_Set(0, true);
			vTaskDelay(1000);
		}
		if(!lim_sw1.read()){
			Board_LED_Set(0, false);
		}
		if(lim_sw2.read()){
			Board_LED_Set(1, true);
			vTaskDelay(1000);

		}
		if(!lim_sw2.read()){
			Board_LED_Set(1, false);
		}
		vTaskDelay(1);
	}
}

static void vTask2(void *pvParameters) { // button pressed time
	DigitalIoPin dir(1,0,DigitalIoPin::output,true);
	DigitalIoPin step(0,24,DigitalIoPin::output,true);
	dir.write(false);
	int count=0,buffer=0,running=0,i=0;
	bool isReach=false;

	if(xSemaphoreTake(syslogB, portMAX_DELAY)==pdTRUE){
		while(isReach==false){
			if(!lim_sw1.read()||!lim_sw2.read()){
				step.write(true);
				vTaskDelay(1);
				step.write(false);
				vTaskDelay(1);
				count++;

				if(lim_sw1.read()) {
					dir.write(false);
					buffer++;
					count=0;
					DEBUGOUT("Buffer is %d\r\n",buffer);
				}

				if(lim_sw2.read()){
					dir.write(true);
					DEBUGOUT("the testing distance is: %d\r\n",count);
					Board_LED_Set(2, true);
					vTaskDelay(2000);
					Board_LED_Set(2, false);
					if(count>1723){
						isReach=true;
					}
				}
				vTaskDelay(1);
			}
		}

		for(i=0;i<10;i++){
			step.write(true);
			vTaskDelay(1);
			step.write(false);
			vTaskDelay(1);

		}
		while(isReach==true){

			DEBUGOUT("Isreach = true. the testing distance is: %d\r\n",count);
			step.write(true);
			vTaskDelay(1);
			step.write(false);
			vTaskDelay(1);
			running++;

			DEBUGOUT("running distance is: %d\r\n",running);
			if(running<(count-13)){
				dir.write(true);
			}
			if(running>(count-13)&&running<2*(count-13)){
				dir.write(false);

			}
			if(running>2*(count-13)){
				running=0;
			}

			if(lim_sw1.read()&&lim_sw2.read()){
				step.write(false);
				vTaskDelay(5000);
			}
			vTaskDelay(1);
		}

	}
	else{
		step.write(false);
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

	syslogB = xSemaphoreCreateBinary();
	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

}

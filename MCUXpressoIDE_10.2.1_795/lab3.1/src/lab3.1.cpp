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
SemaphoreHandle_t mutex;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}
//不需要用semaphore 多task下排队 系统会分配
static void vTask1(void *pvParameters) {
	int c;
	int size=0, temp=0;
	while(1){
		if(xSemaphoreTake(mutex, portMAX_DELAY ) == pdTRUE){
			c=Board_UARTGetChar();
			if(c!=EOF) {
				if(c=='\r'|| c=='\n'){
					size=temp;
					if(xQueueSendToBack(que,(void*)&size,(TickType_t)10)==pdPASS){
						temp=0;
						size=0;
					}

				}
				else{
					temp++;
				}
				Board_UARTPutChar(c);
			}
			xSemaphoreGive(mutex);
		}
		vTaskDelay(1);
	}
}
static void vTask2(void *pvParameters) {
	DigitalIoPin sw1(0,17,DigitalIoPin::pullup,false);
	int i = -1;
	bool pressed=false;
	while(1){
		if(sw1.read()){
			pressed=true;
		}
		if(!sw1.read()&&pressed==true){
			if(xQueueSendToBack(que, (void*)&i,(TickType_t)10)==pdPASS){
				pressed=false;
			}
		}
		vTaskDelay(1);
	}
}
//不需要用semaphore 多task下排队 系统会分配
static void vTask3(void *pvParameters) {
	int rxsize=0;
	int temp=0;
	while(1){

		if(xSemaphoreTake(mutex, portMAX_DELAY ) == pdTRUE){
			if(xQueueReceive(que,(void*)&temp,(TickType_t)10)==pdPASS){

				if(temp!=-1){
					rxsize += temp;
				}
				else{
					DEBUGOUT("\r\nYou have typed %d characters\r\n",rxsize);
					temp=0;
					rxsize=0;
				}
			}
			xSemaphoreGive(mutex);
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

	mutex=xSemaphoreCreateMutex();
	que=xQueueCreate(5,sizeof(int));

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask3,"Task3",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

}

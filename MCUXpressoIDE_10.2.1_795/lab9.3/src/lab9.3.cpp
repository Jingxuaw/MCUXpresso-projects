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
#include <string>
#include "event_groups.h"
#include <ctime>
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


SemaphoreHandle_t guard;
EventGroupHandle_t evt;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}
#define task_1	( 1 << 0 )
#define task_2	( 1 << 1 )
#define task_3	( 1 << 2 )

#define ALL_SYNC_BITS (task_1 | task_2 | task_3)

static void vTask1(void *pvParameters) {
	DigitalIoPin sw1(0,17,DigitalIoPin::pullup,true);
	bool pressed=false;
	while(1){

		if(sw1.read()){
			pressed=true;
		}
		if(!sw1.read()&&pressed==true){
			pressed=false;

			xEventGroupSetBits(evt,task_1);

		}
		vTaskDelay(1);
	}
}

static void vTask2(void *pvParameters) {
	DigitalIoPin sw2(1,11,DigitalIoPin::pullup,true);
	bool pressed=false;
	while(1){

		if(sw2.read()){
			pressed=true;
		}
		if(!sw2.read()&&pressed==true){
			pressed=false;

			xEventGroupSetBits(evt,task_2);
		}
		vTaskDelay(1);
	}
}


static void vTask3(void *pvParameters) {
	DigitalIoPin sw3(1,9,DigitalIoPin::pullup,true);
	bool pressed=false;
	while(1){

		if(sw3.read()){
			pressed=true;
		}
		if(!sw3.read()&&pressed==true){
			pressed=false;
			xEventGroupSetBits(evt,task_3);
		}
		vTaskDelay(1);

	}
}
static void vTask4(void *pvParameters) {
	long tickPre=0;
	long tickbtn1=tickPre;
	long tickbtn2=tickPre;
	long tickbtn3=tickPre;
	EventBits_t uxReturn;
	while(1){

		uxReturn = xEventGroupWaitBits( evt,ALL_SYNC_BITS,pdTRUE,pdTRUE,5000 );

		if( ( uxReturn & ALL_SYNC_BITS ) == ALL_SYNC_BITS )
		{
			DEBUGOUT("OK. The elapsed time is %d\r\n", xTaskGetTickCount()-tickPre);
			tickPre=xTaskGetTickCount();
			xEventGroupClearBits(evt, ALL_SYNC_BITS );
		}
		else{
			xEventGroupClearBits(evt, ALL_SYNC_BITS );
		}
		if( (uxReturn & task_1) ==0){
			DEBUGOUT("FAIL. The task1 elapsed time is %d\r\n", xTaskGetTickCount()-tickbtn1);
		}
		else{
			tickbtn1=xTaskGetTickCount();
		}
		if( (uxReturn & task_2) ==0){
			DEBUGOUT("FAIL. The task2 elapsed time is %d\r\n", xTaskGetTickCount()-tickbtn2);
		}
		else{
			tickbtn2=xTaskGetTickCount();
		}
		if( (uxReturn & task_3) ==0){
			DEBUGOUT("FAIL. The task3 elapsed time is %d\r\n", xTaskGetTickCount()-tickbtn3);
		}
		else{
			tickbtn3=xTaskGetTickCount();

		}
		DEBUGOUT("\r\n");
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

	guard = xSemaphoreCreateMutex();
	evt = xEventGroupCreate();

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask3,"Task3",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask4,"Task4",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

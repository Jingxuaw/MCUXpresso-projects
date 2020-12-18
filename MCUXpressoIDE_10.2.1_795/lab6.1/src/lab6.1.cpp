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



volatile uint32_t RIT_count;
xSemaphoreHandle sbRIT = NULL;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	// initialize RIT (= enable clocking etc.)
	Chip_RIT_Init(LPC_RITIMER);
	// set the priority level of the interrupt
	// The level must be equal or lower than the maximum priority specified in FreeRTOS config
	// Note that in a Cortex-M3 a higher number indicates lower interrupt priority
	NVIC_SetPriority( RITIMER_IRQn,  configMAX_SYSCALL_INTERRUPT_PRIORITY+ 1 );
	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}
void RIT_start(int count, int us)
{
	uint64_t cmp_value;
	// Determine approximate compare value based on clock rate and passed interval
	cmp_value = (uint64_t) Chip_Clock_GetSystemClockRate() * (uint64_t) us / 1000000;
	// disable timer during configuration
	Chip_RIT_Disable(LPC_RITIMER);
	RIT_count = count;
	// enable automatic clear on when compare value==timer value
	// this makes interrupts trigger periodically
	Chip_RIT_EnableCompClear(LPC_RITIMER);
	// reset the counter
	Chip_RIT_SetCounter(LPC_RITIMER, 0);
	Chip_RIT_SetCompareValue(LPC_RITIMER, cmp_value);
	// start counting
	Chip_RIT_Enable(LPC_RITIMER);
	// Enable the interrupt signal in NVIC (the interrupt controller)
	NVIC_EnableIRQ(RITIMER_IRQn);
	// wait for ISR to tell that we're done
	if(xSemaphoreTake(sbRIT, portMAX_DELAY) == pdTRUE) {
		// Disable the interrupt signal in NVIC (the interrupt controller)
		NVIC_DisableIRQ(RITIMER_IRQn);
	}
	else {
		// unexpected error
	}
}
bool state=false;
/* the following is required if runtime statistics are to be collected */
extern "C" {
void RIT_IRQHandler(void)
{
	DigitalIoPin step(0,24,DigitalIoPin::output,true);

	// This used to check if a context switch is required
	portBASE_TYPE xHigherPriorityWoken = pdFALSE;
	// Tell timer that we have processed the interrupt.
	// Timer then removes the IRQ until next match occurs
	Chip_RIT_ClearIntStatus(LPC_RITIMER); // clear IRQ flag
	if(RIT_count > 0) {
		RIT_count--;
		// do something useful here...
		step.write(state);
		state=!(bool)state;
	}
	else {
		Chip_RIT_Disable(LPC_RITIMER); // disable timer
		// Give semaphore and set context switch flag if a higher priority task was woken up
		xSemaphoreGiveFromISR(sbRIT, &xHigherPriorityWoken);
	}
	// End the ISR and (possibly) do a context switch
	portEND_SWITCHING_ISR(xHigherPriorityWoken);
}
}
/* end runtime statictics collection */


static void vTask1(void *pvParameters) { //input to serial port
	DigitalIoPin lim_sw1(0,27,DigitalIoPin::pullup,true);
	DigitalIoPin lim_sw2(0,28,DigitalIoPin::pullup,true);
	DigitalIoPin dir(1,0,DigitalIoPin::output,true);
	DigitalIoPin step(0,24,DigitalIoPin::output,true);
	char m[11]={0};
	int c;
	int i = 0;
	int count;
	int speed=400;
	char st[8];
	//	RIT_start(1000,500);
	int us=500000/speed;

	bool get_state=false;
	while(1){
		while (get_state==false){
			m[11]={0};
			c = Board_UARTGetChar();

			if(i<10 && c!=EOF){
				Board_UARTPutChar(c);
				m[i]=c;
				i++;
				if (c == '\r' || c == '\n') {
					m[i] = '\0';
					i = 0;
					if(m[0]=='l'&&m[1]=='e'&&m[2]=='f'&&m[3]=='t'){
						dir.write(true);
						sscanf(m,"%s %d",st,&count);
						get_state=true;
					}
					else if(m[0]=='r'&&m[1]=='i'&&m[2]=='g'&&m[3]=='h'&&m[4]=='t'){
						dir.write(false);
						sscanf(m,"%s %d",st,&count);
						get_state=true;
					}
					else if(m[0]=='p'&&m[1]=='p'&&m[2]=='s'){
						sscanf(m,"%s %d",st,&speed);
						us=500000/speed;
						DEBUGOUT("the speed now is %d and the dealay is %d \r\n",speed,us);
					}
				}
			}
			vTaskDelay(1);
		}

		while(get_state==true){
			RIT_start(count,us);
			DEBUGOUT("\r\nMOVE TO %s for %d steps with %d delay\r\n",st,count,us);
			get_state=false;
			vTaskDelay(1);
		}
		vTaskDelay(1);
	}
}


static void vTask2(void *pvParameters) { // button pressed time
	DigitalIoPin lim_sw1(0,27,DigitalIoPin::pullup,true);
	DigitalIoPin lim_sw2(0,28,DigitalIoPin::pullup,true);
	DigitalIoPin dir(1,0,DigitalIoPin::output,true);
	DigitalIoPin step(0,24,DigitalIoPin::output,true);
	while(1){

		if(!lim_sw1.read()&&!lim_sw2.read()){

		}
		else{
			RIT_count=0;
			step.write(false);
		}
		vTaskDelay(1);
	}
}


/*****************************************************************************
 * Public functions
 ****************************************************************************/



/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void) {

	prvSetupHardware();

	sbRIT = xSemaphoreCreateBinary();
	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+256, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

}

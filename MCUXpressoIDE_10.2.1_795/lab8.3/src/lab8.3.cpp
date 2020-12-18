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
using namespace std;
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>
#include <string>
#include <stdlib.h>
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include <mutex>
#include "semphr.h"
#include "ITM_write.h"
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
struct myEvent {
	long tickCount;
	uint8_t btnEven;
};

enum STATE {LOCKED, UNLOCKED, EDIT};

uint8_t LOCK_CODE[8] = {1,1,1,1,0,1,1,0};
uint8_t KEY_CODE[8] = {0,0,0,0,0,0,0,0};

STATE state = LOCKED;

long filter_time = 300;

/* the following is required if runtime statistics are to be collected */
extern "C" {


void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}
/* end runtime statictics collection */

void PIN_INT0_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH0);
	myEvent btEvent{(long)xTaskGetTickCountFromISR(),1};
	xQueueSendToBackFromISR(que,&btEvent,&xHigherPriorityTaskWoken);
	//DEBUGOUT("SW1 pressed \r\n");
	portEND_SWITCHING_ISR(BaseType_t xHigherPriorityTaskWoken = pdFALSE);

}

void PIN_INT1_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH1);
	myEvent btEvent{(long)xTaskGetTickCountFromISR(),2};
	xQueueSendToBackFromISR(que,&btEvent,&xHigherPriorityTaskWoken);
	//DEBUGOUT("SW2 pressed \r\n");
	portEND_SWITCHING_ISR(BaseType_t xHigherPriorityTaskWoken = pdFALSE);


}

static void PINITR_Init(){
	//Initialize pin interrupt hardware
	Chip_PININT_Init(LPC_GPIO_PIN_INT);

	//Set priority of each pin interrupt you plan to use (NVIC)
	// Map pin to the interrupt
	Chip_INMUX_PinIntSel(0, 0, 17);
	Chip_INMUX_PinIntSel(1, 1, 11);


	NVIC_SetPriority(PIN_INT0_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);
	NVIC_SetPriority(PIN_INT1_IRQn, configMAX_SYSCALL_INTERRUPT_PRIORITY + 1);

	//Configure and enable pin interrupts in the pin interrupt hardware

	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_MODE_PULLUP | IOCON_MODE_PULLUP);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 11, IOCON_MODE_PULLUP | IOCON_MODE_PULLUP);

	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 11);

	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT, PININTCH1);

	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH0); // Falling Edge
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT, PININTCH1); // Falling Edge

	Chip_PININT_DisableIntHigh(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_DisableIntHigh(LPC_GPIO_PIN_INT, PININTCH1);

	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);

	//Enable each interrupt on NVIC

	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);

	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);

}

}

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

	ITM_init();
}

//BTN3 TASK HANDLE
static void btn3_task(void *xSemaphore) {
	DigitalIoPin sw3 (1,9, DigitalIoPin::pullup, true);
	TickType_t  pressCount = 0;
	myEvent btn3 = {0,3};

	while(1) {
		if(sw3.read()){
			pressCount++;
		}
		else{
			pressCount = 0;
		}

		if(pressCount == 3000){
			pressCount = 0;
			xQueueSendToFront(que, &btn3, (TickType_t) 10);
		}

		vTaskDelay(1);
	}
}


uint8_t handleLocked(uint8_t btnEven, uint8_t index){
	uint8_t matchCount = 0;

	if (index < 8) {
		index++;
	} else {
		for (int i = 1; i<=7; i++ ) {
			KEY_CODE[i-1] = KEY_CODE[i];
		}
	}

	KEY_CODE[index-1] = btnEven-1;
	DEBUGOUT("KEY ENTERED: %d \r\n", KEY_CODE[index-1]);

	for (int i = 0; i<=7; i++){
		if (LOCK_CODE[i]==KEY_CODE[i]){
			matchCount++;
		}
	}

	if (matchCount == 8) {
		state = UNLOCKED;
		DEBUGOUT("DOOR IS UNLOCKED. \r\n");
		index = 0;
		memset(KEY_CODE,'2',8);
	}

	return index;
}

uint8_t handleEdit(uint8_t btnEven, uint8_t index){
	if (btnEven == 1) {
		LOCK_CODE[index] = 0;
		index++;
		DEBUGOUT("KEY ENTERED: 0 \r\n");
	} else if (btnEven == 2) {
		LOCK_CODE[index] = 1;
		index++;
		DEBUGOUT("KEY ENTERED: 1 \r\n");
	}


	if (index == 8) {
		index = 0;
		state = LOCKED;
		DEBUGOUT("NEW KEY SEQUENCE: %d%d%d%d%d%d%d%d\r\n", LOCK_CODE[0], LOCK_CODE[1], LOCK_CODE[2], LOCK_CODE[3], LOCK_CODE[4], LOCK_CODE[5]
																																		   , LOCK_CODE[6] , LOCK_CODE[7]);
	}
	return index;
}

/* send data and toggle thread */
static void door_task(void *xSemaphore) {
	long lastTick = 0;
	myEvent swEvent = {0,0};
	uint8_t index = 0;
	bool isTimeOutAlready = true;

	while(1) {
		if(xQueueReceive(que, (void*) &swEvent, (TickType_t) 10)) {
			isTimeOutAlready = false;
			if (swEvent.btnEven == 3){
				state = EDIT;
				index = 0;
				DEBUGOUT("PLEASE ENTER NEW KEY \r\n");
			}

			long diff = swEvent.tickCount - lastTick;
			if (diff >= filter_time) {
				//Handle LOCKED OR EDIT EVENT
				//KEY PRESSED FOR LOCKED IS HANDLED HERE, SO WE NEED FILTER
				if (state == LOCKED) {
					index = handleLocked(swEvent.btnEven, index);
				} else if (state == EDIT) {
					index = handleEdit(swEvent.btnEven, index);
				}
			}
			lastTick = swEvent.tickCount;
		}

		if ((state == LOCKED)&&!isTimeOutAlready) {
			if(lastTick == 0) {
				lastTick = xTaskGetTickCount();
			} else {
				long diff = xTaskGetTickCount() - lastTick;
				if (diff >= 15000) {
					state = LOCKED;
					DEBUGOUT("TIMEOUT. \r\n");
					isTimeOutAlready = true;
					lastTick = 0;
					index = 0;
					memset(KEY_CODE,'2',8);
				}
			}
		}

	}

}


static void led_task(void *xSemaphore) {
	long lastTick = 0;

	while(1) {
		if(state != EDIT) {
			Board_LED_Set(0, state==LOCKED);
			Board_LED_Set(1, state==UNLOCKED);
			Board_LED_Set(2, false);

			//HANDLE UNLOCKED EVENT AND TIME OUT
			//No key pressed is needed, just count to 5sec then timeout.
			if (state == UNLOCKED) {
				if(lastTick == 0) {
					lastTick = xTaskGetTickCount();
				} else {
					long diff = xTaskGetTickCount() - lastTick;

					if (diff >= 5000) {
						state = LOCKED;
						DEBUGOUT("DOOR IS LOCKED AFTER 5 SECONDS. \r\n");
						lastTick = 0;
					}
				}
			}
		} else {
			Board_LED_Set(0, false);
			Board_LED_Set(1, false);
			Board_LED_Set(2, true);
		}
		vTaskDelay(10);
	}
}


int main(void) {

	prvSetupHardware();
	PINITR_Init();
	DEBUGOUT("Program ran \r\n");

	que = xQueueCreate(10, sizeof( myEvent ) );

	/* LED1 toggle thread */
	xTaskCreate(btn3_task, "btn3",
			configMINIMAL_STACK_SIZE*10, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(door_task, "door",
			configMINIMAL_STACK_SIZE*10, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(led_task, "led",
			configMINIMAL_STACK_SIZE*10, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

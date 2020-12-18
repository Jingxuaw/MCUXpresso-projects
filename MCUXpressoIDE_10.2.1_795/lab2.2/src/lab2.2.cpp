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

static void vTask1(void *pvParameters) {
	int c;
	while(1){
		c=Board_UARTGetChar();
		if(syslogB!=NULL && c!=EOF) {
			if(c == '\n')	Board_UARTPutChar('\r'); // precede line feed with carriage return
			if(c == '\r')	Board_UARTPutChar('\n'); // send line feed after carriage return
			xSemaphoreGive(pvParameters);
			Board_UARTPutChar(c);

		}

	}
}

static void vTask2(void *pvParameters) {
	bool LedState = true;

	while(1){
		if(xSemaphoreTake(pvParameters, portMAX_DELAY)==pdTRUE){

			Board_LED_Set(0, LedState);
			LedState = (bool) !LedState;
			vTaskDelay(configTICK_RATE_HZ/10);
			Board_LED_Set(0, LedState);
			vTaskDelay(configTICK_RATE_HZ/10);
			LedState = (bool) !LedState;
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

	syslogB = xSemaphoreCreateBinary();

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, syslogB, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, syslogB, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();



	/* Should never arrive here */
	return 1;

}

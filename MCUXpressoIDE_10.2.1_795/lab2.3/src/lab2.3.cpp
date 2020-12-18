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
// TODO: insert other include files here


SemaphoreHandle_t syslogC;
SemaphoreHandle_t guard;


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
	char message[11]={0};
	char c;
	int i = 0;
	while(1){
		if(xSemaphoreTake( guard, portMAX_DELAY ) == pdTRUE){
			c = Board_UARTGetChar();
			xSemaphoreGive(guard);
		}

		if (c <127) {
			if ( xSemaphoreTake( guard, portMAX_DELAY ) == pdTRUE) {
				Board_UARTPutChar(c);
				xSemaphoreGive(guard);
			}
			if (i <= 10) {
				message[i] = c;
				i++;
				if (c == '\r' || c == '\n') {
					message[i] = '\0';
					i = 0;
					if ( xSemaphoreTake( guard, portMAX_DELAY ) == pdTRUE) {
						Board_UARTPutSTR("[YOU]: ");
						Board_UARTPutSTR(message);
						Board_UARTPutSTR("\r\n");
						xSemaphoreGive(guard);
					}
				}
				else if (c == '?') {
					message[i] = '\0';
					i = 0;
					if ( xSemaphoreTake( guard, portMAX_DELAY ) == pdTRUE) {
						Board_UARTPutSTR("[YOU]: ");
						Board_UARTPutSTR(message);
						Board_UARTPutSTR("\r\n");
						xSemaphoreGive(guard);
					}
					xSemaphoreGive(syslogC);
				}
			}
			else {
				message[i] = '\0';
				i = 0;
				if ( xSemaphoreTake( guard, portMAX_DELAY ) == pdTRUE) {
					Board_UARTPutSTR("[YOU]: ");
					Board_UARTPutSTR(message);
					Board_UARTPutSTR("\r\n");
					xSemaphoreGive(guard);
				}
			}
		}
		vTaskDelay(1);
	}
}


static void vTask2(void *pvParameters) {
	int i;
	while(1){
		i=rand() % 5;
		if(xSemaphoreTake(syslogC, portMAX_DELAY)==pdTRUE){


			if(xSemaphoreTake(guard, portMAX_DELAY)==pdTRUE){
				Board_UARTPutSTR("[Oracle] Hmmm...\r\n");
				xSemaphoreGive(guard);
				vTaskDelay(configTICK_RATE_HZ*3);
			}

			//			char myANswers[2] = {"[Oracle] You will meet a tall dark stranger.", "[Oracle] You will meet a tall dark stranger."};
			//			Board_UARTPutChar(myANswers[i]);


			if(xSemaphoreTake(guard, portMAX_DELAY)==pdTRUE){
				switch (i){

				case 1:
					Board_UARTPutSTR("[Oracle] You will meet a tall dark stranger. \r\n");
					break;
				case 2:
					Board_UARTPutSTR("[Oracle] Why would you go there? \r\n");
					break;
				case 3:
					Board_UARTPutSTR("[Oracle] Hey man! \r\n");
					break;
				case 4:
					Board_UARTPutSTR("[Oracle] Who are you? \r\n");
					break;
				case 5:
					Board_UARTPutSTR("[Oracle] Are u ok? \r\n");
					break;
				default:
					Board_UARTPutSTR("[Oracle]: This is the default case. \r\n");
					break;
				}

				xSemaphoreGive(guard);
				vTaskDelay(configTICK_RATE_HZ*2);
			}
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

	syslogC = xSemaphoreCreateCounting(5,0);
	guard = xSemaphoreCreateMutex();

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

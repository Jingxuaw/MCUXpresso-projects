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
#include <stdio.h>
#include <string.h>
// TODO: insert other include files here
#include "FreeRTOS.h"
#include "task.h"
#include "ITM_write.h"

#include <mutex>
#include "Fmutex.h"
#include "user_vcom.h"

// TODO: insert other definitions and declarations here


/* the following is required if runtime statistics are to be collected */
extern "C" {

void vConfigureTimerForRunTimeStats( void ) {
	Chip_SCT_Init(LPC_SCTSMALL1);
	LPC_SCTSMALL1->CONFIG = SCT_CONFIG_32BIT_COUNTER;
	LPC_SCTSMALL1->CTRL_U = SCT_CTRL_PRE_L(255) | SCT_CTRL_CLRCTR_L; // set prescaler to 256 (255 + 1), and start timer
}

}
/* end runtime statictics collection */

/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);

}
SemaphoreHandle_t syslogC;
SemaphoreHandle_t mutex;

static void vTask1(void *pvParameters) {
	int i = 0,j=0;
	char buff[80];
	char message[80]={0};
	while(1){
		uint32_t len = USB_receive((uint8_t *)buff, 79);
		buff[len] = 0; /* make sure we have a zero at the end so that we can print the data */
		USB_send((uint8_t *)buff, len);

		for(i=0;i<len;i++) {
			if(j<80){
				message[j]=buff[i];
				j++;
				if (buff[i] == '\r' || buff[i] == '\n') {
					message[j] = '\0';
					j=0;
					if(xSemaphoreTake(mutex, portMAX_DELAY)==pdTRUE){
						USB_send((uint8_t *)"[YOU]: ", strlen("[YOU]: "));
						USB_send((uint8_t *)message, strlen(message));
						USB_send((uint8_t *)"\r\n", strlen("\r\n"));
						xSemaphoreGive(mutex);
					}
				}
				else if (buff[i] == '?') {
					buff[i]='\0';
					message[j]='\0';
					j=0;
					if(xSemaphoreTake(mutex, portMAX_DELAY)==pdTRUE){
						USB_send((uint8_t *)"[YOU]: ", strlen("[YOU]: "));
						USB_send((uint8_t *)message, strlen(message));
						USB_send((uint8_t *)"\r\n", strlen("\r\n"));
						xSemaphoreGive(mutex);
					}
					xSemaphoreGive(syslogC);
				}
			}
		}
		vTaskDelay(100);
	}
}

static void vTask2(void *pvParameters) {
	int i;
	while(1){
		i=rand() % 5;
		if(xSemaphoreTake(syslogC, portMAX_DELAY)==pdTRUE){
			USB_send((uint8_t *)"[Oracle] I find your lack of faith disturbing\r\n", strlen("[Oracle] I find your lack of faith disturbing\r\n"));
			vTaskDelay(3000);

			switch (i){

			case 1:
				USB_send((uint8_t *)"[Oracle] You will meet a tall dark stranger.\r\n", strlen("[Oracle] You will meet a tall dark stranger.\r\n"));
				break;

			case 2:
				USB_send((uint8_t *)"[Oracle] Why would you go there?\r\n", strlen("[Oracle] Why would you go there?\r\n"));
				break;

			case 3:
				USB_send((uint8_t *)"[Oracle] Hey man!\r\n", strlen("[Oracle] Hey man!\r\n"));
				break;

			case 4:
				USB_send((uint8_t *)"[Oracle] Who are you? \r\n", strlen("[Oracle] Who are you? \r\n"));
				break;

			case 5:
				USB_send((uint8_t *)"[Oracle] Are u ok? \r\n", strlen("[Oracle] Are u ok? \r\n"));
				break;

			default:
				USB_send((uint8_t *)"[Oracle] This is the default case. \r\n", strlen("[Oracle] This is the default case.\r\n"));
				break;
			}
			vTaskDelay(500);
		}
	}
}


int main(void) {

	prvSetupHardware();
	ITM_init();

	mutex = xSemaphoreCreateMutex();
	syslogC = xSemaphoreCreateCounting(5,0);

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			79, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);


	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;
}

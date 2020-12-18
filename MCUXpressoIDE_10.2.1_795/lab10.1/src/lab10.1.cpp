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
#include "timers.h"


QueueHandle_t que;
SemaphoreHandle_t syslogB = NULL;
TimerHandle_t one_shot;
TimerHandle_t auto_reload;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


void one_shot_Callback( TimerHandle_t xTimer )
{
	xSemaphoreGive(syslogB);
}

void auto_reload_Callback( TimerHandle_t xTimer ){
	int c=1;
	xQueueSendToBack(que, (void*)&c,0);
}

static void vTask1(void *pvParameters) {
	int c=0;
	while(1){
			if(xQueueReceive(que,(void*)&c,10)){
				if(c==1){
					DEBUGOUT("hello\r\n");
				}
				if(c==2){
					DEBUGOUT("aargh\r\n");
				}
			}

		vTaskDelay(1);
	}
}
static void vTask2(void *pvParameters) {
	int c=2;
	while(1){
			if(xSemaphoreTake(syslogB, portMAX_DELAY ) == pdTRUE){
				xQueueSendToBack(que, (void*)&c,0);
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

	que=xQueueCreate(20,sizeof(int));
	syslogB = xSemaphoreCreateBinary();

	one_shot = xTimerCreate( "one_shot_timer", pdMS_TO_TICKS( 20000 ), pdFALSE, ( void * ) 0, one_shot_Callback);
	auto_reload = xTimerCreate( "auto_reload_timer", pdMS_TO_TICKS( 5000 ), pdTRUE, ( void * ) 0, auto_reload_Callback);
	xTimerStart(one_shot, 0);
	xTimerStart(auto_reload, 0);

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE+128, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

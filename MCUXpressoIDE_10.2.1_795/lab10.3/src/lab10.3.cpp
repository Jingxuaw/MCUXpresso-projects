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


SemaphoreHandle_t syslogB = NULL;
TimerHandle_t timer1;
TimerHandle_t timer2;

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

void timer1_Callback( TimerHandle_t xTimer ){

	xSemaphoreGive(syslogB);

}
float tickPre=0;
void timer2_Callback( TimerHandle_t xTimer ){
	tickPre=xTaskGetTickCount();
	Board_LED_Toggle(1);

}

static void vTask1(void *pvParameters) {
	int c=0;
	int i=0;
	int time_interval=0;
	char buff[20]={0};
	char str[20]={0};

	float tickDiff=0;
	while(1){
		if(xSemaphoreTake(syslogB, 0 ) == pdTRUE){
			DEBUGOUT("Inactive\r\n");
			memset(buff,0,20);
		}
		else{
			c=Board_UARTGetChar();
			if(i<80 && c!=EOF){
				buff[i]=c;
				i++;
				Board_UARTPutChar(c);
				if(c=='\r'||c=='\n'){
					i=0;
					if(buff[0]=='h'&&buff[1]=='e'&&buff[2]=='l'&&buff[3]=='p'){
						DEBUGOUT("-------USAGE INSTRUCTION-------\r\n");
						DEBUGOUT("---Type interval to change the time interval---\r\n");
						DEBUGOUT("---Type time to check the last toggle time---\r\n");

					}
					if(buff[0]=='i'&&buff[1]=='n'&&buff[2]=='t'&&buff[3]=='e'&&buff[4]=='r'&&buff[5]=='v'&&buff[6]=='a'&&buff[7]=='l'){
						sscanf(buff,"%s %d",str,&time_interval);
						xTimerChangePeriod(timer2,time_interval,0);
						DEBUGOUT("the interval changed to %d ms\r\n", time_interval);
					}
					if(buff[0]=='t'&&buff[1]=='i'&&buff[2]=='m'&&buff[3]=='e'){
						tickDiff=(xTaskGetTickCount()-tickPre)/1000;
						tickPre=xTaskGetTickCount();
						DEBUGOUT("TIME: %.1f s\r\n", tickDiff);
					}

				}
				xTimerReset(timer1,0);
			}

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

	syslogB = xSemaphoreCreateBinary();

	timer1 = xTimerCreate( "timer1", pdMS_TO_TICKS( 30000 ), pdFALSE, ( void * ) 0, timer1_Callback);
	timer2 = xTimerCreate( "timer2", pdMS_TO_TICKS( 5000 ), pdTRUE, ( void * ) 0, timer2_Callback);

	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTimerStart(timer1, 0);
	xTimerStart(timer2, 0);
	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

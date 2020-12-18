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

// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();
	//Initialize pin interrupt hardware

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}

typedef struct{
	long tickcnt;
	int btnc;
}myEvent;

int filter=50;
QueueHandle_t que;
static void vTask1(void *pvParameters) { //input to serial port

	int c=0;
	char mess[20]={0};
	int i=0;
	char str[20]={0};

	while (1) {
		c=Board_UARTGetChar();
		if(c!=EOF&&i<20){
			mess[i]=c;
			i++;
			if(c=='\r'|| c=='\n'){
				mess[i] = '\0';
				i=0;
				if(mess[0]=='f'&&mess[1]=='i'&&mess[2]=='l'&&mess[3]=='t'&&mess[4]=='e'&&mess[5]=='r'){
					sscanf(mess,"%s %d",str,&filter);
					DEBUGOUT("\r\nThe filter is %d now\r\n",filter);
				}
			}
			Board_UARTPutChar(c);
		}
		vTaskDelay(1);
	}
}

static void vTask2(void *pvParameters) { //input to serial port

	long tickPre=0;
	long tickDiff=0;
	myEvent e;

	while (1) {
		if(xQueueReceive(que,(void*)&e,(TickType_t)10)==pdPASS){

			tickDiff=e.tickcnt-tickPre;
			tickPre=e.tickcnt;
			if(tickDiff>filter){
				DEBUGOUT("%d ms Button %d pressed\r\n",tickDiff,e.btnc);
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
void PIN_INT2_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT,PININTCH2);
	myEvent btEvent{(long)xTaskGetTickCountFromISR(),3};
	xQueueSendToBackFromISR(que,&btEvent,&xHigherPriorityTaskWoken);
	//DEBUGOUT("SW3 pressed \r\n");
	portEND_SWITCHING_ISR(BaseType_t xHigherPriorityTaskWoken = pdFALSE);
}

}

static void GPIO_Init(void)
{
	Chip_PININT_Init(LPC_GPIO_PIN_INT);
	//Chip_GPIO_Init(LPC_GPIO);

	NVIC_SetPriority( PIN_INT0_IRQn,  configMAX_SYSCALL_INTERRUPT_PRIORITY+ 1 );
	NVIC_SetPriority( PIN_INT1_IRQn,  configMAX_SYSCALL_INTERRUPT_PRIORITY+ 1 );
	NVIC_SetPriority( PIN_INT2_IRQn,  configMAX_SYSCALL_INTERRUPT_PRIORITY+ 1 );

	/* Set pin back to GPIO */
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 17, IOCON_MODE_PULLUP | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 11, IOCON_MODE_PULLUP | IOCON_DIGMODE_EN);
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, IOCON_MODE_PULLUP | IOCON_DIGMODE_EN);

	/* Configure interrupt channel for the GPIO pin in INMUX block */
	Chip_INMUX_PinIntSel(0, 0, 17);
	Chip_INMUX_PinIntSel(1, 1, 11);
	Chip_INMUX_PinIntSel(2, 1, 9);

	/* Configure GPIO pin as input */
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 17);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 11);
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 9);

	/* Configure channel interrupt as edge sensitive and falling edge interrupt */

	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH0);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH0);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH0);

	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH1);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH1);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH1);

	Chip_PININT_SetPinModeEdge(LPC_GPIO_PIN_INT,PININTCH2);
	Chip_PININT_EnableIntLow(LPC_GPIO_PIN_INT,PININTCH2);
	Chip_PININT_ClearIntStatus(LPC_GPIO_PIN_INT, PININTCH2);

	/* Enable interrupt in the NVIC */
	NVIC_ClearPendingIRQ(PIN_INT0_IRQn);
	NVIC_EnableIRQ(PIN_INT0_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT1_IRQn);
	NVIC_EnableIRQ(PIN_INT1_IRQn);
	NVIC_ClearPendingIRQ(PIN_INT2_IRQn);
	NVIC_EnableIRQ(PIN_INT2_IRQn);

}


/* end runtime statictics collection */

/**
 * @brief	main routine for FreeRTOS blinky example
 * @return	Nothing, function should not exit
 */
int main(void) {

	prvSetupHardware();
	que=xQueueCreate(20,sizeof(myEvent));

	GPIO_Init();
	xTaskCreate(vTask1,"Task1",configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(vTask2,"Task2",configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();


	/* Should never arrive here */
	return 1;

}

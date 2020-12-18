
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
#include <string.h>
#include <stdlib.h>
#include <cr_section_macros.h>
#include "FreeRTOS.h"
#include <mutex>
#include "task.h"
#include "Fmutex.h"
#include "user_vcom.h"
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
// TODO: insert other definitions and declarations here
/* Sets up system hardware */
static void prvSetupHardware(void)
{
	SystemCoreClockUpdate();
	Board_Init();

	/* Initial LED0 state is off */
	Board_LED_Set(0, false);
}


void SCT_Init(void)
{
	Chip_SCT_Init(LPC_SCTLARGE0);
	Chip_SCT_Init(LPC_SCTLARGE1);

	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT0_O,0,25);//for red led
	Chip_SWM_MovablePortPinAssign(SWM_SCT0_OUT1_O,0,3);//for green led
	Chip_SWM_MovablePortPinAssign(SWM_SCT1_OUT0_O,1,1);//for blue led

	LPC_SCTLARGE0->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCTLARGE0->CONFIG |= (1 << 18); // two 16-bit timers, auto limit
	LPC_SCTLARGE0->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz
	LPC_SCTLARGE0->CTRL_H |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCTLARGE0->MATCHREL[0].L = 1000-1; // set the max frequency to 1kHz for the SCT1
	LPC_SCTLARGE0->MATCHREL[1].L = 500; //
	LPC_SCTLARGE0->MATCHREL[0].H = 1000-1; // set the max frequency to 1kHz for the SCT2
	LPC_SCTLARGE0->MATCHREL[3].H = 500; //

	LPC_SCTLARGE0->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE0->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE0->EVENT[2].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE0->EVENT[2].CTRL = (1 << 12) | (1 << 4); // match 1 condition only
	LPC_SCTLARGE0->EVENT[3].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE0->EVENT[3].CTRL = (3 << 0) | (1 << 12) | (1 << 4); // match 3 conditions
	LPC_SCTLARGE0->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE0->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0

	LPC_SCTLARGE0->OUT[1].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE0->OUT[1].CLR = (1 << 3); // event 1 will clear SCTx_OUT0

	LPC_SCTLARGE0->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
	LPC_SCTLARGE0->CTRL_H &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg


	LPC_SCTLARGE1->CONFIG |= (1 << 17); // two 16-bit timers, auto limit
	LPC_SCTLARGE1->CTRL_L |= (72-1) << 5; // set prescaler, SCTimer/PWM clock = 1 MHz

	LPC_SCTLARGE1->MATCHREL[0].L = 1000-1; // set the max frequency to 1kHz
	LPC_SCTLARGE1->MATCHREL[1].L = 950; // match 1 used for duty cycle (in 10 steps)

	LPC_SCTLARGE1->EVENT[0].STATE = 0xFFFFFFFF; // event 0 happens in all states
	LPC_SCTLARGE1->EVENT[0].CTRL = (1 << 12); // match 0 condition only
	LPC_SCTLARGE1->EVENT[1].STATE = 0xFFFFFFFF; // event 1 happens in all states
	LPC_SCTLARGE1->EVENT[1].CTRL = (1 << 0) | (1 << 12); // match 1 condition only
	LPC_SCTLARGE1->OUT[0].SET = (1 << 0); // event 0 will set SCTx_OUT0
	LPC_SCTLARGE1->OUT[0].CLR = (1 << 1); // event 1 will clear SCTx_OUT0
	LPC_SCTLARGE1->CTRL_L &= ~(1 << 2); // unhalt it by clearing bit 2 of CTRL reg
}

void PWM_set1(int val) {
	LPC_SCTLARGE0->MATCHREL[1].L = val;
}
void PWM_set3(int val) {
	LPC_SCTLARGE0->MATCHREL[3].H = val;
}
void PWM_set5(int val) {
	LPC_SCTLARGE1->MATCHREL[1].L = val;
}



static void vTask1(void *pvParameters) {
	int i=0,j=0;
	char buff[80];
	char message[20]={0};
	unsigned int color=0;
	char str[8]={0};
	int result=0;
	char p[80]={0};
	while(1){
		uint32_t len = USB_receive((uint8_t *)buff, 79);
		buff[len] = 0; /* make sure we have a zero at the end so that we can print the data */
		USB_send((uint8_t *)buff, len);

		for(i=0;i<len;i++) {
			if(j<20){
				message[j]=buff[i];
				j++;
				if (buff[i] == '\r' || buff[i] == '\n') {
					message[j] = '\0';
					j=0;

					if(message[0]=='r'&&message[1]=='g'&&message[2]=='b'&&message[3]==' '&&message[4]=='#'){

						sscanf(message," %s #%x",str,&color);
						result=color&0xff0000;
						result=result>>16;
						result = ((255-result)*1000)/255;
						sprintf(p,"\r\nThe red is %d \r\n",result);
						PWM_set1(result);
						USB_send((uint8_t *)p, strlen(p));

						result=color&0x00ff00;
						result=result>>8;
						result = ((255-result)*1000)/255;
						sprintf(p,"\r\nThe green is %d \r\n",result);
						PWM_set3(result);
						USB_send((uint8_t *)p, strlen(p));

						result=color&0x0000ff;
						result =((255-result)*1000)/255;
						sprintf(p,"\r\nThe blue is %d \r\n",result);
						PWM_set5(result);
						USB_send((uint8_t *)p, strlen(p));

					}
					else{
						USB_send((uint8_t *)"\r\nCommand is wrong!\r\n", strlen("\r\nCommand is wrong!\r\n"));
					}
				}
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

	SCT_Init();

	xTaskCreate(vTask1, "vTask1",
			configMINIMAL_STACK_SIZE*3, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	xTaskCreate(cdc_task, "CDC",
			79, NULL, (tskIDLE_PRIORITY + 1UL),
			(TaskHandle_t *) NULL);

	/* Start the scheduler */
	vTaskStartScheduler();

	/* Should never arrive here */
	return 1;

}

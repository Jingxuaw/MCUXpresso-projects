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
#include<stdint.h>

#else
#include "board.h"
#endif
#endif
#include <cr_section_macros.h>
#include "ITM_write.h"
#include <atomic>
static volatile std::atomic_int counter;
#ifdef __cplusplus
extern "C" {

#endif /**  * @brief Handle interrupt from SysTick timer  * @return Nothing  */

void SysTick_Handler(void) {
	if(counter > 0) counter--;
}

#ifdef __cplusplus
}
#endif

void Sleep(int ms) {
	counter = ms;  while(counter > 0) {
		__WFI();
	}
}

// TODO: insert other include files here

// TODO: insert other definitions and declarations here
class DigitalIoPin {
public:
	DigitalIoPin(uint8_t a, uint8_t b, bool input = true, bool pullup = true, bool inv = false);
	//virtual ~DigitalIoPin();
	bool Read();
	void Write(bool value);
private:
	uint8_t port;
	uint8_t pin;
	bool invert;

};

DigitalIoPin::DigitalIoPin(uint8_t a,uint8_t b, bool input,  bool pullup, bool inv):port(a),pin(b),invert(inv){

	if (input){
		Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, ((pullup? IOCON_MODE_PULLUP : IOCON_MODE_PULLDOWN)| IOCON_DIGMODE_EN | (inv?  IOCON_INV_EN : 0)));
		Chip_GPIO_SetPinDIRInput(LPC_GPIO, a, b);
	}
	if(input==false){
		Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
		Chip_GPIO_SetPinDIROutput(LPC_GPIO, a, b);

		if(inv){
			Chip_GPIO_SetPinState(LPC_GPIO, a, b, true);
		}
		else{
			Chip_GPIO_SetPinState(LPC_GPIO, a, b, false);
		}
	}

}
//	if (input){
//
//		if(pullup&&inv){
//		 Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
//		 Chip_GPIO_SetPinDIRInput(LPC_GPIO, a, b);
//		 }
//		 if(pullup && inv==false){
//			Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN ));
//			Chip_GPIO_SetPinDIRInput(LPC_GPIO, a, b);
//		}
//		if(inv && pullup==false){
//			Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, (IOCON_MODE_PULLDOWN | IOCON_DIGMODE_EN | IOCON_INV_EN));
//			Chip_GPIO_SetPinDIRInput(LPC_GPIO, a, b);
//		}
//		if(pullup==false&&inv==false){
//		Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, (IOCON_MODE_PULLDOWN | IOCON_DIGMODE_EN));
//		}
//
//	}
//	if(input==false){
//
//		Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
//			Chip_GPIO_SetPinDIROutput(LPC_GPIO, a, b);
//			if(inv){
//				Chip_GPIO_SetPinState(LPC_GPIO, a, b, true);
//			}
//			else{
//				Chip_GPIO_SetPinState(LPC_GPIO, a, b, false);
//			}
//
//
//
//	}
//}

bool DigitalIoPin::Read(){

	return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);

}

void DigitalIoPin::Write(bool value){  //for output

	if(Chip_GPIO_GetPinDIR(LPC_GPIO, port, pin)) {	//if pin is output
		if(invert){
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin, (!value));
		}
		else {Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
		}
	}


	//Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
}

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, false);
#endif
#endif
	ITM_init();
	// TODO: insert code here

	DigitalIoPin sw1(0,17,true,true,true);
	DigitalIoPin sw2(1,11,true,true,true);
	DigitalIoPin sw3(1,9,true,true,true);

	while(1){


		if(sw1.Read()){

			ITM_write("sw1 pressed\n");
		}
		if(sw2.Read()){

			ITM_write("sw2 pressed\n");
		}
		if(sw3.Read()){

			ITM_write("sw3 pressed\n");
		}


	}
	return 0 ;

}

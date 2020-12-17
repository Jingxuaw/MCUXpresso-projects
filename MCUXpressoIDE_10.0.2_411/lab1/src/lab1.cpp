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
//#include<cstdlib>
//#include <time.h>
//#include "chip.h"
class Dice{
public:
	Dice();
	void show(uint8_t number);




};
Dice::Dice(){



	//PD1
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 9, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 9);
	//PD2
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 29, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 29);

	//PD3
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 9, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 9);
	//PD4
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 10, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 10);
	//PD5
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 16, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 16);
	//PD6
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 3, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 1, 3);
	//PD7
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 0, IOCON_MODE_INACT | IOCON_DIGMODE_EN);
	Chip_GPIO_SetPinDIROutput(LPC_GPIO, 0, 0);
	//Chip_GPIO_SetPinState(LPC_GPIO, port, pin, false);
}
void Dice::show(uint8_t n){


	switch(n){

	case 0:
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, false);
		break;
	case 1:
		//PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		break;
	case 2:
		//PD7&PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		break;
	case 3:
		//PD7&PD3&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		break;
	case 4:
		//PD7&PD5&PD3&PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		break;
	case 5:
		//PD7&PD5&PD3&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		break;
	case 6:
		//PD7&PD6&PD5&PD3&PD2&PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		break;
	case 7:
		//PD7&PD6&PD5&PD3&PD2&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		break;
	default:
		//PD7&PD6&PD5&PD3&PD2&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
		break;

	}



	//n = std::rand() % 7;
	/*if(n==0){
		//PD7&PD6&PD5&PD3&PD2&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, false);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, false);
	}

	if(n==1){
		//PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}
	if(n==2){
		//PD7&PD3
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
	}
	if(n==3){
		//PD7&PD3&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}
	if(n==4){
		//PD7&PD5&PD3&PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
	}
	if(n==5){
		//PD7&PD5&PD3&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}
	if(n==6){
		//PD7&PD6&PD5&PD3&PD2&PD1
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
	}
	if(n==7){
		//PD7&PD6&PD5&PD3&PD2&PD1&PD4
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 0, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 3, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 16, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 29, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 1, 9, true);
		Chip_GPIO_SetPinState(LPC_GPIO, 0, 10, true);
	}*/
}

// TODO: insert other include files here

// TODO: insert other definitions and declarations here

int main(void) {

#if defined (__USE_LPCOPEN)
	// Read clock settings and update SystemCoreClock variable
	SystemCoreClockUpdate();
#if !defined(NO_BOARD_LIB)
	// Set up and initialize all required blocks and
	// functions related to the board hardware
	Board_Init();
	// Set the LED to the state of "On"
	Board_LED_Set(0, true);
#endif
#endif

	// TODO: insert code here


	Dice dice;
	//PC0
	Chip_IOCON_PinMuxSet(LPC_IOCON, 0, 8, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 0, 8);
	//PC1
	Chip_IOCON_PinMuxSet(LPC_IOCON, 1, 6, (IOCON_MODE_PULLUP | IOCON_DIGMODE_EN | IOCON_INV_EN));
	Chip_GPIO_SetPinDIRInput(LPC_GPIO, 1, 6);

	uint8_t counter=1;
	bool button1,button2;

	while(1) {

		button1=Chip_GPIO_GetPinState(LPC_GPIO, 0,8);
		button2=Chip_GPIO_GetPinState(LPC_GPIO, 1,6);
		while(button1){

			dice.show(7);
			button1=Chip_GPIO_GetPinState(LPC_GPIO, 0,8);
			if(!button1){
				dice.show(0);
			}

		}



		while(button2){
			dice.show(0);
			counter++;
			if(counter > 6){

				counter=1;

			}
			button2=Chip_GPIO_GetPinState(LPC_GPIO, 1,6);
			if(!button2){
				//dice.show(0);
				dice.show(counter);
			}



		}

		/*if(!Chip_GPIO_GetPinState(LPC_GPIO, 1,6)) {
    		int counter = std::rand() % 6+1;
    		dice.show(counter);
    	}*/
	}
	return 0 ;
}

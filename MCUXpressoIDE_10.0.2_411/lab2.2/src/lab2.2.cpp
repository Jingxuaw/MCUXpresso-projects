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

#include <string>
#include <cstdio>
#define TICKRATE_HZ1 (1000)	/* 1000 ticks per second */

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
		Chip_IOCON_PinMuxSet(LPC_IOCON, a, b, ((pullup? IOCON_MODE_PULLUP : IOCON_MODE_PULLDOWN) | IOCON_DIGMODE_EN | (inv?  IOCON_INV_EN : 0)));
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
class ITMoutput{
public:
	ITMoutput(){
		ITM_init();
	}
	int print(const char* str){
		return ITM_write(str);
	}
	int print(std::string str){
		const char* cstr = str.c_str();
		return ITM_write(cstr);
	}
	int print(int value){
		char buffer[50];
		snprintf(buffer, 50, "%d ms \n", value);
		return print(buffer);
	}
};



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
	uint32_t sysTickRate;
	SystemCoreClockUpdate();
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ1);

	// TODO: insert code here

	DigitalIoPin sw1(0,17,true,true,true);
	DigitalIoPin sw2(1,11,true,true,true);
	DigitalIoPin sw3(1,9,true,true,true);
	int i=0,j=0,k=0;
	bool press1=false,press2=false,press3=false;
	ITMoutput out;
	while(1){

		if(sw1.Read()){
			i++;
			Sleep(10);
			press1=true;
			if(sw2.Read()){
				j++;
				press2=true;
			}
			if(sw3.Read()){
				k++;
				press3=true;
			}
		}

		if(!sw1.Read()&&press1==true){
			out.print("SW1 is pressed for ");
			out.print(i*10);
			press1 = false;
			i=0;
		}

		if(sw2.Read()){
			j++;
			Sleep(10);
			press2=true;
			if(sw1.Read()){
				i++;
				press1=true;
			}
			if(sw3.Read()){
				k++;
				press3=true;
			}
		}

		if(!sw2.Read()&&press2==true){
			out.print("SW2 is pressed for ");
			out.print(j*10);
			press2 = false;
			j=0;
		}

		if(sw3.Read()){
			k++;
			Sleep(10);
			press3=true;
			if(sw1.Read()){
				i++;
				press1=true;
			}
			if(sw2.Read()){
				j++;
				press2=true;
			}
		}

		if(!sw3.Read()&&press3==true){
			out.print("SW3 is pressed for ");
			out.print(k*10);
			press3=false;
			k=0;
		}


	}
	return 0 ;
}

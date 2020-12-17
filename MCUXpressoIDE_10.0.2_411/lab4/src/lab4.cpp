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
#include <string>
#include <cstdio>
#include <atomic>
#include <ctype.h>
static volatile std::atomic_int counter;
#define TICKRATE_HZ (1000)	/* 1000 ticks  per second */

#ifdef __cplusplus
extern "C" {
#endif
/**
 * @brief Handle interrupt from SysTick timer
 * @return Nothing
 */
void SysTick_Handler(void)
{
	if(counter > 0) counter--;
}
#ifdef __cplusplus
}
#endif
void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}


// TODO: insert other include files here

// TODO: insert other definitions and declarations here

class DigitalIOPin {
public:
	DigitalIOPin(int prt, int pn, bool inp = true, bool pllup = true, bool inv = false):port(prt),pin(pn),invert(inv),state(false) {
		unsigned int setting = 0;
		if(inp){
			setting |= (pllup ? IOCON_MODE_PULLUP : IOCON_MODE_PULLDOWN) | IOCON_DIGMODE_EN | (inv ? IOCON_INV_EN : 0);
			Chip_IOCON_PinMuxSet(LPC_IOCON, prt, pn, setting);
			Chip_GPIO_SetPinDIRInput(LPC_GPIO, prt, pn);
		}
		else {
			setting |= IOCON_MODE_INACT | IOCON_DIGMODE_EN;
			Chip_IOCON_PinMuxSet(LPC_IOCON, prt, pn, setting);
			Chip_GPIO_SetPinDIROutput(LPC_GPIO, prt, pn);
			Chip_GPIO_SetPinState(LPC_GPIO, prt, pn, inv ? true : false);
		}
	}
	bool Read() {
		return Chip_GPIO_GetPinState(LPC_GPIO, port, pin);
	}
	void Write(bool value){
		if(Chip_GPIO_GetPinDIR(LPC_GPIO, port, pin)) {	//if pin is output
			if(invert){
				Chip_GPIO_SetPinState(LPC_GPIO, port, pin, (!value));
				state = !value;
			}
			else{
				Chip_GPIO_SetPinState(LPC_GPIO, port, pin, value);
				state = value;
			}
		}
	}
	void Toggle(){
		if(Chip_GPIO_GetPinState(LPC_GPIO, port, pin)){
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin, false);
			state = !state;
		}
		else{
			Chip_GPIO_SetPinState(LPC_GPIO, port, pin, true);
			state = !state;
		}
	}
	bool GetState(){
		if(invert)
			return !state;
		return state;
	}
private:
	int port;
	int pin;
	bool invert;
	bool state;
};

class ITMclass {
public:
	ITMclass(){
		ITM_init();
	}
	int print(const char* str){
		return ITM_write(str);
	}
	int print(std::string str){
		const char* cstr = str.c_str();
		return ITM_write(cstr);
	}
	int print(unsigned int value){
		char buffer[10];
		snprintf(buffer, 10, "%d", value);
		return print(buffer);
	}
};



int main(void) {

	uint32_t sysTickRate;
	SystemCoreClockUpdate();
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ);
	Board_Init();

	DigitalIOPin sw3(1,9,true,true,true);

	DigitalIOPin greenled(0, 3, false, false, true);


	bool isupper=true;

	Board_UARTPutSTR("\r\nHello, World\r\n");
	Board_UARTPutChar('!');
	Board_UARTPutChar('\r');
	Board_UARTPutChar('\n');
	int c;

	ITMclass itm;

	while(1) {
		//echo back what we receive

		c = Board_UARTGetChar();



		if(c != EOF) {
			if(c == '\n')	Board_UARTPutChar('\r'); // precede line feed with carriage return
			if(c == '\r')	Board_UARTPutChar('\n'); // send line feed after carriage return
			if(isupper)	Board_UARTPutChar(tolower(c));
			else	Board_UARTPutChar(toupper(c));
		}
		if(sw3.Read()) {
			Sleep(200);
			isupper = !isupper;
			if(isupper){
				greenled.Write(true);
			}
			else{
				greenled.Write(false);
			}
		}
	} return 0;
}



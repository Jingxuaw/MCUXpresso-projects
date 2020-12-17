/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#include "board.h"
#include "chip.h"
#include <cr_section_macros.h>
#include "ITM_write.h"
#include <atomic>
#include <string>
#include <cstdio>
#define TICKRATE_HZ1 (1000)	/* 1000 ticks  per second */

static volatile std::atomic_int counter;
extern "C" {
/**
* @brief Handle interrupt from SysTick timer
* @return Nothing
*/
void SysTick_Handler(void)
{
	if(counter > 0) counter--;
}
}

void Sleep(int ms)
{
	counter = ms;
	while(counter > 0) {
		__WFI();
	}
}

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
	uint8_t cursor = 1;
	bool printOK = true;
	std::string message;
	uint32_t sysTickRate;
    SystemCoreClockUpdate();
    Chip_Clock_SetSysTickClockDiv(1);
    sysTickRate = Chip_Clock_GetSysTickClockRate();
    SysTick_Config(sysTickRate / TICKRATE_HZ1);
    Board_Init();

    DigitalIOPin redled(0, 25, false, false, true);
    DigitalIOPin greenled(0, 3, false, false, true);
    DigitalIOPin blueled(1, 1, false, false, true);
    redled.Write(false);
    greenled.Write(false);
    blueled.Write(false);
    DigitalIOPin sw1(0, 17, true, true, true);
    DigitalIOPin sw2(1, 11, true, true, true);
    DigitalIOPin sw3(1, 9 , true, true, true);
    ITMclass itm;
    while(1){
        while(sw1.Read() || sw2.Read() || sw3.Read()){
        	if(sw1.Read()){
        		if(cursor > 1)
        			cursor--;
        		printOK = true;
        	}
        	if(sw2.Read()){
        		switch(cursor){
        		case 1:
        			redled.Toggle();
        			break;
        		case 2:
        			greenled.Toggle();
        			break;
        		case 3:
        			blueled.Toggle();
        			break;
        		default:
        			break;
        		}
        		printOK = true;
        	}
        	if(sw3.Read()){
        	    if(cursor < 3)
        	    	cursor++;
        	    printOK = true;
        	}
        	Sleep(200);
        	}
        	if(printOK){
        		itm.print("Select led:\n");
        		message = "RED\t\t";
        		message += (redled.GetState() ? "ON\t" : "OFF\t");
        		message += ((cursor == 1) ? "<--\n" : "\n");
        		itm.print(message);

        		message = "GREEN\t";
        		message += (greenled.GetState() ? "ON\t" : "OFF\t");

        		message += ((cursor == 2) ? "<--\n" : "\n");
        		itm.print(message);

        		message = "BLUE\t";
        		message += (blueled.GetState() ? "ON\t" : "OFF\t");
        		message += ((cursor == 3) ? "<--\n" : "\n");
        		itm.print(message);
        		printOK = false;
        	}
        }
    return 0;
}

















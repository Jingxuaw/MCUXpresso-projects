
#include "chip.h"
#include<stdint.h>
#include "board.h"
#include <cr_section_macros.h>
#include "ITM_write.h"
#include <string>
#include <cstdio>
#include <atomic>
#include <ctype.h>

class DigitalIoPin {
public:
	DigitalIoPin(int prt, int pn, bool inp = true, bool pllup = true, bool inv = false):port(prt),pin(pn),invert(inv),state(false) {
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
	void write(bool value){
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

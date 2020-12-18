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
#include <alloca.h>
#include <cstring>
#include <cstdio>
#include <atomic>
#include <ctype.h>
static volatile int state;
static volatile std::atomic_int counter;
#define TICKRATE_HZ (1000)	/* 1000 ticks  per second */

extern "C"{
void SysTick_Handler(void)
{
	static int ticks = 0;
	if(counter > 0) counter--;
	ticks++;
	if (ticks > TICKRATE_HZ) {
		ticks = 0;
		state = 1 - state;
	}
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



// TODO: insert other include files here
const int DOT = 1;
const int DASH = 3;
int TIME = 10;
struct MorseCode {
	char symbol;
	unsigned char code[7];
};
const MorseCode ITU_morse[] = {
		{ 'A', { DOT, DASH } }, // A
		{ 'B', { DASH, DOT, DOT, DOT } }, // B
		{ 'C', { DASH, DOT, DASH, DOT } }, // C
		{ 'D', { DASH, DOT, DOT } }, // D
		{ 'E', { DOT } }, // E
		{ 'F', { DOT, DOT, DASH, DOT } }, // F
		{ 'G', { DASH, DASH, DOT } }, // G
		{ 'H', { DOT, DOT, DOT, DOT } }, // H
		{ 'I', { DOT, DOT } }, // I
		{ 'J', { DOT, DASH, DASH, DASH } }, // J
		{ 'K', { DASH, DOT, DASH } }, // K
		{ 'L', { DOT, DASH, DOT, DOT } }, // L
		{ 'M', { DASH, DASH } }, // M
		{ 'N', { DASH, DOT } }, // N
		{ 'O', { DASH, DASH, DASH } }, // O
		{ 'P', { DOT, DASH, DASH, DOT } }, // P
		{ 'Q', { DASH, DASH, DOT, DASH } }, // Q
		{ 'R', { DOT, DASH, DOT } }, // R
		{ 'S', { DOT, DOT, DOT } }, // S
		{ 'T', { DASH } }, // T
		{ 'U', { DOT, DOT, DASH } }, // U
		{ 'V', { DOT, DOT, DOT, DASH } }, // V
		{ 'W', { DOT, DASH, DASH } }, // W
		{ 'X', { DASH, DOT, DOT, DASH } }, // X
		{ 'Y', { DASH, DOT, DASH, DASH } }, // Y
		{ 'Z', { DASH, DASH, DOT, DOT } }, // Z
		{ '1', { DOT, DASH, DASH, DASH, DASH } }, // 1
		{ '2', { DOT, DOT, DASH, DASH, DASH } }, // 2
		{ '3', { DOT, DOT, DOT, DASH, DASH } }, // 3
		{ '4', { DOT, DOT, DOT, DOT, DASH } }, // 4
		{ '5', { DOT, DOT, DOT, DOT, DOT } }, // 5
		{ '6', { DASH, DOT, DOT, DOT, DOT } }, // 6
		{ '7', { DASH, DASH, DOT, DOT, DOT } }, // 7
		{ '8', { DASH, DASH, DASH, DOT, DOT } }, // 8
		{ '9', { DASH, DASH, DASH, DASH, DOT } }, // 9
		{ '0', { DASH, DASH, DASH, DASH, DASH } }, // 0
		{ 0, { 0 } } // terminating entry - Do not remove!
};
// TODO: insert other definitions and declarations here
//
//class Code{
//public:
//	Code(DigitalIOPin* l, DigitalIOPin* p) {
//
//		led=l;
//		pin=p;
//
//	}
//	~Code(){
//
//		delete led;
//		delete pin;
//
//	}
//
//	//	void toggle(unsigned const char *to){
//	//		int i;
//	//		for(i=0;i<sizeof(to); i++){
//	//			if(to[i]==1){
//	//				led->Write(true);
//	//				pin->Write(true);
//	//				Sleep(1000);
//	//				led->Write(false);
//	//				pin->Write(false);
//	//				Sleep(1000);
//	//			}
//	//			if(to[i]==3){
//	//				led->Write(true);
//	//				pin->Write(true);
//	//				Sleep(3000);
//	//				led->Write(false);
//	//				pin->Write(false);
//	//				Sleep(1000);
//	//			}
//	//			if(to[i]==7){
//	//				led->Write(true);
//	//				pin->Write(true);
//	//				Sleep(7000);
//	//				led->Write(false);
//	//				pin->Write(false);
//	//				Sleep(1000);
//	//			}
//	//
//	//		}
//	//	}
//
//
//	void send(char c[]){
//		int i,j,k;
//		for (i=0;i<c[i];i++){
//			if(c[i]!='/0'){
//				if((c[i]>='A'&&c[i]<'Z') || (c[i]>='a'&&c[i]<='z')){
//					//c[i]=toupper(c[i]);
//					for(int j = 0; ITU_morse[c[i] - 'A'].code[j] != 0; j++){
//						for(int k = 0; k < ITU_morse[c[i] - 'A'].code[j]; k++){
//							//toggle(ITU_morse[k].code);
//							led->Write(true);
//							pin->Write(true);
//							Sleep(TIME);
//						}
//						led->Write(false);
//						pin->Write(false);
//						Sleep(TIME);
//					}
//
//				}
//				else if(c[i]>='0'&&c[i]<='9'){
//					for(int j = 0; ITU_morse[c[i] + 26].code[j] != 0; j++){
//						for(int k = 0; k < ITU_morse[c[i] + 26].code[j]; k++){
//							//toggle(ITU_morse[k].code);
//							led->Write(true);
//							pin->Write(true);
//							Sleep(TIME);
//
//						}
//					}
//					led->Write(false);
//					pin->Write(false);
//					Sleep(TIME);
//
//				}
//			}
//
//			else if(c[i]==' '){
//				for(int j = 0; ITU_morse['X' - 'A'].code[j] != 0; j++) {		//send X
//					for(int k = 0; k < ITU_morse['X' - 'A'].code[j]; k++) {
//						//toggle(ITU_morse[k].code);
//						led->Write(true);
//						pin->Write(true);
//						Sleep(TIME);
//
//					}
//
//				}
//				led->Write(false);
//				pin->Write(false);
//				Sleep(1000);
//
//			}
//
//			if(c[i]==' '){
//				if(c[i+1]==' '){
//					Sleep(7*TIME);
//
//				}
//				else if(c[i]!='\0'){
//					Sleep(3*TIME);
//				}
//			}
//
//
//		}
//	}
//
//	void send(std::string s){
//		char ch[81];
//		std::strncpy(ch, s.c_str(), 81);
//		this->send(ch);
//
//
//	}
//
//
//
//private:
//	DigitalIOPin *led;
//	DigitalIOPin *pin;
//
//};

class MorseSender {
public:
	MorseSender(DigitalIOPin *ld, DigitalIOPin *dcr) {
		led = ld;
		pin = dcr;
	}
	void Send(char message[]) {

		for(int i = 0; message[i] != '\0'; i++) {			//scan every letter
			if((message[i] >= 'a' && message[i] <= 'z') || (message[i] >= 'A' && message[i] <= 'Z')) {
				message[i] = toupper(message[i]);

				for(int j = 0; ITU_morse[message[i] - 'A'].code[j] != 0; j++) {		//scan every code in letter
					for(int k = 0; k < ITU_morse[message[i] - 'A'].code[j]; k++) {
						led->Write(true);
						pin->Write(true);
						Sleep(TIME);
					}
					led->Write(false);
					pin->Write(false);
					Sleep(TIME);
				}
			}
			else if(message[i] >= '0' && message[i] <= '9') {


				for(int j = 0; ITU_morse[message[i] - '0' + 25].code[j] != 0; j++) {
					for(int k = 0; k < ITU_morse[message[i] - '0' + 25].code[j]; k++) {
						led->Write(true);
						pin->Write(true);
						Sleep(TIME);
					}
					led->Write(false);
					pin->Write(false);
					Sleep(TIME);
				}
			}
			else if(message[i] != ' ') {

				for(int j = 0; ITU_morse['X' - 'A'].code[j] != 0; j++) {		//send X
					for(int k = 0; k < ITU_morse['X' - 'A'].code[j]; k++) {
						led->Write(true);
						pin->Write(true);
						Sleep(TIME);
					}
					led->Write(false);
					pin->Write(false);
					Sleep(TIME);
				}
			}
			if(message[i] != ' ') {
				if(message[i+1] == ' ')
					Sleep(6*TIME);
				else if(message[i+1] != '\0') Sleep(3*TIME);
			}
		}
	}
	void Send(std::string message) {
		char cmess[81];
		std::strncpy(cmess, message.c_str(), 81);
		this->Send(cmess);
	}

	~MorseSender() {
		delete led;
		delete pin;
	}
private:
	DigitalIOPin *led;
	DigitalIOPin *pin;
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

	DigitalIOPin led(0,25, false, true, true);
	DigitalIOPin pin(0, 8, false, true, false);
	MorseSender sender(&led, &pin);
	char message[81];
	char sendm[81];
	int c;
	int t;
	int i = 0;
	int j=0;
	std::string str = "send ";
	while(1) {
		c = Board_UARTGetChar();
		for(i=0;i<81;i++){
			if(message[0]=='s'){

				if(message[1]=='e'){

					if(message[2]=='n'){

						if(message[3]=='d'){
							if(message[4]==' '){
								for(j=0;j<81-5;j++){
									sendm[j]=message[i+5];

								}

							}
						}
					}
				}
			}
		}

		for(i=0;i<81;i++){
					if(message[0]=='s'){

						if(message[1]=='e'){

							if(message[2]=='n'){

								if(message[3]=='d'){
									if(message[4]==' '){
										for(j=0;j<81-5;j++){
											sendm[j]=message[i+5];

										}

									}
								}
							}
						}
					}
				}




		while(i <= 80 && c != '\r') {
			if(c != EOF) {

				Board_UARTPutChar(c);
				message[i] = c;
				sendm[j] = c;
			    j++;
				i++;




				//    			if(message[0]=='w'&&message[1]=='p'&&message[2]=='m'&&message[3]==' '){
				//    				Board_UARTPutSTR("You can set the time unit ");
				//    				TIME=Board_UARTGetChar();
				//    				sendm[i]=message[i+4];
				//    				pwm 50
				//					1000/50
				//    			}
				//    			else if(message[0]=='s'&&message[1]=='e'&&message[2]=='n'&&message[3]=='d'&&message[4]==' '){
				//    				for (j=0;j<i;j++){
				//    					sendm[j]=message[i+5];
				//    				}
				//
				//    				send hello
				//    			}
				//
				//    			else if(message[0]=='s'&&message[1]=='e'&&message[2]=='t'&&message[3]==' '){
				//    				Board_UARTPutSTR("The wpm setting is ");
				//    				Board_UARTPutChar(TIME/10);
				//    				sendm[i]=message[i+4];
				//    			}
				//    			else{
				//    				//Board_UARTPutSTR("Invalid command ");
				//    			}

			}
			c = Board_UARTGetChar();
		}
		if(c == '\r')	Board_UARTPutChar('\n'); // send line feed after carriage return
		message[i] = '\0';
		Board_UARTPutSTR(sendm);
		sender.Send(sendm);
		i = 0;
	}
	return 0 ;
}

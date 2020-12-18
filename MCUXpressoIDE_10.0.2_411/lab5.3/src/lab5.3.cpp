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
#include "LiquidCrystal.h"
#include "ritimer_15xx.h"
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

	//Chip_Clock_GetSystemClockRate();
	uint32_t sysTickRate;
	SystemCoreClockUpdate();
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();
	SysTick_Config(sysTickRate / TICKRATE_HZ);
	Board_Init();
	Chip_RIT_Init(LPC_RITIMER); // initialize RIT (enable clocking etc.)

	DigitalIoPin rs(0,8,false,false,false);
	DigitalIoPin en(1,6,false,false,false);
	DigitalIoPin d4(1,8,false,false,false);
	DigitalIoPin d5(0,5,false,false,false);
	DigitalIoPin d6(0,6,false,false,false);
	DigitalIoPin d7(0,7,false,false,false);

	DigitalIoPin sw1(0,17,true,true,true);
	DigitalIoPin sw2(1,11,true,true,true);
	DigitalIoPin sw3(1,9,true,true,true);

	LiquidCrystal lcd(&rs, &en, &d4, &d5, &d6, &d7);
	// configure display geometry
	lcd.begin(16, 2);
	// set the cursor to column 0, line 1
	// (note: line 1 is the second row, since counting begins with 0):
	lcd.setCursor(0, 1);
	// Print a message to the LCD.
	//lcd.write('A');
	//lcd.print("B1");
	ITMclass itm;
	bool pressed=false;


	while(1) {

		lcd.clear();
		lcd.setCursor(0,0);
		lcd.print("B1");
		lcd.setCursor(6,0);
		lcd.print("B2");
		lcd.setCursor(10,0);
		lcd.print("B3");

		if(sw1.Read()){

			itm.print("B1\n");
			itm.print("DOWN");
			lcd.setCursor(0,1);
			lcd.print("DOWN");

		}
		else{

			itm.print("B1\n");
			itm.print("UP");
			lcd.setCursor(0,1);
			lcd.print("UP");


		}
		if(sw2.Read()){

			itm.print("B2\n");
			itm.print("DOWN");
			lcd.setCursor(6,1);
			lcd.print("DOWN");

		}
		else{

			itm.print("B2\n");
			itm.print("UP");
			lcd.setCursor(6,1);
			lcd.print("UP");

		}
		if(sw3.Read()){

			itm.print("B3\n");
			itm.print("DOWN");
			lcd.setCursor(10,1);
			lcd.print("DOWN");
		}
		else{

			itm.print("B3\n");
			itm.print("UP");
			lcd.setCursor(10,1);
			lcd.print("UP");

		}
		delayMicroseconds(200000);

	}
	return 0;
}



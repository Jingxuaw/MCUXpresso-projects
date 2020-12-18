
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
#include <ctime>
#include <mutex>
#define TICKRATE         	(1000)	//no. ticks per second

/*******STCTRL bits*******/
#define SBIT_ENABLE     0 //This bit is used to enable/disable the systick counter. 0-Disable the systick timer. 1-Enables the systick timer.
#define SBIT_TICKINT    1 //This bit is used to enable/disable the systick timer interrupt. When enabled the SysTick_Handler ISR will be called. 0-Disable the systick timer Interrupt. 1-Enables the systick timer Interrupt.
#define SBIT_CLKSOURCE  2 //This bit is used to select clock source for System Tick timer. 0-The external clock pin (STCLK) is selected. 1-CPU clock is selected.

/* Systick Register address, refer datasheet for more info */
#define STCTRL      (*( ( volatile unsigned long *) 0xE000E010 ))
#define STRELOAD    (*( ( volatile unsigned long *) 0xE000E014 ))
#define STCURR      (*( ( volatile unsigned long *) 0xE000E018 ))
// TODO: insert other include files here

// TODO: insert other definitions and declarations here
class Imutex {
public:
	Imutex() : enable(false) {
	}
	~Imutex() {
	}
	void lock() {
		enable = (__get_PRIMASK() & 1) == 0;
		__disable_irq();
	}
	void unlock() {
		if(enable) {
			enable = false;
			__enable_irq();
		}
	}
private:
	bool enable;
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


class RealTimeClock {
public:     // You must also provide a variant with systick rate and current time
	RealTimeClock(int ticksPerSecond):guard(){

		hour=23;
		min=58;
		sec=35;
		tick = 0;
		ticksPsec = ticksPerSecond;

	}
	void gettime(tm &now){

		std::lock_guard<Imutex> lock(guard);

		now.tm_hour=hour;
		now.tm_min=min;
		now.tm_sec=sec;

	}


	void Tick(){
		this->tick++;
		if(this->tick >= ticksPsec) {
			this->tick = 0;
			this->sec++;
			if(this->sec >= 60) {
				this->sec = 0;
				this->min++;
				if(this->min >= 60) {
					this->min = 0;
					this->hour++;
					if(this->hour >= 24) {
						this->hour = 0;
					}
				}
			}
		}
	}

private:
	volatile int hour;
	volatile int min;
	volatile int sec;
	volatile int tick;
	int ticksPsec;
	Imutex guard;
};

static RealTimeClock *rtc;
extern "C" {
void SysTick_Handler(void) {
	static int ticks = 0;
	if(rtc != NULL)
		rtc->Tick();
	ticks++;
	if (ticks > TICKRATE) {
		ticks = 0;
	}
}
}

int main(void) {

	//Chip_Clock_GetSystemClockRate();
	uint32_t sysTickRate;
	SystemCoreClockUpdate();
	Chip_Clock_SetSysTickClockDiv(1);
	sysTickRate = Chip_Clock_GetSysTickClockRate();

	Board_Init();
	Chip_RIT_Init(LPC_RITIMER); // initialize RIT (enable clocking etc.)

	STRELOAD=sysTickRate/TICKRATE-1;

	/* Enable the Systick, Systick Interrup and select CPU Clock Source */
	STCTRL = (1<<SBIT_ENABLE) | (1<<SBIT_TICKINT) | (1<<SBIT_CLKSOURCE);

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
	RealTimeClock rtclock(TICKRATE);
	rtc = &rtclock;
	lcd.setCursor(2, 0);
	// Print a message to the LCD.
	lcd.write('A');

	ITMclass itm;
	tm now;
	char h[10],m[10],s[10];


	while(1) {


		lcd.clear();

		rtc->gettime(now);
		itm.print(now.tm_hour);
		itm.print(":");
		itm.print(now.tm_min);
		itm.print(":");
		itm.print(now.tm_sec);
		itm.print("\n");

		sprintf(h, "%d", now.tm_hour);
		sprintf(m, "%d", now.tm_min);
		sprintf(s, "%d", now.tm_sec);

		lcd.setCursor(0,1);
		lcd.print(h);
		lcd.setCursor(3,1);
		lcd.print(":");
		lcd.setCursor(5,1);
		lcd.print(m);
		lcd.setCursor(8,1);
		lcd.print(":");
		lcd.setCursor(10,1);
		lcd.print(s);

		delayMicroseconds(1000000);

	}
	return 0;
}



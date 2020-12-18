/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
 */

#include "chip.h"
#include "board.h"
#include "ITM_write.h"
#include "ritimer_15xx.h"
#include <cr_section_macros.h>
#include<stdint.h>
#include <string>
#include <cstdio>

#include "BarGraph.h"
// TODO: insert other include files here

// TODO: insert other definitions and declarations here
#define TICKRATE_HZ (1000)	/* 100 ticks per second */
static volatile uint32_t ticks;
static volatile uint32_t counter;
int ishigher= -1;  //-1 for lower than the sensor value, +1 for higher than the sensor value, 0 for equal to the senser value
static volatile uint32_t blink_rate=500;  //ms
extern "C" {

void SysTick_Handler(void)
{
	static uint32_t count;

	/* Every 1/2 second */
	count++;
	if (count >= blink_rate) {
		count = 0;
		if(ishigher < 0)	{
			Board_LED_Toggle(2);
			Board_LED_Set(1, false);
			Board_LED_Set(0, false);
		}
		else if(ishigher > 0) {
			Board_LED_Toggle(0);
			Board_LED_Set(1, false);
			Board_LED_Set(2, false);
		}
		else {
			Board_LED_Set(1, true);
			Board_LED_Set(0, false);
			Board_LED_Set(2, false);
		}
	}

	ticks++;

	if(counter > 0) counter--;
}

} // extern "C"


// returns the interrupt enable state before entering critical section
bool enter_critical(void)
{
	uint32_t pm = __get_PRIMASK();
	__disable_irq();
	return (pm & 1) == 0;
}

// restore interrupt enable state
void leave_critical(bool enable)
{
	if(enable) __enable_irq();
}

// Example:
// bool irq = enter_critical();
// Change variables that are shared with an ISR
// leave_critical(irq);

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

void Sleep(uint32_t time)
{
	counter = time;
	while(counter > 0) {
		__WFI();
	}
}

int main(void) {
    SystemCoreClockUpdate();
    Board_Init();


    /* Setup ADC for 12-bit mode and normal power */
	Chip_ADC_Init(LPC_ADC0, 0);

	/* Setup for maximum ADC clock rate */
	Chip_ADC_SetClockRate(LPC_ADC0, ADC_MAX_SAMPLE_RATE);

	/* For ADC0, sequencer A will be used without threshold events.
	   It will be triggered manually, convert CH8 and CH10 in the sequence  */
	Chip_ADC_SetupSequencer(LPC_ADC0, ADC_SEQA_IDX, (ADC_SEQ_CTRL_CHANSEL(8) | ADC_SEQ_CTRL_CHANSEL(10) | ADC_SEQ_CTRL_MODE_EOS));

	/* For ADC0, select analog input pin for channel 0 on ADC0 */
	Chip_ADC_SetADC0Input(LPC_ADC0, 0);

	/* Use higher voltage trim for both ADC */
	Chip_ADC_SetTrim(LPC_ADC0, ADC_TRIM_VRANGE_HIGHV);

	/* Assign ADC0_8 to PIO1_0 via SWM (fixed pin) and ADC0_10 to PIO0_0 */
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_8);
	Chip_SWM_EnableFixedPin(SWM_FIXED_ADC0_10);

	/* Need to do a calibration after initialization and trim */
	Chip_ADC_StartCalibration(LPC_ADC0);
	while (!(Chip_ADC_IsCalibrationDone(LPC_ADC0)));

	/* Clear all pending interrupts and status flags */
	Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));

	/* Enable sequence A completion interrupts for ADC0 */
	Chip_ADC_EnableInt(LPC_ADC0, ADC_INTEN_SEQA_ENABLE);
	/* We don't enable the corresponding interrupt in NVIC so the flag is set but no interrupt is triggered */

	/* Enable sequencer */
	Chip_ADC_EnableSequencer(LPC_ADC0, ADC_SEQA_IDX);

	/* Configure systick timer */
	SysTick_Config(Chip_Clock_GetSysTickClockRate() / TICKRATE_HZ);
	Chip_RIT_Init(LPC_RITIMER);

	DigitalIoPin RS(0, 8, false, false, false);
	DigitalIoPin EN(1, 6, false, false, false);
	DigitalIoPin D4(1, 8, false, false, false);
	DigitalIoPin D5(0, 5, false, false, false);
	DigitalIoPin D6(0, 6, false, false, false);
	DigitalIoPin D7(0, 7, false, false, false);

	LiquidCrystal lcd(&RS, &EN, &D4, &D5, &D6, &D7);
	lcd.begin(16, 2);
	BarGraph bargraph(lcd, 50);
	uint32_t raw_nob;
	uint32_t d_nob;
	uint32_t raw_sen;
	uint32_t d_sen;
	int blink, n = 1, sum = 0, nominal = 0;			//n: to calculate nominal value
	int bar_nob, bar_sen;
	bool irq;
	char str[80], str_nob[4], str_sen[4];
	ITMclass itm;
	while(1) {
		Chip_ADC_StartSequencer(LPC_ADC0, ADC_SEQA_IDX);
		// poll sequence complete flag
		while(!(Chip_ADC_GetFlags(LPC_ADC0) & ADC_FLAGS_SEQA_INT_MASK));
		// clear the flags
		Chip_ADC_ClearFlags(LPC_ADC0, Chip_ADC_GetFlags(LPC_ADC0));
		// get data from ADC channels
		raw_nob = Chip_ADC_GetDataReg(LPC_ADC0, 8); // raw value
		d_nob = ADC_DR_RESULT(raw_nob); // ADC result with status bits masked to zero and shifted to start from zero
		raw_sen = Chip_ADC_GetDataReg(LPC_ADC0, 10);
		d_sen = ADC_DR_RESULT(raw_sen);
		blink = d_nob - d_sen;
		if(n <= 300) {
			sum += d_sen;
			n++;
		}
		nominal = sum/(n-1);
		irq = enter_critical();
		if(blink > 30)	ishigher = -1;
		else if(blink < -30) ishigher = 1;
		else ishigher = 0;
		leave_critical(irq);

		blink += 350;
		if(blink < 0) blink = -blink;
		blink = 2100 - blink;
		if(blink < 0) blink = -blink;
		irq = enter_critical();
		blink_rate = blink;
		leave_critical(irq);

		bar_nob = 50*d_nob/4095;
		sprintf(str_nob, "%04d", d_nob);
		lcd.setCursor(0, 0);
		lcd.print(str_nob);
		bargraph.draw(bar_nob);

		bar_sen = ((int)d_sen - nominal)/2 + 25;
		sprintf(str_sen, "%04d", d_sen);
		lcd.setCursor(0, 1);
		lcd.print(str_sen);
		bargraph.draw(bar_sen);

		sprintf(str, "sens = %lu   nob = %lu   blink rate = %d\n", d_sen, d_nob, blink);
		itm.print(str);

		Sleep(100);

	}

    return 0 ;
}




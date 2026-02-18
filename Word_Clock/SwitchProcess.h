/* 
count renamed to pressedCount
added releasedCount

	This program demonstrates Switch Processing for multiple for hardware switches and ADC switch ladder.
	Tested on ATMega328p and ATTiny85
	Only processes one switch at a time.

SwitchProcess.h
needs to be in a timer loop of 1ms
see demo: SwitchProcess_library_demo
****************************************************************************
- debounces, auto repeat, long press, toggle, double press, release, clockTick functions

- does noise & spike filtering and debouncing for press and release
- counts switch presses
- does not use any interrupts, pins or ADC
- auto repeat based on rate of running scan function.

Put sw.scan(swValue) in a timing loop of 1khz, 2.2-5.6uS
 - can use pin change ISR to get swValue but switch needs to be debounced in hardware, and only switchNumber works

Function definitions:
	SwitchProcess(swValueOpen, debounceCount = 10, repeatRate = 50, repeatStartDelay = 800, doublePressDelay=300, longPressDelay=1000);
	_debounceCount = loops before reading input.
	use 0 = no debounce for ISR, will need debounce caps else multiple interrupts

	sw.scan(input);			//scan, recommend 1ms

	sw.switchNumber();		// returns switch number, valid only if SwitchPressedCount() is not zero
	sw.pressedCount();		// count is reset when read
	sw.longPress();

 === How to use ====================================================================

#include "SwitchProcess.h"				// library

#define SW0_VALUE	0b00100000		// mask, 0 is the active state for this switch
#define SW1_VALUE	0b00010000		// mask, 0 is the active state for this switch
#define SW2_VALUE	0b00000000		// mask, 0 is the active state for this switch (both switches pressed)
#define SWITCH_OPEN 0b00110000		// mask, 11 is the inactive state mask, compare value read from sw.switchNumber() against this constant
SwitchProcess sw(SWITCH_OPEN);			// instigate
//SwitchProcess sw(SWITCH_OPEN, 0);	// no debounce
//SwitchProcess sw(SWITCH_OPEN, 10, 50, 800, 300, 1000);

//setup
	PORTD |= SW0_VALUE | SW1_VALUE;			// pur, use mask constants

//in timing loops.
// === for analog switches ===============================================
	uint16_t adcValue1 = analog.read_startConversion(1);	// read adc
	uint8_t swValue1 = swDecode1.decode(adcValue1);			// decode ADC to a Switch Value
	sw.scan(swValue1);										// debounce and count
// =======================================================================
// or
// === for hw switches with pur ==========================================
ISR(PCINT2_vect) {								// PCINT0_vect=portB PCINT1_vect=portC PCINT2_vect=portD
	_interrupt_count++;							// just an counter to count interrupts (for pin change interrupts)
	uint8_t read_pin = PIND & SWITCH_OPEN;		// only look at the SWITCH_OPEN bits
	sw.scan(read_pin);							// debounce and count switch events
}
// =======================================================================

//in display loop
		uint8_t swValue = sw.switchNumber();
		uint8_t longPress = sw.longPress();
		uint8_t doublePress = sw.doublePress();

		if (swValue == SW0_VALUE) {
			sw0pressed += sw.pressedCount();
			rep0 += sw.pressedCountRepeat();
			sw0released += sw.releasedCount();
		}

		if (swValue == SW1_VALUE) {
			sw1pressed += sw.pressedCount();
			sw1released += sw.releasedCount();
			rep1 += sw.pressedCountRepeat();
		}

		if (swValue == SW2_VALUE) {
			sw2released += sw.releasedCount();
			sw2pressed += sw.pressedCount();
		}
==================================================================================== */

#ifndef SWITCHPROCESS_H_
#define SWITCHPROCESS_H_

#include <avr/io.h>

class SwitchProcess {

public:
	//swValueOpen, debounceCount, repeatRate, repeatStartDelay, doublePressDelay, longPressDelay
	SwitchProcess(
		uint8_t  swValueOpen,
		uint8_t  debounceCount		=	10,
		uint8_t  repeatRate			=	50,
		uint16_t repeatStartDelay	=	800,
		uint16_t doublePressDelay	=	300,
		uint16_t longPressDelay		=	1000);

	void	scan(uint8_t swValue);	// main function, this increments clockTick and processes value of swValue

	// get functions for swValue
	uint8_t	switchNumber();			// switch number
	uint8_t	pressedCount();			// counts switch presses, resets when read
	uint8_t	releasedCount();		// counts switch releases, resets when read
	uint8_t	pressedCountRepeat();	// count, resets when read
	uint8_t	doublePress();			// count, resets when read
	uint8_t	longPress();			// return 1 if switch pressed a long time, DOES NOT reset when read, resets when sw released
	uint8_t	toggle();				// return 0 or 1 for toggleState variable, does not care what switch is pressed

	uint16_t scanTicks();				// just exposing clockTick (16 bit)

private:
	uint8_t  _swValueOld;			// scan state
	uint8_t	 _swValueValid;			// scan state
	uint8_t	 _isActive;				// flag to indicate further processing required
	uint8_t	 _swValueDebounced;		// flag to indicate switch value is valid

	uint16_t _repeatStartDelay_16;	// init variable	variables below are initialized by init
	uint8_t  _repeatRate;			// init variable
	uint16_t _longPressDelay_16;	// init variable
	uint16_t _doublePressDelay_16;	// init variable, loops less then this is flagged as double press
	uint8_t  _debounceDelay;		// init variable, wait this amount before reading value, 0=no loop, 1=one loop delay
	uint8_t	 _swValueOpen;			// init variable, switch value when no switch pressed

	uint8_t	_toggleState;				// for toggle function
	uint8_t	_pressedCounter;			// for pressedCount function
	uint8_t	_releasedCounter;			// for releasedCount function
	uint8_t	_pressedCounterRepeat;		// for pressedCountRepeat function
	uint8_t	_doubleState;				// for doublePress()
	uint8_t	_isLongPress;				// for longPress()
	uint8_t	_repeatRateCounter;

	uint16_t _scanTicks_16;				// all counters are referenced off this counter
	uint16_t _timeLastActivity16;		// resets to 0, on switch activity
	uint16_t _timeLastValidActivity16;	// resets to 0, on switch activity
	uint16_t _timeOfLastPress16;		// time since last switch activity, needs to be saved between function calls
};

#endif /* SWITCHPROCESS_H_ */
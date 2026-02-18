// Library - reads ADC

#include "ADC_read.h"
#include <avr/io.h>

ADC_readClass::ADC_readClass(void){} //this is the constructor, also it is extern, no parameters allowed

#if ( defined (__AVR_ATmega328P__) ||  defined (__AVR_ATtiny85__) )
//	# warning "okay, ADC_read.cpp (__AVR_ATmega328P__) or (__AVR_ATtiny85__) detected."

void ADC_readClass::init(uint8_t aref, uint8_t divisionFactor, uint8_t mux) {		// manual triggered mode
	uint8_t prescaler = 0;
	while(divisionFactor>1) {
		prescaler++;
		divisionFactor = divisionFactor>>1;
	}
	ADMUX	= aref<<6 | (aref & 4)<<2 | mux;	// set Voltage Reference Selection and initial MUX channel
	ADCSRA	= 1<<ADEN | prescaler;				// ADC Enable + prescaler
	ADCSRA |= 1<<ADSC;							// start a conversion
}
#else
	# warning "Unknown device."
#endif

// set MUX address
void ADC_readClass::setMux(uint8_t mux) {
	uint8_t muxTemp = ADMUX & 0xf0;		// update ADMUX with new MUX channel (0-15)
	ADMUX = muxTemp | mux;
}

// set MUX address and start a conversion
void ADC_readClass::setMux_startConversion(uint8_t mux) {
	uint8_t muxTemp = ADMUX & 0xf0;		// update ADMUX with new MUX channel (0-15)
	ADMUX = muxTemp | mux;

	while (ADCSRA & (1<<ADSC));			// wait for previous conversion to complete, don't need this conversion
	ADCSRA |= 1<<ADSC;					// start a conversion now
}

/*Read previous conversion when completed,  MUX and trigger now conversion (2.7uS or 110uS).
The first call to this function will return the mux selected in init (default ADC0).
Note: conversion occurs in background, ADC inputs may require decoupling to reduce noise from active port activity. */
uint16_t ADC_readClass::read_startConversion(uint8_t mux) {
	while (ADCSRA & (1<<ADSC));		//	wait for previous conversion to complete
	uint16_t adcData = ADC;			//	read ADC registers

	uint8_t muxTemp = ADMUX & 0xf0;	// update ADMUX with new MUX channel (0-15)
	ADMUX = muxTemp | mux;

	ADCSRA |= 1<<ADSC;				// start a conversion now
	return adcData;
}

/*Wait for current conversion to complete, start new conversion with new MUX channel, wait 116uS, read ADC.
This function is less noisy because CPU is stuck in tight wait loop while conversion is in progress,
and all I/O ports are frozen during conversion time. */
uint16_t ADC_readClass::startConversion_read(uint8_t mux) {
	uint8_t muxTemp = ADMUX & 0xf0; // update ADMUX with new MUX channel (0-15)
	ADMUX = muxTemp | mux;

	while (ADCSRA & (1<<ADSC));		// wait for previous conversion to complete, don't need this conversion

	ADCSRA |= 1<<ADSC;				// start a conversion now with new MUX values
	while (ADCSRA & (1<<ADSC));		// wait for conversion to complete

	return ADC;						// read ADC registers
}

void ADC_readClass::digitalDisable(uint8_t mask) {
	DIDR0 = mask;					// page 257
}

#if defined (__AVR_ATtiny85__)
void ADC_readClass::bipolarInputMode(uint8_t modeState) {
	if (modeState)
		ADCSRB |= 1<<BIN;		// turn on BIN mode
	else
		ADCSRB &= ~(1<<BIN);	// turn off BIN mode
}
#endif

ADC_readClass analog;	//extern
/*  ADC_read.h Library

new function names, same code. Added support for legacy names
Changed init to choose mux channel. because init does first conversion.

Hardware:
Recommend 10nF cap on Aref, this allows Aref to switch to new voltage <500uS
A 100nF will take several ms to settle.

328P aref:
	0=Ext AREF pin
	1=AVcc (default)
	2=Reserved
	3=bandgap 1.1V with cap on AREF pin

TINY85 aref:
	0=Vcc (default)
	1=Ext AREF pin
	2=Internal 1.1V
	3=Reserved
	6=Internal 3.56V
	7=Internal 2.56V with cap on AREF pin

 * new conversions take 13 cycles ADC clock cycles, at 250Khz (div 64) this is about 52us (17k samples/sec)

=== How to use ==============================================
#include <ADC_read.h>			//library
#define  LDR_MASK	1<<PORTC3	// photo resistor

void setup()
	analog.digitalDisable(LDR_MASK);
	analog.init(1, 128, 0);	// initialize ADC converter, AVcc=Vcc, Prescale=128, mux=0 (default)
	analog.init(1, 128);	// defaults to ADC0 mux
//	Division Factor: 2,4,8,16,32,64,128 (128 for 16Mhz, 64 for 8Mhz)
//
// 	328P   aref:
// 		0=Ext AREF pin    1=AVcc (default)
// 		2=Reserved        3=bandgap 1.1V with cap on AREF pin
// 	TINY85 aref:
// 		0=Vcc (default)   1=Ext AREF pin
// 		2=Internal 1.1V   3=Reserved
// 		6=Internal 3.56V  7=Internal 2.56V with cap on AREF pin

	analog.digitalDisable(1<<PB4 | 1<<PB0);	// turn off digital pin to reduce power
											// same as	DIDR0 = 0b00010001;

void loop()
	uint16_t raw = analog.startConversion_read(0);	// slower
	uint16_t raw = analog.read_startConversion(0);	// faster
	uint16_t raw = analogRead(0);	// arduino compatible (define in ADC_read.h Library)
===============================================
// After changing MUX to Band Gap need to wait 450us, else reading will be influenced by previous mux channel.
// worse at lower voltages
		// set mux 14
		// delay 450us
		// read adc
===============================================
*/

#ifndef ADC_READ_H_
#define ADC_READ_H_

#include <avr/io.h>

// support legacy names
// #define ADC_read_then_update_mux(ADC_MUX)	read_startConversion(ADC_MUX)
// #define ADC_read_now(ADC_MUX)				startConversion_read(ADC_MUX)
#define analogRead(ADC_MUX)						analog.startConversion_read(ADC_MUX)

class ADC_readClass {	//extern

public:
	ADC_readClass();

	/* initialize ADC converter
	aref 328: 00= aref off, 01=Vcc, 11=1.1V
	divisionFactor: 125, 250, 500, 1000, 2000		128 prescaler is Arduino for 16Mhz
	mux: 0-15
	*/
	void init(uint8_t aref, uint8_t divisionFactor, uint8_t mux = 0);

	// set MUX address
	void setMux(uint8_t mux);

	// set MUX address and start a conversion
	void setMux_startConversion(uint8_t mux);

	/* Read previous conversion when completed,  MUX and trigger now conversion (2.7uS or 110uS).
	The first call to this function will return the mux selected in init (default mux=ADC0).
	Note: conversion occurs in background, ADC inputs may require decoupling to reduce noise from active port activity. */
	uint16_t read_startConversion(uint8_t mux);

	/* Wait for current conversion to complete, start new conversion with new MUX channel, wait 116uS, read ADC.
	This function is less noisy because CPU is stuck in tight wait loop (port activity frozen) while conversion is in progress,
	and all I/O ports are frozen during conversion time. */
	uint16_t startConversion_read(uint8_t mux);

	// digital input disable, set DIDR0 to mask, eg PORTC1 is b00000001
	// reduce power, page 257
	void digitalDisable(uint8_t mask);

	/* set ATTiny85 to Bipolar Input Mode */
	void bipolarInputMode(uint8_t mask);

private:

};

extern ADC_readClass analog;	//extern

#endif /* ADC_READ_H_ */
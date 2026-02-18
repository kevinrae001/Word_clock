#include "Tone2.h"
// global variables for ISR
uint8_t isSoftwareToggle;

volatile uint32_t TIMER0_ToggleCount;
uint8_t			  TIMER0_pin;
volatile uint8_t *TIMER0_port;

volatile uint32_t TIMER1_ToggleCount;
uint8_t			  TIMER1_pin;
volatile uint8_t *TIMER1_port;

volatile uint32_t TIMER2_ToggleCount;
uint8_t			  TIMER2_pin;
volatile uint8_t *TIMER2_port;

#if defined ENABLE_TIMER0_ISR
#warning "Tone2.cpp using TIMER0 ISR"
#endif

#ifdef ENABLE_TIMER1_ISR
#warning "Tone2.cpp using TIMER1 ISR"
#endif

#ifdef ENABLE_TIMER2_ISR
#warning "Tone2.cpp using TIMER2 ISR"
#endif

static const uint16_t prescalerTable[3][7] = { {1,8, 64, 256, 1024}, {1,8, 64, 256, 1024}, {1,8,32,64,128,256,1024} };
static const uint8_t pin[][2] =			{ {PORTD6, PORTD5}, {PORTB1, PORTB2}, {PORTB3, PORTD3} };
static volatile uint8_t *port[][2] =	{ {&PORTD, &PORTD}, {&PORTB, &PORTB}, {&PORTB, &PORTD} };

//this is the hardware constructor
Tone::Tone(_t_OCRx reg, _t_timerSel timer) {
	isSoftwareToggle = 0;	// global var
	_timer = timer;
	_reg = reg;

	_pin = pin[timer][reg];
	_port = port[timer][reg];
 }

 //this is the software constructor
Tone::Tone(volatile uint8_t *port, uint8_t pin, _t_timerSel timer) {
	isSoftwareToggle = 1;	// global var
	_timer = timer;
	_reg   = 0;				// only use A channel for software burst
	_pin   = pin;
	_port  = port;
}

double Tone::burst(uint16_t freq, uint32_t cycles = 0) {
	// dont allow reinitialization of running tone, this will stall timer.
	switch (_timer) {
		case 0:
			if (TIMER0_ToggleCount > 1)
				return 0;
			break;
		case 1:
			if (TIMER1_ToggleCount > 1)
				return 0;
			break;
		case 2:
			if (TIMER2_ToggleCount > 1)
				return 0;
			break;
	}


	// cycles to toggle count
	// want even number else output will exit high
	uint32_t toggle_count =  cycles * 2;

	if(!isSoftwareToggle) {			// turn on HW timer ports
		uint8_t channel = _timer* 2 + _reg;
		switch (channel) {
			case 0:
				TCCR0A = 1<<COM0A0;	break; // HW toggle OC0A
			case 1:
				TCCR0A = 1<<COM0B0;	break; // HW toggle OC0B
			case 2:
				TCCR1A = 1<<COM1A0;	break; // HW toggle OC1A
			case 3:
				TCCR1A = 1<<COM1B0;	break; // HW toggle OC1B
			case 4:
				TCCR2A = 1<<COM2A0;	break; // HW toggle OC2A
			case 5:
				TCCR2A = 1<<COM2B0;	break; // HW toggle OC2B
		}
	}
	else
		toggle_count++;	// software toggle requires one extra count else output will exit high

	// find best OCR value for the given timer
	// set prescaler
	// set the toggle count
	// then turn on the interrupts
	uint32_t ocr = 0;
	switch (_timer) {
		case 0:
			TIMER0_port = _port;
			TIMER0_pin  =  _pin;
			*(_port-1)  =  1<<TIMER0_pin;	// set DDR to output

			TCCR0A |= 1 << WGM01;			// mode2, CTC
			TCCR0B = 5;						// 1024 prescaler value
			OCR0A  = 255;					// default to lowest value

			for (int i = 0; i < 5 ; i++) {
				_prescaler = prescalerTable[0][i];
				ocr = F_CPU / 2 / freq / _prescaler - 1;
				if (ocr < 256) {
					TCCR0B = i + 1;	// i+1 is the prescaler value
					OCR0A = ocr;
					break;
				}
			}

			TIMER0_ToggleCount = toggle_count;
			TCNT0 = 0;						// sync timer start with function
			OCR0B  = OCR0A;					// initialize it to same value as A, resulting in same interrupt
			GTCCR  = 1<<PSRSYNC;			// prescaler reset for timer0 (8bit) and timer1 (16 bit)
			TIFR0  = 1<< OCF0A;				// must clear any pending interrupts
			TIMSK0 = 1<< OCIE0A;			// turn on interrupts
			break;

		case 1:
			TIMER1_port = _port;
			TIMER1_pin =  _pin;
			*(_port-1) =  1<<TIMER1_pin;	// set DDR to output

//			TCCR1A |= 0;					// need to add HW Toggle
			TCCR1B = 1 << WGM12;			// mode4, CTC, timer stopped
			OCR1A  = 65535;					// default to lowest value

			for (int i = 0; i < 5 ; i++) {
				_prescaler = prescalerTable[1][i];
				ocr = F_CPU /2 / freq / _prescaler - 1;
				if (ocr < 65536) {
					TCCR1B |= i + 1;		// need to add bits, i+1 is the prescaler value
					OCR1A = ocr;
					break;
				}
			}

			TIMER1_ToggleCount = toggle_count;
			TCNT1 = 0;						// sync timer start with function
			OCR1B  = OCR1A;					// initialize it to same value as A, resulting in same interrupt
			GTCCR  = 1<<PSRSYNC;			// prescaler reset for timer0 (8bit) and timer1 (16 bit)
			TIFR1  = 1<< OCF1A;				// must clear any pending interrupts
			TIMSK1 = 1<< OCIE1A;			// turn on interrupts
			break;

		case 2:
			TIMER2_port = _port;
			TIMER2_pin =  _pin;
			*(_port-1) =  1<<TIMER2_pin;	// set DDR to output

			TCCR2A |= 1 << WGM21;			// CTC mode2,  need to add HW Toggle
			TCCR2B = 7;						// 1024 prescaler
			OCR2A  = 255;					// default to lowest value
			OCR2B  = 0;						// initialize it

			for (int i = 0; i < 7; i++) {
				_prescaler = prescalerTable[2][i];
				ocr = F_CPU / 2 / freq / _prescaler - 1;
				if (ocr < 256) {
					TCCR2B = i + 1;			// i+1 is the prescaler value
					OCR2A = ocr;
					break;
				}
			}

			TIMER2_ToggleCount = toggle_count;
			TCNT2  = 0;						// sync timer start with function
			OCR2B  = OCR2A;					// initialize it to same value as A, resulting in same interrupt
			GTCCR  = 1<<PSRASY;				// prescaler reset for timer2 (8bit)
			TIFR2  = 1<< OCF2A;				// must clear any pending interrupts
			TIMSK2 = 1<< OCIE2A;			// turn on interrupts
			break;
	} // switch ends

	return static_cast<double>(F_CPU) / 2 / _prescaler / (ocr+1);
}

double Tone::tone(uint16_t freq, uint32_t duration) {
	// Calculate the toggle count
	// want even number else output will exit with a short pulse
	uint32_t cycles;
	if(duration)
		cycles = freq * duration / 1000;
	else
		cycles = 0;

	return burst(freq, cycles);
}

uint16_t Tone::prescaler() {
	return _prescaler;
}

#ifdef ENABLE_TIMER0_ISR
ISR(TIMER0_COMPA_vect) {	//1.9us 30 clock cycles
	if (TIMER0_ToggleCount != 1) {
		if(isSoftwareToggle)
			*TIMER0_port ^= 1<<TIMER0_pin;	// toggle the pin
	}
	else {
		TCCR0A = 1<<WGM01;		// turn off hardware toggle and release port, keep CTC mode
		TIMSK0 = 0;				// turn off timer0 interrupts
	}
	if (TIMER0_ToggleCount > 1)
		TIMER0_ToggleCount--;
}
#endif

#ifdef ENABLE_TIMER1_ISR
ISR(TIMER1_COMPA_vect) {
	if (TIMER1_ToggleCount != 1) {
		if(isSoftwareToggle)
			*TIMER1_port ^= 1<<TIMER1_pin;	// toggle the pin
	}
	else {
		TCCR1B = 1 << WGM12;	// turn off hardware toggle and release port, keep CTC mode
		TIMSK1 = 0;				// turn off timer2 interrupts
	}
	if (TIMER1_ToggleCount > 1)
		TIMER1_ToggleCount--;
}
#endif

// 0 continuous
// 1 off
#ifdef ENABLE_TIMER2_ISR
ISR(TIMER2_COMPA_vect) {	// 5.369us or 3.31us
	if (TIMER2_ToggleCount != 1) {
		if(isSoftwareToggle)
			*TIMER2_port ^= 1<<TIMER2_pin;	// software toggle
	}
	else {
		TCCR2A = 1 << WGM21;	// turn off hardware toggle and release port, keep CTC mode
		TIMSK2 = 0;				// turn off timer2 interrupts
	}
	if (TIMER2_ToggleCount > 1)
		TIMER2_ToggleCount--;
}
#endif
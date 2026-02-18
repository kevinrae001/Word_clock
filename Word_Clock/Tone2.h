/*  
- fixed output state and jitter
- to do:
-frozen output if tone is in loop
-sometimes output will exit high if previous tone not completed when a new tone is requested.
- HW tone faulty when using 2 tones that use different prescalars
- or short wait times before reinitialization of timer causes pulse in Hw mode!
- burst with 0 duration and SW will result in no sound because of the toggle_count++ in sw mode!
WHERE DO I PUT THE DEFINES????
in tool chain?
#define ENABLE_TIMER0_ISR
#define ENABLE_TIMER1_ISR
#define ENABLE_TIMER2_ISR

Tone2.h AVR library

Function: output square wave on any port using software output toggle or
		  output square wave on timerX port (A/B)

		tone:	returns: (double)frequency
		burst:	returns: (double)frequency

Need to enable ISR for timer in tool chain symbols:
#define ENABLE_TIMER0_ISR
#define ENABLE_TIMER1_ISR
#define ENABLE_TIMER2_ISR

// ======== instigate, hardware toggle ========================
Tone(uint8_t reg, _t_timerSel timer);
Tone tone0(0,	TIMER1);	// timer0:PD6,PD5	timer1: PB1, PB2	timer2: PB3, PD3
Output:
	timer0 PD6
	timer1 PB1
	timer2 PB3 (MOSI)

// ========  instigate, software toggle ========================
//Tone tone1(*port,  pin,    timer);	// software mode
//Tone tone1(&PORTB, PORTB0, TIMER2);	// PORTB0, timer2

tone0.tone(freq, duration);		// hz, ms

Note: OCRxB uses OCRxA ISR

=== how to use ===========================================
#include <avr/io.h>
#include "tone2.h"				// library

Tone tone0(0, TIMER1);			// timer0:PD6,PD5	timer1: PB1, PB2	timer2: PB3, PD3
Tone tone1(&PORTB, PORTB0, TIMER_2);	// PORTB0, timer2

int main(void) {
	tone0.tone(1000, 100);		// hz, ms
	tone1.tone(2000, 500);		// hz, ms
	tone1.tone(2000);			// hz, continuous
}
==========================================================*/

#ifndef Tone_h
#define Tone_h

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

//#include "timming_markers.h"

//#define ENABLE_TIMER0_ISR
//#define ENABLE_TIMER1_ISR
#define ENABLE_TIMER2_ISR

// #define TONE_ACTIVE_LOW_OUTPUT
// #define TONE_COUNT_MODE

typedef enum {
	TIMER_0,
	TIMER_1,
	TIMER_2
}_t_timerSel;

typedef enum {
	A,
	B
}_t_OCRx;

class Tone {
	public:
		Tone(_t_OCRx reg, _t_timerSel timer);	// hardware mode
		Tone(volatile uint8_t *port, uint8_t pin, _t_timerSel timer);	// software mode

		double burst(uint16_t freq, uint32_t cycles);
		double tone(uint16_t freq, uint32_t duration=0);
		uint16_t prescaler();			// returns prescaler value
		void count(uint16_t freq, uint32_t count);

	private:
		uint8_t  _timer;
		uint16_t _prescaler;
		uint8_t _reg;		// timer channel A=0 or B=1
		uint8_t _pin;
		volatile uint8_t *_port;
};

#endif //Tone_h
/* word clock
 * hardwareInterface.h
 * Author: Kevin Rae

 * sw1 connected to PC1
 * sw0 connected to PC0
 * blink LED     to PB5
 * LDR pur 4k7   to PB4
 * LDR           to PC3
 * Piezo         to PC5

 */
#ifndef hardwareInterface_H_
#define hardwareInterface_H_

const uint8_t LDR_MASK_5V     = 1<<PORTB4;						// this port provides 5V for LDR voltage divider
const uint8_t BLINK_PORT_MASK = 1<<PORTB5;						// one port
const uint8_t PIEZO_PORT_MASK = 1<<PORTC5;						// one port
#define  LDR_MASK				1<<PORTC3						// photo resistor
const uint8_t ANNOUNCEMENT_PORT_MASK = 1<<PORTC4 | 1<<PORTC2;	// two ports
const uint8_t SWITCH_PIN_MASK = 1<<PORTC1 | 1<<PORTC0;			// two switches

#define SW0_VALUE	0b00000010		// mask, 0 is the active state for this switch
#define SW1_VALUE	0b00000001		// mask, 0 is the active state for this switch
#define SWITCH_OPEN 0b00000011		// mask, 11 is the inactive state mask, compare value read from sw.switchNumber() against this constant
SwitchProcess sw(SWITCH_OPEN);		// instigate

uint8_t _LEDblinkStateMask;			// this mask is used in LED_Hour_on to turn blink LED on/off

// word and minute index to display LED PORT and DDR
// bit3=D, bit2=C, bit1=B, bit0=A see schematic
// turn on ports (High/Low)
//                             12     1     2     3     4     5     6     7     8     9     10    11
uint8_t  hour_DDR_Array[10] = {0X03,		0X06,		0X05,		0X09,		0X0A,		0X0C};
uint8_t  hour_Port_Array[20]= {0X01,  0x02, 0X02, 0X04, 0X01, 0X04, 0X01, 0X08, 0X02, 0X08, 0X04, 0X08};

//                             A      five  20    quart na    na    half  ten   to    past
uint8_t  led_DDR_Array[10]  = {0X0C,		0X18,		0X00,		0X24,		0xC0};
uint8_t  led_Port_Array[20]=  {0X04,  0x08, 0X08, 0X10, 0X00, 0X00, 0X04, 0X20, 0X40, 0X80};

// initilize timer0, note that timer0 is the millis() timer on Arduino, need to append ISR A & B
// interrupts every 1.024ms
void timer0_Init() {
	OCR0A  = 0;			// interrupts on 0 count
	OCR0B  = 200;		//_interrupts on _brightness value
	TIMSK0 |= 1<<OCIE0A | 1<<OCIE0B;	// add interrupts mask A & B (OVF on by default on Arduino)
}

// turn on pull up resistors for the two switches
void switch_Init() {
	PORTC |= SWITCH_PIN_MASK;	// pur
}

// initialize ADC for LDR and voltage (set ADC clock), turn off digital ports on the ADC channel to reduce power
void ADC_Init() {
	analog.digitalDisable(LDR_MASK);	// disable digital port to save power
	analog.init(1, 250, 3);	// initialize ADC converter, AVcc=Vcc, Prescale=128, mux=3
}

// dim the display if low light or low system voltage (3V)
void brightnessControl() {
	if (_voltageBandGap > POWER_DOWN_VOLTAGE)
	OCR0B = 1;
	else
	if (_lightLevel > DARKNESS_LEVEL_TRIP_POINT)
	OCR0B  = _brightnessNight;
	else
	OCR0B  = _brightnessDay;
}

 void processSwitches() {
	 // switch process, this function process switch presses.
	 // SW0 short press increments hours
	 // SW0 long press toggles chime
	 // SW1 short release increments minutes

	 uint8_t sw0released = 0, sw1Release = 0;
	 static uint8_t sw0long(0);
	 static uint8_t chimeStateChanged = 0;

	 uint8_t swValue = sw.switchNumber();		// get switch number

	 // process switch 0 actions if any, for hours & chime
	 if (swValue == SW0_VALUE) {
		 sw0released += sw.releasedCount();	// keep track of total number of releases
		 sw0long     |= sw.longPress();		// dont reset sw0long when switch released

		 if(sw0long) {						// long switch press
			 if (chimeStateChanged == 0) {	// this is the first long press, do long press once only
				 chimeStateChanged = 1;		// prevent doing long press multiple times
				 _chimeOn = !_chimeOn;
				 Serial.print("Chime is: ");
				 if (_chimeOn) {
					 Serial.println("on");
					 tone2.tone(2600, 60);	// hz, ms
				 }
				 else {
					 Serial.println("off");
					 tone2.tone(800, 200);	// hz, ms
				 }
			 }

		 } else								// long press released, reset state flag
		 chimeStateChanged = 0;

		 if (sw0long && sw0released) {		// if long press, reset short release,
			 sw0long = 0;					// else hour will also count up when doing long press
			 sw0released = 0;
		 }
	 }

	 // process switch 1 actions if any, for minutes
	 if (swValue == SW1_VALUE)
	 sw1Release += sw.releasedCount();

	 // update hours
	 if (sw0released) {
		 _real_hours += sw0released;
		 sw0released = 0;
		 if (_real_hours > 23) {		// new day
			 _real_hours = 0;
			 tone2.tone(2600, 20);	// hz, ms
		 }
		 saveValuesToEEPROM();
		 updateDisplayFlag = 1;		// force display to be updated.

	 }
	 // update minutes
	 if (sw1Release) {
		 _real_minutes += sw1Release;
		 sw1Release = 0;
		 _real_seconds = 0;

		 if (_real_minutes >= 60) _real_minutes = 0;
		 saveValuesToEEPROM();
		 updateDisplayFlag = 1;		// force display to be updated.
	 }
 }

// runs inside timer0 ISR B once per second, increments seconds, minutes, hours
inline void incrementSeconds() {
	if (++_real_seconds == 60) {	// 60
		_real_seconds = 0;
		updateDisplayFlag = 1;			// set flag so that display can be updated
		if (++_real_minutes == 60) {
			_real_minutes = 0;
			if (++_real_hours == 24)
			_real_hours = 0;	// 0-23
		}
	}
}

/*
 Charlieplexed LED display, turn on minutes nad hours
 Look up each _word in array, if it is 1 then turn on matching LED
 Total ports: PD2 - PD7 (6 ports)
 WORDS_MINUTES_START is the index into first _word
 by default also turn on "THE TIME" , "IS NOW" (ANNOUNCEMENT)
 */
inline void wordToLED_driver() {
// turn on minutes LED
	if(announcement) {		// turn on "THE TIME" , "IS NOW" if enabled
		DDRC  |= ANNOUNCEMENT_PORT_MASK;		// make ports output, force all other ports to be inputs
		PORTC |= ANNOUNCEMENT_PORT_MASK;		// turn on "THE TIME" , "IS NOW", active high
	}

	for (uint8_t i = 0; i < 10; i++) {
		if (_word[i + WORDS_MINUTES_START]) {
			DDRD  |= led_DDR_Array[i/2];		// word index to DDRD, enable multiple LEDs
			PORTD |= led_Port_Array[i];			// word index to PORTD, enable multiple LEDs
		}
	}

// turn on hours LED
// also, take into account other devices attached to DDRB/PORTB
// word[0] is 'twelve'
// word[1] is 'one'
	for (uint8_t i = 0; i < 12; i++) {
		if(_word[i]) {
			DDRB  = hour_DDR_Array[i/2] | LDR_MASK_5V | BLINK_PORT_MASK;	// word index to DDRB, enable port direction register
			PORTB = hour_Port_Array[i]  | LDR_MASK_5V | _LEDblinkStateMask;	// word index to PORTB, enable output
			break;
		}
	}
}

inline void turnOffLEDs() {
	PORTC &= ~ANNOUNCEMENT_PORT_MASK;	// turn off "THE TIME" , "IS NOW"

	PORTB = LDR_MASK_5V | _LEDblinkStateMask;			// turn off: hour LEDs & blink LED, keep LDR_MASK_5V pin on

	DDRD  = 0;		// turn off output for minutes LEDs
	PORTD = 0;		// turn off minutes LED, PB0 & PB1 is UART, okay to reset entire port
}

// turn on LEDs, trigger ADC
ISR(TIMER0_COMPA_vect) {
	#define MUX_BANDGAP 14
	#define MUX_ADC3	3

	wordToLED_driver();	// turn on LEDs

	// read light level, covert to 8 bit, PB4 port needs to be high, it's the PUR 4k7
	// alternates between reading BandGap (mux14) and LDR (mux3)
	static uint8_t adcLoop = 3;
	switch (--adcLoop) {
		case 2:
			analog.read_startConversion(MUX_BANDGAP);		// need dummy conversion before reading BandGap
			break;
		case 1:
			_voltageBandGap = analog.read_startConversion(MUX_ADC3);	//  read BandGap voltage, set mux for next conversion
			break;
		case 0:
			_lightLevel = analog.read_startConversion(MUX_BANDGAP)/4;			// raw0 is mux 0, set mux for next conversion
			adcLoop = 3;									// repeat mux loop, 5 cases
			break;
	}
} // ISR ends

// scan switches, increment second counter, generate long term accurate 1 second pulse for blink and calibration
ISR(TIMER0_COMPB_vect) {
	#define BLINK_DURATION 10		// ms
	turnOffLEDs();

	uint8_t read_pin = PINC & SWITCH_OPEN;		// only look at the SWITCH_OPEN bits
	sw.scan(read_pin);							// debounce and count switch events

	// get a true second event from a 1.024ms interrupt
	// every 15625 interrupts is exactly 16 seconds	(15625 * 1.024ms = 16 seconds)
	// factor to get 1 sec from 1.024ms is: 976.5625
	// 15625/16 or 125/128
	// 977 * 9 + 976 * 7 = 15625
	static uint8_t  loop16 = 0;
	static uint16_t secPulse = 0;

	if (secPulse == BLINK_DURATION) _LEDblinkStateMask = BLINK_PORT_MASK;	// turn on Blink LED, 10.24ms

	if (secPulse == 0) {			// closest interrupt to 1 seconds
		incrementSeconds();
		secPulse = 976;				// start another loop count of 976
		_LEDblinkStateMask = 0;		// turn off blink LED

		// loop16 is a MOD16 counter, if loop16 is odd or 0 add 1 extra count to secPulse
		// result every 16 cycles:
		//		secPulseCount = 977 9 times
		//		secPulseCount = 976 7 times
		//		total secPulseCount every 16 cycles is 15625
		 if ( (loop16 & 1) || (loop16 & 0x0f) == 0 )	// true when loop16 is odd or 0
			secPulse++;				// make count 977 instead of 976
		loop16++;

	}
	secPulse--;						// start another 1 second pulse count

} // ISR ends

// turn on each LED, beep
void powerOnSeltTest() {
	for (uint8_t i = 0; i < WORDS_TOTAL; i++) {
		_word[i] = 1;
		_delay_ms(20);
		_word[i] = 0;
	}
	tone2.tone(2600, 30);		// hz, ms
}
#endif /* hardwareInterface_H_ */
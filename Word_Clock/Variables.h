/* word clock
 * Serial_Console.h
 * Author: Kevin Rae
 */

#ifndef VARIABLES_H_
#define VARIABLES_H_

// EEPROM
uint8_t  EEMEM minutes_E =		25;
uint8_t  EEMEM hours_E	 =		 5;
uint8_t  EEMEM chimeOn_E =		 1;
uint8_t  EEMEM brightnessDay_E  = 200;
uint8_t  EEMEM brightnessNight_E = 128;

// rounded time, used to display word time
uint8_t _hours;								// 0 - 11
uint8_t _minutes;							// 0 - 59
uint8_t _seconds;							// 0 - 59
volatile uint8_t	updateDisplayFlag;

// real time variables, updated via timer0 ISR, set via EEPROM at power up
volatile uint8_t _real_hours;				// 0 - 23 24 hour clock
volatile uint8_t _real_minutes;				// 0 - 59
volatile uint8_t _real_seconds;				// 0 - 59

// light level
uint8_t _brightnessDay;						// 0 - 255 updated via EEPROM at power up
uint8_t _brightnessNight;					// 0 - 255 updated via EEPROM at power up
const uint8_t DARKNESS_LEVEL_TRIP_POINT = 100;		// 0 -255
volatile uint8_t _lightLevel;				// 0 - 255,  real time ADC conversion of actual light level

// voltage
volatile uint16_t _voltageBandGap;			// 0 - 1023, real time ADC conversion of BandGap
const uint16_t POWER_DOWN_VOLTAGE = 370;	// about 3V, ADC conversion of BG voltage, value gets higher with lower voltage
const float VOLTAGE_LEVEL_LOW = 4.5;		// trigger point for vow voltage
const float VOLTAGE_FACTOR = 1132.0;		// calibration factor to display volts from ADC conversion

// chime
uint8_t _chimeOn;							// 1=chime on, updated via EEPROM at power up
const uint8_t CHIME_ON_HOUR = 9;
const uint8_t CHIME_OFF_HOUR = 22;

uint16_t _powerOnHours;						// simply counts the hours powered on.  Resets when powered off

// word array
const uint8_t WORDS_TOTAL = 22;
const uint8_t WORDS_MINUTES_START = 12;
uint8_t _word[WORDS_TOTAL];

// http://stackoverflow.com/questions/1088622/how-do-i-create-an-array-of-strings-in-c
 const char *wordsPrint[WORDS_TOTAL] = {"TWELVE","ONE","TWO","THREE","FOUR","FIVE","SIX","SEVEN",
	"EIGHT","NINE","TEN","ELEVEN", "A","FIVE","TWENTY","QUARTER","","","HALF","TEN","TO","PAST"};

#define twelve   _word[0]	// hours on port Pb0 - PB3
#define one      _word[1]
#define two      _word[2]
#define three    _word[3]
#define four     _word[4]
#define fiveH    _word[5]
#define six      _word[6]
#define seven    _word[7]
#define eight    _word[8]
#define nine     _word[9]
#define ten      _word[10]
#define eleven   _word[11]

#define char_A   _word[12]	// minutes on PB0-Pb3
#define fiveM    _word[13]
#define twenty   _word[14]
#define quarter  _word[15]
#define announcement _word[16]	// special, on portc  PC4, PC2
//				 blank[17]
#define half     _word[18]
#define tenM     _word[19]
#define to       _word[20]
#define past     _word[21]

void loadValuesFromEEPROM() {
	_real_minutes		= eeprom_read_byte (&minutes_E);
	_real_hours			= eeprom_read_byte (&hours_E);
	_brightnessDay		= eeprom_read_byte (&brightnessDay_E);
	_brightnessNight	= eeprom_read_byte (&brightnessNight_E);
	_chimeOn			= eeprom_read_byte (&chimeOn_E);
}

void saveValuesToEEPROM() {
	eeprom_update_byte (&minutes_E,			_real_minutes);
	eeprom_update_byte (&hours_E,			_real_hours);
	eeprom_update_byte (&brightnessDay_E,	_brightnessDay);
	eeprom_update_byte (&brightnessNight_E,	_brightnessNight);
	eeprom_update_byte (&chimeOn_E,			_chimeOn);
}

#endif /* VARIABLES_H_ */

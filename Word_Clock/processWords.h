/* word clock
 * processWords.h
 * Author: Kevin Rae
 */

#ifndef PROCESSWORDS_H_
#define PROCESSWORDS_H_

// used in roundTimeToNearest5() function
void roundTimeToNearest5() {
	// change below to round to nearest 5
	// eg: 4:33 --> 4:35, 2:12 --> 2:10
	_minutes = _real_minutes;

	_hours =   _real_hours;
	if (_real_hours > 12)
		_hours -= 12;			// convert 24 hour to 12 hour am/pm

	uint8_t remainder = _minutes % 5;
	if(remainder < 3)
		_minutes -= remainder;
	else
		_minutes+= 5 - remainder;

	if(_minutes == 60) {
		_hours++;
		_minutes = 0;
	}

	if(_hours >= 12)
		_hours = 0;
}

// update _words array with rounded time
void timeToWords() {
	roundTimeToNearest5();

	// clear _words first
	for (uint8_t i = 0; i < WORDS_TOTAL; i++)
		_word[i] = 0;

	announcement = 1;	// turn on THE TIME IS NOW

	switch (_minutes) {
		case 5:
			fiveM   = 1;
			past    = 1;
			break;

		case 10:
			tenM    = 1;
			past    = 1;
			break;

		case 15:
			char_A  = 1;
			quarter = 1;
			past    = 1;
			break;

		case 20:
			twenty  = 1;
			past    = 1;
			break;

		case 25:
			twenty  = 1;
			fiveM   = 1;
			past    = 1;
			break;

		case 30:
			char_A  = 1;
			half    = 1;
			past    = 1;
			break;

		case 35:
			twenty  = 1;
			fiveM   = 1;
			to      = 1;
			break;

		case 40:
			twenty  = 1;
			to      = 1;
			break;

		case 45:
			char_A  = 1;
			quarter = 1;
			to      = 1;
			break;

		case 50:
			tenM    = 1;
			to      = 1;
			break;

		case 55:
			fiveM   = 1;
			to      = 1;
			break;
	}

	if(_minutes > 30)
		_hours++;

	if (_hours >= 12)
		_hours = 0;

	_word[_hours]=1;	// set hours
}

#endif /* PROCESSWORDS_H_ */
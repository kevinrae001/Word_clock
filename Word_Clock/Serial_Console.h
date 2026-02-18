/* word clock
 * Serial_Console.h
 * Author: Kevin Rae
 */ 

#ifndef SERIAL_CONSOLE_H_
#define SERIAL_CONSOLE_H_

uint8_t  _stringComplete;				// tells program when an input string is ready for processing
static char _inputString[65] = "";		// a c-string to hold incoming data
void processInputString(char str[]);
void buildInputString();

// display:   Time now: 9:6:31 AM
void consoleDisplayTime() {
	uint8_t hour = _real_hours;
	uint8_t pm = 0;
	
	if (_real_hours > 12) {
		hour -= 12;			// convert 24 hour to 12 hour am/pm
		pm = 1;
	}
	
	if (_real_hours == 0)
		hour = 12;
		
	Serial.print("Time now: "); Serial.print(hour); Serial.print(":");
	Serial.print(_real_minutes); Serial.print(":");	Serial.print(_real_seconds);
	if (pm) Serial.println(" PM");
	else Serial.println(" AM");
}

// display:   Word Time: THE TIME IS NOW FIVE PAST NINE
void consoleDisplayWordTime() {
	Serial.print("THE TIME IS NOW ");

	uint8_t i = WORDS_MINUTES_START;
	if (_word[i]) {
		Serial.print(wordsPrint[i]);
		Serial.print(" ");
	}
	i +=2;
	if (_word[i]) {
		Serial.print(wordsPrint[i]);
		Serial.print(" ");
	}
	i++;
	if (_word[i]) {
		Serial.print(wordsPrint[i]);
		Serial.print(" ");
	}

	if (_word[WORDS_MINUTES_START+1]) {
		Serial.print(wordsPrint[WORDS_MINUTES_START+1]);
		Serial.print(" ");
	}

	for (uint8_t i = WORDS_MINUTES_START+6; i < WORDS_TOTAL; i++) {
		if (_word[i]) {
			Serial.print(wordsPrint[i]);
			Serial.print(" ");
		}
	}
	// print hour
	for (uint8_t i = 0; i < WORDS_MINUTES_START; i++)
	if (_word[i])
	Serial.print(wordsPrint[i]);

	Serial.println();
}

// with input string str from console update real time hour, minutes, seconds
void setTime(char str[]) {
	uint8_t hours = atoi(str+2);
	uint8_t length = strlen(str);
	uint8_t minutes = atoi(str+4);

	if(hours > 23 || minutes > 59 || length < 4 || str[1] != ' ' ) {
		Serial.println("Invalid time. Enter ? for help.");
		return;
		}
	_real_hours	  =	hours;
	_real_minutes = minutes;
	_real_seconds = 0;
}

// set global variables _brightnessDay or _brightnessNight
void setBrightness(char str[], uint8_t& brightness, const char Day_Night_string[]) {
	uint8_t brightnessPercent = atoi(str+1);
	uint8_t length = strlen(str);
	if(length < 2 || brightnessPercent > 100) {			// error checking
		Serial.println("Invalid entry, enter ? for help.");
		return;
	}

	brightness = brightnessPercent * 255 / 100;
	if (brightness < 255)
	brightness++;									// round up, but dont allow overflow
	Serial.print(Day_Night_string); Serial.print(" brightness set to: ");
	Serial.print(brightnessPercent); Serial.println("%");
}

// display help screen when user enters '?'
void showHelp() {
	Serial.println("================================================");
	Serial.print("t hh mm (24h)         ");
	consoleDisplayTime();

	Serial.print("B nn            Brightness Day: ");
	Serial.print(_brightnessDay*100/255);   Serial.println("%");

	Serial.print("b nn          Brightness Night: ");
	Serial.print(_brightnessNight*100/255); Serial.println("%");

	Serial.print("C/c           Chime (9am-10pm): ");
	if(_chimeOn)
		Serial.println("On");
	else
		Serial.println("Off");

	Serial.println("\n?       This help");
	Serial.println();

	Serial.print("System Voltage:");
	double voltage = VOLTAGE_FACTOR / _voltageBandGap; //(voltage x bandgap)
	Serial.print(voltage); Serial.print("V   ADC value:");Serial.print(_voltageBandGap);
	if(voltage < VOLTAGE_LEVEL_LOW)
		Serial.println("  Weak");
	else
		Serial.println("  Good");
		
	Serial.print("Light Level:   ");
	Serial.print((255-_lightLevel)*100/255); Serial.print("%     ADC value:"); Serial.print(_lightLevel);
	if(_lightLevel > DARKNESS_LEVEL_TRIP_POINT)
		Serial.println("  Dim");
	else
		Serial.println("  Bright");
	
	Serial.print("Power On Hours:"); Serial.println(_powerOnHours);

	Serial.print("Word Time: ");
	timeToWords();
	consoleDisplayWordTime();
	Serial.println("================================================");
}

//setBrightness(char str[], uint8_t& brightness, char day_night[]) {
// process console input string
void processInputString(char str[]) {
	Serial.println();
	char firstChar = str[0];
	switch(firstChar) {
		case 't':   setTime(str);				break;
		case 'B':   setBrightness(str, _brightnessDay,   "Day");	break;
		case 'b':   setBrightness(str, _brightnessNight, "Night");	break;
		case '?':   showHelp();					break;
		case ' ':   showHelp();					break;
		case 'C':   _chimeOn = 1;
					Serial.println("Chime On");
					tone2.tone(2600, 60);	// hz, ms
					break;
					 
		case 'c':   _chimeOn = 0;
					Serial.println("Chime Off");
					tone2.tone(800, 200);	// hz, ms
					break;

		default:  Serial.println("Invalid command, press ? for help");  break;	
	}
}

// processes a char when detected, adds it to c-string inputString
// sets a flag stringComplete to tell calling program to process c-string
void buildInputString() {	// for c-string
	while (Serial.available()) {
		char c = Serial.read();

		if (c == '\r' || c == '\n')  {		// char c is CR or LF, don't append it to inputString
			if (strlen(_inputString) > 0)	// set flag if data in string, dont return if user simply hits enter key
			_stringComplete = 1;
		}
		else {								// not CR or LF, continue to build C-string until enter key is detected
			char tempString[2]="";			// create a temp char string
			tempString[0] = c;				// add the new char

			strcat(_inputString,tempString);	// build up string
			Serial.print(c);				// echo your input
		}
	}
}

#endif /* SERIAL_CONSOLE_H_ */
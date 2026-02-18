/*
 * Word Clock
 * Kevin Rae
   Ver 1.0

 */
#include <avr/wdt.h>     // watchdog timer
#include <avr/eeprom.h>      // standard AVR interface library
#include "ADC_read.h"     // library
#include "SwitchProcess.h"    // library
#include "Tone2.h"        // tone library for Piezo speaker

Tone tone2(&PORTC, PORTC5, TIMER_2); // PORTB0, timer2

#include "Variables.h"
#include "processWords.h"
#include "hardwareInterface.h"
#include "Serial_Console.h"

void processSwitches();
void chime();
void brightnessControl();

void setup() {
  wdt_disable();      // disable watch dog timer
  Serial.begin(9600);
  Serial.println("System restarted");
  Serial.println("Word Clock Ver: 1");
  switch_Init();      // PUR for switches
  ADC_Init();       // initialize ADC for LDR and voltage measurement
  timer0_Init();
  loadValuesFromEEPROM();
  powerOnSeltTest();    // blink LEDs in sequence and beep speaker
  showHelp();       // serial console display help screen
}

void loop() {
  //////////////////////////////////////////////////////////////////////////
  /// display
  if (updateDisplayFlag) {
    updateDisplayFlag = 0;
    timeToWords();
    consoleDisplayTime();
  }
  //////////////////////////////////////////////////////////////////////////
  /// console
  if (_stringComplete) {    // a valid input string is available for processing
    _stringComplete = false;
    processInputString(_inputString);
    _inputString[0] = 0;  // clear out c-string
    updateDisplayFlag = 1;
    saveValuesToEEPROM();
  }
  // monitor serial communication
  if (Serial.available()) // something in serial buffer
    buildInputString();
  //////////////////////////////////////////////////////////////////////////
  // change brightness with LDR and when on battery power
  brightnessControl();
  //////////////////////////////////////////////////////////////////////////
  // switch process, this function process switch presses.
  // SW0 short release increments hours
  // SW0 long press toggles chime
  // SW1 short release increments minutes
  processSwitches();
  //////////////////////////////////////////////////////////////////////////
  // chime
  chime();
  //////////////////////////////////////////////////////////////////////////
}  // loop ends


 void chime() {
  // update EEPROM every hour and increment PowerOnHours
  static uint8_t chimeOccured = 0;
  // every hour active chime once
  // if chime enabled and within allow hours
  if(_real_minutes == 0) {
    if (chimeOccured == 0) {
      chimeOccured = 1;   // only chime once per hour
      _powerOnHours++;
      saveValuesToEEPROM();
      if (_chimeOn)
        if ( _real_hours >= CHIME_ON_HOUR && _real_hours <= CHIME_OFF_HOUR)
          tone2.tone(2600, 30);   // hz, ms

    }
  }
  else
    chimeOccured = 0;   // reset beep flag, so that it can happen again
} // chime ends


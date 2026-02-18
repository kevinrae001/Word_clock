#include "SwitchProcess.h"

SwitchProcess::SwitchProcess(uint8_t swValueOpen, uint8_t debounceCount, uint8_t repeatRate,
							uint16_t repeatStartDelay, uint16_t doublePressDelay, uint16_t longPressDelay) {
	_repeatStartDelay_16	= repeatStartDelay;
	_repeatRate				= repeatRate;
	_repeatRateCounter		= repeatRate;		// initial value for scan function
	_longPressDelay_16		= longPressDelay;
	_doublePressDelay_16	= doublePressDelay;
	_debounceDelay			= debounceCount;
	_swValueOpen			= swValueOpen;		// switch press compares to this
	_swValueValid			= swValueOpen;		// else false double press
	_swValueOld				= swValueOpen;		// else a false switch change state detected
	_swValueDebounced		= swValueOpen;		// else may miss first press if swValue is 0
}

// default state is 16 bit inc, 2 ifs in loop, 35 clock cycles
void SwitchProcess::scan(uint8_t swValue) {
	_scanTicks_16++;						// 10 clock cycles

	if (swValue ^ _swValueOld)	{		// switch changed state, time stamp and set _isActive flag
		_swValueOld = swValue;
		_timeLastActivity16 = _scanTicks_16;
		_isActive = 1;
	}

	if (!_isActive)	return;	// switch not active, else exit function

	// switch has changed state
	uint16_t periodLastEvent_16 = _scanTicks_16 - _timeLastActivity16;	// period from last activity
	if (periodLastEvent_16 >= _debounceDelay) {						// is debounced

		// swValue is debounced and valid now
		// repeats now
		if (swValue ^ _swValueDebounced) {							// one valid switch activity detected (switch state changed)
			_swValueDebounced = swValue;
			_timeLastValidActivity16 = _scanTicks_16;
		}

		uint16_t periodLastValidEvent_16 = _scanTicks_16 - _timeLastValidActivity16;

		if (swValue != _swValueOpen) {				// switch pressed
			if (periodLastValidEvent_16 == 0) {		// do once per debounce event

				// for doublePress function
				if (swValue == _swValueValid) {				// same key pressed
					uint16_t periodBetweenPresses = _scanTicks_16 - _timeOfLastPress16;
					if (periodBetweenPresses < _doublePressDelay_16)
						_doubleState = 1;
				}
				_timeOfLastPress16 = _scanTicks_16;			// reset time stamp

				// for pressedCount function
				_swValueValid = swValue;					// save swValue value for switchNumber result function
				_pressedCounter++;							// track presses for count result function
				_toggleState = !_toggleState;				// track toggle for toggle result function
			}

			// for countRepeat function
			if (periodLastEvent_16 > _repeatStartDelay_16) { // start repeat
				_repeatRateCounter--;
				if(_repeatRateCounter == 0) {
					_repeatRateCounter = _repeatRate;
					_pressedCounterRepeat++;
				}
			}

			// for longPress function
			if (periodLastEvent_16 > _longPressDelay_16)
				_isLongPress = 1;

		}
		else {	// debounced switch released (swValue = _swValueOpen)

			// for releasedCount function
			if (periodLastValidEvent_16 == 0) {			// do once per debounce event
				_releasedCounter++;						// track releases for count result function
			}

			// reset switch states
			_isActive = 0;
			_isLongPress = 0;
		}			// else ends, not swValueOpen
	} // debounce loop
} // scan function

uint16_t SwitchProcess::scanTicks() {
	return _scanTicks_16;
}

uint8_t SwitchProcess::pressedCount() {
	uint8_t n = _pressedCounter;
	_pressedCounter = 0;			//clear value after function executed
	return n;
}

uint8_t SwitchProcess::releasedCount() {
	uint8_t n = _releasedCounter;
	_releasedCounter = 0;
	return n;
}

uint8_t SwitchProcess::pressedCountRepeat() {
	uint8_t n = _pressedCounter + _pressedCounterRepeat;
	_pressedCounter = 0;			//clear value after function executed
	_pressedCounterRepeat = 0;		//clear value after function executed
	return n;
}

uint8_t SwitchProcess::doublePress() {
	uint8_t state = _doubleState;
	_doubleState = 0;				//clear value after function executed
	return state;
}

uint8_t SwitchProcess::switchNumber() {
	return _swValueValid;
}

uint8_t SwitchProcess::longPress() {
	return _isLongPress;
}

uint8_t SwitchProcess::toggle() {
	return _toggleState;
}
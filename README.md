# Word Clock

Firmware + hardware files for a word clock that displays the time in words (rounded to the nearest 5 minutes), with button time-setting, optional hourly chime, and LDR-based brightness control.

## Features
- Word-based time display (5-minute rounding: FIVE, TEN, QUARTER, TWENTY, HALF, PAST, TO, hour words)
- Interrupt-driven multiplexing/charlieplexing for word LEDs
- Two-button time setting
  - SW0 short press: increment hour
  - SW1 short press: increment minutes (seconds reset)
  - SW0 long press: toggle chime on/off
- Automatic brightness control using LDR (separate day/night brightness)
- Optional hourly chime via piezo buzzer (limited to configured hours)
- Simple serial console for time + brightness settings

> Note: Timekeeping is done in software using timers. For best accuracy, add an RTC and adapt the code.

## Repo layout
- `Word_Clock/` — firmware (Arduino sketch + headers)
- `Schematics/` — hardware schematics/design files
- `BOM.xlsx` — bill of materials

## Hardware / pins (as used in firmware)
- SW0 (hours / chime toggle): **PC0**
- SW1 (minutes): **PC1**
- LDR analog input: **PC3**
- LDR pull-up drive: **PB4**
- Piezo buzzer: **PC5**
- Blink LED: **PB5**
- Word LEDs: multiplexed/charlieplexed using **PD2–PD7** (minutes/phrases) and **PORTB/PORTC** for hour/announcement words

If your wiring differs, update the mapping/lookup tables in `Word_Clock/hardwareInterface.h`.

## Build / upload
1. Clone:
   ```bash
   git clone https://github.com/kevinrae001/Word_clock.git
Open Word_Clock/Word_Clock.ino in Arduino IDE.

Select the AVR board/processor that matches your hardware (this project uses AVR registers directly).

Upload.

Setting the time
Buttons
SW0: increment hours

SW1: increment minutes

Long-press SW0: toggle chime

Serial console (9600 baud)
Open Serial Monitor at 9600.

Typical commands:

t hh mm — set time (24h). Example: t 13 45

B nn — set day brightness (0–100). Example: B 80

b nn — set night brightness (0–100). Example: b 15

C / c — chime on / off

? — help/status

Settings and time are stored in EEPROM.

Customization
Word formatting/rounding: Word_Clock/processWords.h

Defaults (brightness, thresholds, chime hours): Word_Clock/Variables.h

LED mapping / multiplexing: Word_Clock/hardwareInterface.h

Troubleshooting
Wrong words / ghosting: check wiring + mapping in hardwareInterface.h and verify current limiting.

Brightness inverted/stuck: verify LDR wiring and adjust threshold in Variables.h.

Chime not working: ensure enabled and within configured hours; verify piezo pin/wiring.

Serial not responding: confirm 9600 baud; note firmware reconfigures I/O heavily.

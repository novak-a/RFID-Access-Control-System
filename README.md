# RFID-Access-Control-System

This project implements an RFID-based access control system using an Arduino. It allows authorized users to unlock a system by scanning their RFID tag and can store new RFID tags for future access. The system  includes an indicator system using LEDs for user feedback.

## Features

- **RFID Tag Reading**: Uses the MFRC522 module to read RFID tags.
- **EEPROM Storage**: Stores RFID tag data in the EEPROM for persistent storage.
- **LED Indicators**: Uses three LEDs (Red, Yellow, Green) to indicate various states:
- **Password Output**: Automatically types a predefined password via USB when a valid tag is detected.
- **Factory Setup Option**: Clears all stored RFID tags when enabled.

## Hardware Requirements

- Arduino board
- MFRC522 RFID Reader Module
- EEPROM
- 3 LEDs
- Connecting wires
- RFID tags

## Setup

1. **Connections**:
   - **RFID Module**:
     - SDA_PIN -> Pin 10
     - RST_PIN -> Pin 5
   - **LEDs**:
     - Red LED -> Pin 13
     - Yellow LED -> Pin 12
     - Green LED -> Pin 11

2. **Upload the Code**: Upload the provided code to your Arduino board using the Arduino IDE.

3. **Configure Factory Setup** (optional):
   - Set `#define FACTORY_SETUP 1` to clear all stored RFID tags on the next upload.
   - Set `FACTORY_SETUP` back to `0` after resetting the memory.

## How it works

1. **System Initialization**:
   - On startup, the system initializes the RFID reader, sets up the LEDs, and initializes the keyboard interface for password output.

2. **Idle State**:
   - The system remains in the idle state, waiting for an RFID tag to be scanned.

3. **Tag Scanning**:
   - When a tag is scanned, the system checks if it matches the predefined green tag.
     - If it matches, the system enters the "Add Tag" mode, allowing new tags to be stored.
     - If it doesn't match, the system checks if the tag is already stored in the EEPROM.
       - If found, the system automatically types the password.
       - If not found, an error signal is given via the red LED.

4. **Add Tag Mode**:
   - In "Add Tag" mode, any non-green tag can be added to the EEPROM for future access.
   - If the tag is already stored, an error signal is given.
   - If the tag is successfully stored, a success signal is given, and the system returns to the idle state.

## Limitations

- **Keyboard Compatibility**: This system does not support number keys on a Czech keyboard layout. It is recommended to use only alphabetic characters (a-z, A-Z) in the password.
- **Tag Compatibility**: Not all types of RFID cards are supported. 
- **Memory Capacity**: Avoid exceeding its capacity. If the EEPROM becomes full, the system will not be able to store additional tags.

## License

This project is licensed under the MIT License - see the LICENSE file for details.


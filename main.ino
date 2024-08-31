/*
 * 
 * 2024 Adam Novak
 * MIT License
 * 
 * IMPORTANT! Numbers on czech keyboard does not work.
 * You should use only a-z A-Z characters
 *
 * IMPORTANT! Cannot read all types of cards. I read about
 * PN532 module... which can help.
 *  
 */

#include <EEPROM.h>
#include <SPI.h>
#include <MFRC522.h>
#include <HID.h>
#include <Keyboard.h>

// FACTORY SETUP (0/1)
#define FACTORY_SETUP 0

// LED
#define RED 13
#define YELLOW 12
#define GREEN 11

// RFID reader
#define SDA_PIN 10
#define RST_PIN 5

// green tag for adding new tags
const byte GREEN_TAG[] = {0xA3, 0x43, 0x08, 0xDA};
const int GREEN_TAG_SIZE = sizeof(GREEN_TAG) / sizeof(GREEN_TAG[0]);

// PC password for output 
const char *password = "AlohomorA";

// create global instance of RFID reader
MFRC522 rfid(SDA_PIN, RST_PIN);

// states of arduino
#define STATE_IDLE 0
#define STATE_ADD_TAG 1

int arduino_state = STATE_IDLE;


/**
 * 
 * @brief Erases the entire EEPROM, resetting all stored tags.
 * 
 */

void erase_memory() {
    for(int i = 0; i < EEPROM.length(); i++) {
        EEPROM.write(i, 0);
    }
}


/**
 * 
 * @brief Prints the contents of the EEPROM to the Serial monitor.
 * 
 */

void print_memory() {
    
    for(int i = 0; i < EEPROM.length(); i++) {

        byte value;
        EEPROM.get(i, value);

        Serial.print((unsigned char) value);
        Serial.print(" ");

    }

    Serial.print("\n");

}


/**
 * 
 * @brief Calculates the number of stored tags in the EEPROM.
 * 
 * @return The number of tags stored in the EEPROM.
 * 
 */

int get_no_stored_tags() {

    int counter = 0;
    int i = 0;

    while(i < EEPROM.length()) {
        
        byte size;
        EEPROM.get(i, size);
        
        if(size == 0) {
            break;
        }
        
        i += size + 1;
        counter++;

    }

    return counter;

}


/**
 * 
 * @brief Checks if a given tag is already stored in the EEPROM.
 * 
 * @param tag_size The size of the tag in bytes.
 * @param tag Pointer to the byte array representing the tag.
 * 
 * @return 1 if the tag is found in the EEPROM, 0 otherwise.
 * 
 */

int is_in_memory(int tag_size, byte *tag) {

    int i = 0;

    // iterate tags
    while(i < EEPROM.length()) {
        
        byte size;
        EEPROM.get(i, size);

        if(size == 0) {
            break;
        }

        if(size == tag_size) {

            int is_same = 1;

            // compare bytes
            for(int j = 0; j < size; j++) {

                byte b;
                EEPROM.get(i + 1 + j, b);

                if(b != tag[j]) {
                    is_same = 0;
                    break;
                }

            }

            if(is_same == 1) {
                return 1;
            }

        }

        i += size + 1;

    }

    return 0;

}


/**
 * 
 * @brief Saves a new tag to the EEPROM.
 * 
 * @param tag_size The size of the tag in bytes.
 * @param tag Pointer to the byte array representing the tag.
 * 
 * @return 1 if the tag was successfully saved, 0 if there was not enough space.
 * 
 */

int save_tag_to_memory(int tag_size, byte *tag) {

    // find index after last element
    int i = 0;

    while(i < EEPROM.length()) {
        
        byte size;
        EEPROM.get(i, size);
        
        if(size == 0) {
            break;
        }
        
        i += size + 1;

    }

    // check if there is enough memory for tag
    if(i + 1 + tag_size >= EEPROM.length()) {
        return 0;
    }

    // write tag
    EEPROM.write(i, tag_size);

    for(int j = 0; j < tag_size; j++) {
        EEPROM.write(i + 1 + j, tag[j]);
    }

    // return
    return 1;

}


/**
 * 
 * @brief Checks if the given tag matches the predefined green tag.
 * 
 * @param tag_size The size of the tag in bytes.
 * @param tag Pointer to the byte array representing the tag.
 * 
 * @return 1 if the tag matches the green tag, 0 otherwise.
 * 
 */

int is_green_tag(int tag_size, byte *tag) {

    int is_green = 0;

    if(tag_size == GREEN_TAG_SIZE) {
        
        is_green = 1;

        for(int i = 0; i < tag_size; i++) {
            if(tag[i] != GREEN_TAG[i]) {
                is_green = 0;
                break;
            }
        }

    }

    return is_green;

}



/**
 * 
 * @brief Signals an error by blinking the red LED.
 * 
 */

void error_signal() {
    digitalWrite(RED, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
    delay(500);
    digitalWrite(RED, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
    delay(500);
    digitalWrite(RED, HIGH);
    delay(500);
    digitalWrite(RED, LOW);
}


/**
 * 
 * @brief Signals success by blinking the green LED.
 * 
 */

void ok_signal() {
    digitalWrite(GREEN, HIGH);
    delay(500);
    digitalWrite(GREEN, LOW);
    delay(500);
    digitalWrite(GREEN, HIGH);
    delay(500);
    digitalWrite(GREEN, LOW);
    delay(500);
    digitalWrite(GREEN, HIGH);
    delay(500);
    digitalWrite(GREEN, LOW);
}


/**
 * 
 * @brief Initializes the system, including setting up LEDs, serial communication, and RFID reader.
 * 
 * @note If FACTORY_SETUP is enabled, the EEPROM is cleared.
 * 
 */

void setup() {

    // set mode for LED pins
    pinMode(RED, OUTPUT);
    pinMode(YELLOW, OUTPUT);
    pinMode(GREEN, OUTPUT);

    // turn off all diodes
    digitalWrite(RED, LOW);
    digitalWrite(YELLOW, LOW);
    digitalWrite(GREEN, LOW);

    // serial speed communication
    Serial.begin(9600);

    // init SPI communication
    SPI.begin();

    // init RFID communication
    rfid.PCD_Init();
    
    // antenna gain
    // rfid.PCD_SetAntennaGain(rfid.RxGain_max);

    // Initialize the Keyboard library
    Keyboard.begin();
    
    // erase memory in case of factory setup
    if(FACTORY_SETUP == 1) {
        erase_memory();
    }

    // busy wait for serial setup
    while(!Serial)
        ;

    // control print
    Serial.print("Device memory capacity: ");
    Serial.println(EEPROM.length());

    Serial.print("Number of stored tags: ");
    Serial.println(get_no_stored_tags());

    Serial.println("Memory: ");
    print_memory();

    Serial.println("Green tag size: ");
    Serial.println(GREEN_TAG_SIZE);

    Serial.println("Green tag: ");
    for(int i = 0; i < GREEN_TAG_SIZE; i++) {
        Serial.print(GREEN_TAG[i]);
        Serial.print(" ");
    }
    Serial.print("\n");

    // light up yellow diode
    digitalWrite(YELLOW, HIGH);

}


/**
 * 
 * @brief Main loop function that continuously checks for RFID tags.
 * 
 */

void loop() {
    
    // RFID tag is not present
    if(!rfid.PICC_IsNewCardPresent()) {
        delay(50);
        return;
    }

    // invalid read of RFID tag
    if(!rfid.PICC_ReadCardSerial()) {
        delay(50);
        return;
    }

    // tag size
    int size = rfid.uid.size;

    // copy tag to local memory
    byte tag[size];

    for(int i = 0; i < size; i++) {
        tag[i] = rfid.uid.uidByte[i];
    }

    Serial.println("Scanned tag: ");
    for(int i = 0; i < size; i++) {
        Serial.print(tag[i]);
        Serial.print(" ");
    }
    Serial.print("\n");

    // IDLE state
    if(arduino_state == STATE_IDLE) {
        // green tag used
        if(is_green_tag(size, tag) == 1) {
            digitalWrite(GREEN, HIGH);
            delay(3000);
            digitalWrite(YELLOW, LOW);
            arduino_state = STATE_ADD_TAG;
        }
        // attemp to log in
        else {
            // tag exists in memory
            if(is_in_memory(size, tag) == 1) {
                Keyboard.print(password);
                ok_signal();
            }
            else {
                error_signal();
            }
        }
    }
    else if(arduino_state == STATE_ADD_TAG) {
        // used tag is not green
        if(is_green_tag(size, tag) == 0) {
            // attemp to add already stored tag
            if(is_in_memory(size, tag) == 1) {
                error_signal();
            }
            else {
                if(save_tag_to_memory(size, tag) == 1) {
                    ok_signal();
                    digitalWrite(YELLOW, HIGH);
                    digitalWrite(GREEN, LOW);
                    arduino_state = STATE_IDLE;
                }
                else {
                    error_signal();
                }
            }
        }
        // green tag used
        else {
            digitalWrite(YELLOW, HIGH);
            delay(3000);
            digitalWrite(GREEN, LOW);
            arduino_state = STATE_IDLE;
        }
    }

    // Serial.println("Memory: ");
    // print_memory();

}

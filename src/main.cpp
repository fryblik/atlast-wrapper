/* Atlast wrapper For ESP32.
 * Copyright (c) 2021 Vojtech Fryblik
 * This code is licensed under MIT license (see LICENSE.txt for details).
 */

#include <Arduino.h>
#include "atlast-wrapper.h"

#define BUF_SIZE 256

// A buffer to hold incoming data
char buf[BUF_SIZE];


/**
 * Serial read line
 * 
 * Store chars from serial input into buffer.
 * Loops until newline is read or when size limit is met.
 * Returns true if a complete line has been read, false otherwise.
 * WARNING: Discards the rest of incoming UART buffer!
 */
bool serialReadLine() {
    // Length of incomplete line stored in buffer
    size_t len = 0;

    while (len < BUF_SIZE) {
        // Wait for input
        while (!Serial.available()) {
            delay(50);
        }

        // Read byte
        char newByte = (char) Serial.read();

        // Add byte to buffer, terminate on newline
        if (newByte != '\n' && newByte != '\r') {
            buf[len] = newByte;
        } else {
            buf[len] = '\0';
            // Dump unread bytes
            while (Serial.available()) {
                Serial.read();
            }
            return true;
        }

        len++;
    }

    // Buffer size limit reached, dump unread bytes
    while (Serial.available()) {
        Serial.read();
    }
    return false;
}

void setup() {
    // Initialize UART
    Serial.begin(115200);

    // Initialize ATLAST interpreter and create interpreter task
    atlastInit();
}

void loop() {
    // Read UART input
    if (Serial.available() && serialReadLine()) {
        if (!strcmp(buf, "TESTBREAK")) {
            // Atlast BREAK signal
            atlastKill(false);
        } else if (!strcmp(buf, "TESTKILL")) {
            // Restart Atlast task
            atlastKill(true);
        } else {
            // Give command to Atlast
            atlastCommand(buf);
        }
    }

    // Write Atlast output to UART
    if (atlastOutputAvailable())
    {
        Serial.print(atlastOutput().c_str());
    }
}
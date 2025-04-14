#include <Arduino.h>
// #include <DS1307RTC.h>
// #include <SD.h>
// #include <TimeLib.h>
// #include <Wire.h>
#include <SPI.h>

#include "hamming.h"


// SPI Configuration
const byte CHUNK_SIZE = 56;
const unsigned long SPI_TIMEOUT_MS = 1000;

// Volatile communication variables
volatile bool messageReceived = false;
volatile byte receivedIndex = 0;
volatile uint8_t receivedData[CHUNK_SIZE];
volatile uint32_t lastUpdateTime = 0;

////////////////////////////////////////
// Helper Functions
////////////////////////////////////////

void printChunk(uint8_t *buffer) {
    Serial.println("\nReceived Chunk:");
    
    // Print hex values
    for (int i = 0; i < CHUNK_SIZE; i++) {
        if (i % 16 == 0 && i != 0) Serial.println();        
        Serial.print(buffer[i] < 0x10 ? "0x0" : "0x");
        Serial.print(buffer[i], HEX);
        Serial.print(" ");
    }    
    Serial.println("\nEnd of Chunk\n");
}

// Process the fully received chunk
void handleReceivedData() {
    // Safely copy data from volatile buffer
    noInterrupts();
    uint8_t tempBuffer[CHUNK_SIZE];
    memcpy(tempBuffer, (void*)receivedData, CHUNK_SIZE);
    bool valid = (receivedIndex == CHUNK_SIZE);
    receivedIndex = 0;
    messageReceived = false;
    interrupts();
    
    // Process data only if full chunk received
    if (valid) {
        printChunk(tempBuffer);
    } else {
        Serial.println("Error: Incomplete chunk received");
    }
    
    lastUpdateTime = millis();
}

// Reset partially received chunk after a timeout
void handleTimeout() {
    noInterrupts();
    if (receivedIndex > 0) {
        Serial.print("Timeout - Resetting partial chunk (");
        Serial.print(receivedIndex);
        Serial.println(" bytes)");
        receivedIndex = 0;
    }
    interrupts();
    
    lastUpdateTime = millis();
}

////////////////////////////////////////
// Arduino Core Functions
////////////////////////////////////////

void setup() {
    Serial.begin(115200);
    while (!Serial); // Wait for serial connection
    
    // Configure SPI Slave
    pinMode(SS, INPUT);
    SPCR |= _BV(SPE);  // Enable SPI
    SPI.attachInterrupt();
    
    // Initialize timers
    lastUpdateTime = millis();
    
    Serial.println("Arduino SPI Slave Initialized");
}

void loop() {
    // Handle received data if a full chunk was received
    if (messageReceived) {
        handleReceivedData();
    }
    
    // Handle timeout for partial data reception
    if (millis() - lastUpdateTime > SPI_TIMEOUT_MS) {
        handleTimeout();
    }
}

////////////////////////////////////////
// SPI Interrupt Service Routine
////////////////////////////////////////

ISR(SPI_STC_vect) {
    // Store received byte
    if (receivedIndex < CHUNK_SIZE) {
        receivedData[receivedIndex] = SPDR;
        receivedIndex++;
        
        // Check for complete chunk
        if (receivedIndex == CHUNK_SIZE) {
            messageReceived = true;
        }
    }
    
    // Update timer and send dummy byte to maintain SPI communication
    lastUpdateTime = millis();
    SPDR = 0x00;
}
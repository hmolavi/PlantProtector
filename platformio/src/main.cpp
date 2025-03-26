#include <Arduino.h>
#include <Wire.h>
#include <TimeLib.h>
#include <DS1307RTC.h>
#include <SD.h>

#include "hamming.h"

const int chipSelect = 10;

void print_array(int *arr, int n, const char *label)
{
    Serial.print(label);
    Serial.print(": ");
    for (int i = 0; i < n; i++) {
      Serial.print(arr[i]);
      Serial.print(' ');
    }
    Serial.println();
}

void print2digits(int number) {
  if (number >= 0 && number < 10) {
    Serial.write('0');
  }
  Serial.print(number);
}

void readRTC() {
  tmElements_t tm;

  if (RTC.read(tm)) {
    Serial.print("Ok, Time = ");
    print2digits(tm.Hour);
    Serial.write(':');
    print2digits(tm.Minute);
    Serial.write(':');
    print2digits(tm.Second);
    Serial.print(", Date (D/M/Y) = ");
    Serial.print(tm.Day);
    Serial.write('/');
    Serial.print(tm.Month);
    Serial.write('/');
    Serial.print(tmYearToCalendar(tm.Year));
    Serial.println();
  } else {
    if (RTC.chipPresent()) {
      Serial.println("The DS1307 is stopped.  Please run the SetTime");
      Serial.println("example to initialize the time and begin running.");
      Serial.println();
    } else {
      Serial.println("DS1307 read error!  Please check the circuitry.");
      Serial.println();
    }
  }
}

void writeToCard(){
  File dataFile = SD.open("log.txt", FILE_WRITE);

  if (dataFile) {
    Serial.println("Writing to log.txt...");
    dataFile.println("Hello, world");
    dataFile.println("This a test fr");
    dataFile.close();
    Serial.println("Done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening log.txt");
  }

  dataFile.close();
}

void readFromCard(){
    // open the file for reading:
  File dataFile = SD.open("log.txt");

  if (dataFile) {
    // read the file one character at a time:
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
    Serial.println();
  } else {
    // if the file didn't open, print an error:
    Serial.println("Error opening log.txt");
  }
}

void setup() {
  Serial.begin(9600);
  while (!Serial); // wait for Serial Monitor to open

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  Serial.println("card initialized.");

  writeToCard();

  readFromCard();

  delay(200);
  tmElements_t tm;

  // Set the desired time and date manually
  tm.Year = CalendarYrToTm(2025);  // Year 2025
  tm.Month = 1;   // January
  tm.Day = 3;     // 3rd day of the month
  tm.Hour = 13;   // 13:51:42 (1:51:42 PM)
  tm.Minute = 51;
  tm.Second = 42;

  if (RTC.write(tm)) {
    Serial.println("RTC successfully set to: Jan 3, 2025 13:51:42");
  } else {
    Serial.println("Failed to set RTC. Check wiring.");
  }

  // Run single demonstration test
  int demo_data[] = {1, 1, 1, 0};
  int demo_bit_to_change = 1;
  int data_bits = sizeof(demo_data) / sizeof(demo_data[0]);
  int parity_bits = 0;
  while ((1 << parity_bits) < (data_bits + parity_bits + 1)) parity_bits++;
  int n = data_bits + parity_bits;

  int encoded[n], decoded[data_bits];

  printf("=== Demonstration Test ===\n");
  hamming_encode(demo_data, data_bits, encoded);
  print_array(encoded, n, "Encoded Data");

  encoded[demo_bit_to_change] ^= 1;
  print_array(encoded, n, "With Error  ");

  hamming_decode(encoded, n, decoded);
  print_array(decoded, data_bits, "Decoded Data");
}

void loop() {
  // readRTC();
  // delay(1000);
}

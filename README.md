# Plant Protector



## Getting Started

This project is intended to run on an ESP32S3 connected to an Arduino ATMega328p. The Platformio folder holds the code for the Arduino side, while the esp-idf folder contains the code for the ESP32S3.

Because of the aws sdk import in ESP-IDF, you must also update the submodules that comes with it, you can do it with:
``` bash
git submodule update --init --recursive
```
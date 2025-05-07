# Plant Protector

<p align="center">
<img src="https://github.com/user-attachments/assets/49dcf414-d3b9-4081-9259-3601e2cb3a9e" width="900"/>
<br>
<em>ASCII Art displayed when reading serial monitor from esp-idf side upon launch</em>  
<br><br>

<img src="https://github.com/user-attachments/assets/81a8a352-30bc-4e79-a403-1727ca632b10" width="450"/>
<img src="https://github.com/user-attachments/assets/689ba080-d810-4638-8fb7-3a4b6d97f650" width="450"/>
<br>
<em>Custom PCB used to hold the ESP32S3 and ATMega328 chips</em>
<br><br>

<img src="https://github.com/user-attachments/assets/010d351d-6f9d-43f1-be8e-59e6b5be1547" width="900"/>
<br>
<em>Device using the onboard device certificate to self-provision to AWS IoT Core and subscribing to its dedicated Pub/Sub MQTT endpoint upon WiFi connectivity</em>

</p>


## Getting Started

This project is intended to run on an ESP32S3 connected to an Arduino ATMega328p. The Platformio folder holds the code for the Arduino side, while the esp-idf folder contains the code for the ESP32S3.

Because of the aws sdk import in ESP-IDF, you must also update the submodules that comes with it, you can do it with:
``` bash
git submodule update --init --recursive
```

girf
====

An Arduino-framework based library for the communication with the [Gira Dual Q RF module](https://katalog.gira.de/en/datenblatt.html?id=636882). The main purpose of the library is to get the alerts, test alerts and battery warnings of the remotly connected smokedetectors so they can processed by an arduino compatible device. In conjunction with Marving Roger's excellent [Homie framework](https://github.com/marvinroger/homie) (which is implemented only for ESP8266-based MCUs at the moment) you can easily connect you smokedetectors to your smart home (e.g. by hooking them up to the - once again - excellent [OpenHAB](https://www.openhab.org/) smart home server).

## Download

The Git repository on GitHub contains the development version of the library. As this is work in progress it may take some time if you see "stable" releases [on the releases page](https://github.com/metaneutrons/girf/releases).

## Features

* complete capsulation of serial communcation with the RF module
* event-based: OnAlarm, OnAlarmTest, OnBatteryWarning
* the use of StreamObject makes it usable with UARTs and software serial

## Hardware

The [Gira Dual Q RF module](https://katalog.gira.de/en/datenblatt.html?id=636882) is a battery powered RF module for the 433-ISM-band. It's connected to the Dual Q smokedetector with a serial interface on 9600bps using 3.3V TTL signals via a 10-pin connector. For operation with the library it's necessary to connect the modules RX and TX line to the TX and RX lines of the used microcontroller (e.g. the excellent [Teensy](https://www.pjrc.com/teensy/) or an ESP8266 clone) and a common GND.

[![Gira RF module pinout](https://github.com/metaneutrons/girf/blob/master/docs/assets/rf_module.jpeg)]

Please note that the signals of the RF module are on 3.3V level (although there is a 5V VCC pin). If you want to connect the RF module to a 5V-arduino (e.g. the UNO) you need some level shifter circuit.

## Example 
```c++
#include <Arduino.h>
#include <girf.h>

girf rf = girf(Serial);

// girf Handler
void OnAlarmTestHandler(bool value) {
  Serial.println("OnAlarmTest");
}

void OnAlarmHandler(bool value) {
  Serial.println("OnAlarm");
}

void OnBatteryWarningHandler(bool value) {
  Serial.println("OnBatteryWarning");
}

void setup() {
  Serial.begin(115200);
  Serial2.begin(9600);

  rf.SetOnAlarmTestHandler(OnAlarmTestHandler);
  rf.SetOnAlarmHandler(OnAlarmHandler);
  rf.SetOnBatteryWarningHandler(OnBatteryWarningHandler);
}

void loop() {
  rf.loop();
}
```

## Requirements, installation and usage

The library itself has no dependencies. Put it your libs folder an include 'girf.h'.

## Donate

I developed this library for my personal learning experience. The library is far from being perfect and probably is not even a good example of how to develop a library. Therefore, I do not ask for money, but I hope that others will develop the library and remove my mistakes. But if you absolutely want to send an attention to me, then I do like an invite for a coffee...

[![Donate button](https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.me/metaneutrons)

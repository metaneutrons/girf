girf
====

An Arduino-framework based library for the communication with the [Gira Dual Q RF module](https://katalog.gira.de/en/datenblatt.html?id=636882). The main purpose of the library is to get the alerts, test alerts and battery warnings of the remotly connected smokedetectors so they can processed by an arduino compatible device. In conjunction with Marving Roger's excellent [Homie framework](https://github.com/marvinroger/homie) (which is implemented only for ESP8266-based MCUs at the moment) you can easily connect your smokedetectors to your smart home (e.g. by hooking them up to the - once again - excellent [OpenHAB](https://www.openhab.org/) smart home server).

## Download

The git repository on GitHub contains the development version of the library. As this is work in progress it may take some time to see "stable" releases [on the releases page](https://github.com/metaneutrons/girf/releases).

## Features

* complete capsulation of serial communcation with the RF module
* event-based: OnAlarm, OnAlarmTest, OnBatteryWarning
* use of StreamObject makes the library compatible with hardware UARTs and software serial

## Hardware

The [Gira Dual Q RF module](https://katalog.gira.de/en/datenblatt.html?id=636882) is a battery powered RF module for the 433-ISM-band. It's connected to the Dual Q smokedetector with a serial interface on 9600bps using 3.3V TTL signals via a 10-pin connector. For operation with the library it's necessary to connect the modules RX and TX lines crossed to the TX and RX lines of the used microcontroller (e.g. the excellent [Teensy](https://www.pjrc.com/teensy/) or an ESP8266 clone) and a common GND.

![RF module pinout](https://github.com/metaneutrons/girf/blob/master/docs/assets/rf_module.jpeg)

Please note that the signals of the RF module are on 3.3V level, although there is a 5V VCC pin on the header of the smokedetector. If you want to connect the RF module to a 5V-MCU (e.g. the Arduino UNO) you need some level shifter circuit.

## Example 
```c++
#include <Arduino.h>
#include <girf.h>

girf rf = girf(Serial1);

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
  Serial1.begin(9600);

  rf.SetOnAlarmTestHandler(OnAlarmTestHandler);
  rf.SetOnAlarmHandler(OnAlarmHandler);
  rf.SetOnBatteryWarningHandler(OnBatteryWarningHandler);
}

void loop() {
  rf.loop();
}
```

## Requirements, installation and usage

The library itself has no dependencies. Put it in your libs folder and include 'girf.h'.

As there is no special magic in the library it should work on all platforms the Arduino-framework is ported to. I personally tested the library on an ESP8266 and a Teensy 3.1 and 3.6.

## Todos

At the moment it's only possible to hook on the three events (alarm, test alarm and battery warning) of the RF module. The next step is to implement the option to send events to the RF module (e.g. initiate a test alarm).

## Licensing

The library is licensed unter [GNU Lesser General Public License v3](https://www.gnu.org/licenses/lgpl-3.0.en.html). The examples are licensed under [MIT license](https://opensource.org/licenses/MIT).

## Donate

I developed this library for my personal learning experience. The library is far from being perfect and probably is not even a good example of how to develop a library. Therefore, I do not ask for money, but I hope that others will develop the library and remove my mistakes. But if you absolutely want to send an attention to me, then I do like an invite for a coffee...

[![Donate button](https://www.paypal.com/en_US/i/btn/btn_donateCC_LG.gif)](https://www.paypal.me/metaneutrons)

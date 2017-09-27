/*
    Minimal example for girf library
    Copyright 2017 Fabian Schmieder

    This example only is licensed under MIT license.

    Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
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
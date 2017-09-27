/*
  Library for communication with Gira Dual Q RF Module (234700)

  Copyright (c) 2017 Fabian Schmieder. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 3 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

#ifndef __GIRF_h__
#define __GIRF_h__

// #define GIRF_DEBUG // enable some debugging for the library

#include <Arduino.h>

// constant for control bytes
const char STX = 0x02;
const char ETX = 0x03;
const char NUL = 0x00;
const char ACK = 0x06;
const char NAK = 0x15;

// send and receive buffer sizes
const uint SIZE_RECEIVE_BUFFER = 32;
const uint SIZE_SEND_BUFFER = 32;

// protocol command bytes
const char RF_REQUEST_STATUS = 0x02;
const char RF_STATUS = 0x03;
const char RF_REQUEST_SERIAL = 0x04;
const char RF_DIAGNOSIS = 0x07;
const char RF_UNKNOWN_08 = 0x08;
const char RF_UPTIME = 0x09;
const char RF_SMOKEBOX = 0x0B;
const char RF_BATTERY_TEMP = 0x0C;
const char RF_ALARM_COUNT = 0x0D;
const char RF_TESTALARM_COUNT = 0x0E;
const char RF_UNKOWN_0F = 0x0F;

// fake serial number of emulated smoke detector
const char SD_SERIAL[] = {0xC4, 0x0F, 0xAB, 0x1A, 0x90, 0x00};

// maximum message transmit tries and timeout for ACK of RF module
const int MAXIMUM_TX_TRYS = 3;
const uint32_t ACK_TIMEOUT = 2000;

// girf class
class girf {
public:
  // public functions
  girf(Stream &s = Serial) : stream(s) {}
#ifdef GIRF_DEBUG
  void send_message(char *cmd);
  void SetOnDebugHandler(void (*func)(String));
#endif
  void SetOnAlarmHandler(void (*func)(bool));
  void SetOnAlarmTestHandler(void (*func)(bool));
  void SetOnBatteryWarningHandler(void (*func)(bool));
  void loop();
  // public variables

private:
// private functions
#ifndef GIRF_DEBUG
  void send_message(char *cmd);
#endif
#ifdef GIRF_DEBUG
  void debug(String message);
#endif
  void process_byte(char c);
  char calculate_checksum(char *cmd, int len);
  bool process_command(char *cmd);
  void send_status(bool _status_requested = false);
  void (*_OnDebugHandler)(String);
  void (*_OnAlarmHandler)(bool);
  void (*_OnAlarmTestHandler)(bool);
  void (*_OnBatteryWarningHandler)(bool);
  void hex2str(char *h, char *s);
  void str2hex(char *s, char *h);
  // private variables
  Stream &stream;
  char cmd_send[SIZE_SEND_BUFFER]; // NUL-terminated string of hex string values
  uint cmd_send_counter = 0;
  uint32_t cmd_send_timestamp = millis();
  bool _status_alarm_local_test = false;
  bool _status_alarm_local = false;
  bool _status_alarm_wired_test = false;
  bool _status_alarm_wired = false;
  bool _status_alarm_wireless_test = false;
  bool _status_alarm_wireless = false;
  bool _status_button = false;
  bool _status_battery_low = false;
  bool _status_battery_powered = false;
  bool _remote_status_battery_low = false;
  bool _remote_status_pairing = false;
  bool _remote_status_alarm_test = false;
  bool _remote_status_alarm = false;
};
#endif
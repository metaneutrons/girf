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

#include <Arduino.h>
#include <array>
#include <girf.h>
#include <queue>
#include <string>

// public methods

#ifdef GIRF_DEBUG
void girf::debug(String message) {
  if (_OnDebugHandler != NULL) {
    _OnDebugHandler(message);
  }
}

void girf::SetOnDebugHandler(void (*func)(String)) { _OnDebugHandler = func; }
#endif

void girf::SetOnAlarmTestHandler(void (*func)(bool)) {
  _OnAlarmTestHandler = func;
}

void girf::SetOnAlarmHandler(void (*func)(bool)) { _OnAlarmHandler = func; }

void girf::SetOnBatteryWarningHandler(void (*func)(bool)) {
  _OnBatteryWarningHandler = func;
}

void girf::SendStatus() { send_status(false); }

// private methods

char girf::calculate_checksum(char *cmd, int len) {
  int chksum = 0;
  for (int i = 0; i < len; i++) {
    chksum = chksum + cmd[i];
  }
  return (chksum & 255);
}

void girf::hex2str(char *h, char *s) {
  // converts "30313233" to "0123";
  char v[] = "00";
  memset(s, 0, strlen(s));
  for (uint i = 0; i < strlen(h); i = i + 2) {
    v[0] = h[i];
    v[1] = h[i + 1];
    s[i / 2] = strtol(v, NULL, 16);
  }
}

void girf::str2hex(char *s, char *h) {
  // converts "0123" to "30313233";
  char v[] = "00";
  memset(h, 0, strlen(h));
  for (uint i = 0; i <= sizeof(s); i++) {
    sprintf(v, "%02X", s[i]);
    strcat(h, v);
  }
}

void girf::send_status(bool status_requested) {
  char c[] = {0x82, 0x00, 0x00, 0x00, 0x00};
  if (status_requested) {
    c[0] = 0xC2;
  }

  /*         1. byte        2. byte             3. byte   4. byte
      0x01 = ??? (Q)        battery low         ???       ???
      0x02 = defect det     ???                 ???       ???
      0x04 = ???            local alarm         ???       temp sensor 1 defect
      0x08 = btn pressed    wire alarm          ???       ???
      0x10 = (smoke?)alarm  wireless alarm      ???       temp sensor 2 defect
      0x20 = battery pwr.   local test alarm    ???       ???
      0x40 = ???            wired test alarm    ???       ???
      0x80 = ???            wirless test alarm  ???       ???
  */

  if (_status_local[STATUS_BUTTON]) {
    c[1] += 0x08;
  }
  if (_status_local[STATUS_ALARM_LOCAL]) {
    c[1] += 0x10;
  }
  if (_status_local[STATUS_BATTERY_POWERED]) {
    c[1] += 0x20;
  }

  if (_status_local[STATUS_BATTERY_LOW]) {
    c[2] += 0x01;
  }

  if (_status_local[STATUS_ALARM_LOCAL]) {
    c[2] += 0x04;
  }
  if (_status_local[STATUS_ALARM_WIRED]) {
    c[2] += 0x08;
  }
  if (_status_local[STATUS_ALARM_WIRELESS]) {
    c[2] += 0x10;
  }

  if (_status_local[STATUS_ALARM_LOCAL_TEST]) {
    c[2] += 0x20;
  }
  if (_status_local[STATUS_ALARM_WIRED_TEST]) {
    c[2] += 0x40;
  }
  if (_status_local[STATUS_ALARM_WIRELESS_TEST]) {
    c[2] += 0x80;
  }

  send_message(c);
}

void girf::send_message(char *cmd) {
  char chksum[] = "00";
  str2hex(cmd, cmd_send);
  sprintf(chksum, "%02X", calculate_checksum(cmd_send, strlen(cmd_send)));
  strcat(cmd_send, chksum);
#ifdef GIRF_DEBUG
  debug("send=" + String(cmd_send));
#endif
  cmd_send_counter = 1;
}

bool girf::process_message(char *cmd) {
  char msg5[5];
  char msg3[3];
  int uptime = millis() / 250; // quarter seconds
  switch (cmd[0]) {
  case RF_REQUEST_STATUS:
#ifdef GIRF_DEBUG
    debug("RF_REQUEST_STATUS");
#endif
    send_status(true);
    break;
  case RF_REQUEST_SERIAL:
#ifdef GIRF_DEBUG
    debug("RF_REQUEST_SERIAL");
#endif
    msg5[0] = 0xC4;
    msg5[1] = 0x0F;
    msg5[2] = 0xAB;
    msg5[3] = 0x1A;
    msg5[4] = 0x90;
    send_message(msg5);
    break;
  case RF_STATUS:
#ifdef GIRF_DEBUG
    debug("RF_STATUS");
#endif
    /*
        1. Byte                 2. Byte
        0x01 = ?                0x01 = battery low (via RF)
        0x02 = ?                0x02 = pairing
                                0x04 = ?
                                0x08 = ? (via diagnosis software)
                                0x10 = alarm (via RF)
                                0x20 = ?
                                0x40 = ?
                                0x80 = test alarm (via RF)
    */
    if (_status_remote[STATUS_REMOTE_BATTERY_LOW] != (cmd[2] & 0x01)) {
      _status_remote[STATUS_REMOTE_BATTERY_LOW] = (cmd[2] & 0x01);
#ifdef GIRF_DEBUG
      debug("_OnBatteryWarningHandler called.");
#endif
      if (_OnBatteryWarningHandler != NULL) {
        _OnBatteryWarningHandler(_status_remote[STATUS_REMOTE_BATTERY_LOW] ||
                                 _status_local[STATUS_BATTERY_LOW]);
      }
    } else if (_status_remote[STATUS_REMOTE_PAIRING] != (cmd[2] & 0x02)) {
      _status_remote[STATUS_REMOTE_PAIRING] = (cmd[2] & 0x02);
#ifdef GIRF_DEBUG
      debug("_pairing=" + String(_status_remote[STATUS_REMOTE_PAIRING]));
#endif
    } else if (_status_remote[STATUS_REMOTE_ALARM] != (cmd[2] & 0x10)) {
      _status_local[STATUS_ALARM_WIRELESS] =
          _status_local[STATUS_REMOTE_ALARM] = (cmd[2] & 0x10);
#ifdef GIRF_DEBUG
      debug("_OnAlarmHandler called.");
#endif
      if (_OnAlarmHandler != NULL) {
        _OnAlarmHandler(_status_remote[STATUS_REMOTE_ALARM] ||
                        _status_local[STATUS_ALARM_LOCAL] ||
                        _status_local[STATUS_ALARM_WIRED] ||
                        _status_local[STATUS_ALARM_WIRELESS]);
      }
    } else if (_status_remote[STATUS_REMOTE_ALARM_TEST] != (cmd[2] & 0x80)) {
      _status_local[STATUS_ALARM_WIRELESS_TEST] =
          _status_remote[STATUS_REMOTE_ALARM_TEST] = (cmd[2] & 0x80);
#ifdef GIRF_DEBUG
      debug("_OnAlarmTestHandler called.");
#endif
      if (_OnAlarmTestHandler != NULL) {
        _OnAlarmTestHandler(_status_remote[STATUS_REMOTE_ALARM_TEST] ||
                            _status_local[STATUS_ALARM_LOCAL_TEST] ||
                            _status_local[STATUS_ALARM_WIRED_TEST] ||
                            _status_local[STATUS_ALARM_WIRELESS_TEST]);
      }
    }
    break;
  case RF_DIAGNOSIS:
#ifdef GIRF_DEBUG
    debug("RF_DIAGNOSIS");
#endif
    break;
  case RF_UNKNOWN_08:
#ifdef GIRF_DEBUG
    debug("RF_UNKNOWN_08");
#endif
    msg5[0] = 0xC8;
    msg5[1] = 0x01;
    msg5[2] = 0x72;
    msg5[3] = 0x01;
    msg5[4] = 0x45;
    send_message(msg5);
    break;
  case RF_UPTIME:
#ifdef GIRF_DEBUG
    debug("RF_UPTIME");
#endif
    msg5[0] = 0xC9;
    // not elegant, but endiansafe
    msg5[1] = uptime & 0xff;
    msg5[2] = (uptime >> 8) & 0xff;
    msg5[3] = (uptime >> 16) & 0xff;
    msg5[4] = (uptime >> 24) & 0xff;
    send_message(msg5);
  case RF_SMOKEBOX:
#ifdef GIRF_DEBUG
    debug("RF_SMOKEBOX");
#endif
    // as we emulate the smokedetector there is no magic here
    msg5[0] = 0xCB;
    msg5[1] = 0x00;
    msg5[2] = 0x5C;
    msg5[3] = 0x00;
    msg5[4] = 0x00;
    send_message(msg5);
    break;
  case RF_BATTERY_TEMP:
#ifdef GIRF_DEBUG
    debug("RF_BATTERY_TEMP");
#endif
    // as we emulate the smokedetector there is no magic here (we just send 0x52
    // two times)
    msg5[0] = 0xCC;
    msg5[1] = 0x00;
    msg5[2] = 0x01;
    msg5[3] = 0x52; // temp of 1st sensor
    msg5[4] = 0x52; // temp of 2nd sensor
    send_message(msg5);
    break;
  case RF_ALARM_COUNT:
#ifdef GIRF_DEBUG
    debug("RF_ALARM_COUNT");
#endif
    msg5[0] = 0xCD;
    msg5[1] = 0x00; // no. of local alarms
    msg5[2] = 0x00; // no. of local testalarms
    msg5[3] = 0x00; // no. of wired alarms
    msg5[4] = 0x00; // no. of wireless alarm
    send_message(msg5);
    break;
  case RF_TESTALARM_COUNT:
#ifdef GIRF_DEBUG
    debug("RF_TESTALARM_COUNT");
#endif
    msg3[0] = 0xCE;
    msg3[1] = 0x00; // no. of wired testalarms
    msg3[2] = 0x00; // no. of wireless testalarms
    send_message(msg3);
    break;
  case RF_UNKOWN_0F:
#ifdef GIRF_DEBUG
    debug("RF_UNKNOWN_0F");
#endif
    msg5[0] = 0xCF;
    msg5[1] = 0x00;
    msg5[2] = 0x84;
    msg5[3] = 0x00;
    msg5[4] = 0x77;
    send_message(msg5);
    break;
  default:
#ifdef GIRF_DEBUG
    debug("unknown command");
#endif
  }
  return true;
}

void girf::process_byte(char c) {
  static char _receive_buffer[SIZE_RECEIVE_BUFFER];
  static bool rx_state = false;

  if (c == NUL) {
    // do nothing
  } else if (c == ACK) {
#ifdef GIRF_DEBUG
    debug("RF-ACK");
#endif
    cmd_send_counter = 0;
  } else if (c == NAK) {
#ifdef GIRF_DEBUG
    debug("RF-NAK");
#endif
    cmd_send_timestamp = millis() - ACK_TIMEOUT; // speedup timeout to resend
  } else if (c == STX) {
#ifdef GIRF_DEBUG
    debug("RF-STX");
#endif
    rx_state = true;
    memset(_receive_buffer, 0, SIZE_RECEIVE_BUFFER);
  } else if (rx_state && (c == ETX)) {
    rx_state = false;
    if (strlen(_receive_buffer) % 2 > 0) {
#ifdef GIRF_DEBUG
      debug("ESP(SD)-NAK - command string length is not even");
#endif
      stream.write(NAK);
    } else {
      // convert hex to string
      char _received_cmd[strlen(_receive_buffer) / 2];
      hex2str(_receive_buffer, _received_cmd);
      debug("cmd: " + String(_receive_buffer));
      // process the command
      if (calculate_checksum(_receive_buffer, strlen(_receive_buffer) - 2) ==
          _received_cmd[sizeof(_received_cmd) - 1]) {
        if (process_message(_received_cmd)) {
#ifdef GIRF_DEBUG
          debug("ESP(SD)-ACK");
#endif
          stream.write(ACK);
        } else {
#ifdef GIRF_DEBUG
          debug("ESP(SD)-NAK - command not accpepted");
#endif
          stream.write(NAK);
        }
      } else {
#ifdef GIRF_DEBUG
        debug("ESP(SD)-NAK - improper checksum");
#endif
        stream.write(NAK);
      }
      memset(_receive_buffer, 0, SIZE_RECEIVE_BUFFER);
    }
  } else if (rx_state && strlen(_receive_buffer) < SIZE_RECEIVE_BUFFER) {
    _receive_buffer[strlen(_receive_buffer)] = c;
  }
}

// public loop method

void girf::loop() {
  // send out buffered messages and manage resends
  if ((cmd_send_counter == 1) ||
      ((cmd_send_counter > 1) && (cmd_send_counter <= MAXIMUM_TX_TRYS + 1) &&
       ((millis() - cmd_send_timestamp) >= ACK_TIMEOUT))) {

    if (cmd_send_counter == MAXIMUM_TX_TRYS + 1) {
      cmd_send_counter = 0;
      cmd_send[0] = 0;
#ifdef GIRF_DEBUG
      debug("command not accpeted, maximium retries exceeded or NAK received");
#endif
    } else {
      stream.write(NUL);
      stream.write(STX);
      stream.print(cmd_send);
      stream.write(ETX);
      cmd_send_counter++;
      cmd_send_timestamp = millis();
#ifdef GIRF_DEBUG
      debug("send cmd: " + String(cmd_send));
#endif
    }
  }

  // processing of incomming bytes
  if (stream.available()) {
    // read the incoming byte:
    process_byte(stream.read());
  }

  // handle automatic status updates (if enabled)
  if ((cmd_send_counter == 0) && (StatusUpdateInterval > 0) &&
      ((millis() - status_update_timestamp) >= StatusUpdateInterval * 1000)) {
    send_status(false);
  }

  // update public status object if private object changed
  if (!std::equal(std::begin(_status_local), std::end(_status_local),
                  std::begin(StatusLocal))) {
#ifdef GIRF_DEBUG
    debug("Local Status array updated");
#endif
    std::copy(std::begin(_status_local), std::end(_status_local),
              std::begin(StatusLocal));
    send_status();
  }

  if (!std::equal(std::begin(_status_remote), std::end(_status_remote),
                  std::begin(StatusRemote))) {
#ifdef GIRF_DEBUG
    debug("Status array updated");
#endif
    std::copy(std::begin(_status_remote), std::end(_status_remote),
              std::begin(StatusRemote));
    send_status();
  }
}
/*
  serial.c - Low level functions for sending and recieving bytes via the serial port
  Part of Grbl

  Copyright (c) 2011-2016 Sungeun K. Jeon for Gnea Research LLC
  Copyright (c) 2009-2011 Simen Svale Skogsrud

  Grbl is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  Grbl is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with Grbl.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "grbl.hpp"
#include <Ticker.h>

#define RX_RING_BUFFER (RX_BUFFER_SIZE+1)
#define TX_RING_BUFFER (TX_BUFFER_SIZE+1)

uint8_t serial_rx_buffer[CLIENT_COUNT][RX_RING_BUFFER];
uint8_t serial_rx_buffer_head[CLIENT_COUNT] = {0};
volatile uint8_t serial_rx_buffer_tail[CLIENT_COUNT] = {0};

Ticker serial_poll_task;

// Returns the number of bytes available in the RX serial buffer.
uint8_t serial_get_rx_buffer_available(uint8_t client)
{
  uint8_t client_idx = client - 1;

  uint8_t rtail = serial_rx_buffer_tail[client_idx]; // Copy to limit multiple calls to volatile
  if (serial_rx_buffer_head[client_idx] >= rtail) { return(RX_BUFFER_SIZE - (serial_rx_buffer_head[client_idx]-rtail)); }
  return((rtail-serial_rx_buffer_head[client_idx]-1));
}

void serial_init()
{
  Serial.begin(BAUD_RATE);

  Serial.setDebugOutput(false);
  serial_poll_task.attach_ms(1, serial_poll_rx);
}

void serial_poll_rx()
{
  uint8_t data = 0;
  uint8_t next_head;
  uint8_t client = CLIENT_SERIAL;  // who sent the data
  uint8_t client_idx = 0;  // index of data buffer

  while (Serial.available()
  #if (defined ENABLE_WIFI) && (defined ENABLE_WEBSOCKET)
    || Serial2Socket.available()
  #endif
  #if (defined ENABLE_WIFI) && (defined ENABLE_TELNET)
    || telnetServer.available()
  #endif
    ) {
    if (Serial.available()) {
      client = CLIENT_SERIAL;
      data = Serial.read();
    }
    #if (defined ENABLE_WIFI) && (defined ENABLE_WEBSOCKET)
    else if (Serial2Socket.available()) {
      client = CLIENT_WEBSOCKET;
      data = Serial2Socket.read();
    }
    #endif
    #if (defined ENABLE_WIFI) && (defined ENABLE_TELNET)
    else if (telnetServer.available()) {
      client = CLIENT_TELNET;
      data = telnetServer.read();
    }
    #endif

    client_idx = client - 1;  // for zero based array

    // Pick off realtime command characters directly from the serial stream. These characters are
    // not passed into the main buffer, but these set system state flag bits for realtime execution.
    switch (data) {
    case CMD_RESET:         mc_reset(); break; // Call motion control reset routine.
    case CMD_STATUS_REPORT: report_realtime_status(client); break;
    case CMD_CYCLE_START:   system_set_exec_state_flag(EXEC_CYCLE_START); break; // Set as true
    case CMD_FEED_HOLD:     system_set_exec_state_flag(EXEC_FEED_HOLD); break; // Set as true
    default :
      if (data > 0x7F) { // Real-time control characters are extended ACSII only.
        switch(data) {
          case CMD_SAFETY_DOOR:   system_set_exec_state_flag(EXEC_SAFETY_DOOR); break; // Set as true
          case CMD_JOG_CANCEL:
            if (sys.state & STATE_JOG) { // Block all other states from invoking motion cancel.
              system_set_exec_state_flag(EXEC_MOTION_CANCEL);
            }
            break;
          #ifdef DEBUG
            case CMD_DEBUG_REPORT: bit_true(sys_rt_exec_debug,EXEC_DEBUG_REPORT); break;
          #endif
          case CMD_FEED_OVR_RESET: system_set_exec_motion_override_flag(EXEC_FEED_OVR_RESET); break;
          case CMD_FEED_OVR_COARSE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_PLUS); break;
          case CMD_FEED_OVR_COARSE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_COARSE_MINUS); break;
          case CMD_FEED_OVR_FINE_PLUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_PLUS); break;
          case CMD_FEED_OVR_FINE_MINUS: system_set_exec_motion_override_flag(EXEC_FEED_OVR_FINE_MINUS); break;
          case CMD_RAPID_OVR_RESET: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_RESET); break;
          case CMD_RAPID_OVR_MEDIUM: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_MEDIUM); break;
          case CMD_RAPID_OVR_LOW: system_set_exec_motion_override_flag(EXEC_RAPID_OVR_LOW); break;
          case CMD_SPINDLE_OVR_RESET: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_RESET); break;
          case CMD_SPINDLE_OVR_COARSE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_PLUS); break;
          case CMD_SPINDLE_OVR_COARSE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_COARSE_MINUS); break;
          case CMD_SPINDLE_OVR_FINE_PLUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_PLUS); break;
          case CMD_SPINDLE_OVR_FINE_MINUS: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_FINE_MINUS); break;
          case CMD_SPINDLE_OVR_STOP: system_set_exec_accessory_override_flag(EXEC_SPINDLE_OVR_STOP); break;
          case CMD_COOLANT_FLOOD_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_FLOOD_OVR_TOGGLE); break;
          #ifdef ENABLE_M7
            case CMD_COOLANT_MIST_OVR_TOGGLE: system_set_exec_accessory_override_flag(EXEC_COOLANT_MIST_OVR_TOGGLE); break;
          #endif
        }
        // Throw away any unfound extended-ASCII character by not passing it to the serial buffer.
      } else { // Write character to buffer
        // enter mutex
        next_head = serial_rx_buffer_head[client_idx] + 1;
        if (next_head == RX_RING_BUFFER) { next_head = 0; }

        // Write data to buffer unless it is full.
        if (next_head != serial_rx_buffer_tail[client_idx]) {
          serial_rx_buffer[client_idx][serial_rx_buffer_head[client_idx]] = data;
          serial_rx_buffer_head[client_idx] = next_head;
        }
        // exit mutex
      }
    }
  }
#ifdef ENABLE_WIFI
  wifi_handle();
#endif
#if (defined ENABLE_WIFI) && (defined ENABLE_WEBSOCKET)
  Serial2Socket.handle_flush();
#endif
}

void serial_reset_read_buffer(uint8_t client)
{
  for (uint8_t client_num = 1; client_num <= CLIENT_COUNT; client_num++) {
    if (client == client_num || client == CLIENT_ALL) {
      serial_rx_buffer_tail[client_num-1] = serial_rx_buffer_head[client_num-1];
    }
  }
}

// Writes one byte to the TX serial buffer. Called by main program.
void serial_write(uint8_t data) {
  Serial.write((char)data);
}

// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read(uint8_t client)
{
  uint8_t client_idx = client - 1;

  uint8_t tail = serial_rx_buffer_tail[client_idx]; // Temporary serial_rx_buffer_tail (to optimize for volatile)
  if (serial_rx_buffer_head[client_idx] == tail) {
    return SERIAL_NO_DATA;
  } else {
    // enter mutex
    uint8_t data = serial_rx_buffer[client_idx][tail];

    tail++;
    if (tail == RX_RING_BUFFER) { tail = 0; }
    serial_rx_buffer_tail[client_idx] = tail;
    // exit mutex
    return data;
  }
}

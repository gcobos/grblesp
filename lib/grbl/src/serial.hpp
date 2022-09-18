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

#ifndef serial_h
#define serial_h

#ifndef RX_BUFFER_SIZE
  #define RX_BUFFER_SIZE 342
#endif
#ifndef TX_BUFFER_SIZE
  #ifdef USE_LINE_NUMBERS
    #define TX_BUFFER_SIZE 300
  #else
    #define TX_BUFFER_SIZE 278
  #endif
#endif

#define SERIAL_NO_DATA 0xff

void serial_init();

// Writes one byte to the TX serial buffer. Called by main program.
void serial_write(uint8_t data);

// Fetches the first byte in the serial read buffer. Called by main program.
uint8_t serial_read(uint8_t client);

// Reset and empty data in read buffer. Used by e-stop and reset.
void serial_reset_read_buffer(uint8_t client);

// Returns the number of bytes available in the RX serial buffer.
uint8_t serial_get_rx_buffer_available(uint8_t client);

// Serial rx "interrupt"
void serial_poll_rx();

#endif

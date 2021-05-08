# GRBLESP

Implements the latest version of GRBL using ESP8266 only, being able to connect by serial port, raw socket or websocket. 
To overcome the problem of the lack of pins, it uses SPI to drive 4 shift registers of 8 bits each. The aim of having
32 pins is to be able to handle up to 8 axis.

Interfaces
----------

For the raw socket connection, you can use pyserial, which supports it through "socket" protocol.
**socket://[IP_OF_ESP_DEVICE]:23**. It can be tested easily with [bCNC](https://github.com/vlachoudis/bCNC)


For the websocket connection, the url is: **ws://[IP_OF_ESP_DEVICE]:80/ws**


Wiring
------

- GPIO0 controls inputs as RESET, FEED_HOLD, START_CYCLE and SAFETY_DOOR

- GPIO2 controls all limits switches

- GPIO12 remains unused (because it functions as SPI HMISO)

- GPIO13 sends data to the shift registers (SPI HMOSI)

- GPIO14 clock signal for the shift registers (SPI HSCLK)

- GPIO15 serves as CHIP_SELECT for the SPI

The board should use 4 x 8-bit shift registers. They are arranged this way:
- 8 bits for step pins
- 8 bits for direction pins
- 8 bits for Misc, inputs and outputs, following this structure (although it might change still a bit):

  #define CONTROL_RESET_BIT         0  
  #define CONTROL_FEED_HOLD_BIT     1  
  #define CONTROL_CYCLE_START_BIT   2  
  #define CONTROL_SAFETY_DOOR_BIT   3  
  #define PROBE_BIT                 4  
  #define COOLANT_FLOOD_BIT         5  
  #define COOLANT_MIST_BIT          6    
  #define STEPPERS_DISABLE_BIT      7

- 8 bits for limits. All inputs

Probe, spindle, and probably other features, are not yet supported

![Connection diagram](https://github.com/gcobos/grblesp/blob/master/schemas/spi-connection_schem.png)

#### J4
|Pin|Function|Type|
|-|-|-|
| 1 | STEP X | (output) |
| 2 | STEP Y | (output) |
| 3 | STEP Z | (output) |
| 4 | STEP A | (output) |
| 5 | STEP B | (output) |
| 6 | STEP C | (output) |
| 7 | STEP D | (output) |
| 8 | STEP E | (output) |

#### J3
|Pin|Function|Type|
|-|-|-|
| 1 | DIR X |(output) |
| 2 | DIR Y |(output) |
| 3 | DIR Z |(output) |
| 4 | DIR A |(output) |
| 5 | DIR B |(output) |
| 6 | DIR C |(output) |
| 7 | DIR D |(output) |
| 8 | DIR E |(output) |

#### J2
|Pin|Function|Type|
|-|-|-|
| 1 | RESET | (input) |
| 2 | FEED HOLD | (input) |
| 3 | CYCLE START | (input) |
| 4 | SAFETY DOOR | (input) |
| 5 | PROBE | (input) |
| 6 | COOLANT/FLOOD  |(output) |
| 7 | COOLANT/MIST  |(output) |
| 8 | STEPPERS_DISABLE |(output) |

#### J1
|Pin|Function|Type|
|-|-|-|
| 1 | LIMIT X | (input) |
| 2 | LIMIT Y | (input) |
| 3 | LIMIT Z | (input) |
| 4 | LIMIT A | (input) |
| 5 | LIMIT B | (input) |
| 6 | LIMIT C | (input) |
| 7 | LIMIT D | (input) |
| 8 | LIMIT E | (input) |

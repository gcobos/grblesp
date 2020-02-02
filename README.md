# GRBLESP

Implements the latest version of GRBL using ESP8266 only. To overcome the problem of the lack of pins, 
it uses SPI to drive 4 shift registers of 8 bits each.

The aim of having 32 pins is to be able to handle up to 8 axis.

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
  #define PROBE_BIT               4 
  #define COOLANT_FLOOD_BIT   5
  #define COOLANT_MIST_BIT    6
  #define SPINDLE_ENABLE_BIT    7
- 8 bits for limits. All inputs

Probe, spindle, and probably other features, are not yet supported


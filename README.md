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

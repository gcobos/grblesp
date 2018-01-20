// This file has been prepared for Doxygen automatic documentation generation.
/*! \file ********************************************************************
* Taken from ArduinoESP examples
******************************************************************************/

#include <EEPROM.h>

/*! \brief  Read byte from EEPROM.
 *
 *  This function Initializes the EEPROM buffer.
 *
 */
void eeprom_init()
{
	EEPROM.begin(2048);
}

/*! \brief  Read byte from EEPROM.
 *
 *  This function reads one byte from a given EEPROM address.
 *
 *  \note  The CPU is halted for 4 clock cycles during EEPROM read.
 *
 *  \param  addr  EEPROM address to read from.
 *  \return  The byte read from the EEPROM address.
 */
unsigned char eeprom_get_char( unsigned int addr )
{
	return EEPROM.read(addr); // Return the byte read from EEPROM.
}

/*! \brief  Write byte to EEPROM.
 *
 *  This function writes one byte to a given EEPROM address.
 *
 *  \param  addr  EEPROM address to write to.
 *  \param  new_value  New EEPROM value.
 */
void eeprom_put_char( unsigned int addr, unsigned char new_value )
{
	EEPROM.write(addr, new_value);
}

// Extensions added as part of Grbl


void memcpy_to_eeprom_with_checksum(unsigned int destination, char *source, unsigned int size) {
  unsigned char checksum = 0;
  for(; size > 0; size--) {
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += *source;
    eeprom_put_char(destination++, *(source++));
  }
  eeprom_put_char(destination, checksum);
	EEPROM.commit();
}

int memcpy_from_eeprom_with_checksum(char *destination, unsigned int source, unsigned int size) {
  unsigned char data, checksum = 0;
  for(; size > 0; size--) {
    data = eeprom_get_char(source++);
    checksum = (checksum << 1) || (checksum >> 7);
    checksum += data;
    *(destination++) = data;
  }
  return(checksum == eeprom_get_char(source));
}

// end of file

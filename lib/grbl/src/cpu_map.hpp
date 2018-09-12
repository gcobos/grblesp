/*
  cpu_map.hpp - Machine dependent code for the ESP8266 and family
  Part of Grbl

  Copyright (c) 2016 Sungeun K. Jeon for Gnea Research LLC

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

#ifndef cpu_map_h
#define cpu_map_h

#if (__GNUC__ == 4 && __GNUC_MINOR__ >= 1) || (__GNUC__ > 4)
#  define __INTR_ATTRS used, externally_visible
#else /* GCC < 4.1 */
#  define __INTR_ATTRS used
#endif

#define ISR(vector, ...)            \
    void vector (void) __attribute__ ((signal,__INTR_ATTRS)) __VA_ARGS__; \
    void vector (void)

#define save_SREG() xt_rsil(2); // this routine will allow level 3 and above (returns an uint32_t)
#define restore_SREG(state) xt_wsr_ps(state); sei(); // restore the state (uint32_t)

volatile static union {
  uint32_t data;
  struct {
    uint8_t LIMIT_PORT:8;     // All inputs
    uint8_t MISC_PORT:8;      // Inputs and outputs
    uint8_t STEP_PORT:8;      // All outputs
    uint8_t DIRECTION_PORT:8; // All outputs
  };
} regs;

#ifdef CPU_MAP_ESP8266

  // Define serial port pins and interrupt vectors.
  #define SERIAL_RX     USART_RX_vect
  #define SERIAL_UDRE   USART_UDRE_vect

  // Define step pulse output pins. NOTE: All step bit pins must be on the same port.
  #define STEP_PORT       regs.STEP_PORT
  #define X_STEP_BIT      0
  #define Y_STEP_BIT      1
  #define Z_STEP_BIT      2
  #define A_STEP_BIT      3
  #define B_STEP_BIT      4
  #define C_STEP_BIT      5
  #define D_STEP_BIT      6
  #define E_STEP_BIT      7
  #define STEP_MASK       ((1<<X_STEP_BIT)|(1<<Y_STEP_BIT)|(1<<Z_STEP_BIT)|(1<<A_STEP_BIT)|(1<<B_STEP_BIT)|(1<<C_STEP_BIT)|(1<<D_STEP_BIT)|(1<<E_STEP_BIT)) // All step bits

  // Define step direction output pins. NOTE: All direction pins must be on the same port.
  #define DIRECTION_PORT    regs.DIRECTION_PORT
  #define X_DIRECTION_BIT      0
  #define Y_DIRECTION_BIT      1
  #define Z_DIRECTION_BIT      2
  #define A_DIRECTION_BIT      3
  #define B_DIRECTION_BIT      4
  #define C_DIRECTION_BIT      5
  #define D_DIRECTION_BIT      6
  #define E_DIRECTION_BIT      7
  #define DIRECTION_MASK       ((1<<X_DIRECTION_BIT)|(1<<Y_DIRECTION_BIT)|(1<<Z_DIRECTION_BIT)|(1<<A_DIRECTION_BIT)|(1<<B_DIRECTION_BIT)|(1<<C_DIRECTION_BIT)|(1<<D_DIRECTION_BIT)|(1<<E_DIRECTION_BIT)) // All direction bits

  // Define stepper driver enable/disable output pin.
  //#define STEPPERS_DISABLE_PORT   regs.MISC_PORT
  //#define STEPPERS_DISABLE_BIT    0
  //#define STEPPERS_DISABLE_MASK   (1<<STEPPERS_DISABLE_BIT)

  // Define homing/hard limit switch input pins and limit interrupt vectors.
  // NOTE: All limit bit pins must be on the same port, but not on a port with other input pins (CONTROL).
  #define LIMIT_PORT       regs.LIMIT_PORT
  #define X_LIMIT_BIT      0
  #define Y_LIMIT_BIT      1
  #define Z_LIMIT_BIT	     2
  #define A_LIMIT_BIT	     3
  #define B_LIMIT_BIT	     4
  #define C_LIMIT_BIT	     5
  #define D_LIMIT_BIT	     6
  #define E_LIMIT_BIT	     7
  #define LIMIT_MASK       ((1<<X_LIMIT_BIT)|(1<<Y_LIMIT_BIT)|(1<<Z_LIMIT_BIT)|(1<<A_LIMIT_BIT)|(1<<B_LIMIT_BIT)|(1<<C_LIMIT_BIT)|(1<<D_LIMIT_BIT)|(1<<E_LIMIT_BIT)) // All limit bits

  //#define LIMIT_INT        PCIE0  // Pin change interrupt enable pin
  //#define LIMIT_INT_vect   PCINT0_vect
  //#define LIMIT_PCMSK      PCMSK0 // Pin change interrupt register

  // Define spindle enable and spindle direction output pins.
  #define SPINDLE_ENABLE_PORT  regs.MISC_PORT
  // Z Limit pin and spindle PWM/enable pin swapped to access hardware PWM on Pin 11.
  #ifdef VARIABLE_SPINDLE
    #ifdef USE_SPINDLE_DIR_AS_ENABLE_PIN
      // If enabled, spindle direction pin now used as spindle enable, while PWM remains on D11.
      #define SPINDLE_ENABLE_BIT    7
    #else
      #define SPINDLE_ENABLE_BIT    7
    #endif
  #else
    #define SPINDLE_ENABLE_BIT    7
  #endif
  #ifndef USE_SPINDLE_DIR_AS_ENABLE_PIN
    #define SPINDLE_DIRECTION_BIT   7
  #endif

  // Define flood and mist coolant enable output pins.
  #define COOLANT_FLOOD_PORT  regs.MISC_PORT
  #define COOLANT_FLOOD_BIT   5
  #define COOLANT_MIST_PORT   regs.MISC_PORT
  #define COOLANT_MIST_BIT    6

  // Define user-control controls (cycle start, reset, feed hold) input pins.
  // NOTE: All CONTROLs pins must be on the same port and not on a port with other input pins (limits).
  #define CONTROL_PORT        regs.MISC_PORT
  #define CONTROL_RESET_BIT         0
  #define CONTROL_FEED_HOLD_BIT     1
  #define CONTROL_CYCLE_START_BIT   2
  #define CONTROL_SAFETY_DOOR_BIT   3

  //#define CONTROL_INT       PCIE1  // Pin change interrupt enable pin
  //#define CONTROL_INT_vect  PCINT1_vect
  //#define CONTROL_PCMSK     PCMSK1 // Pin change interrupt register
  #define CONTROL_MASK      ((1<<CONTROL_RESET_BIT)|(1<<CONTROL_FEED_HOLD_BIT)|(1<<CONTROL_CYCLE_START_BIT)|(1<<CONTROL_SAFETY_DOOR_BIT))
  #define CONTROL_INVERT_MASK   CONTROL_MASK // May be re-defined to only invert certain control pins.

  // Define probe switch input pin.
  #define PROBE_PORT        regs.MISC_PORT
  #define PROBE_BIT               4
  #define PROBE_MASK      (1<<PROBE_BIT)

  // Variable spindle configuration below. Do not change unless you know what you are doing.
  // NOTE: Only used when variable spindle is enabled.
  #define SPINDLE_PWM_MAX_VALUE     255 // Don't change. 328p fast PWM mode fixes top value as 255.
  #ifndef SPINDLE_PWM_MIN_VALUE
    #define SPINDLE_PWM_MIN_VALUE   1   // Must be greater than zero.
  #endif
  #define SPINDLE_PWM_OFF_VALUE     0
  #define SPINDLE_PWM_RANGE         (SPINDLE_PWM_MAX_VALUE-SPINDLE_PWM_MIN_VALUE)
  //#define SPINDLE_TCCRA_REGISTER	  TCCR2A
  //#define SPINDLE_TCCRB_REGISTER	  TCCR2B
  //#define SPINDLE_OCR_REGISTER      OCR2A
  //#define SPINDLE_COMB_BIT	        COM2A1

  // Prescaled, 8-bit Fast PWM mode.
  #define SPINDLE_TCCRA_INIT_MASK   ((1<<WGM20) | (1<<WGM21))  // Configures fast PWM mode.
  // #define SPINDLE_TCCRB_INIT_MASK   (1<<CS20)               // Disable prescaler -> 62.5kHz
  // #define SPINDLE_TCCRB_INIT_MASK   (1<<CS21)               // 1/8 prescaler -> 7.8kHz (Used in v0.9)
  // #define SPINDLE_TCCRB_INIT_MASK   ((1<<CS21) | (1<<CS20)) // 1/32 prescaler -> 1.96kHz
  #define SPINDLE_TCCRB_INIT_MASK      (1<<CS22)               // 1/64 prescaler -> 0.98kHz (J-tech laser)

  // NOTE: On the 328p, these must be the same as the SPINDLE_ENABLE settings.
  #define SPINDLE_PWM_DDR	  DDRB
  #define SPINDLE_PWM_PORT  PORTB
  #define SPINDLE_PWM_BIT	  3    // Uno Digital Pin 11

#endif

/*
#ifdef CPU_MAP_CUSTOM_PROC
  // For a custom pin map or different processor, copy and edit one of the available cpu
  // map files and modify it to your needs. Make sure the defined name is also changed in
  // the config.h file.
#endif
*/

#endif /* CPU_MAP_H_ */

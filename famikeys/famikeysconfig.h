#ifndef FAMIKEYSCONFIG_H
#define FAMIKEYSCONFIG_H

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)

// Famicom Pin 7 - Controller 2 D1
// Arduino Mega Analog Pin A0
#define FK_D1_DDR  DDRF
#define FK_D1_PORT PORTF
#define FK_D1_PIN  PINF
#define FK_D1_MASK 0x01

// Famicom Pin 6 - Controller 2 D2
// Arduino Mega Analog Pin A1
#define FK_D2_DDR  DDRF
#define FK_D2_PORT PORTF
#define FK_D2_PIN  PINF
#define FK_D2_MASK 0x02

// Famicom Pin 5 - Controller 2 D3
// Arduino Mega Analog Pin A2
#define FK_D3_DDR  DDRF
#define FK_D3_PORT PORTF
#define FK_D3_PIN  PINF
#define FK_D3_MASK 0x04

// Famicom Pin 4 - Controller 2 D4
// Arduino Mega Analog Pin A3
#define FK_D4_DDR  DDRF
#define FK_D4_PORT PORTF
#define FK_D4_PIN  PINF
#define FK_D4_MASK 0x08

// Famicom Pin 12 - OUT0 - $4016 bit 0 - keyboard reset
// Arduino Mega Analog Pin A4
#define FK_RST_DDR  DDRF
#define FK_RST_PORT PORTF
#define FK_RST_PIN  PINF
#define FK_RST_MASK 0x10

// Famicom Pin 11 - OUT1 - $4016 bit 1 - column/row increment
// Arduino Mega Analog Pin A5
#define FK_INC_DDR  DDRF
#define FK_INC_PORT PORTF
#define FK_INC_PIN  PINF
#define FK_INC_MASK 0x20

// Famicom Pin 10 - OUT2 - $4016 bit 2 - keyboard enable
// Arduino Mega Digital Pin D6
#define FK_ENA_DDR  DDRH
#define FK_ENA_PORT PORTH
#define FK_ENA_PIN  PINH
#define FK_ENA_MASK 0x08

#else

// Famicom Pin 7 - Controller 2 D1
// Arduino Uno Analog Pin A0
#define FK_D1_DDR  DDRC
#define FK_D1_PORT PORTC
#define FK_D1_PIN  PINC
#define FK_D1_MASK 0x01

// Famicom Pin 6 - Controller 2 D2
// Arduino Uno Analog Pin A1
#define FK_D2_DDR  DDRC
#define FK_D2_PORT PORTC
#define FK_D2_PIN  PINC
#define FK_D2_MASK 0x02

// Famicom Pin 5 - Controller 2 D3
// Arduino Uno Analog Pin A2
#define FK_D3_DDR  DDRC
#define FK_D3_PORT PORTC
#define FK_D3_PIN  PINC
#define FK_D3_MASK 0x04

// Famicom Pin 4 - Controller 2 D4
// Arduino Uno Analog Pin A3
#define FK_D4_DDR  DDRC
#define FK_D4_PORT PORTC
#define FK_D4_PIN  PINC
#define FK_D4_MASK 0x08

// Famicom Pin 12 - OUT0 - $4016 bit 0 - keyboard reset
// Arduino Uno Analog Pin A4
#define FK_RST_DDR  DDRC
#define FK_RST_PORT PORTC
#define FK_RST_PIN  PINC
#define FK_RST_MASK 0x10

// Famicom Pin 11 - OUT1 - $4016 bit 1 - column/row increment
// Arduino Uno Analog Pin A5
#define FK_INC_DDR  DDRC
#define FK_INC_PORT PORTC
#define FK_INC_PIN  PINC
#define FK_INC_MASK 0x20

// Famicom Pin 10 - OUT2 - $4016 bit 2 - keyboard enable
// Arduino Uno Digital Pin D6
#define FK_ENA_DDR  DDRD
#define FK_ENA_PORT PORTD
#define FK_ENA_PIN  PIND
#define FK_ENA_MASK 0x40

// Pin Change Interrupts on OUT0, OUT1, OUT2
// Arduino Uno Pins A4, A5, and D6
#define FK_PCICR  0x06
#define FK_PCMSK1 0x30
#define FK_PCMSK2 0x40

#endif

#endif

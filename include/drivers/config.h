#ifndef CONFIG_H
#define CONFIG_H

// This header file defines a set of macros for configuring pins on AVR 
// microcontrollers. These macros simplify the process of setting pins as input 
// or output, enabling or disabling pull-up resistors, and manipulating bit 
// values within data direction (DDR) and PORT registers based on pin type 
// and number.

#include <avr/io.h>

//=============================================================================
// MCU Clock Configuration
// Example usage: #define F_CPU 16000000UL
//=============================================================================
// Check if F_CPU is defined
#ifndef F_CPU
#define F_CPU 16000000UL // Default to 16 MHz if not defined
#endif

//=============================================================================
// Pin Configuration
// Example usage: ENABLE_INPUT(DIGITAL_PIN, 9);
//=============================================================================
enum PinType {
    DIGITAL_PIN,
    ANALOG_PIN
};

// Macros for configuring pins
#define ENABLE_INPUT(pinType, pin) \
    (*(DDR_FOR_PIN(pinType, pin))  &= ~(1 << BIT_FOR_PIN(pinType, pin)))
#define ENABLE_OUTPUT(pinType, pin) \
    (*(DDR_FOR_PIN(pinType, pin))  |=  (1 << BIT_FOR_PIN(pinType, pin)))
#define ENABLE_PULLUP(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) |=  (1 << BIT_FOR_PIN(pinType, pin)))

#define DISABLE_INPUT(pinType, pin) \
    (*(DDR_FOR_PIN(pinType, pin))  |=  (1 << BIT_FOR_PIN(pinType, pin)))
#define DISABLE_OUTPUT(pinType, pin) \
    (*(DDR_FOR_PIN(pinType, pin))  &= ~(1 << BIT_FOR_PIN(pinType, pin)))
#define DISABLE_PULLUP(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) &= ~(1 << BIT_FOR_PIN(pinType, pin)))

// Macro to obtain a pointer to the appropriate DDR register for a given pin
#define DDR_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) >= 0 && (pin) <= 7 ? &DDRD : \
         (pin) >= 8 && (pin) <= 13 ? &DDRB : nullptr) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) >= 0 && (pin) <= 5 ? &DDRC : nullptr) : nullptr)

// Macro to obtain a pointer to the appropriate PORT register for a given pin
#define PORT_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) >= 0 && (pin) <= 7 ? &PORTD : \
         (pin) >= 8 && (pin) <= 13 ? &PORTB : nullptr) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) >= 0 && (pin) <= 5 ? &PORTC : nullptr) : nullptr)

// Macro to determine the correct bit within a register for a given pin
#define BIT_FOR_PIN(pinType, pin) \
    ((pinType) == DIGITAL_PIN ? \
        ((pin) == 0 ?  PORTD0 : \
         (pin) == 1 ?  PORTD1 : \
         (pin) == 2 ?  PORTD2 : \
         (pin) == 3 ?  PORTD3 : \
         (pin) == 4 ?  PORTD4 : \
         (pin) == 5 ?  PORTD5 : \
         (pin) == 6 ?  PORTD6 : \
         (pin) == 7 ?  PORTD7 : \
         (pin) == 8 ?  PORTB0 : \
         (pin) == 9 ?  PORTB1 : \
         (pin) == 10 ? PORTB2 : \
         (pin) == 11 ? PORTB3 : \
         (pin) == 12 ? PORTB4 : \
         (pin) == 13 ? PORTB5 : 0) : \
     (pinType) == ANALOG_PIN ? \
        ((pin) == 0 ?  PORTC0 : \
         (pin) == 1 ?  PORTC1 : \
         (pin) == 2 ?  PORTC2 : \
         (pin) == 3 ?  PORTC3 : \
         (pin) == 4 ?  PORTC4 : \
         (pin) == 5 ?  PORTC5 : 0) : 0)

//=============================================================================
// Pin Macros
// Example: SET_HIGH(DIGITAL_PIN, 9);
//=============================================================================
#define SET_HIGH(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) |=  (1 << BIT_FOR_PIN(pinType, pin)))
#define SET_LOW(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) &= ~(1 << BIT_FOR_PIN(pinType, pin)))
#define TOGGLE(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) ^=  (1 << BIT_FOR_PIN(pinType, pin)))
#define IS_HIGH(pinType, pin) \
    (*(PORT_FOR_PIN(pinType, pin)) & (1 << BIT_FOR_PIN(pinType, pin)))
#define IS_LOW(pinType, pin) \
    !(*(PORT_FOR_PIN(pinType, pin)) & (1 << BIT_FOR_PIN(pinType, pin)))

//=============================================================================
// RX/TX UART Configuration
// Example: ENABLE_UART(9600, 8);
//=============================================================================
// Enable UART with specific baud rate and data bits
#define ENABLE_UART(baudRate, dataBits) do { \
    SET_UART_BAUD_RATE(baudRate); \
    ENABLE_UART_TX(); \
    ENABLE_UART_RX(); \
    SET_UART_DATA_BITS(dataBits); \
} while (0)

// Disable UART by turning off transmitter and receiver
#define DISABLE_UART() do { \
    DISABLE_UART_TX(); \
    DISABLE_UART_RX(); \
} while (0)

// Bitmask for checking RXEN0 and TXEN0 bits in UCSR0B (RX/TX enable bits)
#define UART_ENABLED_MASK ((1 << RXEN0) | (1 << TXEN0))

// Boolean macro to check if UART is enabled
#define IS_UART_ENABLED() \
    ((UCSR0B & UART_ENABLED_MASK) == UART_ENABLED_MASK ? true : false)

// Helper macros for enabling/disabling UART transmitter and receiver
#define ENABLE_UART_TX()  (UCSR0B |=  (1 << TXEN0))
#define ENABLE_UART_RX()  (UCSR0B |=  (1 << RXEN0))
#define DISABLE_UART_TX() (UCSR0B &= ~(1 << TXEN0))
#define DISABLE_UART_RX() (UCSR0B &= ~(1 << RXEN0))

// Macro to calculate UBRR (Baud rate register) for given baud rate
#define UBRR_VALUE(baudRate) ((F_CPU / 16 / (baudRate)) - 1)

// Set UART baud rate (e.g., 9600, 19200, 38400, 57600, 115200, etc.)
#define SET_UART_BAUD_RATE(baudRate) do { \
    UBRR0H = (UBRR_VALUE(baudRate) >> 8); \
    UBRR0L =  UBRR_VALUE(baudRate); \
} while (0)

// Set UART data bits (5, 6, 7, 8, or 9 bits)
#define SET_UART_DATA_BITS(bits) do { \
    UCSR0C = (UCSR0C & ~((1 << UCSZ00) | (1 << UCSZ01))) | \
              (((bits) - 5) << UCSZ00); \
    if ((bits) == 9) UCSR0B |= (1 << UCSZ02); \
    else UCSR0B &= ~(1 << UCSZ02); \
} while (0)

// Define useful UART macros
#define UART_DATA_REGISTER       UDR0
#define UART_DATA_REGISTER_EMPTY (!(UCSR0A & (1<<UDRE0)))
#define UART_RECEIVE_COMPLETE    (UCSR0A & (1<<RXC0))

// Enable UART Interrupts
#define ENABLE_UART_RX_INTERRUPT() (UCSR0B |= (1 << RXCIE0))

//=============================================================================
// Interupt Configuration
// Example: ENABLE_INTERRUPT_FOR_PIN(9);
//=============================================================================
#define ENABLE_INTERRUPT_FOR_PIN(pin) do { \
    if ((pin) >= 0 && (pin) <= 7) { \
        ENABLE_PCINT_FOR_PORTD(); \
        ENABLE_PCINT_FOR_PIND(pin); \
    } else if ((pin) >= 8 && (pin) <= 13) { \
        ENABLE_PCINT_FOR_PORTB(); \
        ENABLE_PCINT_FOR_PINB(pin - 8); \
    } else if ((pin) >= 14 && (pin) <= 19) { \
        ENABLE_PCINT_FOR_PORTC(); \
        ENABLE_PCINT_FOR_PINC(pin - 14); \
    } \
} while (0)

// Enable pin change interrupt for all pins on a specific port
#define ENABLE_PCINT_FOR_PORTB()   (PCICR |=  (1 << PCIE0))
#define ENABLE_PCINT_FOR_PORTC()   (PCICR |=  (1 << PCIE1))
#define ENABLE_PCINT_FOR_PORTD()   (PCICR |=  (1 << PCIE2))

// Disable pin change interrupt for all pins on a specific port
#define DISABLE_PCINT_FOR_PORTB()  (PCICR &= ~(1 << PCIE0))
#define DISABLE_PCINT_FOR_PORTC()  (PCICR &= ~(1 << PCIE1))
#define DISABLE_PCINT_FOR_PORTD()  (PCICR &= ~(1 << PCIE2))

// Enable pin change interrupt for a specific pin on a specific port
#define ENABLE_PCINT_FOR_PINB(pin)  (PCMSK0 |=  (1 << (pin)))
#define ENABLE_PCINT_FOR_PINC(pin)  (PCMSK1 |=  (1 << (pin)))
#define ENABLE_PCINT_FOR_PIND(pin)  (PCMSK2 |=  (1 << (pin)))

// Disable pin change interrupt for a specific pin on a specific port
#define DISABLE_PCINT_FOR_PINB(pin) (PCMSK0 &= ~(1 << (pin)))
#define DISABLE_PCINT_FOR_PINC(pin) (PCMSK1 &= ~(1 << (pin)))
#define DISABLE_PCINT_FOR_PIND(pin) (PCMSK2 &= ~(1 << (pin)))

//======================================================================
// PWM Definitions
// Description: Macros for configuring PWM pins on AVR microcontrollers
// Example usage: SETUP_PWM_FOR_PIN(9); will set up PWM for pin 9
//======================================================================
#define PWM_DDR(pin) (*(&DDR##pin))
#define PWM_PORT(pin) (*(&PORT##pin))
#define PWM_PIN(pin) (*(&PIN##pin))

#define CONFIGURE_PWM_DDR(ddr, port) ((ddr) |= (1 << (port)))

#define CONFIGURE_PWM(ddr, port, ocr, com, wgm0, wgm1, cs) \
    ddr |= (1 << port), /* Set pin as output */ \
    TCCR##ocr##A = (1 << com) | (1 << wgm0), /* Setup PWM mode */ \
    TCCR##ocr##B = (1 << cs) | (1 << wgm1) /* Setup prescaler and additional mode bits */

#define SETUP_PWM_FOR_PIN(pin) \
    if ((pin) == 3) { CONFIGURE_PWM(DDRD, PORTD3, 2, COM2B1, WGM20, WGM21, CS21); } \
    else if ((pin) == 5) { CONFIGURE_PWM(DDRD, PORTD5, 0, COM0B1, WGM00, WGM01, CS01); } \
    else if ((pin) == 6) { CONFIGURE_PWM(DDRD, PORTD6, 0, COM0A1, WGM00, WGM01, CS01); } \
    else if ((pin) == 9) { CONFIGURE_PWM(DDRB, PORTB1, 1, COM1A1, WGM10, WGM11, CS11); } \
    else if ((pin) == 10) { CONFIGURE_PWM(DDRB, PORTB2, 1, COM1B1, WGM10, WGM11, CS11); } \
    else if ((pin) == 11) { CONFIGURE_PWM(DDRB, PORTB3, 2, COM2A1, WGM20, WGM21, CS21); }

#endif // CONFIG_H
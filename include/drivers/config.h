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



#endif // CONFIG_H
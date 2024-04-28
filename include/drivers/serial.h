#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

//======================================================================
// Macros for UART Configuration
//======================================================================
#ifndef F_CPU
#define F_CPU 16000000UL // Define MCU clock speed if not defined
#endif

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

#define BUFFER_SIZE 64

extern volatile char uart_buffer[BUFFER_SIZE];
extern volatile uint8_t uart_read_pos;
extern volatile uint8_t uart_write_pos;
extern volatile bool uart_command_ready;

class Serial {
public:
    // Constructor
    Serial();

    // Public init method
    void uart_init(const uint32_t& baud_rate, const uint8_t& data_bits);

    // Public UART methods
    void uart_put_char(unsigned char data);
    void uart_put_str(const char* str);
    bool uart_get_char(char* character);
    void uart_rec_str(char* buffer, const uint8_t& buf_size);
    void uart_echo();

private:
    bool initialized;    // Flag to check if UART is initialized

    // Private constexpr methods (compile-time validations)
    static constexpr bool valid_baud(const uint32_t baud_rate);
    static constexpr bool valid_bits(const uint8_t data_bits);
    static constexpr uint8_t valid_buf_size();
};

#endif // SERIAL_H
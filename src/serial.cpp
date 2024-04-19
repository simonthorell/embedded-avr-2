//======================================================================
// Serial Class Implementation
//======================================================================
#include "drivers/serial.h"
#include "drivers/config.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

// Circular buffer for UART data
volatile char    uart_buffer[BUFFER_SIZE];
volatile uint8_t uart_next_pos = 0;
volatile uint8_t uart_read_pos = 0;
volatile uint8_t uart_write_pos = 0;
volatile bool    uart_command_ready = false;
volatile bool    uart_buffer_overflow = false;

// Interrupt Service Routine for UART receive
ISR(USART_RX_vect) {
    char rec_char = UART_DATA_REGISTER;
    uart_next_pos = (uart_write_pos + 1) % BUFFER_SIZE;

    // Check for string terminator to set command ready flag
    if (rec_char == '\n') {
        uart_command_ready = true;
    }

    // Check for buffer overflow (if no overflow, write to buffer)
    if (uart_next_pos != uart_read_pos) {
        uart_buffer_overflow = false;
        uart_buffer[uart_write_pos] = rec_char;
        uart_write_pos = uart_next_pos;
    } else {
        uart_buffer_overflow = true;
        // Buffer is full/overflowed - Error is handled in read method!
    }
}

//======================================================================
// Constructor: Serial
// Description: Initializes the Serial object with the specified buffer
//              size and checks if the UART is initialized in registers.
//======================================================================
Serial::Serial() : initialized(IS_UART_ENABLED()) {
}

//======================================================================
// Public Method: init
// Description: This method initializes the UART module with the
//              specified baud rate and data bits.
//======================================================================
void Serial::uart_init(const uint32_t& baud_rate, const uint8_t& data_bits){
    // Check if arguments are valid before initializing
    if (valid_baud(baud_rate) && valid_bits(data_bits)) {

        // Initialize UART using macro defined in ´config.h´
        ENABLE_UART(baud_rate, data_bits);
        initialized = true;
        char buffer[64];
        sprintf(buffer, "UART Initialized with %lu baud rate and %u-bits\r\n", baud_rate, data_bits);
        uart_put_str(buffer);

    } else {
        initialized = false;
        uart_put_str("UART Initialization Failed!\r\n");
    }
}

//======================================================================
// Public Methods: put_char, put_str, uart_getchar, uart_rec_str
// Description: These methods are used to transmit and receive data
//              via UART.
//======================================================================
// Transmits a single character
void Serial::uart_put_char(unsigned char data) {
    // Return if UART is not initialized
    if (!initialized) return;

    while (!(UCSR0A & (1<<UDRE0))); // Wait for empty transmit buffer
    UDR0 = data;                    // Send data
}

// Prints a string via USART
void Serial::uart_put_str(const char* str) {
    // Return if UART is not initialized
    if (!initialized) return;

    while (*str) {
        this->uart_put_char(*str++); // Transmit each character
    }
}

bool Serial::uart_get_char(char* character) {
    if (!initialized) {
        return false;
    }

    if (uart_read_pos != uart_write_pos) {
        // There is data available
        *character = uart_buffer[uart_read_pos++];
        if (uart_read_pos >= BUFFER_SIZE) {
            uart_read_pos = 0; // Wrap around if at the end of the buffer
        }
        return true; // Indicate that a character was read
    }
    return false; // No data was available to read
}

void Serial::uart_rec_str(char* buffer, const uint8_t& buf_size) {
    if (!initialized) {
        buffer[0] = '\0'; // Ensure the buffer is null-terminated
        return;
    }

    // Make sure the buffer is <= specified BUFFER_SIZE
    if (buf_size < BUFFER_SIZE) {
        char* error_msg = "Specified buffer size is too small!\n";
        uart_put_str(error_msg);
        buffer[0] = '\0'; // Ensure the buffer is null-terminated
        return;
    }

    // Command longer than buffer size => buffer overflow...
    if (uart_buffer_overflow) {
        char* error_msg = "Buffer overflowed, make sure your command is within buffer range!\n";
        uart_put_str(error_msg);
        buffer[0] = '\0'; // Ensure the buffer is null-terminated
        return;
    }

    unsigned char charCount = 0;

    // Loop until newline or buffer is full
    while (uart_read_pos != uart_write_pos && charCount < (BUFFER_SIZE - 1)) {
        // Read a character from the circular buffer
        char receivedChar = uart_buffer[uart_read_pos++];
        if (uart_read_pos >= BUFFER_SIZE) uart_read_pos = 0;

        if (receivedChar == '\n') break;    // Stop on newline
        buffer[charCount++] = receivedChar; // Store character
    }

    buffer[charCount] = '\0'; // Null-terminate the string
}

void Serial::uart_echo() {
    char c;
    // Check if a character was successfully read
    if (uart_get_char(&c)) {
        uart_put_char(c); // Echo the character back
    }
}

//======================================================================
// Private Methods: valid_baud, valid_bits, valid_buf_size
// Description: Using constexpr to validate baud rate, data bits, and
//              buffer size at compile time for saving memory.
//======================================================================
constexpr bool Serial::valid_baud(uint32_t baud_rate) {
    return baud_rate == 9600 || baud_rate == 19200 || baud_rate == 38400 ||
           baud_rate == 57600 || baud_rate == 115200;
}

constexpr bool Serial::valid_bits(uint8_t data_bits) {
    return data_bits == 5 || data_bits == 6 || data_bits == 7 || 
           data_bits == 8 || data_bits == 9;
}

constexpr uint8_t Serial::valid_buf_size() {
    return BUFFER_SIZE < 1 ? 1 : (BUFFER_SIZE >= 255 ? 255 : BUFFER_SIZE);
}
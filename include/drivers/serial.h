#ifndef SERIAL_H
#define SERIAL_H

#include <avr/io.h>

#define BUFFER_SIZE 64 // Buffer size for UART communication

//==============================================================================
// Global Variables
//==============================================================================
extern volatile char uart_buffer[BUFFER_SIZE];
extern volatile uint8_t uart_read_pos;
extern volatile uint8_t uart_write_pos;
extern volatile bool uart_command_ready;

//==============================================================================
// Serial Class Declaration
//==============================================================================
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
    bool initialized; // Flag to check if UART is initialized

    // Private constexpr methods (compile-time validations)
    static constexpr bool valid_baud(const uint32_t baud_rate);
    static constexpr bool valid_bits(const uint8_t data_bits);
    static constexpr uint8_t valid_buf_size();
};

#endif // SERIAL_H
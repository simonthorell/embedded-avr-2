// USART Class Implementation
#include "usart.h"
#include <util/delay.h>

// Constructor
USART::USART() {}

// Initializes USART with given baud rate
void USART::init(unsigned int ubrr) {
    UBRR0H = (unsigned char)(ubrr >> 8); // Set high byte of baud rate
    UBRR0L = (unsigned char)ubrr;        // Set low byte of baud rate
    UCSR0B = (1<<RXEN0) | (1<<TXEN0);    // Enable receiver and transmitter
    UCSR0C = (1<<USBS0) | (3<<UCSZ00);   // Set frame format: 8 data bits, 2 stop bits
}

// Transmits a single character
void USART::transmit(unsigned char data) {
    while (!(UCSR0A & (1<<UDRE0))); // Wait for empty transmit buffer
    UDR0 = data;                    // Send data
}

// Receives a single character
unsigned char USART::receive() {
    while (!(UCSR0A & (1<<RXC0))); // Wait for data to be received
    return UDR0;                   // Return received data
}

// Prints a string via USART
void USART::print(const char* str) {
    while (*str) {
        this->transmit(*str++); // Transmit each character
    }
}

// Receives a string until newline or buffer limit
void USART::receiveString(char* buffer, unsigned char max_length) {
    unsigned char receivedChar;
    unsigned char charCount = 0;

    // Clear buffer
    for (unsigned char i = 0; i < max_length; i++) buffer[i] = '\0';

    do {
        receivedChar = this->receive(); // Receive a character

        // Store character if it's not newline and buffer isn't full
        if (receivedChar != '\n' && charCount < (max_length - 1)) {
            buffer[charCount++] = receivedChar;
        }
    } while (receivedChar != '\n' && charCount < (max_length - 1));

    buffer[charCount] = '\0'; // Null-terminate the received string
}
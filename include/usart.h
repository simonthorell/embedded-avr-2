#ifndef USART_H
#define USART_H

#include <avr/io.h>

#define F_CPU 16000000UL         // CPU frequency in Hz
#define BAUD 9600                // USART baud rate
#define MY_UBRR F_CPU/16/BAUD-1  // UBRR value for 9600 baud rate
#define USART_CMD_BUFFER 32      // USART command buffer size

class USART {
public:
    USART();
    void init(unsigned int ubrr = MY_UBRR);
    void transmit(unsigned char data);
    unsigned char receive();
    void print(const char* str);
    void receiveString(char* buffer, unsigned char max_length);
};

#endif // USART_H

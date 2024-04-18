// Main application file
#include <avr/io.h>
#include <avr/pgmspace.h>
#include "usart.h"

void loop(USART &serial);

int main(void) {
    USART serial;
    serial.init(); // Init USART with default baud rate

    loop(serial);  // Run the app loop
    
    return 0;
}

void loop(USART &serial) {
    char receivedCommand[USART_CMD_BUFFER]; // command buffer

    while (1) {
        serial.print("Type a command: ");
        serial.receiveString(receivedCommand, USART_CMD_BUFFER);
        
        /* TODO: Add command processing logic here... */

        // Echo back the received string for testing
        serial.print("\r\nYou typed: ");
        serial.print(receivedCommand);
        serial.print("\r\n");
    }
}
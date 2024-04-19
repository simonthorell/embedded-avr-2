//=============================================================================
// Main application file
//=============================================================================
#include <avr/io.h>
#include "drivers/serial.h"

#define F_CPU     16000000UL  // Set MCU clock speed to 16MHz
#define DATA_BITS 8           // Set data bits for 8-bit MCU
#define BAUD_RATE 9600        // Set baud rate to 9600 bps

// Declare function prototypes
void loop(Serial &serial);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    // Initialize RX/TX with defined baud rate and data bits
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    // Run Main Loop Forever
    loop(serial);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial) {
    while (true) {

        // Check serial buffer if command is ready
        if (uart_command_ready) {
            char buffer[64];
            serial.uart_rec_str(buffer, 64);
            serial.uart_put_str(buffer);
        }
    }

}
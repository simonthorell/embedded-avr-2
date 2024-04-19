//=============================================================================
// Main application file
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "led.h"

#define F_CPU     16000000UL  // Set MCU clock speed to 16MHz
#define DATA_BITS 8           // Set data bits for 8-bit MCU
#define BAUD_RATE 9600        // Set baud rate to 9600 bps

#define BLINK_INTERVAL 1000   // Set LED blink interval in ms

// Declare function prototypes
void loop(Serial &serial, LED &led, Timer &timer);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    // Initialize RX/TX with defined baud rate and data bits
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    // Create a timer using Timer1 with 200ms duration
    Timer timer1(TIMER_1, MILLIS, F_CPU);
    // serial.uart_put_str("Initialized Timer...\n");

    // Create an LED object using digital pin 3
    LED led_purple(8);
    // serial.uart_put_str("Initialized LED...\n");

    // Enable Interupts
    sei();

    // Run Main Loop Forever
    loop(serial, led_purple, timer1);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_purple, Timer &timer1) {
    serial.uart_put_str("Loop started...\n");

    while (true) {
        
        // Check if timer is completed
        if (timer1.checkInterval(BLINK_INTERVAL)) {
            serial.uart_put_str("Timer completed...\n");
            led_purple.toggle();
        }

        // Check serial buffer if command is ready
        if (uart_command_ready) {
            char rec_cmd[BUFFER_SIZE];
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            serial.uart_put_str("Received Command: ");
            serial.uart_put_str(rec_cmd);  // Print received command
            serial.uart_put_str("\r\n");   // Ensure newline after command
            uart_command_ready = false;    // Reset command ready flag
        }

    }

}
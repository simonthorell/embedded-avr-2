//=============================================================================
// Main application file
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "led.h"
#include <stdlib.h>

#define F_CPU     16000000UL  // Set MCU clock speed to 16MHz
#define DATA_BITS 8           // Set data bits for 8-bit MCU
#define BAUD_RATE 9600        // Set baud rate to 9600 bps

#define BLINK_INTERVAL 1000  // Set LED blink interval (us/ms based on timer unit)

// Declare function prototypes
void loop(Serial &serial, LED &led, Timer &timer_1);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    sei(); // Enable Interupts globally

    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    /* TODO: MILLIS works fine for all timers, but micros is way off... */
    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.start();

    LED led_purple(8);

    loop(serial, led_purple, timer_1);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_purple, Timer &timer_1) {
    serial.uart_put_str("Loop started...\n");

    while (true) {

        if (timer_1.overflow_counter >= BLINK_INTERVAL) {
            led_purple.toggle();
            timer_1.overflow_counter = 0; // Reset overflow counter
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
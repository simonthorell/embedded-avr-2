//=============================================================================
// Main application file
// AtMega328p simulation: https://wokwi.com/projects/395865725914835969
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
#define LED_INTERVAL 200      // Set LED blink interval (us/ms based on timer unit)

// Declare function prototypes
void loop(Serial &serial, LED &led, Timer &timer_1);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    // Initialize serial (uart) communication
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    // Initialize timers
    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.start();

    // Initialize GPIO pheripherals
    LED led_d3(3); // LED on digital pin 3

    // Enable Interupts globally
    sei();

    // Loop forever
    loop(serial, led_d3, timer_1);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_d3, Timer &timer_1) {
    serial.uart_put_str("Loop started...\n"); // Debug message

    unsigned long led_time = 0; // Initialize LED counter

    while (true) {
        // increment timer counters and reset the timers
        led_time += timer_1.overflow_counter;
        timer_1.reset();

        // Toggle LED based on interval
        if (led_time >= LED_INTERVAL) {
            led_d3.toggle();
            led_time = 0; // Reset LED counter
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
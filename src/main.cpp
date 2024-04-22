//=============================================================================
// Main application file
// AtMega328p simulation: https://wokwi.com/projects/395865725914835969
//*****************************************************************************
// Application Commands
// Deluppgift 1: led on
// Deluppgift 2: read adc
// Deluppgift 3: ledpowerfreq <power> <freq>  (power: 0-255, freq: 200-5000)
// Deluppgift 4: button
// Deluppgift 5: ledramptime <time>           (time(ms): 0-5000)
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "drivers/adc.h"
#include "led.h"
#include <stdlib.h>
#include <stdio.h>

#define F_CPU     16000000UL  // Set MCU clock speed to 16MHz
#define DATA_BITS 8           // Set data bits for 8-bit MCU
#define BAUD_RATE 9600        // Set baud rate to 9600 bps

// Declare function prototypes
void loop(Serial &serial, ADConverter &adc, LED &led_d3, LED &led_d8, Timer &timer_1);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    // Initialize hardware drivers
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);
    ADConverter adc;

    // Initialize timers
    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.init();

    // Initialize GPIO pheripherals
    LED led_d3(3);
    LED led_d8(8);

    // Enable Interupts globally
    sei();

    // Loop forever
    loop(serial, adc, led_d3, led_d8, timer_1);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, ADConverter &adc, LED &led_d3, LED &led_d8, Timer &timer_1) {
    serial.uart_put_str("Loop started...\n"); // Debug message

    unsigned long led_d3_time = 0; // Initialize LED counter
    unsigned long led_d8_time = 0; // Initialize LED counter

    uint16_t prev_blink_time = 0; // Initialize previous blink time (ms)
    uint16_t blink_time = 200;    // Initialize blink time (ms)

    while (true) {
        // increment timer counters and reset the timers
        led_d3_time += timer_1.overflow_counter;
        led_d8_time += timer_1.overflow_counter;
        timer_1.reset();

        // Read ADC value from channel 0 and convert to voltage
        prev_blink_time = blink_time;
        uint16_t adc_reading = adc.read_channel(0);
        adc.convert_to_mv(adc_reading);
        blink_time = adc_reading / 50; // Max voltage == 100ms

        // Notify if blink time has changed
        if(blink_time != prev_blink_time) {
            if (blink_time == 0) {
                serial.uart_put_str("Blink time set to fixed light\r\n");
            } else {
                char buffer[20];
                serial.uart_put_str("Blink time: ");
                sprintf(buffer, "%d", blink_time);
                serial.uart_put_str(buffer);
                serial.uart_put_str("ms\r\n");
            }
        }

        if (led_d3_time >= 200) {
            led_d3.toggle();
            led_d3_time = 0; // Reset LED counter
        }

        if (led_d8_time >= blink_time) {
            led_d8.toggle();
            led_d8_time = 0; // Reset LED counter
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
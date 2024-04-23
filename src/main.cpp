//=============================================================================
// Main application file
//*****************************************************************************
// Application Commands:
// Deluppgift 1: led on
// Deluppgift 2: read adc
// Deluppgift 3: ledpowerfreq <power> <freq>  (power: 0-255, freq: 200-5000)
// Deluppgift 4: button
// Deluppgift 5: ledramptime <time>           (time(ms): 0-5000)
//*****************************************************************************
// ATmega328p simulation: https://wokwi.com/projects/395865725914835969
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "led.h"
#include <stdlib.h>
#include <stdio.h>

#define F_CPU     16000000UL  // Set MCU clock speed to 16MHz
#define DATA_BITS 8           // Set data bits for 8-bit MCU
#define BAUD_RATE 9600        // Set baud rate to 9600 bps

#define LED_RAMP_TIME 1000               // LED ramp time (ms)
#define BLINK_TIME    100                // Max blink time (ms)
#define BLINK_FACTOR (5000 / BLINK_TIME) // 5000mV (max voltage) / Max interval

// Declare function prototypes
void loop(Serial &serial, ADConverter &adc, PWModulation &pwm, LED &led_d3, 
          Timer &timer_1);
void parse_command(const char* cmd);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    ADConverter adc;

    PWModulation pwm(11); // PWM digital pin 11 (using timer 2)
    pwm.init();

    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.init();

    LED led_d3(3);

    sei(); // Enable Interupts globally

    loop(serial, adc, pwm, led_d3, timer_1); // Run Main Loop (infinite)
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, ADConverter &adc, PWModulation &pwm, LED &led_d3, 
          Timer &timer_1) {
    
    serial.uart_put_str("Loop started...\n"); // Debug message

    unsigned long led_d3_time = 0; // Initialize LED counter
    uint16_t prev_blink_time = 0;  // Initialize previous blink time (ms)
    uint16_t blink_time = 200;     // Initialize blink time (ms)

    char rec_cmd[256]; // serial command buffer

    while (true) {
        // Ramp Led up and down every 4 overflows/ms (replace with timer0 ?)
        pwm.ramp_output(LED_RAMP_TIME, timer_1);

        // increment timer counters and reset the timers
        led_d3_time += timer_1.overflow_counter;
        timer_1.reset();

        // Read ADC value from channel 0 and convert to voltage
        prev_blink_time = blink_time;
        uint16_t adc_reading = adc.read_channel(0);
        adc.convert_to_mv(adc_reading);
        blink_time = adc_reading / BLINK_FACTOR;

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

        if (led_d3_time >= blink_time) {
            led_d3.toggle();
            led_d3_time = 0; // Reset LED counter
        }

        // Process UART commands if a complete string is ready
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            serial.uart_put_str("Received Command: ");
            serial.uart_put_str(rec_cmd);  // Print received command
            serial.uart_put_str("\r\n");   // Ensure newline after command
            parse_command(rec_cmd);        // Parse the received command
            uart_command_ready = false;    // Reset command ready flag
        }

    }
}

//=============================================================================
// Function:    parse_command
// Description: Parses the received command string and toggles LED based on
//              the power value (0-255). If power > 127, LED is turned on.
//              If power <= 127, LED is turned off.
//=============================================================================
void parse_command(const char* cmd) {

}
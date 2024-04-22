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

#define BLINK_TIME 100                   // Max blink time (ms)
#define BLINK_FACTOR (5000 / BLINK_TIME) // 5000mV (max voltage) / Max interval

// Declare function prototypes
void loop(Serial &serial, ADConverter &adc, PWModulation &pwm, LED &led_d3, 
          Timer &timer_1);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    ADConverter adc;

    PWModulation pwm(11); // PWM digital pin 11 (using timer 2)
    bool pwm_init = pwm.init();
    if (!pwm_init) {
        serial.uart_put_str("PWM initialization failed!\r\n");
    }

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

    uint8_t duty_cycle = 0;        // Initialize duty cycle
    bool ramp_up = true;           // Initialize ramp direction
    static int overflow_count = 0;  // Initialize temp overflow counter

    while (true) {
        // increment timer counters and reset the timers
        led_d3_time += timer_1.overflow_counter;
        // Ramp Led up and down every 4 overflows/ms (replace with timer0 ?)
        overflow_count += timer_1.overflow_counter;
        if (overflow_count >= 4) {  // Update duty cycle every 2 overflows
            if (ramp_up) {
                duty_cycle++;
                if (duty_cycle >= 255) {
                    duty_cycle = 255;
                    ramp_up = false;
                }
            } else {
                duty_cycle--;
                if (duty_cycle <= 0) {
                    duty_cycle = 0;
                    ramp_up = true;
                }
            }
            pwm.set_duty_cycle(duty_cycle);
            overflow_count = 0; // Reset the counter
        }
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
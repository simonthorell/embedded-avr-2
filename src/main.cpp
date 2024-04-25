//=============================================================================
// Main application file
//*****************************************************************************
// Application Commands:
// Part 1: ledblink
// Part 2: ledadc
// Part 3: ledpowerfreq <power> <freq>  (power: 0-255, freq: 200-5000)
// Part 4: button
// Part 5: ledramptime <time>           (time(ms): 0-5000)
//*****************************************************************************
// ATmega328p simulation: https://wokwi.com/projects/395865725914835969
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "cmd_parser.h"
#include "led.h"

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CHANNEL    0    // Set Potentiometer ADC input channel
#define LED_BLINK_INTERVAL 200  // Fixed LED blink interval (ms)
#define MAX_ADC_INTERVAL   100  // Max ADC blink time (ms)

// Declare function prototypes
void loop(Serial &serial, LED &led_d3, Timer &timer_1, CMDParser &cmd_parser);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    LED led_d3(3, LED::PWM_ON);
    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    CMDParser cmd_parser;

    serial.uart_init(BAUD_RATE, DATA_BITS);
    timer_1.init(LED_BLINK_INTERVAL, serial);

    sei(); // Enable Interupts globally

    loop(serial, led_d3, timer_1, cmd_parser);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_d3, Timer &timer_1, CMDParser &cmd_parser) {
    char rec_cmd[BUFFER_SIZE]; // serial command buffer
    bool new_cmd = false;      // for functions that should only run once

    while (true) {
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            cmd_parser.parse_cmd(rec_cmd);
            if (cmd_parser.command) {
                new_cmd = true;
                serial.uart_put_str("Executing: ");
                serial.uart_put_str(rec_cmd);
                serial.uart_put_str("\r\n");
            } else {
                serial.uart_put_str("Invalid Command!\r\n");
            }
            uart_command_ready = false; // Reset command ready flag
        }

        switch(cmd_parser.command) {
            case CMDParser::NO_CMD:
                led_d3.turn_off();
                break;
            case CMDParser::LED_BLINK:
                if (new_cmd)
                    timer_1.set_prescaler(LED_BLINK_INTERVAL, serial);                
                led_d3.blink(1, timer_1); // Blink LED every(1) interupt
                break;
            case CMDParser::LED_ADC:
                if (new_cmd) timer_1.set_prescaler(1, serial);
                led_d3.adc_blink(timer_1, serial, POT_ADC_CHANNEL, MAX_ADC_INTERVAL);
                break;
            case CMDParser::LED_PWR:
                led_d3.set_power(cmd_parser.cmd_val1);
                led_d3.blink(cmd_parser.cmd_val2, timer_1);
                break;
            case CMDParser::BUTTON:
                // TODO: Implement button class
                break;
            case CMDParser::LED_RAMP:
                if (new_cmd) timer_1.set_prescaler(1, serial);
                led_d3.ramp_brightness(cmd_parser.cmd_val1, timer_1);
                break;
        }

        new_cmd = false;    // Reset the new command flag
        timer_1.reset();    // Reset timer overflow counter
    }
}
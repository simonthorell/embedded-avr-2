//=============================================================================
// Main application file
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "cmd_parser.h"
#include "led.h"
#include "button.h"

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CH     0    // Set Potentiometer ADC input channel
#define LED_BLINK_INTV 200  // Fixed LED blink interval (ms)
#define MAX_ADC_INTV   100  // Max ADC input blink interval (ms)

// Declare function prototypes
void loop(Serial &serial, LED &led_d3, Button &btn_d5, Timer &tim_1, 
          CMDParser &cmd_parser);

//=============================================================================
// Main (setup)
//=============================================================================
int main(void) {
    Serial serial;
    LED led_d3(3, LED::PWM_OFF);
    Button btn_d5(5);
    Timer tim_1(Timer::T1, Timer::MS);
    CMDParser cmd_parser;

    serial.uart_init(BAUD_RATE, DATA_BITS);
    tim_1.init(LED_BLINK_INTV, serial);

    sei(); // Enable Interupts globally

    loop(serial, led_d3, btn_d5, tim_1, cmd_parser);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_d3, Button &btn_d5, Timer &tim_1, 
         CMDParser &cmd_parser) {
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
            case CMDParser::CMD_NONE: break;
            case CMDParser::LED_BLINK:
                if (new_cmd) tim_1.set_prescaler(LED_BLINK_INTV, serial);                
                led_d3.blink(1, tim_1); // Blink LED every(1) interupt
                break;
            case CMDParser::LED_ADC:
                if (new_cmd) tim_1.set_prescaler(1, serial);
                led_d3.adc_blink(tim_1, serial, POT_ADC_CH, MAX_ADC_INTV);
                break;
            case CMDParser::LED_PWR:
                led_d3.set_power(cmd_parser.cmd_val1);
                led_d3.blink(cmd_parser.cmd_val2, tim_1);
                break;
            case CMDParser::BTN:
                btn_d5.is_pressed();
                break;
            case CMDParser::LED_RAMP:
                if (new_cmd) tim_1.set_prescaler(1, serial);
                led_d3.ramp_brightness(cmd_parser.cmd_val1, tim_1);
                break;
        }

        new_cmd = false;    // Reset the new command flag
        tim_1.reset();      // Reset timer overflow counter
    }
}
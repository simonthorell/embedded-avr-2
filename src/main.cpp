//=============================================================================
// Main application file
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "cmd.h"
#include "led.h"
#include "button.h"

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CH     0     // Set Potentiometer ADC input channel
#define LED_BLINK_INTV 1000  // Fixed LED blink interval (ms)
#define MAX_ADC_INTV   100   // Max ADC input blink interval (ms)
#define BTN_PRINT_INTV 1000  // Set button press print interval (ms)
#define DEBOUNCE_LIMIT 8000  // Set button debounce limit (presses per interval)
#define ON_INTERUPT    1     // Function interval (run every timer interrupt)
#define MS_TIMER       1     // Timer interval (overflow every millisecond)

// Declare function prototypes
void loop(Serial &serial, LED &led_d3, Button &btn_d5, Timer &timer_1, CMD &cmd);

//=============================================================================
// Main (setup)
//=============================================================================
int main(void) {
    Serial serial;
    LED    led_d3(3, true);
    Button btn_d5(5);
    Timer  timer_1(Timer::TIMER1, Timer::MILLIS); 
    CMD    cmd;

    serial.uart_init(BAUD_RATE, DATA_BITS);
    timer_1.init(LED_BLINK_INTV, serial);
    btn_d5.init();

    sei(); // Enable Interupts globally

    loop(serial, led_d3, btn_d5, timer_1, cmd);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_d3, Button &btn_d5, Timer &timer_1, CMD &cmd) {
    char rec_cmd[BUFFER_SIZE]; // serial command buffer
    bool new_cmd = false;      // for functions that should only run once

    while (true) {
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            cmd.parse_cmd(rec_cmd);
            if (cmd.cmd) {
                new_cmd = true;
                serial.uart_put_str("Executing: ");
                serial.uart_put_str(rec_cmd);
                serial.uart_put_str("\r\n");
            } else {
                serial.uart_put_str("Invalid Command!\r\n");
            }
            uart_command_ready = false; // Reset flag
        }

        switch(cmd.cmd) {
            case CMD::CMD_NONE: break;
            case CMD::LED_BLINK:
                if (new_cmd) {
                    timer_1.set_prescaler(LED_BLINK_INTV, serial);
                }
                led_d3.blink(1, timer_1, serial);
                break;
            case CMD::LED_ADC:
                if (new_cmd) {
                    timer_1.set_prescaler(MS_TIMER, serial);
                }
                led_d3.adc_blink(timer_1, serial, POT_ADC_CH, MAX_ADC_INTV);
                break;
            case CMD::LED_PWR:
                if (new_cmd) {
                    led_d3.set_power(cmd.cmd_val1);
                    timer_1.set_prescaler(cmd.cmd_val2, serial);
                }
                led_d3.blink(ON_INTERUPT, timer_1, serial);
                break;
            case CMD::BTN:
                if (new_cmd) { 
                    timer_1.set_prescaler(BTN_PRINT_INTV, serial); 
                }
                btn_d5.count_presses();
                btn_d5.debounce_presses(ON_INTERUPT, DEBOUNCE_LIMIT, timer_1, serial);
                break;
            case CMD::LED_RAMP:
                if (new_cmd)
                    timer_1.set_prescaler(MS_TIMER, serial);
                led_d3.ramp_brightness(cmd.cmd_val1, timer_1);
                break;
        }

        new_cmd = false;    // Reset the new command flag
    }
}
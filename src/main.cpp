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
#include "drivers/adc.h"
#include "drivers/pwm.h"
#include "cmd_parser.h"
#include "led.h"

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CHANNEL    0    // Set Potentiometer ADC input channel
#define LED_BLINK_INTERVAL 200  // Fixed LED blink interval (ms)
#define MAX_ADC_INTERVAL   100  // Max ADC blink time (ms)
#define LED_RAMP_TIME      1000 // Default LED ramp interval time (ms)

// Declare function prototypes
void loop(Serial &serial, PWModulation &led_pwm_d11, LED &led_d3, 
          Timer &timer_1, CmdParser &cmd_parser);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    LED led_d3(3);

    PWModulation led_pwm_d11(11); // PWM digital pin 11 (using timer 2)
    led_pwm_d11.init();

    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.init(LED_BLINK_INTERVAL, serial);

    CmdParser cmd_parser;

    sei(); // Enable Interupts globally

    loop(serial, led_pwm_d11, led_d3, timer_1, cmd_parser); // Run Main Loop (infinite)
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, PWModulation &led_pwm_d11, LED &led_d3, 
          Timer &timer_1, CmdParser &cmd_parser) {
    
    char rec_cmd[BUFFER_SIZE]; // serial command buffer
    bool new_cmd = false;      // new command flag

    while (true) {
        // Process UART commands if a complete string is ready
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            cmd_parser.parse_cmd(rec_cmd);
            if (cmd_parser.command != CmdParser::NO_CMD) {
                new_cmd = true;
                serial.uart_put_str("Executed Command: ");
                serial.uart_put_str(rec_cmd);
                serial.uart_put_str("\r\n");
            } else {
                serial.uart_put_str("Invalid Command!\r\n");
            }
            uart_command_ready = false;    // Reset command ready flag
        }

        // Check cmd variable and execute set command
        switch(cmd_parser.command) {
            case CmdParser::NO_CMD: 
                break;
            case CmdParser::LED_BLINK:
                if (new_cmd)
                    timer_1.set_prescaler(LED_BLINK_INTERVAL, serial);                
                led_d3.blink(1, timer_1); // Blink LED every(1) timer_1 interupt
                break;
            case CmdParser::LED_ADC:
                if (new_cmd)
                    timer_1.set_prescaler(1, serial); // Millis timer
                led_d3.adc_blink(timer_1, serial, POT_ADC_CHANNEL, MAX_ADC_INTERVAL);
                break;
            case CmdParser::LED_PWR:
                // Implement LED power frequency control
                break;
            case CmdParser::BUTTON:
                // Implement button control
                break;
            case CmdParser::LED_RAMP:
                led_pwm_d11.ramp_output(cmd_parser.cmd_val1, timer_1);
                break;
        }

        new_cmd = false;    // Reset the new command flag
        timer_1.reset();    // Reset timer overflow counter
    }
}
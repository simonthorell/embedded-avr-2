//=============================================================================
// Main application file
//*****************************************************************************
// Application Commands:
// Deluppgift 1: ledblink
// Deluppgift 2: ledadc
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
#include <avr/delay.h>

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CHANNEL    0    // Set Potentiometer ADC input channel to 0
#define LED_BLINK_INTERVAL 200  // Fixed LED blink interval (ms)
#define LED_RAMP_TIME      1000 // LED ramp time (ms)
#define MAX_BLINK_INTERVAL 100  // Max blink time (ms)

// Command Flags
#define LEDBLINK_FLAG 0b00000001
#define LEDADC_FLAG   0b00000010
#define LEDPWR_FLAG   0b00000100
#define BUTTON_FLAG   0b00001000
#define LEDRAMP_FLAG  0b00010000

// Declare function prototypes
void loop(Serial &serial, PWModulation &pwm_led_d11, LED &led_d3, Timer &timer_1);
void update_command_flags(uint8_t bit_position, char &cmd_flags);
void command_parser(char *cmd, char &cmd_flags);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    LED led_d3(3);

    PWModulation pwm_led_d11(11); // PWM digital pin 11 (using timer 2)
    pwm_led_d11.init();

    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.init(1, serial); // Initialize timer for compare A every 1ms

    sei(); // Enable Interupts globally

    loop(serial, pwm_led_d11, led_d3, timer_1); // Run Main Loop (infinite)
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, PWModulation &pwm_led_d11, LED &led_d3, Timer &timer_1) {
    char rec_cmd[BUFFER_SIZE];   // serial command buffer
    char cmd_flags = 0b00000000;  // command flags

    while (true) {
        // Process UART commands if a complete string is ready
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            serial.uart_put_str("Received Command: ");
            serial.uart_put_str(rec_cmd);  // Print received command
            serial.uart_put_str("\r\n");   // Ensure newline after command            
            uart_command_ready = false;    // Reset command ready flag
        }

        // update_command_flags(LEDBLINK_FLAG, cmd_flags);
        update_command_flags(LEDADC_FLAG, cmd_flags);

        if (cmd_flags & LEDBLINK_FLAG) {
            cmd_flags &= ~LEDADC_FLAG; // Disable LED ADC control
            led_d3.blink(100, timer_1);
        }

        if (cmd_flags & LEDADC_FLAG) {
            cmd_flags &= ~LEDBLINK_FLAG; // Disable LED fixed Blink
            led_d3.adc_blink(timer_1, serial, POT_ADC_CHANNEL, MAX_BLINK_INTERVAL);
        }

        if (cmd_flags & LEDPWR_FLAG) {
            // Implement LED power frequency control
        }

        if (cmd_flags & BUTTON_FLAG) {
            // Implement button control
        }

        if (cmd_flags & LEDRAMP_FLAG) {
            pwm_led_d11.ramp_output(LED_RAMP_TIME, timer_1);
        }

        timer_1.reset(); // Reset timer overflow counter
    }
}

// Function to set a specific bit in cmd_flags based on an integer 1-8
void update_command_flags(uint8_t mask, char &cmd_flags) {
    // Set the corresponding bit(s) based on the bitmask provided
    cmd_flags |= mask;
}

void command_parser(char *cmd, char &cmd_flags) {
    // Parse command string and set command flags
    char* cmd_str;
    uint16_t cmd_val1;
    uint16_t cmd_val2;

    // sscanf the command
    sscanf(cmd, "%s %d %d", cmd_str, &cmd_val1, &cmd_val2);

}
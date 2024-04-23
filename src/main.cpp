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

#define F_CPU     16000000UL   // Set MCU clock speed to 16MHz
#define DATA_BITS 8            // Set data bits for 8-bit MCU
#define BAUD_RATE 9600         // Set baud rate to 9600 bps

#define POT_ADC_CHANNEL    0    // Set Potentiometer ADC input channel to 0
#define LED_RAMP_TIME      1000 // LED ramp time (ms)
#define MAX_BLINK_INTERVAL 100  // Max blink time (ms)

// Declare function prototypes
void loop(Serial &serial, ADConverter &adc, PWModulation &pwm_led_d11, 
          LED &led_d3, Timer &timer_1);

//=============================================================================
// Main function
//=============================================================================
int main(void) {
    Serial serial;
    serial.uart_init(BAUD_RATE, DATA_BITS);

    ADConverter adc;

    PWModulation pwm_led_d11(11); // PWM digital pin 11 (using timer 2)
    pwm_led_d11.init();

    Timer timer_1(Timer::TIMER_1, Timer::MILLIS);
    timer_1.init();

    LED led_d3(3);

    sei(); // Enable Interupts globally

    loop(serial, adc, pwm_led_d11, led_d3, timer_1); // Run Main Loop (infinite)
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, ADConverter &adc, PWModulation &pwm_led_d11, 
          LED &led_d3, Timer &timer_1) {
    char rec_cmd[BUFFER_SIZE]; // serial command buffer

    while (true) {
        pwm_led_d11.ramp_output(LED_RAMP_TIME, timer_1);
        // led_d3.blink(blink_time, timer_1);
        led_d3.adc_blink(timer_1, serial, POT_ADC_CHANNEL, MAX_BLINK_INTERVAL);

        // Process UART commands if a complete string is ready
        if (uart_command_ready) {
            serial.uart_rec_str(rec_cmd, BUFFER_SIZE);
            serial.uart_put_str("Received Command: ");
            serial.uart_put_str(rec_cmd);  // Print received command
            serial.uart_put_str("\r\n");   // Ensure newline after command
            uart_command_ready = false;    // Reset command ready flag
        }

        timer_1.reset(); // Reset timer to avoid overflow
    }
}
//=============================================================================
// Main application file
//*****************************************************************************
// Wokwi Link: (It's buggy i noticed, so it depends on the compiler mood...)
// Link: https://wokwi.com/projects/395865725914835969
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "cmd.h"
#include "led.h"
#include "button.h"

//=============================================================================
// Hej Peter!
// Det här är kanske inte den mest effektiva koden jag skrivit, utan jag har
// som sagt lagt mest tid på att försöka få till generiska drivrutiner, vilket
// jag lärt mig extremt mycket av! Hoppas koden ändå ser ok ut...
// Tack för superbra kursinnehåll och grym feedback i dessa kurser! :) 
// Mvh, Simon
//=============================================================================

// Configuration Constants
namespace cfg {
    constexpr uint16_t baud_rate       = 9600;  // UART baud rate
    constexpr uint8_t  data_bits       = 8;     // Data bits for UART communication
    constexpr uint8_t  pot_adc_ch      = 0;     // ADC channel for potentiometer
    constexpr uint16_t led_blink_intvl = 200;   // LED blink interval in milliseconds
    constexpr uint16_t max_adc_intvl   = 100;   // Maximum ADC read interval in ms
    constexpr uint16_t btn_print_intvl = 1000;  // Interval to print button press (ms)
    constexpr uint16_t debounce_limit  = 8000;  // Button debounce limit
    constexpr bool     on_interrupt    = true;  // Enable function on timer interrupt
    constexpr bool     ms_timer        = true;  // Timer interval (ms)
    constexpr uint8_t  led_pwm_pin     = 3;     // LED (PWM) pin
    constexpr uint8_t  btn_pin         = 5;     // Button pin
}

// Main loop declaration
void loop(Serial &serial, LED &led, Button &btn, Timer &timer, CMD &cmd);

//=============================================================================
// Main (setup)
//=============================================================================
int main(void) {
    Serial serial;
    LED    led(cfg::led_pwm_pin, true);
    Button btn(cfg::btn_pin);
    Timer  timer(Timer::TIMER1, Timer::MILLIS); 
    CMD    cmd;

    // Initialize the modules
    serial.uart_init(cfg::baud_rate, cfg::data_bits);
    timer.init(cfg::led_blink_intvl, serial);
    btn.init();

    sei(); // Enable Interupts globally

    loop(serial, led, btn, timer, cmd);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led, Button &btn, Timer &timer, CMD &cmd) {
    char rec_cmd[BUFFER_SIZE]; // Buffer for received commands
    bool new_cmd = false;      // Flag to indicate new command been received

    while (true) {
        // Check if a new command has been received over UART
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
            uart_command_ready = false; // Reset command ready flag
        }

        // Execute the command
        switch(cmd.cmd) {
            case CMD::CMD_NONE: break;
            /***************************** PART 1 *****************************/
            case CMD::LED_BLINK:
                if (new_cmd) timer.set_prescaler(cfg::led_blink_intvl, serial);
                led.blink(cfg::on_interrupt, timer);
                break;
            /***************************** PART 2 *****************************/
            case CMD::LED_ADC:
                if (new_cmd) timer.set_prescaler(cfg::ms_timer, serial);
                led.adc_blink(timer, serial, cfg::pot_adc_ch, 
                                 cfg::max_adc_intvl);
                break;
            /***************************** PART 3 *****************************/
            case CMD::LED_PWR:
                if (new_cmd) {
                    led.set_power(cmd.cmd_val1);
                    timer.set_prescaler(cmd.cmd_val2, serial);
                }
                led.blink(cfg::on_interrupt, timer);
                break;
            /***************************** PART 4 *****************************/
            case CMD::BTN:
                if (new_cmd) timer.set_prescaler(cfg::ms_timer, serial);
                btn.count_presses();
                btn.debounce_presses(cfg::on_interrupt, cfg::debounce_limit, 
                                        timer, serial); 
                break;
            /***************************** PART 5 *****************************/
            case CMD::LED_RAMP:
                if (new_cmd) timer.set_prescaler(cfg::ms_timer, serial);
                led.ramp_brightness(cmd.cmd_val1, timer);
                break;
        }

        new_cmd = false;    // Reset the new command flag
    }
}
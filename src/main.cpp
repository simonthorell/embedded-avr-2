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

// Configuration Constants
namespace cfg {
    constexpr uint32_t clk_spd = 16000000UL;   // MCU clock speed in Hz
    constexpr uint8_t  data_bits = 8;          // Data bits for UART communication
    constexpr uint16_t baud_rate = 9600;       // UART baud rate
    constexpr uint8_t  pot_adc_ch = 0;         // ADC channel for potentiometer
    constexpr uint16_t led_blink_intvl = 1000; // LED blink interval in milliseconds
    constexpr uint16_t max_adc_intvl = 100;    // Maximum ADC read interval in ms
    constexpr uint16_t btn_print_intvl = 1000; // Interval to print button press (ms)
    constexpr uint16_t debounce_limit = 8000;  // Button debounce limit
    constexpr bool     on_interrupt = true;    // Enable function on timer interrupt
    constexpr bool     ms_timer = true;        // Timer interval (ms)
}

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

    serial.uart_init(cfg::baud_rate, cfg::data_bits);
    timer_1.init(cfg::led_blink_intvl, serial);
    btn_d5.init();

    sei(); // Enable Interupts globally

    loop(serial, led_d3, btn_d5, timer_1, cmd);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led_d3, Button &btn_d5, Timer &timer_1, CMD &cmd) {
    char rec_cmd[BUFFER_SIZE]; // Buffer for received commands
    bool new_cmd = false;     // Flag to indicate a new command has been received

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
            /************************* DELUPPGIFT 1 *************************/
            case CMD::LED_BLINK:
                if (new_cmd) timer_1.set_prescaler(cfg::led_blink_intvl, serial);
                led_d3.blink(1, timer_1, serial);
                break;
            /************************* DELUPPGIFT 2 *************************/
            case CMD::LED_ADC:
                if (new_cmd) timer_1.set_prescaler(cfg::ms_timer, serial);
                led_d3.adc_blink(timer_1, serial, cfg::pot_adc_ch, 
                                 cfg::max_adc_intvl);
                break;
            /************************* DELUPPGIFT 3 *************************/
            case CMD::LED_PWR:
                if (new_cmd) {
                    led_d3.set_power(cmd.cmd_val1);
                    timer_1.set_prescaler(cmd.cmd_val2, serial);
                }
                led_d3.blink(cfg::on_interrupt, timer_1, serial);
                break;
            /************************* DELUPPGIFT 4 *************************/
            case CMD::BTN:
                if (new_cmd) timer_1.set_prescaler(cfg::btn_print_intvl, serial);
                btn_d5.count_presses();
                btn_d5.debounce_presses(cfg::on_interrupt, cfg::debounce_limit, 
                                        timer_1, serial);
                break;
            /************************* DELUPPGIFT 5 *************************/
            case CMD::LED_RAMP:
                if (new_cmd) timer_1.set_prescaler(cfg::ms_timer, serial);
                led_d3.ramp_brightness(cmd.cmd_val1, timer_1);
                break;
        }
        
        new_cmd = false;    // Reset the new command flag
    }
}
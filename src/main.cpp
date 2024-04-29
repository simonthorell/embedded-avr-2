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
// Wokwi Simulation: https://wokwi.com/projects/395865725914835969
//=============================================================================
#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/serial.h"
#include "drivers/timer.h"
#include "command.h"
#include "led.h"
#include "button.h"

// Configuration Constants
namespace cfg {
    constexpr uint32_t baud_rate     = 9600;  // UART baud rate
    constexpr uint8_t  data_bits     = 8;     // Data bits for UART communication
    constexpr uint8_t  pot_adc_ch    = 0;     // ADC channel for potentiometer
    constexpr uint8_t  led_pwm_pin   = 3;     // LED (PWM) pin
    constexpr uint8_t  btn_pin       = 5;     // Button pin
    constexpr uint16_t fixed_intvl    = 200;   // LED blink interval in milliseconds
    constexpr uint16_t max_adc_intvl = 100;   // Maximum ADC read interval in ms
    constexpr uint16_t btn_intvl     = 1000;  // Interval to print button press (ms)
    constexpr uint8_t  on_interrupt  = 1;     // Timer interval (every interrupt)
    constexpr uint8_t  ms_timer      = 1;     // Timer interval (every ms)
}

// Main loop declaration
void loop(Serial &serial, LED &led, Button &btn, Timer* timer_0, Timer* timer_1, 
          Command &cmd);

//=============================================================================
// Main (setup)
//=============================================================================
int main(void) {
    Serial  serial;
    LED     led(cfg::led_pwm_pin, true);
    Button  btn(cfg::btn_pin);
    Timer*  timer_0 = Timer::getInstance(Timer::TIMER0);
    Timer*  timer_1 = Timer::getInstance(Timer::TIMER1);
    Command cmd;

    // Initialize the modules
    serial.uart_init(cfg::baud_rate, cfg::data_bits);
    timer_0->configure(Timer::CTC, cfg::ms_timer, serial);
    timer_1->configure(Timer::CTC, cfg::fixed_intvl, serial);
    btn.init();

    sei(); // Enable Interupts globally

    loop(serial, led, btn, timer_0, timer_1, cmd);
    
    return 0;
}

//=============================================================================
// Main loop
//=============================================================================
void loop(Serial &serial, LED &led, Button &btn, Timer* timer_0, Timer* timer_1, 
          Command &cmd) {
    char rec_cmd[serial.buf_size]; // Buffer for received commands
    bool new_cmd = false;          // Flag to indicate new command received

    while (true) {
        // Check if a new command has been received over UART
        if (serial.uart_command_ready) {
            serial.uart_rec_str(rec_cmd, serial.buf_size);
            cmd.parse_cmd(rec_cmd);
            if (cmd.cmd) {
                new_cmd = true;
                serial.uart_put_str("Executing: ");
                serial.uart_put_str(rec_cmd);
                serial.uart_put_str("\r\n");
            } else {
                serial.uart_put_str("Invalid Command!\r\n");
            }
            serial.uart_command_ready = false; // Reset command ready flag
        }

        // Execute the command
        switch(cmd.cmd) {
            case Command::NO_CMD: break;
        /****************************** PART 1 ******************************/
            case Command::LED_BLINK:
                if (new_cmd) {
                    led.set_power(UINT8_MAX); // Set the LED to full power
                    // Reconfig Timer 1 for CTC Mode with set interupt interval
                    timer_1->configure(Timer::CTC, cfg::fixed_intvl, serial);
                }
                led.blink(cfg::on_interrupt, *timer_1);
                break;
        /****************************** PART 2 ******************************/
            case Command::LED_ADC:
                if (new_cmd) {
                    led.set_power(UINT8_MAX); // Set the LED to full power
                    // Reconfig Timer 1 to Millis Timer in CTC Mode
                    timer_1->configure(Timer::CTC, cfg::ms_timer, serial);
                }
                led.adc_blink(*timer_1, serial, cfg::pot_adc_ch, 
                                 cfg::max_adc_intvl);
                break;
        /****************************** PART 3 ******************************/
            case Command::LED_PWR:
                if (new_cmd) {
                    led.set_power(cmd.cmd_val1);
                    // Reconfig Timer 1 for CTC Mode, interupt as per cmd_val2
                    timer_1->configure(Timer::CTC, cmd.cmd_val2, serial);
                }
                led.blink(cfg::on_interrupt, *timer_1);
                break;
        /****************************** PART 4 ******************************/
            case Command::BUTTON:
                if (new_cmd) {
                    led.turn_off();
                    // Reconfig Timer 1 for Counter Mode (External Clock)
                    timer_1->configure(Timer::EXT_CLOCK, 0, serial);
                }
                // Using Timer 0 to print presses every interval (1000ms)
                btn.print_presses(cfg::btn_intvl, *timer_0, serial);
                break;
        /****************************** PART 5 ******************************/
            case Command::LED_RAMP:
                if (new_cmd)
                    // Reconfig Timer 1 to Millis Timer in CTC Mode
                    timer_1->configure(Timer::CTC, cfg::ms_timer, serial);
                led.ramp_brightness(cmd.cmd_val1, *timer_1);
                break;
        /********************************************************************/
        }

        new_cmd = false; // Reset the new command flag
    }
}
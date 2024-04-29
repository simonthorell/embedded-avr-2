#ifndef BUTTON_H
#define BUTTON_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include "drivers/gpio.h"
#include "drivers/timer.h"
#include "drivers/serial.h"

//==============================================================================
// Button Class Declaration
//==============================================================================
class Button {
public:
    // Constructor
    Button(uint8_t pin);

    // Public Methods
    void init();
    bool is_pressed();
    void count_presses();
    void debounce_presses(uint16_t interval, uint16_t debounce_limit, 
                          Timer &timer, Serial &serial);
private:
    // Private Members
    GPIO _gpio;
    uint8_t _pin;
    uint32_t _debounce_counter;
    uint32_t _last_timer_count;
    volatile uint32_t _button_presses;

    // Private Methods
    void handle_press();
    void print_presses(Serial &serial);
    
    // Compile time validation
    static constexpr bool _valid_btn_pin(uint8_t pin);
};

#endif // BUTTON_H
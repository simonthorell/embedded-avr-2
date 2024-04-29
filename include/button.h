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
    void print_presses(const uint16_t &interval, Timer &timer, Serial &serial);

private:
    // Private Members
    GPIO _gpio;
    uint8_t _pin;
    volatile uint32_t _button_presses;
    
    // Compile time validation
    static constexpr bool _valid_btn_pin(uint8_t pin);
};

#endif // BUTTON_H
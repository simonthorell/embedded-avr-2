//======================================================================
// GPIO Driver Class Implementation
//======================================================================
#include "drivers/gpio.h"

//======================================================================
// GPIO Constructor
//======================================================================
GPIO::GPIO(PinType pin_type, uint8_t pin) 
    : _pin_type(pin_type), _pin(pin) {}

//======================================================================
// GPIO Public Methods
//======================================================================
void GPIO::enable_output() {
    ENABLE_OUTPUT(_pin_type, _pin);
}

void GPIO::enable_input() {
    ENABLE_INPUT(_pin_type, _pin);
}

void GPIO::enable_input() {
    ENABLE_INPUT(_pin_type, _pin);
}

void GPIO::set_high() {
    SET_HIGH(_pin_type, _pin);
}

void GPIO::set_low() {
    SET_LOW(_pin_type, _pin);
}

void GPIO::toggle() {
    TOGGLE(_pin_type, _pin);
}

bool GPIO::is_high() {
    return IS_HIGH(_pin_type, _pin);
}

bool GPIO::is_low() {
    return IS_LOW(_pin_type, _pin);
}
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
// GPIO Public Methods: enable_output, enable_input, set_high, set_low,
//                      toggle, is_high, is_low
//======================================================================
void GPIO::enable_output() {
    (*(DDR_FOR_PIN(_pin_type, _pin))  |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::enable_input() {
    (*(DDR_FOR_PIN(_pin_type, _pin))  &= ~(1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::enable_pullup() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::set_high() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) |=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::set_low() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) &= ~(1 << BIT_FOR_PIN(_pin_type, _pin)));
}

void GPIO::toggle() {
    (*(PORT_FOR_PIN(_pin_type, _pin)) ^=  (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

bool GPIO::is_high() {
    return (*(PIN_FOR_PIN(_pin_type, _pin)) & (1 << BIT_FOR_PIN(_pin_type, _pin)));
}

bool GPIO::is_low() {
    return !(*(PIN_FOR_PIN(_pin_type, _pin)) & (1 << BIT_FOR_PIN(_pin_type, _pin)));
}
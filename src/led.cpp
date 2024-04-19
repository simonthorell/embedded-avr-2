//======================================================================
// LED Class Implementation
//======================================================================
#include "led.h"

//======================================================================
// LED Constructor
//======================================================================
LED::LED(uint8_t pin) : _gpio(DIGITAL_PIN, pin) {
    // Set the pin as an output using the GPIO object
    _gpio.enable_output();
}

//======================================================================
// LED Public Methods
//======================================================================
void LED::turn_on() {
    _gpio.set_high();
}

void LED::turn_off() {
    _gpio.set_low();
}

void LED::toggle() {
    _gpio.toggle();
}

bool LED::is_on() {
    return _gpio.is_high();
}

bool LED::is_off() {
    return _gpio.is_low();
}
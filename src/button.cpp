//======================================================================
// Button Class Implementation
//======================================================================
#include "button.h"

//======================================================================
// Button Constructor
//======================================================================
Button::Button(uint8_t pin) : _gpio(DIGITAL_PIN, pin) {
    // Set the pin as an input using the GPIO object
    _gpio.enable_input();
    _gpio.enable_pullup();
}

//======================================================================
// Button Public Method: is_pressed
//======================================================================
bool Button::is_pressed() {
    return _gpio.is_high();
}


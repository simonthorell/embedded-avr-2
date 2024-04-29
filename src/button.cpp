//======================================================================
// Button Class Implementation
//======================================================================
#include "button.h"
#include <stdio.h> // sprintf

//======================================================================
// Button Constructor
//======================================================================
Button::Button(uint8_t pin) : _gpio(GPIO::DIGITAL_PIN, pin), _pin(pin),
                              _button_presses(0) {}

//======================================================================
// Button Public Methods: init
//======================================================================
void Button::init() {
    // Make sure the pin is valid for pin change interrupt
    if (_valid_btn_pin(_pin)) {
        _gpio.enable_input();
        _gpio.enable_pullup();
    }
}

//======================================================================
// Button Public Methods: is_pressed, print_presses
//======================================================================
bool Button::is_pressed() { 
    return _gpio.is_low();
}

void Button::print_presses(const uint16_t &interval, Timer &timer, Serial &serial) {
    if (timer.overflow_counter >= interval) {
        timer.overflow_counter = 0; // Reset the counter after printing

        // Print the presses from Timer1 Counter
        char buf[32];
        sprintf(buf, "Button presses: %u\r\n", TCNT1);
        serial.uart_put_str(buf);
        TCNT1 = 0; // Reset the counter
    }
}

//==============================================================================
// Constexpr validation for pin change interrupt pins on Arduino Uno
//==============================================================================
constexpr bool Button::_valid_btn_pin(const uint8_t _pin) {
    return (_pin <= 19); // All digital pins on the Arduino Uno
}
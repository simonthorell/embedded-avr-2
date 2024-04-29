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
// Button Public Methods: is_pressed, count_presses, print_presses
//======================================================================
bool Button::is_pressed() { 
    return _gpio.is_low();
}

void Button::count_presses() {
    // if (is_pressed()) {
    //     _button_presses++;
    // }
}

void Button::debounce_presses(uint16_t interval, uint16_t debounce_limit, 
                              Timer &timer, Serial &serial) {


    // Rename _debounce_counter to _prev_overflow_counter
    if (timer.overflow_counter < (_debounce_counter + interval))
        return;

    // if (_button_presses > interval) {
        // Print out message! 
        serial.uart_put_str("Press passed debounce limit!\r\n");
    //     return;
    // }


    // if (timer.overflow_counter < (_debounce_counter + interval)) { 
    //     return; 
    // }

    // _button_presses = TCNT1;
    // Reset Timer 1
    // TCNT1 = 0;

    // // Debounce by checking if the button is pressed for a certain interval
    // if (_button_presses > debounce_limit) {
    //     serial.uart_put_str("Press passed debounce limit!\r\n");
    //     handle_press();
    // }

    // print_presses(serial);
    _debounce_counter = timer.overflow_counter; // Reset debounce counter
}

//======================================================================
// Button Private Methods: handle_press, print_presses
//======================================================================
void Button::handle_press() {
    /* Execute any command here... */
}

void Button::print_presses(Serial &serial) {
    char buf[32];
    sprintf(buf, "Button Presses: %lu\r\n", _button_presses);
    serial.uart_put_str(buf);
    _button_presses = 0;
}

//==============================================================================
// Constexpr validation for pin change interrupt pins on Arduino Uno
//==============================================================================
constexpr bool Button::_valid_btn_pin(const uint8_t _pin) {
    return (_pin <= 19); // All digital pins on the Arduino Uno
}
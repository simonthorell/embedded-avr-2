//==============================================================================
// Button Class Implementation
//==============================================================================
#include "button.h"
#include <stdio.h> // sprintf

//==============================================================================
// Button Constructor
//==============================================================================
Button::Button(uint8_t pin) : _gpio(GPIO::DIGITAL_PIN, pin), _pin(pin),
                              _button_presses(0) {}

//==============================================================================
// Button Public Methods: init
//==============================================================================
void Button::init() {
    // Make sure the pin is valid for pin change interrupt
    if (_valid_btn_pin(_pin)) {
        _gpio.enable_input();
        _gpio.enable_pullup();
    }
}

//==============================================================================
// Button Public Methods: is_pressed, print_presses
//==============================================================================
bool Button::is_pressed() { 
    return _gpio.is_low();
}

void Button::print_presses(const uint16_t &interval, Timer &timer, Serial &serial) {

    /*
    * This function uses a hardware timer to track button presses, efficiently
    * reducing CPU load by avoiding continuous polling. The timer's overflow
    * is checked to report button presses.
    * 
    * To enhance button press detection reliability, consider hardware
    * debouncing. Options include:
    * 
    * 1. Capacitor Debouncing:
    *    Attach a capacitor (e.g., 0.1 µF) between the button pin and ground.
    *    This smooths out the noise from button bounce, stabilizing the input
    *    signal. It's simple and effective for non-frequent pressing.
    * 
    * 2. Transistor Debouncing:
    *    A transistor-based switch can sharply cut off bouncing, ideal for
    *    buttons used frequently needing rapid response. The transistor acts
    *    as a fast switch that activates firmly when pressed, filtering minor
    *    bounces.
    * 
    * Implementing these methods will greatly reduce signal noise, leading to
    * more accurate software responses to user inputs.
    */

    if (timer.overflow_counter >= interval) {
        timer.overflow_counter = 0; // Reset the counter after printing
        char buf[32];
        sprintf(buf, "Button presses: %u\r\n", TCNT1);
        serial.uart_put_str(buf);
        TCNT1 = 0; // Reset the timer counter 
    }
}

//==============================================================================
// Constexpr validation for pin change interrupt pins on Arduino Uno
//==============================================================================
constexpr bool Button::_valid_btn_pin(const uint8_t _pin) {
    return (_pin <= 19); // All digital pins on the Arduino Uno
}
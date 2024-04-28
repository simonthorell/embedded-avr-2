//======================================================================
// Button Class Implementation
//======================================================================
#include "button.h"
#include <stdio.h>

#define DEBOUNCE_LIMIT 8000

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

        // Enable pin change interrupt for the button pin
        if (_pin <= 7) {
            // PCINT16-23 range (D8 to D13)
            PCICR |= (1 << PCIE2);
            PCMSK2 |= (1 << (_pin - 8 + PCINT16));
        } else if (_pin <= 13) {
            // PCINT8-14 range (D0 to D6)
            PCICR |= (1 << PCIE0);
            PCMSK0 |= (1 << (_pin - 0 + PCINT8));
        } else if (_pin >= 14 && _pin <= 21) {
            // PCINT0-7 range (A0 to A7)
            PCICR |= (1 << PCIE1);
            PCMSK1 |= (1 << (_pin - 14 + PCINT0));
        }
    }
}

//======================================================================
// Button Public Methods: is_pressed, count_presses, print_presses
//======================================================================
bool Button::is_pressed() { 
    return _gpio.is_low();
}


void Button::count_presses() {
    if (is_pressed()) {
        _button_presses++;
    }
}

void Button::debounce_presses(uint16_t interval, uint16_t debounce_limit, 
                              Timer &timer, Serial &serial) {
    if (timer.overflow_counter < (_debounce_counter + interval)) { 
        return; 
    }

    // Debounce by checking if the button is pressed for a certain interval
    if (_button_presses > debounce_limit) {
        serial.uart_put_str("Press passed debounce limit!\r\n");
        handle_press();
    }

    print_presses(serial);
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

//======================================================================
// Button Interrupt Service Routine
//======================================================================
ISR(PCINT0_vect) { /* Empty ISR (Handle in main loop) */ }
ISR(PCINT1_vect) { /* Empty ISR (Handle in main loop) */ }
ISR(PCINT2_vect) { /* Empty ISR (Handle in main loop) */ }

//==============================================================================
// Constexpr validation for pin change interrupt pins on Arduino Uno
//==============================================================================
constexpr bool Button::_valid_btn_pin(const uint8_t _pin) {
    return (_pin <= 19); // All digital pins on the Arduino Uno
}
//======================================================================
// Button Class Implementation
//======================================================================
#include "button.h"
#include <stdio.h>

#define DEBOUNCE_LIMIT 8000

// Static pointer initialization
Button* Button::instance = nullptr;

//======================================================================
// Button Constructor
//======================================================================
Button::Button(uint8_t pin) : _gpio(DIGITAL_PIN, pin) {
    _button_presses = 0;
    // Set the pin as an input using the GPIO object
    _gpio.enable_input();
    _gpio.enable_pullup();
    instance = this;

    // Enable pin change interrupt for PCINT9 (e.g., PD5)
    PCICR |= (1 << PCIE2);   // Enable PCINT for the group including PCINT9
    PCMSK1 |= (1 << PCINT21); // Enable PCINT on pin PCINT9 (PD5)
}

//======================================================================
// Button Public Method: is_pressed
//======================================================================
bool Button::is_pressed() {
    return _gpio.is_low();
}

//======================================================================
// Button Public Method: count_presses
//======================================================================
void Button::count_presses() {
    if (is_pressed()) {
        _button_presses++;
    }
}

//======================================================================
// Button Public Method: print_presses
//======================================================================
void Button::print_presses(uint16_t interval, Timer &timer, Serial &serial) {
    if (timer.overflow_counter < (_prev_print_time + interval)) {
        return;
    }

    if (_button_presses > DEBOUNCE_LIMIT) {
        serial.uart_put_str("Passed debounce limit! Execute command...\r\n");
        // EXECUTE SOME COMMAND HERE
    }

    char buf[32];
    sprintf(buf, "Button Presses: %lu\r\n", _button_presses);
    serial.uart_put_str(buf);
    _button_presses = 0;
    _prev_print_time = timer.overflow_counter;
}

//======================================================================
// Button Interrupt Service Routine
//======================================================================
ISR(PCINT2_vect) {
}
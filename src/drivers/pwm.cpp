//==============================================================================
// PWM Interface Class Implementation
//==============================================================================
#include "drivers/pwm.h"

//==============================================================================
// Constructor
//==============================================================================
PWModulation::PWModulation(const uint8_t &pwm_pin) : _pin(pwm_pin) {
    if (_valid_pwm_pin(_pin)) {
        // Assign the correct output compare register based on the PWM pin
        switch (pwm_pin) {
            // Assign to _ocr8 or _ocr16 depending on the timer resolution
            case 3:  _ocr8  = &OCR2B; break;             // 8-bit
            case 5:  _ocr8  = &OCR0B; break;             // 8-bit
            case 6:  _ocr8  = &OCR0A; break;             // 8-bit
            case 9:  _ocr16 = (uint16_t*)&OCR1A; break;  // 16-bit
            case 10: _ocr16 = (uint16_t*)&OCR1B; break;  // 16-bit
            case 11: _ocr8  = &OCR2A; break;             // 8-bit
        }
    }
}

//==============================================================================
// Public Method: init, reset
// Description:   configure the PWM output pin and set the initial duty cycle
//==============================================================================
bool PWModulation::init() {
    if (_valid_pwm_pin(_pin)) {
        SETUP_PWM_FOR_PIN(_pin);   // Register bits defined in pwm.h
        reset();                   // Set the initial duty cycle
        return true;               // Valid pin
    } else {
        return false;              // Invalid pin
    }
}

void PWModulation::reset() {
    set_duty_cycle(UINT8_MAX);  // Set duty cycle to 8-bit max (255)
}

//==============================================================================
// Public Method: setDutyCycle
// Description:   Set the duty cycle of the PWM signal. The duty cycle is
//                a value between 0 and 255, where 0 is 0% and 255 is 100%.
//==============================================================================
void PWModulation::set_duty_cycle(uint8_t duty) {
    _duty_cycle = duty;                          // Just store duty cycle value
    if (_ocr8) *_ocr8 = (uint8_t)(duty & 0xFF);  // Cast to 8-bit where needed
    if (_ocr16) *_ocr16 = duty;                  // Directly assign to 16-bit
}

//==============================================================================
// Public Method: rampOutput
// Description:   Ramp the PWM output up and down over a specified cycle time.
//                If you pass a cycle time of 1000ms and a millis timer, the
//                duty cycle will ramp up and down once every second.
//==============================================================================
void PWModulation::ramp_output(const uint16_t &cycle_time, Timer &timer) {
    // Increment overflow counter based on timer overflows
    _overflow_counter += timer.overflow_counter;

    // Calculate the number of timer overflows required for a single step of duty cycle change
    uint16_t steps = cycle_time / (UINT8_MAX * 2); // 255 * 2 overflows per cycle

    // Update duty cycle every 'steps' overflows
    if (_overflow_counter >= steps) {
        if (_ramp_up) {
            _duty_cycle++;
            if (_duty_cycle >= UINT8_MAX) {
                _duty_cycle = UINT8_MAX;
                _ramp_up = false;
            }
        } else {
            _duty_cycle--;
            if (_duty_cycle <= 0) {
                _duty_cycle = 0;
                _ramp_up = true;
            }
        }
        set_duty_cycle(_duty_cycle); // Set the current duty cycle
        _overflow_counter = 0;        // Reset the overflow counter
    }
}

//==============================================================================
// Constexpr validation for PWM pins
// Description: Ensure the PWM pin is valid at compile time, the valid pins
//              are 3, 5, 6, 9, 10, and 11 on the Arduino Uno.
//==============================================================================
constexpr bool PWModulation::_valid_pwm_pin(const uint8_t _pin) {
    return _pin == 3 || _pin == 5 || _pin == 6 || 
           _pin == 9 || _pin == 10 || _pin == 11;
}
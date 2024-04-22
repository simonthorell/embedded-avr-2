//==============================================================================
// PWM Interface Class Implementation
//==============================================================================
#include "drivers/pwm.h"

const float PROPORTIONAL_GAIN = 0.1; // Gain for proportional control

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
// Public Method: init
// Description:   configure the PWM output pin and set the initial duty cycle
//==============================================================================
bool PWModulation::init() {
    if (_valid_pwm_pin(_pin)) {
        SETUP_PWM_FOR_PIN(_pin);   // Register bits defined in pwm.h
        set_duty_cycle(255);       // Set initial duty cycle to max (0-255)
        return true;               // Valid pin
    } else {
        return false;              // Invalid pin
    }
}

//==============================================================================
// Public Method: setDutyCycle, adjustDutyCycle
// Description:   Set the duty cycle of the PWM signal. The duty cycle is
//                a value between 0 and 255, where 0 is 0% and 255 is 100%.
//                The adjustDutyCycle method is used to adjust the duty cycle
//                based on a proportional control algorithm.
//==============================================================================
void PWModulation::set_duty_cycle(uint8_t duty) {
    _duty_cycle = duty;      // Save the duty cycle for future adjustments
    /* This got a bit more complicated that expected to cover both the 
       8 and 16-bit timers... */
    if (_ocr8) *_ocr8 = (uint8_t)(duty & 0xFF);  // Cast to 8-bit where needed
    if (_ocr16) *_ocr16 = duty;                  // Directly assign to 16-bit
}

void PWModulation::adjust_duty_cycle(int error) {    
    // Calculate the adjustment needed
    int adjustment = (int)(error * PROPORTIONAL_GAIN);
    
    // Calculate the new duty cycle with range (0-255)
    int new_duty_cycle = _duty_cycle + adjustment;
    if (new_duty_cycle > 255) new_duty_cycle = 255;
    if (new_duty_cycle < 0) new_duty_cycle = 0;
    
    // Set the new duty cycle
    set_duty_cycle((uint8_t)new_duty_cycle);
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
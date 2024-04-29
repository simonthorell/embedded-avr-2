//==============================================================================
// LED Class Implementation
//==============================================================================
#include "led.h"

//==============================================================================
// LED Constructor
// Description: If PWM is set to PWM_ON, the LED will be initialized
//              with PWM functionality. Otherwise, the LED will be
//              initialized as a digital output.
//==============================================================================
LED::LED(uint8_t pin, bool enable_pwm) 
    : _gpio(GPIO::DIGITAL_PIN, pin),
      _adc(),
      _pwm(pin),
      _pwm_enabled(enable_pwm),
      _power(255),
      _blink_interval(0),
      _prev_blink_interval(0),
      _prev_overflows(0),
      _overflow_counter(0)
{
    _gpio.enable_output(); // Set the GPIO pin as output

    if (enable_pwm) {
        _pwm.init(); // Initialize PWM functionality if enabled
    }
}

//==============================================================================
// LED Public Methods: turn_on, turn_off, toggle, is_on, is_off
//==============================================================================
void LED::turn_on() {
    if (_pwm_enabled) {
        _pwm.set_duty_cycle(_power);
    } else {
        _gpio.set_high();
    }
}

void LED::turn_off() {
    if (_pwm_enabled) {
        _pwm.set_duty_cycle(0); // Off
    } else {
        _gpio.set_low();
    }
}

void LED::toggle() {
    if (_pwm_enabled) {
        if (_pwm._duty_cycle == 0) {
            _pwm.set_duty_cycle(_power);
        } else {
            _power = _pwm._duty_cycle;
            _pwm.set_duty_cycle(0); // Off
        }
    } else {
        _gpio.toggle();
    }
}

bool LED::is_on() {
    if (_pwm_enabled) {
        return _pwm._duty_cycle > 0;
    } else {
        return _gpio.is_high();
    }
}

bool LED::is_off() {
    if (_pwm_enabled) {
        return _pwm._duty_cycle == 0;
    } else {
        return _gpio.is_low();
    }
}

//==============================================================================
// LED Public Methods: blink, adc_blink
// Description: These methods are used to blink the LED at a given
//              interval. The blink method is used to blink the LED
//              at a fixed interval, while the adc_blink method is used
//              to blink the LED at an interval based on the ADC reading
//==============================================================================
void LED::blink(uint16_t blink_interval, Timer &timer) {
    // Return early if the timer has not reached threshold (blink_interval)
    if (timer.overflow_counter < (_prev_overflows + blink_interval))
        return;

    toggle(); // Toggle the LED
    _prev_overflows = timer.overflow_counter; // Reset the overflow counter
}

void LED::adc_blink(Timer &timer, Serial &serial, 
                    const uint8_t &adc_ch, const uint16_t &max_interval) {
    
    _prev_blink_interval = _blink_interval;
    uint16_t adc_reading = _adc.read_channel(adc_ch);
    uint16_t adc_voltage = adc_reading;
    _adc.convert_to_mv(adc_voltage);
    _blink_interval = adc_voltage / 
                     (ADConverter::MAX_INPUT_VOLTAGE / max_interval);

    if(_blink_interval == 0) {
        turn_on();
    } else {
        blink(_blink_interval, timer);
    }

    // Notify if blink time has changed
    if (_blink_interval != _prev_blink_interval) {
        if (_blink_interval == 0) {
            serial.uart_put_str("Blink off. LED set to fixed light.\r\n");
        } else {
            char message[64]; // Buffer to hold UART message
            sprintf(message, 
                    "Blink interval: %dms (ADC value: %d, Voltage: %dmV)\r\n",
                    _blink_interval, adc_reading, adc_voltage);
            serial.uart_put_str(message);
        }
    }
}

void LED::set_power(const uint16_t &cycle_time) {
    _pwm.set_duty_cycle(cycle_time);
}

void LED::ramp_brightness(const uint16_t &cycle_time, Timer &timer) {
    _pwm.ramp_output(cycle_time, timer);
}
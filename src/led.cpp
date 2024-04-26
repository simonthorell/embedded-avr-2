//======================================================================
// LED Class Implementation
//======================================================================
#include "led.h"

//======================================================================
// LED Constructor
// Description: If PWM is set to PWM_ON, the LED will be initialized
//======================================================================
LED::LED(uint8_t pin, bool pwm) : _gpio(DIGITAL_PIN, pin), _pwm(pin) {
    // Set the pin as an output using the GPIO object
    _gpio.enable_output();
    if (pwm) {
        _pwm_enabled = true;
        _pwm.init();
    }
}

//======================================================================
// LED Public Methods: turn_on, turn_off, toggle, is_on, is_off
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

//======================================================================
// LED Public Methods: blink, adc_blink
// Description: These methods are used to blink the LED at a given
//              interval. The blink method is used to blink the LED
//              at a fixed interval, while the adc_blink method is used
//              to blink the LED at an interval based on the ADC reading
//======================================================================
void LED::blink(const uint16_t &blink_interval, const Timer &timer) {
    _overflow_counter += timer.overflow_counter;

    if ((_overflow_counter >= blink_interval)) {

        if (_pwm_enabled) {
            if (_pwm._duty_cycle == 0) {
                _pwm.set_duty_cycle(_pwm_power);                
            } else {
                _pwm_power = _pwm._duty_cycle;
                _pwm.set_duty_cycle(0);
            }
        } else {
            toggle();
        }
        
        _overflow_counter = 0; // Reset LED counter
    }
}

void LED::adc_blink(const Timer &timer, Serial &serial, 
                    const uint8_t &adc_ch, const uint16_t &max_interval) {
    
    _prev_blink_interval = _blink_interval;
    uint16_t adc_reading = _adc.read_channel(adc_ch);
    uint16_t adc_voltage = adc_reading;
    _adc.convert_to_mv(adc_voltage);
    _blink_interval = adc_voltage / (MAX_INPUT_VOLTAGE / max_interval);

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
            // Buffer to hold UART message
            char message[128];
            sprintf(message, "Blink interval: %dms (ADC value: %d, Voltage: %dmV)\r\n",
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
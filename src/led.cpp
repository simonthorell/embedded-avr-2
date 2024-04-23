//======================================================================
// LED Class Implementation
//======================================================================
#include "led.h"

//======================================================================
// LED Constructor
//======================================================================
LED::LED(uint8_t pin) : _gpio(DIGITAL_PIN, pin) {
    // Set the pin as an output using the GPIO object
    _gpio.enable_output();
}

//======================================================================
// LED Public Methods
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

void LED::blink(const uint16_t &blink_interval, const Timer &timer) {
    _overflow_counter += timer.overflow_counter;

    if (_overflow_counter >= blink_interval) {
        toggle();
        _overflow_counter = 0; // Reset LED counter
    }
}

void LED::adc_blink(const Timer &timer, Serial &serial, 
                    const uint8_t &adc_ch, const uint16_t &max_interval) {
    
    _prev_blink_interval = _blink_interval;
    uint16_t adc_reading = adc.read_channel(adc_ch);
    adc.convert_to_mv(adc_reading);
    _blink_interval = adc_reading / (MAX_INPUT_VOLTAGE / max_interval);

    if(_blink_interval == 0) {
        turn_on();
    } else {
        blink(_blink_interval, timer);
    }

    // Notify if blink time has changed
    if(_blink_interval != _prev_blink_interval) {
        if (_blink_interval == 0) {
            serial.uart_put_str("Blink off and LED set to fixed light\r\n");
        } else {
            char buffer[20];
            serial.uart_put_str("Blink time: ");
            sprintf(buffer, "%d", _blink_interval);
            serial.uart_put_str(buffer);
            serial.uart_put_str("ms\r\n");
        }
    }

}
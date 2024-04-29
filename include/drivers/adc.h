#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

//==============================================================================
// ADC Class Declaration
//==============================================================================
class ADConverter {
public:
    static constexpr int MAX_INPUT_VOLTAGE = 5000;  // Maximum input voltage(mV)
    static constexpr int MAX_ADC_VALUE = 1023;      // Maximum ADC value

    // Constructor
    ADConverter();

    // Public Methods
    uint16_t read_channel(uint8_t ch);
    void convert_to_mv(uint16_t &adc_value);
};

#endif // ADC_H
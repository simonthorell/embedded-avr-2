#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

#define MAX_INPUT_VOLTAGE 5000 // Maximum input voltage in mV
#define MAX_ADC_VALUE     1023

//==============================================================================
// ADC Class Declaration
//==============================================================================
class ADConverter {
public:
    // Constructor
    ADConverter();

    // Public Methods
    uint16_t read_channel(uint8_t ch);
    void convert_to_mv(uint16_t &adc_value);
};

#endif // ADC_H
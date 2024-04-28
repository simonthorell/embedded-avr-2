//==============================================================================
// ADC Interface Class Implementation
//==============================================================================
#include "drivers/adc.h"

//==============================================================================
// Constructor
//==============================================================================
ADConverter::ADConverter() {
    ADMUX = (1<<REFS0);              // Set reference to AVCC
    ADCSRA = (1<<ADEN)               // Enable ADC
           | (1<<ADPS2) | (1<<ADPS1) // Set prescaler to 128
           | (1<<ADPS0);             // 16MHz/128 = 125kHz the ADC sample rate
}

//==============================================================================
// Public Method: Read
// Description:   Read the value from the specified ADC channel
//==============================================================================
uint16_t ADConverter::read_channel(uint8_t ch) {
    // Mask the channel number to ensure it is within 0 to 7
    ch &= 0b00000111;

    // Set the selected channel on ADMUX; clear previous channel selection
    ADMUX = (ADMUX & 0xF8) | ch;  // Clear lowest three bits and set new channel

    // Start the conversion
    ADCSRA |= (1<<ADSC);

    // Wait for the conversion to complete (ADSC becomes '0' again)
    while (ADCSRA & (1<<ADSC));

    // Return the ADC conversion result
    return ADC;
}

void ADConverter::convert_to_mv(uint16_t &adc_value) {
    adc_value = static_cast<uint16_t>((static_cast<float>(adc_value) * MAX_INPUT_VOLTAGE) / MAX_ADC_VALUE);
}
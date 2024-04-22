#ifndef ADC_H
#define ADC_H

#include <avr/io.h>

class ADConverter {
public:
    ADConverter(); // Constructor
    uint16_t readADC(uint8_t ch); 
};

#endif // ADC_H
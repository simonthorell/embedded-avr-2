#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>         // size_t
#include <stdio.h>          // For sprintf 
#include "drivers/serial.h"

//==============================================================================
// Timer Class Declaration
//==============================================================================
class Timer {
public:
    enum TimerNum { TIMER0, TIMER1, TIMER2 };
    enum TimeUnit { MILLIS, MICROS };

    struct PrescalerSetting {
        uint32_t threshold;
        uint16_t prescaler;
    };

    struct TimerConfig {
        uint16_t tccrA;
        uint16_t tccrB;
    };

    // Constructor
    Timer(TimerNum num, TimeUnit unit);

    // Public methods
    void init(const uint32_t &interval, Serial &serial);
    void set_prescaler(uint32_t interval, Serial &serial);
    void start();
    void stop();
    
    // Static variables
    volatile uint16_t overflow_counter;
    static Timer* instance; // Pointer to the timer instance

private:
    // Private variables
    TimerNum _num;
    TimeUnit _unit;

    // Private methods
    TimerConfig _clear_tccr();
    void _set_tccr(const TimerConfig &config); 
};

#endif // TIMER_H
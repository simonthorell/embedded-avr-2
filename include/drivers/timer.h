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
    enum TimerNum  { TIMER0, TIMER1, TIMER2 };
    enum TimeUnit  { MILLIS, MICROS };
    enum TimerMode { NORMAL, CTC, EXT_CLOCK };

    struct PrescalerSettings {
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
    void init(const uint32_t &interval, TimerMode mode, Serial &serial);
    void configure(TimerMode mode, uint32_t interval, Serial &serial);
    
    // Static variables
    volatile uint16_t overflow_counter;
    volatile uint16_t captured_value;
    volatile uint16_t interval_devisor;
    volatile uint16_t temp_interval_devisor;
    static Timer* instance; // Pointer to the timer instance

private:
    // Private variables
    TimerNum _num;
    TimeUnit _unit;
    uint16_t _adjusted_interval;
    
    // Private methods
    void start();
    void stop();
    uint16_t set_prescaler(uint32_t interval, Serial &serial);
    void set_mode(TimerMode mode);
    void _clear_prescaler_bits();

    // TimerConfig _clear_tccr();
    // void _set_tccr(const TimerConfig &config); 
};

#endif // TIMER_H
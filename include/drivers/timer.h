#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h> // size_t

#include "drivers/serial.h"

#define F_CPU        16000000UL  // Clock frequency
#define PRESCALER_US 1           // Prescaler for microseconds
#define PRESCALER_MS 64          // Prescaler for milliseconds
#define US_PER_SEC   1000000UL   // us per second
#define MS_PER_SEC   1000.0      // ms per second

#define TIMER0_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS00) : \
    (prescaler)  == 8 ?     (1 << CS01) : \
    (prescaler)  == 64 ?   ((1 << CS01) | (1 << CS00)) : \
    (prescaler)  == 256 ?   (1 << CS02) : \
    (prescaler)  == 1024 ? ((1 << CS02) | (1 << CS00)) : \
    0)

#define TIMER1_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS10) : \
    (prescaler)  == 8 ?     (1 << CS11) : \
    (prescaler)  == 64 ?   ((1 << CS11) | (1 << CS10)) : \
    (prescaler)  == 256 ?   (1 << CS12) : \
    (prescaler)  == 1024 ? ((1 << CS12) | (1 << CS10)) : \
    0)

#define TIMER2_PS_BITS(prescaler) \
    ((prescaler) == 1 ?     (1 << CS20) : \
    (prescaler)  == 8 ?     (1 << CS21) : \
    (prescaler)  == 32 ?   ((1 << CS21) | (1 << CS20)) : \
    (prescaler)  == 64 ?    (1 << CS22) : \
    (prescaler)  == 128 ?  ((1 << CS22) | (1 << CS20)) : \
    (prescaler)  == 256 ?  ((1 << CS22) | (1 << CS21)) : \
    (prescaler)  == 1024 ? ((1 << CS22) | (1 << CS21) | (1 << CS20)) : \
    0)

struct PrescalerSetting {
    uint32_t threshold;
    uint16_t prescaler;
};

class Timer {
public:
    enum TimerNum { TIMER_0, TIMER_1, TIMER_2 };
    enum TimeUnit { MILLIS, MICROS };

    Timer(TimerNum num, TimeUnit unit); // Constructor
    void init(const uint32_t &interval, Serial &serial);
    void set_prescaler(uint32_t interval, Serial &serial);
    void start();
    void stop();
    void reset();
    volatile bool timer_overflowed;
    volatile unsigned long overflow_counter;
    static Timer* timer_ptr; // Pointer to the timer instance

private:
    TimerNum _num;
    TimeUnit _unit;
};

#endif

#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h> // size_t

#include "drivers/serial.h"

#define F_CPU        16000000UL  // Clock frequency (TODO: Define globally...)
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

struct TimerConfig {
    uint16_t tccrA;
    uint16_t tccrB;
};

class Timer {
public:
    enum TimerNum { TIMER0, TIMER1, TIMER2 };
    enum TimeUnit { MILLIS, MICROS };

    Timer(TimerNum num, TimeUnit unit); // Constructor

    void init(const uint32_t &interval, Serial &serial);
    void set_prescaler(uint32_t interval, Serial &serial);
    void start();
    void stop();
    void reset();

    volatile bool timer_overflowed;
    volatile uint16_t overflow_counter;
    static Timer* instance; // Pointer to the timer instance

private:
    TimerConfig _clear_tccr();
    void _set_tccr(const TimerConfig &config);
    TimerNum _num;
    TimeUnit _unit;
};

#endif

/*
#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include "serial.h"

class Timer {
public:
    enum timer_num { TIMER0, TIMER1, TIMER2 };
    enum time_unit { MILLIS, MICROS };

    Timer(timer_num num, time_unit unit);
    ~Timer();

    void init(uint32_t interval, Serial& serial);
    void start();
    void stop();
    void reset();

    static Timer* instance;
    volatile uint32_t overflow_counter;
    volatile bool timer_overflowed;

private:

    timer_num num;
    time_unit unit;

    uint16_t _prescaler;
    uint32_t _ocr_value;

    void set_prescaler(uint32_t interval);
    uint8_t calculate_prescaler_bits(uint16_t prescaler) const;
    void configure_timer();
    void clear_timer_registers();
    void set_timer_registers(uint8_t prescaler_bits, uint16_t ocr_value);
};

    // ISR prototypes, assuming they are implemented to use Timer::instance
    ISR(TIMER0_COMPA_vect);
    ISR(TIMER1_COMPA_vect);
    ISR(TIMER2_COMPA_vect);

#endif // TIMER_H
*/
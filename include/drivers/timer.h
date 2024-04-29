#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stddef.h>         // size_t
#include <stdio.h>          // For sprintf 
#include "drivers/serial.h"

#ifndef F_CPU
#define F_CPU 16000000UL // Define default MCU clock speed if not defined
#endif

#define US_PER_SEC   1000000UL   // us per second
#define MS_PER_SEC   1000.0      // ms per second
#define MAX_INTERVAL 4000        // Maximum interval for 16-bit timers (ms)
#define MAX_INTERVAL_8BIT 255    // Maximum interval for 8-bit  timers

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

    // Remove later?
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

    // Static Pointers for Timer Instances (Singletons)
    static Timer* timer_0_instance; // Pointer to the timer timer_0_instance
    static Timer* timer_1_instance; // Pointer to the timer timer_1_instance
    static Timer* timer_2_instance; // Pointer to the timer timer_1_instance

    static const PrescalerSettings ms_settings_8bit[];
    static const PrescalerSettings us_settings_8bit[];
    static const PrescalerSettings ms_settings_16bit[];
    static const PrescalerSettings us_settings_16bit[];

    static void handle_timer_interupt(Timer* timer); // Timer interrupt handler

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
};

#endif // TIMER_H
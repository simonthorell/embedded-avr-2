#ifndef TIMER_H
#define TIMER_H

#include <avr/io.h>
#include <avr/interrupt.h>

#define F_CPU        16000000UL  // Clock frequency
#define PRESCALER_US 1           // Prescaler for microseconds
#define PRESCALER_MS 64          // Prescaler for milliseconds
#define SEC_TO_US    1000000UL   // us per second
#define SEC_TO_MS    1000        // ms per second
// #define OCR_MICROS   15          // ((F_CPU / (PRESCALER_US * SEC_TO_US)) - 1)
// #define OCR_MILLIS   249         // ((F_CPU / (PRESCALER_MS * SEC_TO_MS)) - 1)
// // For some reason the compiler doesn't like the above calculations for OCR...

class Timer {
public:
    enum TimerType { TIMER_0, TIMER_1, TIMER_2 };
    enum TimeUnit { MILLIS, MICROS };

    Timer(TimerType type, TimeUnit unit);
    void start();
    void stop();
    volatile unsigned long overflow_counter;

    static Timer* timer_0_ptr;  // Pointers to timer instances
    static Timer* timer_1_ptr;
    static Timer* timer_2_ptr;

    // Constants for OCR0A and OCR2A to achive low latency
    static constexpr uint32_t ocr_micros = ((F_CPU / SEC_TO_US / PRESCALER_US) - 1);
    static constexpr uint32_t ocr_millis = ((F_CPU / SEC_TO_MS / PRESCALER_MS) - 1);

private:
    TimerType _type;
    TimeUnit _unit;
    void _init_timer_0(TimeUnit unit);
    void _init_timer_1(TimeUnit unit);
    void _init_timer_2(TimeUnit unit);
};

#endif
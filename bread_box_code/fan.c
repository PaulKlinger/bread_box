#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "fan.h"

volatile uint8_t fan_pwm_cnt = 0; // /4
volatile uint8_t fan_pwm_level = 0; // /4

ISR(TCB0_INT_vect) {
    fan_pwm_cnt = (fan_pwm_cnt + 1) % 4;
    if (fan_pwm_cnt < fan_pwm_level) {
        VPORTB.OUT |= PIN3_bm;
    } else {
        VPORTB.OUT &= ~PIN3_bm;
    }
    TCB0.INTFLAGS |= TCB_CAPT_bm; // clear interrupt flag
}

void setup_fan() {
    /* setup TCB0 as a 4MHz counter for fan pwm */
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // periodic interrupt mode
    TCB0.INTCTRL = TCB_CAPT_bm; // enable periodic interrupt
    TCB0.CCMP = 100; // 25us interval
    
    VPORTB.DIR |= PIN3_bm;
}

void set_fan_low() {
    if (fan_pwm_level == 0) {
        // startup pulse
        fan_pwm_level = 4;
        _delay_ms(300);
    }
    fan_pwm_level = 2;
}

void set_fan_high() {fan_pwm_level = 4;}
void turn_off_fan() {fan_pwm_level = 0;}

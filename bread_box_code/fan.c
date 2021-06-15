#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "fan.h"

volatile uint8_t fan_pwm_cnt; // 0-7

ISR(TCB0_INT_vect) {
    fan_pwm_cnt = (fan_pwm_cnt + 1) % 8;
    if (fan_pwm_cnt < current_fan_state) {
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
    TCB0.CCMP = 50; // 12.5us interval
    
    VPORTB.DIR |= PIN3_bm;
    
    fan_pwm_cnt = 0;
    
    current_fan_state = FAN_OFF;
}

void set_fan_low() {
    if (current_fan_state == FAN_OFF) {
        // startup pulse
        current_fan_state = FAN_HIGH;
        _delay_ms(300);
    }
    current_fan_state = FAN_LOW;
}

void set_fan_high() {current_fan_state = FAN_HIGH;}
void turn_off_fan() {current_fan_state = FAN_OFF;}

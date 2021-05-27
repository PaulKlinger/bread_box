#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>

#include "shutter.h"


void setup_shutter(){
    /* Set up TCA0 as a 125kHz counter */
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.CTRLD = 0;
    TCA0.SINGLE.PER = 5000;
    TCA0.SINGLE.CNT = 0;
    // on time = 8us * TCA0.CMP2 on time per cmp
    close_shutter();
    
    VPORTB.DIR |= PIN2_bm;
}

inline void open_shutter() {
    TCA0.SINGLE.CMP2 = 270;
}

inline void close_shutter() {
    TCA0.SINGLE.CMP2 = 545;
}
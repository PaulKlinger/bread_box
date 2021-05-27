#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#include "buttons.h"


void setup_buttons() {
    /* setup RTC for 8ms interval interrupts*/
    while (RTC.STATUS > 0) {}
    while (RTC.PITSTATUS > 0) {}
    RTC.PITCTRLA = RTC_PERIOD_CYC256_gc /* Period: RTC Clock Cycles 256 */
    				| 1 << RTC_PITEN_bp; /* Enable: enabled */
    RTC.PITINTCTRL = 1 << RTC_PI_bp; /* Periodic Interrupt: enabled */
    RTC.CTRLA = RTC_PRESCALER_DIV1_gc /* Prescaling Factor: RTC Clock / 1 */
    				| 1 << RTC_RTCEN_bp /* Enable: enabled */
    				| 0 << RTC_RUNSTDBY_bp; /* Run In Standby: disabled */
    
    
    VPORTA.DIR &= ~(PIN3_bm & PIN4_bm & PIN5_bm);
    PORTA_PIN3CTRL |= PORT_PULLUPEN_bm;
    PORTA_PIN4CTRL |= PORT_PULLUPEN_bm;
    PORTA_PIN5CTRL |= PORT_PULLUPEN_bm;
    
    btn_state.last_state = BTN_NONE;
    btn_state.debounce_counter = 0;
    btn_state.debounced_state = BTN_NONE;
    btn_state.released_and_not_processed = BTN_NONE;
}


# define DEBOUNCE_THRESHOLD 4 /* x 8ms */


ISR(RTC_PIT_vect)
{
    enum button current_button = BTN_NONE;
    if (~VPORTA.IN & BTN_UP){
        current_button = BTN_UP;
    } else if (~VPORTA.IN & BTN_DOWN){
        current_button = BTN_DOWN;
    } else if (~VPORTA.IN & BTN_SELECT) {
        current_button = BTN_SELECT;
    }
    
    if (current_button != btn_state.last_state) {
        // if the pressed button switched, reset the debounce counter
        btn_state.debounce_counter = 0;
    }
    btn_state.last_state = current_button;
    
    if (current_button != btn_state.debounced_state) {
        // if the pressed button is different from the previous
        // debounced state we start counting
        btn_state.debounce_counter++;
        if (btn_state.debounce_counter >= DEBOUNCE_THRESHOLD) {
            // if the new state is stable for DEBOUNCE_THRESHOLD cycles
            // make it the debounced state
            if (current_button == BTN_NONE) {
                // if a button was released generate a released "event"
                btn_state.released_and_not_processed = btn_state.debounced_state;
            }
            btn_state.debounced_state = current_button;
            btn_state.debounce_counter = 0;
        }
    }
    
    /* TRIGB interrupt flag has to be cleared manually */
    RTC.PITINTFLAGS = RTC_PI_bm;
}
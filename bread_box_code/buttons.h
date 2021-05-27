/* 
 * File:   buttons.h
 * Author: kling
 *
 * Created on 27 May 2021, 17:57
 */

#ifndef BUTTONS_H
#define	BUTTONS_H

#ifdef	__cplusplus
extern "C" {
#endif

enum button {
    BTN_NONE = 0,
    BTN_UP = PIN5_bm,
    BTN_DOWN = PIN3_bm,
    BTN_SELECT = PIN4_bm
};

struct buttons_state {
    enum button last_state;
    uint8_t debounce_counter;
    enum button debounced_state;
    enum button released_and_not_processed;
};

void setup_buttons();

struct buttons_state btn_state;


#ifdef	__cplusplus
}
#endif

#endif	/* BUTTONS_H */


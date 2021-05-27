/*
 * File:   main.c
 * Author: kling
 *
 * Created on 06 March 2021, 16:53
 */

#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

#include "twi.h"
#include "lcd.h"
#include "buttons.h"

#define ADDR 0x44

FUSES = 
{
	.APPEND = 0,
	.BODCFG = ACTIVE_DIS_gc | LVL_BODLEVEL0_gc | SAMPFREQ_1KHZ_gc | SLEEP_DIS_gc,
	.BOOTEND = 0,
	.OSCCFG = FREQSEL_16MHZ_gc,
	.SYSCFG0 = CRCSRC_NOCRC_gc | RSTPINCFG_UPDI_gc,
	.SYSCFG1 = SUT_64MS_gc,
	.WDTCFG = PERIOD_OFF_gc | WINDOW_OFF_gc,
};

#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

#define NUM_OPTIONS  4

struct config {
    uint8_t thresh_shutter, thresh_fan_low, thresh_fan_high, hysteresis;
};

enum option {
    OPT_THRESH_SHUTTER = 0,
    OPT_THRESH_FAN_LOW = 1,
    OPT_THRESH_FAN_HIGH = 2,
    OPT_HYSTERESIS = 3
};

struct ui_state {
    enum option selected_option;
    bool modify;
};

struct sensor_reading {
    float temperature;
    float humidity;
    // add status enum
};

struct sensor_reading read_sensor(uint8_t addr) {
    // start write transaction
    TWI0_start_write(addr);
    TWI0_write(0x24);
    TWI0_write(0x00);
    TWI0_stop();
    
    // wait for measurement to complete
    //(max for high repeatability should be 30ms)
    _delay_ms(50);
    
    TWI0_start_read(addr);
    uint8_t temp_msb = TWI0_read(1);
    uint8_t temp_lsb = TWI0_read(1);
    uint8_t temp_crc = TWI0_read(1);
    
    uint8_t hum_msb = TWI0_read(1);
    uint8_t hum_lsb = TWI0_read(1);
    uint8_t hum_crc = TWI0_read(0);
    TWI0_stop();
    
    uint16_t temp_int = (temp_msb<<8) | temp_lsb;
    uint16_t hum_int = (hum_msb<<8) | hum_lsb;
    
    struct sensor_reading result;
    
    result.temperature = -45.0 + (175.0 / 65535.0) * temp_int;
    result.humidity = (100.0 / 65535.0) * hum_int;
    
    return result;
}

inline void open_shutter() {
    TCA0.SINGLE.CMP2 = 270;
}

inline void close_shutter() {
    TCA0.SINGLE.CMP2 = 545;
}

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


void draw_reading(struct sensor_reading reading) {
    char temp_template[] = "%.1f C";
    char hum_template[] = "%.0f %%";
    char temp_output[15];
    char hum_output[15];
    sprintf(temp_output, temp_template, reading.temperature);
    sprintf(hum_output, hum_template, reading.humidity);
    
    lcd_gotoxy(0, 0);
    lcd_puts(temp_output);
    lcd_gotoxy(17, 0);
    lcd_puts(hum_output);
}

void draw_menu(struct ui_state state, struct config current_config) {
    lcd_gotoxy(0, 2 + state.selected_option);
    if (state.modify) {
        lcd_putc('~');
    } else {
        lcd_putc('>');
    }
    
    char temp[] = "%d%%";
    char setting_out[5];
    
    for (uint8_t i=0; i < NUM_OPTIONS; i++) {
        lcd_gotoxy(1, 2 + i);
        switch (i) {
            case OPT_THRESH_SHUTTER:
                lcd_puts("shutter     > ");
                sprintf(setting_out, temp, current_config.thresh_shutter);
                break;
            case OPT_THRESH_FAN_LOW:
                lcd_puts("fan low     > ");
                sprintf(setting_out, temp, current_config.thresh_fan_low);
                break;
            case OPT_THRESH_FAN_HIGH:
                lcd_puts("fan high    > ");
                sprintf(setting_out, temp, current_config.thresh_fan_high);
                break;
            case OPT_HYSTERESIS:
                lcd_puts("hysteresis     ");
                sprintf(setting_out, temp, current_config.hysteresis);
                break;
        }
        lcd_puts(setting_out);
    }
}


void update_ui_state(
    struct ui_state *state,
    struct config *cfg,
    struct buttons_state *btn_state
) {
    if (!state->modify) {
        switch (btn_state->released_and_not_processed) {
            case BTN_UP:
                state->selected_option = (state->selected_option == 0) ? 0 : state->selected_option - 1;
                break;
            case BTN_DOWN:
                state->selected_option = (state->selected_option == NUM_OPTIONS - 1) ? NUM_OPTIONS - 1 : state->selected_option + 1;
                break;
            case BTN_SELECT:
                state->modify = true;
                break;
            case BTN_NONE:
                return;
        }
    } else{
        int8_t delta = 0;
        switch (btn_state->released_and_not_processed) {
            case BTN_UP:
                delta = 1;
                break;
            case BTN_DOWN:
                delta = -1;
                break;
            case BTN_SELECT:
                state->modify = false;
                break;
            case BTN_NONE:
                return;
        }
        
        switch (state->selected_option) {
            case (OPT_THRESH_SHUTTER):
                cfg->thresh_shutter = (cfg->thresh_shutter + delta) % 101;
                break;
            case (OPT_THRESH_FAN_LOW):
                cfg->thresh_fan_low = (cfg->thresh_fan_low + delta) % 101;
                break;
            case (OPT_THRESH_FAN_HIGH):
                cfg->thresh_fan_high = (cfg->thresh_fan_high + delta) % 101;
                break;
            case (OPT_HYSTERESIS):
                cfg->hysteresis = (cfg->hysteresis + delta) % 20;
                break;
        }
    }
    btn_state->released_and_not_processed = BTN_NONE;
}


int main(void) {
    _PROTECTED_WRITE(
        CLKCTRL.MCLKCTRLB,
            CLKCTRL_PDIV_2X_gc /* Prescaler division: 2X */
			 | (0 << CLKCTRL_PEN_bp) /* Prescaler enable: disabled */
        );
    
    /* Set up TCA0 as a 1MHz (1 us) counter */
    TCA0.SINGLE.CTRLA = TCA_SINGLE_CLKSEL_DIV64_gc | TCA_SINGLE_ENABLE_bm;
    TCA0.SINGLE.CTRLB = TCA_SINGLE_CMP2EN_bm | TCA_SINGLE_WGMODE_SINGLESLOPE_gc;
    TCA0.SINGLE.CTRLD = 0;
    TCA0.SINGLE.PER = 5000;
    TCA0.SINGLE.CNT = 0;
    close_shutter();
    
    //4us per cmp
    
    /*setup TCB0 as a 4MHz counter */
    TCB0.CTRLA = TCB_ENABLE_bm | TCB_CLKSEL_CLKDIV2_gc;
    TCB0.CTRLB = TCB_CNTMODE_INT_gc; // periodic interrupt mode
    TCB0.INTCTRL = TCB_CAPT_bm; // enable periodic interrupt
    TCB0.CCMP = 100; // 25us interval
    
    
    
    sei();
    
    // set servo pin & fan pin to output
    VPORTB.DIR |= (PIN2_bm | PIN3_bm);
    setup_buttons();
    
    _delay_ms(100);
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
    lcd_display();
    
    struct config cfg = {
        .thresh_shutter = 50,
        .thresh_fan_low = 60,
        .thresh_fan_high = 70,
        .hysteresis = 5
    };
    
    struct ui_state current_ui_state = {
        .selected_option = OPT_THRESH_SHUTTER,
        .modify = false,
    };
    
    struct sensor_reading reading;
    uint8_t take_reading_cnt = 0;
    while (1) {
        if (take_reading_cnt % 10 == 0) {
            reading = read_sensor(ADDR);
            take_reading_cnt = 0;
        }
        take_reading_cnt++;
        
        update_ui_state(&current_ui_state, &cfg, &btn_state);
        
        lcd_clear_buffer();
        draw_reading(reading);
        draw_menu(current_ui_state, cfg);
        lcd_display();
        
        if (reading.humidity > cfg.thresh_shutter) {
            open_shutter();
        }
        if (reading.humidity > cfg.thresh_fan_low) {
            if (fan_pwm_level == 0) {
                fan_pwm_level = 4; // startup pulse
            } else {
                fan_pwm_level = 2;
            }
        }
        
        if (reading.humidity > cfg.thresh_fan_high) {
            fan_pwm_level = 4;
        }
        
        if (reading.humidity < cfg.thresh_fan_low - cfg.hysteresis) {
            fan_pwm_level = 0;
        }
        
        if (reading.humidity < cfg.thresh_shutter - cfg.hysteresis) {
            close_shutter();
        }
        _delay_ms(50);
    }
    
}

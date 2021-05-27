#include "config.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdbool.h>

#include "twi.h"
#include "lcd.h"
#include "buttons.h"
#include "shutter.h"
#include "fan.h"
#include "ui.h"
#include "sensor.h"


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


int main(void) {
    _PROTECTED_WRITE(
        CLKCTRL.MCLKCTRLB,
            CLKCTRL_PDIV_2X_gc /* Prescaler division: 2X */
			 | (0 << CLKCTRL_PEN_bp) /* Prescaler enable: disabled */
        );
    
    setup_shutter();
    setup_fan();
    setup_buttons();
    setup_ui();
    
    sei();
    
    struct config cfg = {
        .thresh_shutter = 50,
        .thresh_fan_low = 60,
        .thresh_fan_high = 70,
        .hysteresis = 5
    };
    
    struct sensor_reading reading;
    uint8_t take_reading_cnt = 0;
    
    while (1) {
        if (take_reading_cnt % 10 == 0) {
            reading = read_sensor(SENSOR_ADDR);
            take_reading_cnt = 0;
        }
        take_reading_cnt++;
        
        update_ui(reading, &cfg);
        
        if (reading.humidity > cfg.thresh_shutter) {
            open_shutter();
        }
        if (reading.humidity > cfg.thresh_fan_low) {
            set_fan_low();
        }
        
        if (reading.humidity > cfg.thresh_fan_high) {
            set_fan_high();
        }
        
        if (reading.humidity < cfg.thresh_fan_low - cfg.hysteresis) {
            turn_off_fan();
        }
        
        if (reading.humidity < cfg.thresh_shutter - cfg.hysteresis) {
            close_shutter();
        }
        _delay_ms(50);
    }
    
}

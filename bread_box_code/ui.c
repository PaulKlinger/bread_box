#include "config.h"
#include <stdio.h>
#include <stdbool.h>
#include <util/delay.h>

#include "lcd.h"
#include "ui.h"
#include "buttons.h"
#include "shutter.h"
#include "fan.h"

#define NUM_OPTIONS  4

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

struct ui_state current_ui_state;

void setup_ui() {
    current_ui_state.selected_option = OPT_THRESH_SHUTTER;
    current_ui_state.modify = false;
    
    _delay_ms(100); // wait for stable power before enabling display
    lcd_init(LCD_DISP_ON);
    lcd_clrscr();
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

void draw_status() {
    lcd_gotoxy(9, 0);
    lcd_puts(shutter_open ? "|" : "-");
    lcd_puts("   ");
    switch (current_fan_state) {
        case FAN_HIGH:
            lcd_putc('h');
            break;
        case FAN_LOW:
            lcd_putc('l');
            break;
        case FAN_OFF:
            lcd_putc('o');
            break;
    }
}

void draw_menu(struct ui_state state, struct config *current_config) {
    lcd_gotoxy(0, 3 + state.selected_option);
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
                sprintf(setting_out, temp, current_config->thresh_shutter);
                break;
            case OPT_THRESH_FAN_LOW:
                lcd_puts("fan low     > ");
                sprintf(setting_out, temp, current_config->thresh_fan_low);
                break;
            case OPT_THRESH_FAN_HIGH:
                lcd_puts("fan high    > ");
                sprintf(setting_out, temp, current_config->thresh_fan_high);
                break;
            case OPT_HYSTERESIS:
                lcd_puts("hysteresis     ");
                sprintf(setting_out, temp, current_config->hysteresis);
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
                save_config(*cfg);
                
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

void update_ui(struct sensor_reading reading, struct config *cfg) {
    update_ui_state(&current_ui_state, cfg, &btn_state);
    lcd_clear_buffer();
    draw_reading(reading);
    draw_status();
    draw_menu(current_ui_state, cfg);
    lcd_display();
}
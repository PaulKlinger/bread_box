/*
 * This file is part of lcd library for ssd1306/sh1106 oled-display.
 *
 * lcd library for ssd1306/sh1106 oled-display is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or any later version.
 *
 * lcd library for ssd1306/sh1106 oled-display is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Foobar.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Diese Datei ist Teil von lcd library for ssd1306/sh1106 oled-display.
 *
 * lcd library for ssd1306/sh1106 oled-display ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder jeder späteren
 * veröffentlichten Version, weiterverbreiten und/oder modifizieren.
 *
 * lcd library for ssd1306/sh1106 oled-display wird in der Hoffnung, dass es nützlich sein wird, aber
 * OHNE JEDE GEWÄHRLEISTUNG, bereitgestellt; sogar ohne die implizite
 * Gewährleistung der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Details.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <http://www.gnu.org/licenses/>.
 *
 *  lcd.h
 *
 *  Created by Michael Köhler on 22.12.16.
 *  Copyright 2016 Skie-Systems. All rights reserved.
 *
 * 
 *  lib for OLED-Display with ssd1306/sh1106-Controller
 *  first dev-version only for I2C-Connection
 *  at ATMega328P like Arduino Uno
 *
 *  at GRAPHICMODE lib needs static SRAM for display:
 *  DISPLAY-WIDTH * DISPLAY-HEIGHT + 2 bytes
 *
 *  at TEXTMODE lib need static SRAM for display:
 *  2 bytes (cursorPosition)
 */

#include "lcd.h"
#include "font.h"
#include <string.h>
#include "twi.h"

static struct {
    uint8_t x;
    uint8_t y;
} cursorPosition;
#if defined GRAPHICMODE
#include <stdlib.h>
#elif defined TEXTMODE
#else
#error "No valid displaymode! Refer lcd.h"
#endif


const uint8_t init_sequence [] PROGMEM = {    // Initialization Sequence
    LCD_DISP_OFF,    // Display OFF (sleep mode)
    0x20, 0b00,        // Set Memory Addressing Mode
    // 00=Horizontal Addressing Mode; 01=Vertical Addressing Mode;
    // 10=Page Addressing Mode (RESET); 11=Invalid
    0xB0,            // Set Page Start Address for Page Addressing Mode, 0-7
    0xC0,            // Set COM Output Scan Direction
    0x00,            // --set low column address
    0x10,            // --set high column address
    0x40,            // --set start line address
    0x81, 0x3F,        // Set contrast control register
    0xA0,            // Set Segment Re-map. A0=address mapped; A1=address 127 mapped.
    0xA6,            // Set display mode. A6=Normal; A7=Inverse
    0xA8, 0x3F,        // Set multiplex ratio(1 to 64)
    0xA4,            // Output RAM to Display
    // 0xA4=Output follows RAM content; 0xA5,Output ignores RAM content
    0xD3, 0x00,        // Set display offset. 00 = no offset
    0xD5,            // --set display clock divide ratio/oscillator frequency
    0xF0,            // --set divide ratio
    0xD9, 0x22,        // Set pre-charge period
    0xDA, 0x12,        // Set com pins hardware configuration
    0xDB,            // --set vcomh
    0x20,            // 0x20,0.77xVcc
    0x8D, 0x14,        // Set DC-DC enable
    
    
};

void i2c_byte(uint8_t byte_to_send) {
    TWI0_write(byte_to_send);
}

#pragma mark LCD COMMUNICATION
void lcd_command(uint8_t cmd[], uint8_t size) {
    TWI0_start_write(LCD_I2C_ADR);
    i2c_byte(0x00);    // 0x00 for command, 0x40 for data
    for (uint8_t i=0; i<size; i++) {
        i2c_byte(cmd[i]);
    }
    TWI0_stop();
}
void lcd_data(uint8_t data[], uint16_t size) {
    TWI0_start_write(LCD_I2C_ADR);
    i2c_byte(0x40);    // 0x00 for command, 0x40 for data
    for (uint16_t i = 0; i<size; i++) {
        i2c_byte(data[i]);
    }
    TWI0_stop();
}
#pragma mark -
#pragma mark GENERAL FUNCTIONS
void lcd_init(uint8_t dispAttr){
    TWI0_init();
    uint8_t commandSequence[sizeof(init_sequence)+1];
    for (uint8_t i = 0; i < sizeof (init_sequence); i++) {
        commandSequence[i] = (pgm_read_byte(&init_sequence[i]));
    }
    commandSequence[sizeof(init_sequence)]=(dispAttr);
    lcd_command(commandSequence, sizeof(commandSequence));
    lcd_clrscr();
}
void lcd_gotoxy(uint8_t x, uint8_t y){
    x = x * sizeof(FONT[0]);
    lcd_goto_xpix_y(x, y);
}

void lcd_goto_xpix_y(uint8_t x, uint8_t y){
    if( x > (DISPLAY_WIDTH) || y > (DISPLAY_HEIGHT/8-1)) return;// out of display
    cursorPosition.x=x;
    cursorPosition.y=y;
#if defined SSD1306
#if defined BLACKBOARD
    // The black board uses a different controller which actually follows
    // the SSD1306 datasheet: The commands 0xb0 to 0xb7 are not valid in
    // horizontal addressing mode.
    // So we have to use the longer 0x22 command instead.
    uint8_t commandSequence[] = {0x22, y, 0x07, 0x21, x, 0x7f};
#else
    uint8_t commandSequence[] = {0xb0+y, 0x21, x, 0x7f};
#endif
#elif defined SH1106
    uint8_t commandSequence[] = {0xb0+y, 0x21, 0x00+((2+x) & (0x0f)), 0x10+( ((2+x) & (0xf0)) >> 4 ), 0x7f};
#endif
    lcd_command(commandSequence, sizeof(commandSequence));
}

void lcd_clrscr(void){
#ifdef GRAPHICMODE
    lcd_clear_buffer();
    lcd_display();
#elif defined TEXTMODE
    uint8_t displayBuffer[DISPLAY_WIDTH];
    memset(displayBuffer, 0x00, sizeof(displayBuffer));
    for (uint8_t i = 0; i < DISPLAY_HEIGHT/8; i++){
        lcd_gotoxy(0,i);
        lcd_data(displayBuffer, sizeof(displayBuffer));
    }
#endif
    lcd_home();
}
void lcd_home(void){
    lcd_gotoxy(0, 0);
}
void lcd_invert(uint8_t invert){
    //i2c_start((LCD_I2C_ADR << 1) | 0);
    uint8_t commandSequence[1];
    if (invert != YES) {
        commandSequence[0] = 0xA6;
    } else {
        commandSequence[0] = 0xA7;
    }
    lcd_command(commandSequence, 1);
}
void lcd_sleep(uint8_t sleep){
    //i2c_start((LCD_I2C_ADR << 1) | 0);
    uint8_t commandSequence[1];
    if (sleep != YES) {
        commandSequence[0] = 0xAF;
    } else {
        commandSequence[0] = 0xAE;
    }
    lcd_command(commandSequence, 1);
}
void lcd_set_contrast(uint8_t contrast){
    uint8_t commandSequence[2] = {0x81, contrast};
    lcd_command(commandSequence, sizeof(commandSequence));
}
void lcd_putc(char c){
            // mapping char
            c -= ' ';
        for (uint8_t i = 0; i < sizeof(FONT[0]); i++)
        {
            // load bit-pattern from flash
            displayBuffer[cursorPosition.y][cursorPosition.x+i] =pgm_read_byte(&(FONT[(uint8_t)c][i]));
        }
        cursorPosition.x += sizeof(FONT[0]);
}

void lcd_puts(const char* s){
    while (*s) {
        lcd_putc(*s++);
    }
}
void lcd_puts_p(const char* progmem_s){
    register uint8_t c;
    while ((c = pgm_read_byte(progmem_s++))) {
        lcd_putc(c);
    }
}
#ifdef GRAPHICMODE
#pragma mark -
#pragma mark GRAPHIC FUNCTIONS
void lcd_drawPixel(uint8_t x, uint8_t y, uint8_t color){
    if( color == WHITE){
        displayBuffer[(y / (DISPLAY_HEIGHT/8))][x] |= (1 << (y % (DISPLAY_HEIGHT/8)));
    } else {
        displayBuffer[(y / (DISPLAY_HEIGHT/8))][x] &= ~(1 << (y % (DISPLAY_HEIGHT/8)));
    }
}
void lcd_drawLine(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, uint8_t color){
    if(x1 > DISPLAY_WIDTH-1 ||
       x2 > DISPLAY_WIDTH-1 ||
       y1 > DISPLAY_HEIGHT-1 ||
       y2 > DISPLAY_HEIGHT-1) return;
    int dx =  abs(x2-x1), sx = x1<x2 ? 1 : -1;
    int dy = -abs(y2-y1), sy = y1<y2 ? 1 : -1;
    int err = dx+dy, e2; /* error value e_xy */
    
    while(1){
        lcd_drawPixel(x1, y1, color);
        if (x1==x2 && y1==y2) break;
        e2 = 2*err;
        if (e2 > dy) { err += dy; x1 += sx; } /* e_xy+e_x > 0 */
        if (e2 < dx) { err += dx; y1 += sy; } /* e_xy+e_y < 0 */
    }
}
void lcd_drawRect(uint8_t px1, uint8_t py1, uint8_t px2, uint8_t py2, uint8_t color){
    lcd_drawLine(px1, py1, px2, py1, color);
    lcd_drawLine(px2, py1, px2, py2, color);
    lcd_drawLine(px2, py2, px1, py2, color);
    lcd_drawLine(px1, py2, px1, py1, color);
}
void lcd_fillRect(uint8_t px1, uint8_t py1, uint8_t px2, uint8_t py2, uint8_t color){
    for (uint8_t i=0; i<=(py2-py1); i++){
        lcd_drawLine(px1, py1+i, px2, py1+i, color);
    }
}
void lcd_drawCircle(uint8_t center_x, uint8_t center_y, uint8_t radius, uint8_t color){
    if( ((center_x + radius) > DISPLAY_WIDTH-1) ||
       ((center_y + radius) > DISPLAY_HEIGHT-1) ||
       center_x < radius ||
       center_y < radius) return;
    int16_t f = 1 - radius;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * radius;
    int16_t x = 0;
    int16_t y = radius;
    
    lcd_drawPixel(center_x  , center_y+radius, color);
    lcd_drawPixel(center_x  , center_y-radius, color);
    lcd_drawPixel(center_x+radius, center_y  , color);
    lcd_drawPixel(center_x-radius, center_y  , color);
    
    while (x<y) {
        if (f >= 0) {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;
        
        lcd_drawPixel(center_x + x, center_y + y, color);
        lcd_drawPixel(center_x - x, center_y + y, color);
        lcd_drawPixel(center_x + x, center_y - y, color);
        lcd_drawPixel(center_x - x, center_y - y, color);
        lcd_drawPixel(center_x + y, center_y + x, color);
        lcd_drawPixel(center_x - y, center_y + x, color);
        lcd_drawPixel(center_x + y, center_y - x, color);
        lcd_drawPixel(center_x - y, center_y - x, color);
    }
}
void lcd_fillCircle(uint8_t center_x, uint8_t center_y, uint8_t radius, uint8_t color) {
    for(uint8_t i=0; i<= radius;i++){
        lcd_drawCircle(center_x, center_y, i, color);
    }
}

void lcd_fillCircleSimple(uint8_t center_x, uint8_t center_y, int16_t radius, uint8_t color) {
    for (int16_t dx=-radius; dx <= radius; dx++) {
        for (int16_t dy=-radius; dy <= radius; dy++) {
            if (dx * dx + dy * dy < radius * radius) {
                if (center_x + dx >= DISPLAY_WIDTH || center_x + dx < 0
                    || center_y + dy >= DISPLAY_HEIGHT || center_y + dy < 0) continue;
                lcd_drawPixel(center_x + dx, center_y + dy, color);
            }
        }
    }
}
void lcd_fillTriangle(int16_t x1, int8_t y1, int16_t x2, int8_t y2,
                      int16_t x3, int8_t y3, uint8_t color) {
    // Negative and too large coords are allowed, only the visible part will
    // be drawn
    
    // calc bounds for increased performance (todo: is there a better way?)
    int16_t xmin = (x1 < x2 ? (x1 < x3 ? x1 : x3) : (x2 < x3 ? x2 : x3 ));
    if (xmin < 0) xmin=0;
    int16_t xmax = (x1 > x2 ? (x1 > x3 ? x1 : x3) : (x2 > x3 ? x2 : x3 ));
    if (xmax > DISPLAY_WIDTH-1) xmax = DISPLAY_WIDTH - 1;
    
    int8_t ymin = (y1 < y2 ? (y1 < y3 ? y1 : y3) : (y2 < y3 ? y2 : y3 ));
    if (ymin < 0) ymin=0;
    int8_t ymax = (y1 > y2 ? (y1 > y3 ? y1 : y3) : (y2 > y3 ? y2 : y3 ));
    if (ymax > DISPLAY_HEIGHT-1) ymax = DISPLAY_HEIGHT - 1;
    
    for (uint8_t x = xmin; x<=xmax; x++) {
        for (uint8_t y = ymin; y <= ymax; y++) {
            // point in triangle code from John Bananas on stackoverflow
            // https://stackoverflow.com/a/9755252/7089433
            int8_t p1x = x - x1;
            int8_t p1y = y - y1;
            uint8_t s12 = (x2 - x1) * p1y - (y2 - y1) * p1x > 0;
            if (((x3 - x1) * p1y - (y3 - y1) * p1x > 0) == s12) continue;
            if (((x3 - x2) * (y - y2) - (y3 - y2) * (x - x2) > 0) != s12) continue;
            lcd_drawPixel(x, y, color);
        }
    }
}
void lcd_drawBitmap(uint8_t x, uint8_t y, const uint8_t *picture, uint8_t width, uint8_t height, uint8_t color){
    uint8_t i,j, byteWidth = (width+7)/8;
    for (j = 0; j < height; j++) {
        for(i=0; i < width;i++){
            if(pgm_read_byte(picture + j * byteWidth + i / 8) & (128 >> (i & 7))){
                lcd_drawPixel(x+i, y+j, color);
            }
        }
    }
}
void lcd_display() {
#if defined SSD1306
    lcd_gotoxy(0,0);
    lcd_data(&displayBuffer[0][0], DISPLAY_WIDTH*DISPLAY_HEIGHT/8);
#elif defined SH1106
    for (uint8_t i = 0; i < DISPLAY_HEIGHT/8; i++){
        lcd_gotoxy(0,i);
        lcd_data(displayBuffer[i], sizeof(displayBuffer[i]));
    }
#endif
}

void lcd_clear_buffer() {
    for (uint8_t i = 0; i < DISPLAY_HEIGHT/8; i++){
        memset(displayBuffer[i], 0x00, sizeof(displayBuffer[i]));
    }
}

uint8_t lcd_check_buffer(uint8_t x, uint8_t y) {
    return displayBuffer[(y / (DISPLAY_HEIGHT/8))][x] & (1 << (y % (DISPLAY_HEIGHT/8)));
}

void lcd_display_block(uint8_t x, uint8_t line, uint8_t width) {
    if (line > (DISPLAY_HEIGHT/8-1) || x > DISPLAY_WIDTH - 1){return;}
    if (x + width > DISPLAY_WIDTH) { // no -1 here, x alone is width 1
        width = DISPLAY_WIDTH - x;
    }
    lcd_goto_xpix_y(x,line);
    lcd_data(&displayBuffer[line][x], width);
}

#endif

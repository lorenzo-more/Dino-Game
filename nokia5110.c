/* Nokia 5110 LCD AVR Library
 *
 * Copyright (C) 2015 Sergey Denisov.
 * Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com)
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public Licence
 * as published by the Free Software Foundation; either version 3
 * of the Licence, or (at your option) any later version.
 *
 * Original library written by SkewPL, http://skew.tk
 */

#include "nokia5110.h"

#include <avr/pgmspace.h>
#include <avr/io.h>
#include <util/delay.h>
#include <string.h>
#include "nokia5110_chars.h"

static struct
{
    /* screen byte massive */
    uint8_t screen[504];

    /* cursor position */
    uint8_t cursor_x;
    uint8_t cursor_y;

} nokia_lcd = {
    .cursor_x = 0,
    .cursor_y = 0};

/**
 * Sending data to LCD
 * @bytes: data
 * @is_data: transfer mode: 1 - data; 0 - command;
 */
static void write(uint8_t bytes, uint8_t is_data)
{
    register uint8_t i;
    /* Enable controller */
    PORT_LCD &= ~(1 << LCD_SCE);

    /* We are sending data */
    if (is_data)
        PORT_LCD |= (1 << LCD_DC);
    /* We are sending commands */
    else
        PORT_LCD &= ~(1 << LCD_DC);

    /* Send bytes */
    for (i = 0; i < 8; i++)
    {
        /* Set data pin to byte state */
        if ((bytes >> (7 - i)) & 0x01)
            PORT_LCD |= (1 << LCD_DIN);
        else
            PORT_LCD &= ~(1 << LCD_DIN);

        /* Blink clock */
        PORT_LCD |= (1 << LCD_CLK);
        PORT_LCD &= ~(1 << LCD_CLK);
    }

    /* Disable controller */
    PORT_LCD |= (1 << LCD_SCE);
}

static void write_cmd(uint8_t cmd)
{
    write(cmd, 0);
}

static void write_data(uint8_t data)
{
    write(data, 1);
}

/*
 * Public functions
 */

void nokia_lcd_init(void)
{
    register unsigned i;
    /* Set pins as output */
    DDR_LCD |= (1 << LCD_SCE);
    DDR_LCD |= (1 << LCD_RST);
    DDR_LCD |= (1 << LCD_DC);
    DDR_LCD |= (1 << LCD_DIN);
    DDR_LCD |= (1 << LCD_CLK);

    /* Reset display */
    PORT_LCD |= (1 << LCD_RST);
    PORT_LCD |= (1 << LCD_SCE);
    _delay_ms(10);
    PORT_LCD &= ~(1 << LCD_RST);
    _delay_ms(70);
    PORT_LCD |= (1 << LCD_RST);

    /*
     * Initialize display
     */
    /* Enable controller */
    PORT_LCD &= ~(1 << LCD_SCE);
    /* -LCD Extended Commands mode- */
    write_cmd(0x21);
    /* LCD bias mode 1:48 */
    write_cmd(0x13);
    /* Set temperature coefficient */
    write_cmd(0x06);
    /* Default VOP (3.06 + 66 * 0.06 = 7V) */
    write_cmd(0xC2);
    /* Standard Commands mode, powered down */
    write_cmd(0x20);
    /* LCD in normal mode */
    write_cmd(0x09);

    /* Clear LCD RAM */
    write_cmd(0x80);
    write_cmd(LCD_CONTRAST);
    for (i = 0; i < 504; i++)
        write_data(0x00);

    /* Activate LCD */
    write_cmd(0x08);
    write_cmd(0x0C);
}

void nokia_lcd_clear(void)
{
    /* Set column and row to 0 */
    write_cmd(0x80);
    write_cmd(0x40);
    /*Cursor too */
    nokia_lcd.cursor_x = 0;
    nokia_lcd.cursor_y = 0;
    /* Clear everything (504 bytes = 84cols * 48 rows / 8 bits) */
    memset(nokia_lcd.screen, 0, 504);
}

void nokia_lcd_power(uint8_t on)
{
    write_cmd(on ? 0x20 : 0x24);
}

void nokia_lcd_set_pixel(uint8_t x, uint8_t y, uint8_t value)
{
    uint8_t *byte = &nokia_lcd.screen[y / 8 * 84 + x];
    if (value)
        *byte |= (1 << (y % 8));
    else
        *byte &= ~(1 << (y % 8));
}

void nokia_lcd_write_char(char code, uint8_t scale)
{
    register uint8_t x, y;

    if (code >= 0x80)
        return; // 7 bit ASCII only
    const uint8_t *glyph;
    uint8_t pgm_buffer[5];
    if (code >= ' ')
    {
        memcpy_P(pgm_buffer, &CHARSET[code - ' '], sizeof(pgm_buffer));
        glyph = pgm_buffer;
    }
    else
    {
        // Custom glyphs, on the other hand, are stored in RAM...
        if (CUSTOM[(int)code])
        {
            glyph = CUSTOM[(int)code];
        }
        else
        {
            // Default to a space character if unset...
            memcpy_P(pgm_buffer, &CHARSET[0], sizeof(pgm_buffer));
            glyph = pgm_buffer;
        }
    }
    for (x = 0; x < 5 * scale; x++)
        for (y = 0; y < 7 * scale; y++)
            // if (pgm_read_byte(&CHARSET[code-32][x/scale]) & (1 << y/scale))
            if (glyph[x / scale] & (1 << y / scale))
                nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 1);
            else
                nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 0);

    nokia_lcd.cursor_x += 5 * scale + 1;
    if (nokia_lcd.cursor_x >= 84)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y += 7 * scale + 1;
    }
    if (nokia_lcd.cursor_y >= 48)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y = 0;
    }
}

void nokia_lcd_write_char_opposite(char code, uint8_t scale)
{
    register uint8_t x, y;

    if (code >= 0x80)
        return; // 7 bit ASCII only
    const uint8_t *glyph;
    uint8_t pgm_buffer[5];
    if (code >= ' ')
    {
        memcpy_P(pgm_buffer, &CHARSET[code - ' '], sizeof(pgm_buffer));
        glyph = pgm_buffer;
    }
    else
    {
        // Custom glyphs, on the other hand, are stored in RAM...
        if (CUSTOM[(int)code])
        {
            glyph = CUSTOM[(int)code];
        }
        else
        {
            // Default to a space character if unset...
            memcpy_P(pgm_buffer, &CHARSET[0], sizeof(pgm_buffer));
            glyph = pgm_buffer;
        }
    }
    for (x = 0; x < 7 * scale; x++)
        for (y = 0; y < 5 * scale; y++)
            // if (pgm_read_byte(&CHARSET[code-32][x/scale]) & (1 << y/scale))
            if (glyph[4 - y / scale] & (1 << x / scale))
                nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 1);
            else
                nokia_lcd_set_pixel(nokia_lcd.cursor_x + x, nokia_lcd.cursor_y + y, 0);

    nokia_lcd.cursor_x += 5 * scale + 1;
    if (nokia_lcd.cursor_x >= 84)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y += 7 * scale + 1;
    }
    if (nokia_lcd.cursor_y >= 48)
    {
        nokia_lcd.cursor_x = 0;
        nokia_lcd.cursor_y = 0;
    }
}

void nokia_lcd_custom(char code, uint8_t *glyph)
{
    // Check if valid (ASCII 0 to 31)
    if (code >= ' ')
        return;
    CUSTOM[(int)code] = glyph;
}

void nokia_lcd_write_string(const char *str, uint8_t scale, int opposite)
{
    if (opposite)
    {
        while (*str)
            nokia_lcd_write_char_opposite(*str++, scale);
    }
    else
    {
        while (*str)
            nokia_lcd_write_char(*str++, scale);
    }
}

void nokia_lcd_set_cursor(uint8_t x, uint8_t y)
{
    nokia_lcd.cursor_x = x;
    nokia_lcd.cursor_y = y;
}

void nokia_lcd_render(void)
{
    register unsigned i;
    /* Set column and row to 0 */
    write_cmd(0x80);
    write_cmd(0x40);

    /* Write screen to display */
    for (i = 0; i < 504; i++)
        write_data(nokia_lcd.screen[i]);
}

// Algoritmo DDA
// https://en.wikipedia.org/wiki/Digital_differential_analyzer_(graphics_algorithm)
// Bem ineficiente, mas funciona :-)
void nokia_lcd_drawline(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    float x, y;
    float dx, dy, step;
    dx = (x2 - x1);
    dy = (y2 - y1);
    if (fabs(dx) >= fabs(dy))
        step = fabs(dx);
    else
        step = fabs(dy);
    dx = dx / step;
    dy = dy / step;
    x = x1;
    y = y1;
    int i = 1;
    while (i <= step)
    {
        nokia_lcd_set_pixel(x, y, 1);
        x = x + dx;
        y = y + dy;
        i = i + 1;
    }
}

// Desenha um retângulo (4 linhas)
void nokia_lcd_drawrect(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2)
{
    nokia_lcd_drawline(x1, y1, x2, y1);
    nokia_lcd_drawline(x2, y1, x2, y2);
    nokia_lcd_drawline(x2, y2, x1, y2);
    nokia_lcd_drawline(x1, y2, x1, y1);
}

// Desenha um círculo
// Algoritmo do ponto médio
// https://www.geeksforgeeks.org/mid-point-circle-drawing-algorithm/
void nokia_lcd_drawcircle(uint8_t x0, uint8_t y0, uint8_t r)
{
    int x = r;
    int y = 0;
    int err = 0;

    while (x >= y)
    {
        nokia_lcd_set_pixel(x0 + x, y0 + y, 1);
        nokia_lcd_set_pixel(x0 + y, y0 + x, 1);
        nokia_lcd_set_pixel(x0 - y, y0 + x, 1);
        nokia_lcd_set_pixel(x0 - x, y0 + y, 1);
        nokia_lcd_set_pixel(x0 - x, y0 - y, 1);
        nokia_lcd_set_pixel(x0 - y, y0 - x, 1);
        nokia_lcd_set_pixel(x0 + y, y0 - x, 1);
        nokia_lcd_set_pixel(x0 + x, y0 - y, 1);

        if (err <= 0)
        {
            y += 1;
            err += 2 * y + 1;
        }
        if (err > 0)
        {
            x -= 1;
            err -= 2 * x + 1;
        }
    }
}

#pragma once
#include "graphic_types.h"

#define CHARACTER_SPACING_PIXELS 2 //number of pixels between characters

void gfx_init(void);
void gfx_place_bitmap(const bitmap_t *bitmap, uint16_t pos_x, uint16_t pos_y);
void gfx_write_text(const font_t *font, uint16_t pos_x, uint16_t pos_y, const char *text);
uint16_t gfx_get_text_width(const font_t *font, const char *text);
uint8_t gfx_get_glyph_width(const font_t *font, char c);
uint8_t gfx_get_glyph_height(const font_t *font);
const glyph_t * gfx_get_glyph(const font_t *font, char c);

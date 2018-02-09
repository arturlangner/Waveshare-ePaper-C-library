#include "disp.h"
#include "graphics.h"
#include "resources.h"
#include <stdio.h>
#include <string.h> //for memset

void gfx_init(void){
    disp_init();
    memset(GLOBAL_framebuffer, 0xFF, sizeof(GLOBAL_framebuffer)); //set background to white
}

void gfx_write_text(const font_t *font, uint16_t pos_x, uint16_t pos_y, const char *text){
    pos_x /= 8; //argument is in pixels, internally this function operates on framebuffer bytes

    const bitmap_t *bitmap = font->bitmap;

    while (*text){
        const glyph_t *glyph = gfx_get_glyph(font, *text);
        if (glyph){

            //move the pointer to the appropriate glyph within the bitmap
            const uint8_t *payload = bitmap->payload + (bitmap->size_x/8)*glyph->x_offset;

            for (uint32_t y = pos_y; (y < pos_y+glyph->width) && (y < DISPLAY_HEIGHT); y++){
                for (uint32_t x = pos_x; x < (bitmap->size_x/8) + pos_x; x++){
                    GLOBAL_framebuffer[y][x] = *payload;
                    payload++;
                }
            }
            pos_y += glyph->width;
            pos_y += CHARACTER_SPACING_PIXELS; //this is inter letter spacing
        } if (*text == ' '){ //else glyph not found - check if it is space
            pos_y += font->bitmap->size_x/4; //space is quater of font height
        } //else skip the character
        text++;
    }
}

const glyph_t * gfx_get_glyph(const font_t *font, char c){
    //TODO: rewrite it to use binary search
    for (uint32_t i = 0; i < font->glyph_count; i++){
            if (font->glyphs[i].character == c){
                return &(font->glyphs[i]);
            }
    }
    return NULL;
}

uint8_t gfx_get_glyph_width(const font_t *font, char c){
    const glyph_t *g = gfx_get_glyph(font, c); //can be NULL
    if (g){
        return g->width;
    }
    return 0;
}

uint8_t gfx_get_glyph_height(const font_t *font){
    return font->bitmap->size_x;
}

uint16_t gfx_get_text_width(const font_t *font, const char *text){
    uint32_t width = 0;
    while (*text){
        width += gfx_get_glyph_width(font, *text);
        width += CHARACTER_SPACING_PIXELS;
        text++;
    }
    return width;
}

void gfx_place_bitmap(const bitmap_t *bitmap, uint16_t pos_x, uint16_t pos_y){
    const uint8_t *payload = bitmap->payload;
    for (uint32_t y = pos_y; y < bitmap->size_y+pos_y; y++){
        for (uint32_t x = pos_x; x < (bitmap->size_x+pos_x)/8; x++){
            GLOBAL_framebuffer[y][x] = *payload;
            payload++;
        }
    }
}

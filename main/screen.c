#include "disp.h"
#include "graphics.h"
#include "resources.h"
#include "screen.h"

void _format_temperature(int8_t temperature, char *target_string){
    if (temperature == INT8_MAX || temperature == INT8_MIN){
        *target_string = '\0';
        return;
    }

    if (temperature < 0){
        *target_string = '-';
        target_string++;
        temperature = -temperature;
    }
    *target_string = (temperature / 10) + '0'/*convert digit to ASCII*/;
    if (*target_string != '0'){ //don't add a trailing zero if the number is below 10
        target_string++;
    }
    *target_string = (temperature % 10) + '0';
    target_string++;
    *target_string = '\0';
}

void screen_show_temperatures(int8_t temperature1, int8_t temperature2){

    const font_t *FONT_LARGE = &FONT_generalemono_a_65;
    const font_t *FONT_REGULAR = &FONT_generalemono_a_17;

    char temperature_text1[4]; //2 digits + optional minus sign + null terminator
    char temperature_text2[4];

    _format_temperature(temperature1, temperature_text1);
    _format_temperature(temperature2, temperature_text2);

    const uint32_t temperature_text_height = gfx_get_glyph_height(FONT_LARGE);
    const uint32_t temperature_text1_width = gfx_get_text_width(FONT_LARGE, temperature_text1);
    const uint32_t temperature_text2_width = gfx_get_text_width(FONT_LARGE, temperature_text2);

    const uint32_t temperature_text_pos_x = DISPLAY_WIDTH - temperature_text_height;
    const uint32_t temperature_text_1_pos_y = 0;
    const uint32_t temperature_text_2_pos_y = DISPLAY_HEIGHT - gfx_get_text_width(FONT_LARGE, temperature_text2);

    gfx_write_text(FONT_LARGE, temperature_text_pos_x, temperature_text_1_pos_y, temperature_text1);
    gfx_write_text(FONT_LARGE, temperature_text_pos_x, temperature_text_2_pos_y, temperature_text2);

    gfx_write_text(FONT_REGULAR, 0, 0, "INSIDE");

    const uint32_t outside_label_pos_y = DISPLAY_HEIGHT - gfx_get_text_width(FONT_REGULAR, "OUTSIDE");
    gfx_write_text(FONT_REGULAR, 0, outside_label_pos_y, "OUTSIDE");

    const char units_string[] = { 176/*degree sign*/, 'C', '\0' };

    const uint32_t units_string_width = gfx_get_text_width(FONT_REGULAR, units_string);
    const uint32_t units_string_pos_x = DISPLAY_WIDTH - gfx_get_glyph_height(FONT_REGULAR);

    uint32_t space_between_temperature_texts = DISPLAY_HEIGHT - temperature_text1_width - temperature_text2_width;
    if (temperature2 < 0){
        space_between_temperature_texts += gfx_get_glyph_width(FONT_REGULAR, '-');
    }

    const uint32_t units_label_pos_y = temperature_text1_width + space_between_temperature_texts/2 - units_string_width/2;

    gfx_write_text(FONT_REGULAR, units_string_pos_x, units_label_pos_y, units_string);

    disp_update();
}

void screen_demo_show_fonts(void){
    gfx_write_text(&FONT_generalemono_a_17, 0, 0, "GENERALE MONO");
    gfx_write_text(&FONT_roboto_condensed_regular_14, 32, 0, "Roboto Condensed Regular 14");

    gfx_write_text(&FONT_roboto_condensed_regular_18, 96, 0, "Roboto Condensed");
    gfx_write_text(&FONT_roboto_condensed_regular_18, 64, 64, "Regular 18");

    disp_update();
}

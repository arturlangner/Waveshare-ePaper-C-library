#pragma once
#include <stdint.h>

#define DISPLAY_WIDTH  128
#define DISPLAY_HEIGHT 296

extern uint8_t GLOBAL_framebuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH / 8];
//              this is Y (longer edge) --- ^^          ^^ --- this is X (shorter edge)

void disp_init(void);
void disp_clear(void);

void disp_update(void); //pushes framebuffer to the display

void disp_deinit(void);


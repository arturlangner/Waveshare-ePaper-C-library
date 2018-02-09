#include "disp.h"
#include "disp_hw.h"
#include <string.h>

#define WRITE_DATA true
#define WRITE_COMMAND false

uint8_t GLOBAL_framebuffer[DISPLAY_HEIGHT][DISPLAY_WIDTH / 8];

typedef enum {
    cmd_DRIVER_OUTPUT_CONTROL                = 0x01,
    cmd_BOOSTER_SOFT_START_CONTROL           = 0x0C,
    cmd_GATE_SCAN_START_POSITION             = 0x0F,
    cmd_DEEP_SLEEP_MODE                      = 0x10,
    cmd_DATA_ENTRY_MODE_SETTING              = 0x11,
    cmd_SW_RESET                             = 0x12,
    cmd_TEMPERATURE_SENSOR_CONTROL           = 0x1A,
    cmd_MASTER_ACTIVATION                    = 0x20,
    cmd_DISPLAY_UPDATE_CONTROL_1             = 0x21,
    cmd_DISPLAY_UPDATE_CONTROL_2             = 0x22,
    cmd_WRITE_RAM                            = 0x24,
    cmd_WRITE_VCOM_REGISTER                  = 0x2C,
    cmd_WRITE_LUT_REGISTER                   = 0x32,
    cmd_SET_DUMMY_LINE_PERIOD                = 0x3A,
    cmd_SET_GATE_TIME                        = 0x3B,
    cmd_BORDER_WAVEFORM_CONTROL              = 0x3C,
    cmd_SET_RAM_X_ADDRESS_START_END_POSITION = 0x44,
    cmd_SET_RAM_Y_ADDRESS_START_END_POSITION = 0x45,
    cmd_SET_RAM_X_ADDRESS_COUNTER            = 0x4E,
    cmd_SET_RAM_Y_ADDRESS_COUNTER            = 0x4F,
    cmd_TERMINATE_FRAME_READ_WRITE           = 0xFF,
} disp_cmd_t;


typedef struct {
    uint8_t length; //length of data minus the command byte
    uint8_t data[]; //first byte is always the command byte
} static_transfer_t;

typedef struct {
    uint8_t length; //length of data minus the command byte
    uint8_t *data; //first byte is always the command byte
} transfer_t;

static const static_transfer_t TRANSFER_LUT_FULL_UPDATE = {
    .length = 30,
    { cmd_WRITE_LUT_REGISTER,
    0x02, 0x02, 0x01, 0x11, 0x12, 0x12, 0x22, 0x22,
    0x66, 0x69, 0x69, 0x59, 0x58, 0x99, 0x99, 0x88,
    0x00, 0x00, 0x00, 0x00, 0xF8, 0xB4, 0x13, 0x51,
    0x35, 0x51, 0x51, 0x19, 0x01, 0x00 }
};

static const static_transfer_t INIT_01 = { .length = 3, {cmd_DRIVER_OUTPUT_CONTROL, (DISPLAY_HEIGHT - 1) & 0xFF, ((DISPLAY_HEIGHT - 1) >> 8) & 0xFF, 0 /*GD = 0; SM = 0; TB = 0*/} };
static const static_transfer_t INIT_02 = { .length = 3, {cmd_BOOSTER_SOFT_START_CONTROL, 0xD7, 0xD6, 0x9D} }; //magic values from datasheet
static const static_transfer_t INIT_03 = { .length = 1, {cmd_WRITE_VCOM_REGISTER, 0xA8} }; //magic value
static const static_transfer_t INIT_04 = { .length = 1, {cmd_SET_DUMMY_LINE_PERIOD, 0x1A} }; //magic value: 4 dummy lines per gate
static const static_transfer_t INIT_05 = { .length = 1, {cmd_SET_GATE_TIME, 0x08} }; //magic value: 2us per line
static const static_transfer_t INIT_06 = { .length = 1, {cmd_DATA_ENTRY_MODE_SETTING, 0x03} }; //magic value: X increment, Y increment

static const static_transfer_t *const INIT_SEQUENCE2[] = {
    &INIT_01, &INIT_02, &INIT_03, &INIT_04, &INIT_05, &INIT_06, &TRANSFER_LUT_FULL_UPDATE
};

static void disp_write_const_transfer(const static_transfer_t* transfer);
static void disp_write_transfer(const uint8_t *data/*first byte is command*/, uint32_t length/*does not contain command byte*/);
static void disp_write_generic(const uint8_t *data, uint32_t length, bool write_is_data/*not command*/);
static void disp_write_generic_nocs(const uint8_t *data, uint32_t length, bool write_is_data/*not command*/);
static void disp_trigger_update(void);
static void disp_set_target_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end);
static void disp_set_pointer(uint16_t pos_x, uint16_t pos_y);

void disp_init(void){
    disp_hw_init();
    for (uint32_t i = 0; i < sizeof(INIT_SEQUENCE2)/sizeof(INIT_SEQUENCE2[0]); i++){
        disp_write_const_transfer(INIT_SEQUENCE2[i]);
    }
}

static void disp_write_const_transfer(const static_transfer_t* transfer){
    disp_write_transfer(transfer->data, transfer->length);
}

static void disp_write_transfer(const uint8_t *data/*first byte is command*/, uint32_t length/*does not contain command byte*/){
    disp_hw_busy_wait();
    disp_hw_cs_low();
    disp_write_generic_nocs(data, 1, WRITE_COMMAND);
    disp_write_generic_nocs(data+1, length, WRITE_DATA);
    disp_hw_cs_high();
}

static void disp_set_target_area(uint16_t x_start, uint16_t y_start, uint16_t x_end, uint16_t y_end) {

    disp_write_transfer((uint8_t[]){ cmd_SET_RAM_X_ADDRESS_START_END_POSITION,
        (0/*x_start*/ >> 3) & 0xFF, (/*x_end*/(DISPLAY_WIDTH-1) >> 3) & 0xFF}, 2);
    disp_write_transfer((uint8_t[]){
        cmd_SET_RAM_Y_ADDRESS_START_END_POSITION, 
        0/*y_start*/ & 0xFF,
        (0/*y_start*/ >> 8) & 0xFF,
        (DISPLAY_HEIGHT-1)/*y_end*/ & 0xFF,
        ((DISPLAY_HEIGHT-1)/*y_end*/ >> 8) & 0xFF},
        4);
}

static void disp_set_pointer(uint16_t pos_x, uint16_t pos_y){
    disp_write_transfer((uint8_t[]){ cmd_SET_RAM_X_ADDRESS_COUNTER, (pos_x >> 3) & 0xFF }, 1);
    disp_write_transfer((uint8_t[]){ cmd_SET_RAM_Y_ADDRESS_COUNTER, pos_y & 0xFF, (pos_y >> 8) & 0xFF }, 2);
}

void disp_clear(void){
    memset(GLOBAL_framebuffer, 0xFF, sizeof(GLOBAL_framebuffer));

    disp_set_target_area(0/*x_start*/, 0/*y_start*/, DISPLAY_WIDTH-1/*x_end*/, DISPLAY_HEIGHT-1/*y_end*/);

    const uint8_t buffer[] = {
        cmd_WRITE_RAM
    };

    disp_write_generic(&buffer[0], 1, WRITE_COMMAND);
    disp_write_generic((uint8_t*)GLOBAL_framebuffer, sizeof(GLOBAL_framebuffer), WRITE_DATA);
    disp_write_transfer((uint8_t[]){ cmd_TERMINATE_FRAME_READ_WRITE }, 0);

    disp_trigger_update();
}

void disp_update(void){ //pushes framebuffer to the display
    disp_set_target_area(0/*x_start*/, 0/*y_start*/, DISPLAY_WIDTH-1/*x_end*/, DISPLAY_HEIGHT-1/*y_end*/);
    disp_set_pointer(0, 0);

    disp_write_generic((uint8_t[]){ cmd_WRITE_RAM }, 1, WRITE_COMMAND);
    disp_write_generic((uint8_t*)GLOBAL_framebuffer, sizeof(GLOBAL_framebuffer), WRITE_DATA);
    disp_write_transfer((uint8_t[]){ cmd_TERMINATE_FRAME_READ_WRITE }, 0);

    disp_trigger_update();
}

void disp_deinit(void){
    disp_hw_deinit();
}

static void disp_write_generic(const uint8_t *data, uint32_t length, bool write_is_data/*not command*/){
    disp_hw_set_write_mode(write_is_data);

    disp_hw_cs_low();
    disp_hw_spi_write_blocking(data, length);
    disp_hw_cs_high();
}

static void disp_write_generic_nocs(const uint8_t *data, uint32_t length, bool write_is_data/*not command*/){
    disp_hw_set_write_mode(write_is_data);
    disp_hw_spi_write_blocking(data, length);
}

/**
*  @brief: update the display
*          there are 2 memory areas embedded in the e-paper display
*          but once this function is called,
*          the the next action of SetFrameMemory or ClearFrame will 
*          set the other memory area.
*/
static void disp_trigger_update(void){
    disp_write_transfer((uint8_t[]){ cmd_DISPLAY_UPDATE_CONTROL_2, 0xC4/*magic?*/ }, 1);
    disp_write_transfer((uint8_t[]){ cmd_MASTER_ACTIVATION }, 0);
}

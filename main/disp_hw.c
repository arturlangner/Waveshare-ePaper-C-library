#include "delay.h"
#include "disp_hw.h"
#include "i2c.h"
#include "spi.h"

/* THIS CODE IS FOR REFERENCE ONLY! IT WILL NOT BUILD IN YOUR PROJECT!
 * You have to customize it for your MCU and PCB.
 */

#error "You have to customize this module for your MCU and PCB!"

void disp_hw_init(void){
    spi2_init();
    i2c_expander_set_output(EXPANDER_PIN_DISP_RST, false); //reset is active low
    delay_ms(200);
    i2c_expander_set_output(EXPANDER_PIN_DISP_RST, true); //release display out of reset
    delay_ms(200);
}

void disp_hw_spi_write_blocking(const uint8_t *data, uint32_t length){
    spi2_write(data, length);
}

void disp_hw_cs_low(void){
    spi2_cs1_set(CS_LOW);
}

void disp_hw_cs_high(void){
    spi2_cs1_set(CS_HIGH);
}

void disp_hw_busy_wait(void){
    while (i2c_expander_get_input(EXPANDER_PIN_DISP_BUSY)){
        //TODO: add sleep mode and edge sensing interrupt
    }
}

void disp_hw_set_write_mode(bool write_is_data){
    i2c_expander_set_output(EXPANDER_PIN_DISP_D_C, write_is_data); /*high=data, low=command*/
}

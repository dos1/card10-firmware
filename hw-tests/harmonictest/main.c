/*******************************************************************************
 * License: TBD
 ******************************************************************************/

/***** Includes *****/
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "mxc_config.h"
#include "led.h"
#include "board.h"
#include "tmr_utils.h"
#include "i2c.h"
#include "rtc.h"
#include "spi.h"
#include "pb.h"
#include "MAX30003.h"
#include "max86150.h"
#include "GUI_DEV/GUI_Paint.h"
#include "pmic.h"
#include "card10.h"
#include <stdbool.h>
#include <Heart.h>
#include "leds.h"
#include "adc.h"
#include "MAX77650-Arduino-Library.h"

/***** Definitions *****/

/***** Globals *****/
/***** Functions *****/
int light_sensor_init()
{
	const sys_cfg_adc_t sys_adc_cfg =
		NULL; /* No system specific configuration needed. */
	if (ADC_Init(0x9, &sys_adc_cfg) != E_NO_ERROR) {
		return -1;
	}
	GPIO_Config(&gpio_cfg_adc7);
	return 0;
}

uint16_t read_light_sensor()
{
    uint16_t v;
	ADC_StartConvert(ADC_CH_7, 0, 0);
	ADC_GetData(&v);
    return v;
}


// *****************************************************************************
int main(void)
{

    card10_init();
    card10_diag();
    light_sensor_init();


    max86150_begin();
    max86150_setup(0x1F, 4, 3, 400, 411, 4096);

    Paint_DrawImage(Heart, 0, 0, 160, 80);
    LCD_Update();

    for(int i=0; i<11; i++) {
        leds_set_dim(i, 1);
    }

    for(int i=11; i<15; i++) {
        leds_set(i, 255, 0, 0);
    }

    int count = 0;

    while(1) {
        if(max86150_check()>0) {
            while(max86150_available()) {
                max86150_getFIFORed();
                max86150_getFIFOIR();
                max86150_getFIFOECG();
                max86150_nextSample();
            }
        }

        for(int i=1; i<=4; i++) {
            int leds[] = {0, 11, 14, 12, 13};
            if(PB_Get(i)) {
                printf("%d\n", i);
                leds_set(leds[i], 0, 255, 0);
            } else {
                leds_set(leds[i], 255, 0, 0);
            }
        }
        leds_update();

        if(count++==100) {
            count = 0;
            uint16_t s = read_light_sensor();
            for(int i=0; i<11; i++) {
                if(i < s/3) {
                    leds_set(i, 64, 0, 0);
                } else {
                    leds_set(i, 0, 64, 0);
                }
            }
        }

        TMR_Delay(MXC_TMR0, MSEC(10), 0);
    }
}

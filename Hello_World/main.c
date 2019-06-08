/*******************************************************************************
 * License: TBD
 ******************************************************************************/

/***** Includes *****/
#include "pmic.h"
#include "leds.h"
#include "card10.h"

#include "GUI_Paint.h"

#include "tmr_utils.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <Heart.h>

/***** Definitions *****/

/***** Globals *****/
static const gpio_cfg_t motor_pin = {PORT_0, PIN_8, GPIO_FUNC_OUT, GPIO_PAD_NONE};

void Core1_Start(void) {
    //MXC_GCR->gp0 = (uint32_t)(&__isr_vector_core1);
    MXC_GCR->gp0 = 0x10040000;
    MXC_GCR->perckcn1 &= ~MXC_F_GCR_PERCKCN1_CPU1;
}

void Core1_Stop(void) {
    MXC_GCR->perckcn1 |= MXC_F_GCR_PERCKCN1_CPU1;
}

int main(void)
{
    int count = 0;

    card10_init();
    //card10_diag();

    GPIO_Config(&motor_pin);

    Paint_DrawImage(Heart, 0, 0, 160, 80);
    LCD_Update();

    for(int i=0; i<11; i++) {
        leds_set_dim(i, 1);
    }

    int h = 0;
    Core1_Start();

    while (1) {

#if 1
        #define NUM     15
        for(int i=0; i<NUM; i++) {
            if(i < 12) {
                leds_set_hsv(i, (h + 360/NUM * i) % 360, 1., 1./8);
            } else {
                leds_set_hsv(i, (h + 360/NUM * i) % 360, 1., 1.);
            }
        }
        //leds_set_hsv(0, h, 1., 1.);
        //leds_set_hsv(1, (h + 90) % 360, 1., 1./2);
        //leds_set_hsv(2, (h + 180) % 360, 1., 1./2);
        //leds_set_hsv(3, (h + 270) % 360, 1., 1./2);

        leds_update();
        TMR_Delay(MXC_TMR0, MSEC(10), 0);
        h++;
#endif
#if 0
        pmic_set_led(0, 31);
        pmic_set_led(1, 0);
        pmic_set_led(2, 0);
        TMR_Delay(MXC_TMR0, MSEC(200), 0);

        pmic_set_led(0, 0);
        pmic_set_led(1, 31);
        pmic_set_led(2, 0);
        TMR_Delay(MXC_TMR0, MSEC(200), 0);

        pmic_set_led(0, 0);
        pmic_set_led(1, 0);
        pmic_set_led(2, 31);
        TMR_Delay(MXC_TMR0, MSEC(200), 0);

        pmic_set_led(0, 0);
        pmic_set_led(1, 0);
        pmic_set_led(2, 0);
        //TMR_Delay(MXC_TMR0, MSEC(200), 0);
#endif
#if 0
        //TMR_Delay(MXC_TMR0, MSEC(600), 0);
        GPIO_OutSet(&motor_pin);
        TMR_Delay(MXC_TMR0, MSEC(30), 0);
        GPIO_OutClr(&motor_pin);
        TMR_Delay(MXC_TMR0, MSEC(200), 0);

#endif
#if 0
        LED_Off(0);
        TMR_Delay(MXC_TMR0, MSEC(500), 0);
        LED_On(0);
        TMR_Delay(MXC_TMR0, MSEC(500), 0);
#endif
#if 0
        TMR_Delay(MXC_TMR0, MSEC(1000), 0);
        printf("count = %d\n", count++);
#endif
    }
}

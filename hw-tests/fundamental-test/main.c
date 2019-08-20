/*******************************************************************************
 * License: TBD
 ******************************************************************************/

/***** Includes *****/
#include "bhy_uc_driver.h"
#include "board.h"
#include "display.h"
#include "gfx.h"
#include "gpio.h"
#include "i2c.h"
#include "led.h"
#include "mxc_config.h"
#include "pmic.h"
#include "rtc.h"
#include "spi.h"
#include "tmr_utils.h"
#include "adc.h"
#include "flc.h"

#include "gfx.h"
#include "display.h"

#include "card10.h"
#include "MAX77650-Arduino-Library.h"

#include <math.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define M_PI 3.1415
/***** Definitions *****/

/* should be greater or equal to 69 bytes, page size (50) + maximum packet
 * size(18) + 1 */
#define FIFO_SIZE 300
#define ROTATION_VECTOR_SAMPLE_RATE 10
#define MAX_PACKET_LENGTH 18
#define OUT_BUFFER_SIZE 60

/***** Globals *****/
char out_buffer[OUT_BUFFER_SIZE] =
	" W: 0.999  X: 0.999  Y: 0.999  Z: 0.999   \r";
uint8_t fifo[FIFO_SIZE];

bool pressed = false;

static __attribute__((unused))
const gpio_cfg_t motor_pin = { PORT_0, PIN_8, GPIO_FUNC_OUT, GPIO_PAD_NONE };


#define PARTITION_START (0x10000000 + 64 * 1024)
#define PARTITION_END (0x10000000 + 1024 * 1024 - 1)

/***** Functions *****/
static void sensors_callback_orientation(
	bhy_data_generic_t *sensor_data, bhy_virtual_sensor_t sensor_id
) {
	printf("azimuth=%05d, pitch=%05d, roll=%05d status=%d\n",
	       sensor_data->data_vector.x * 360 / 32768,
	       sensor_data->data_vector.y * 360 / 32768,
	       sensor_data->data_vector.z * 360 / 32768,
	       sensor_data->data_vector.status);

    Color green = gfx_color(&display_screen, GREEN);
    Color black = gfx_color(&display_screen, BLACK);

    char buf[128];
    if(pressed) {
        sprintf(buf, "PASS  \n");
        printf(buf);
        gfx_puts(&Font24, &display_screen, 0, 0, buf, green, black);
        gfx_update(&display_screen);

        int ret = FLC_MultiPageErase(PARTITION_START, PARTITION_END);
        if (ret != E_NO_ERROR) {
            printf("FLC_MultiPageErase failed with %d\n", ret);
            while (1)
                ;
        }
        while(1);
    }

    static int i = 0, s = 0;
    if(i++ == 1) {
        i = 0;
        s++;
        switch(s) {
            case 1:
                pmic_set_led(0, 31);
            break;
            case 2:
                pmic_set_led(0, 0);
            break;
            case 3:
                pmic_set_led(1, 31);
            break;
            case 4:
                pmic_set_led(1, 0);
            break;
            case 5:
                pmic_set_led(2, 31);
            break;
            case 6:
                pmic_set_led(2, 0);
            case 7:
                GPIO_OutSet(&motor_pin);
            break;
            case 8:
                GPIO_OutClr(&motor_pin);
                s = 0;
            break;
        }
    }
#if 0
    uint16_t adc_val[4];
    unsigned int overflow[4];
    ADC_StartConvert(ADC_CH_0, 0, 0);
    overflow[0] = (ADC_GetData(&adc_val[0]) == E_OVERFLOW ? 1 : 0);

    //char buf[128];
    //sprintf(buf, "%.2f,%d,%d,%d,%d,%02x", 3.68 * adc_val[0]/1023. * 1.22l, MAX77650_getCHG(), a, b, c, erc);
    sprintf(buf, "%d", (int)(3.68 * adc_val[0]/1023. * 1.22l * 1000)); // BATTERY FACTOR
    //sprintf(buf, "%d", (int)(3.84 * adc_val[0]/1023. * 1.22l * 1000)); // ? FACTOR
    //Paint_DrawString_EN(0, 0, buf, &Font16, 0x0000, 0xffff);
    //LCD_Update();
#endif
#if 0
    MAX77650_setMUX_SEL(0b11);
    TMR_Delay(MXC_TMR0, MSEC(5), 0);
    uint16_t adc_val;
    unsigned int overflow;
    ADC_StartConvert(ADC_CH_0, 0, 0);
    overflow = (ADC_GetData(&adc_val) == E_OVERFLOW ? 1 : 0);

    bool a = MAX77650_getThermalAlarm1();
    bool b = MAX77650_getThermalAlarm2();
    bool c = MAX77650_getTJ_REG_STAT();
    uint8_t erc = MAX77650_getERCFLAG();

    if(overflow)
        printf("overflow!\n");
    //char buf[128];
    //sprintf(buf, "%.2f,%d,%d,%d,%d,%02x", 3.68 * adc_val[0]/1023. * 1.22l, MAX77650_getCHG(), a, b, c, erc);
    sprintf(buf, "%d,%d,%d,%d,%d,%02x", (int)(3.68 * adc_val/1023. * 1.22l * 1000), MAX77650_getCHG(), a, b, c, erc);


    gfx_puts(&Font12, &display_screen, 32, 0, buf, white, black);
    gfx_update(&display_screen);

    printf(buf); printf("\n");
    MAX77650_setMUX_SEL(0b0);
#endif
}


static void pmic_button(bool falling)
{
	if (falling) {
		pressed = true;
	}
}

// *****************************************************************************
int main(void)
{
	/* BHY Variable*/
	uint8_t *fifoptr           = NULL;
	uint8_t bytes_left_in_fifo = 0;
	uint16_t bytes_remaining   = 0;
	uint16_t bytes_read        = 0;
	bhy_data_generic_t fifo_packet;
	bhy_data_type_t packet_type;
	BHY_RETURN_FUNCTION_TYPE result;
    char buf[128];


	card10_init();
    pmic_set_led(0, 31);

	card10_diag();
	pmic_set_button_callback(pmic_button);

    Color white = gfx_color(&display_screen, WHITE);
    Color black = gfx_color(&display_screen, BLACK);

    uint8_t check_for[] = {0x14, 0x21, 0x28, 0x48, 0x5e, 0x76};

    for(int i=0; i<sizeof(check_for); i++) {
        uint8_t addr = check_for[i];
        uint8_t dummy[1] = {0};

        int res = I2C_MasterWrite(MXC_I2C1_BUS0, addr << 1, dummy, 1, 0);
        if (res != 1) {
            sprintf(buf, "Did not find 0x%02x on I2C1\n", addr);
            printf(buf);
            gfx_puts(&Font16, &display_screen, 0, 0, buf, white, black);
            gfx_update(&display_screen);
            while(1);
        }
    }

    Color yellow = gfx_color(&display_screen, YELLOW);

    sprintf(buf, "<-Press\n");
    printf(buf);
    gfx_puts(&Font24, &display_screen, 0, 0, buf, yellow, black);
    gfx_update(&display_screen);

	MAX77650_setCHG_CV(0b11000); // 4.2 V target battery voltage
	MAX77650_setICHGIN_LIM(1);   // 190 mA limit on USB
	MAX77650_setCHG_CC(0b1011);  // 90 mA fast charge current
	MAX77650_setCHG_EN(1);       // Turn on charger



    MAX77650_setMUX_SEL(0b11);
    //MAX77650_setMUX_SEL(0b1010);

    const sys_cfg_adc_t sys_adc_cfg = NULL; /* No system specific configuration needed. */
    if (ADC_Init(0x9, &sys_adc_cfg) != E_NO_ERROR) {
        printf("Error Bad Parameter\n");
        while (1);
    }

    GPIO_Config(&gpio_cfg_adc0);

	if (bhy_install_sensor_callback(
		    VS_TYPE_ORIENTATION,
		    VS_WAKEUP,
		    sensors_callback_orientation)) {
		printf("Fail to install sensor callback\n");
	}

	if (bhy_enable_virtual_sensor(
		    VS_TYPE_ORIENTATION,
		    VS_WAKEUP,
		    ROTATION_VECTOR_SAMPLE_RATE,
		    0,
		    VS_FLUSH_NONE,
		    0,
		    0)) {
		printf("Fail to enable sensor id=%d\n",
		       VS_TYPE_GEOMAGNETIC_FIELD);
	}

	while (1) {
		/* wait until the interrupt fires */
		/* unless we already know there are bytes remaining in the fifo */
		while (!GPIO_InGet(&bhi_interrupt_pin) && !bytes_remaining) {
            pmic_poll();

		}

		bhy_read_fifo(
			fifo + bytes_left_in_fifo,
			FIFO_SIZE - bytes_left_in_fifo,
			&bytes_read,
			&bytes_remaining
		);
		bytes_read += bytes_left_in_fifo;
		fifoptr     = fifo;
		packet_type = BHY_DATA_TYPE_PADDING;

		do {
			/* this function will call callbacks that are registered */
			result = bhy_parse_next_fifo_packet(
				&fifoptr,
				&bytes_read,
				&fifo_packet,
				&packet_type
			);

			/* prints all the debug packets */
			if (packet_type == BHY_DATA_TYPE_DEBUG) {
				bhy_print_debug_packet(
					&fifo_packet.data_debug, bhy_printf
				);
			}

			/* the logic here is that if doing a partial parsing of the fifo, then we
       * should not parse  */
			/* the last 18 bytes (max length of a packet) so that we don't try to
       * parse an incomplete   */
			/* packet */
		} while ((result == BHY_SUCCESS) &&
			 (bytes_read >
			  (bytes_remaining ? MAX_PACKET_LENGTH : 0)));

		bytes_left_in_fifo = 0;

		if (bytes_remaining) {
			/* shifts the remaining bytes to the beginning of the buffer */
			while (bytes_left_in_fifo < bytes_read) {
				fifo[bytes_left_in_fifo++] = *(fifoptr++);
			}
		}
	}
}

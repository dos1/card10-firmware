#include "py/mpconfig.h"

#ifndef Q
#define Q(x)
#endif

/* leds */
Q(sys_leds)
Q(update)
Q(set)
Q(set_hsv)
Q(prep)
Q(prep_hsv)
Q(set_all)
Q(set_all_hsv)
Q(set_flashlight)
Q(set_rocket)
Q(set_powersave)
Q(set_gamma)
Q(dim_top)
Q(dim_bottom)
Q(BOTTOM_LEFT)
Q(BOTTOM_RIGHT)
Q(TOP_LEFT)
Q(TOP_RIGHT)

/* buttons */
Q(buttons)
Q(read)
Q(BOTTOM_LEFT)
Q(TOP_LEFT)
Q(BOTTOM_RIGHT)
Q(TOP_RIGHT)
Q(RESET)

/* utime */
Q(utime)
Q(alarm)
Q(sleep)
Q(sleep_ms)
Q(sleep_us)
Q(ticks_ms)
Q(ticks_us)
Q(ticks_cpu)
Q(ticks_add)
Q(ticks_diff)
Q(localtime)
Q(mktime)
Q(time)

/* vibra */
Q(vibra)
Q(vibrate)

Q(set_callback)
Q(enable_callback)
Q(disable_callback)
Q(BHI160)
Q(RTC_ALARM)

/* display */
Q(sys_display)
Q(display)
Q(print)
Q(pixel)
Q(line)
Q(rect)
Q(circ)
Q(clear)

/* ambient */
Q(light_sensor)
Q(start)
Q(get_reading)
Q(stop)

/* file */
Q(__del__)
Q(__enter__)
Q(__exit__)
Q(close)
Q(encoding)
Q(file)
Q(FileIO)
Q(flush)
Q(mode)
Q(r)
Q(read)
Q(readinto)
Q(readline)
Q(readlines)
Q(seek)
Q(tell)
Q(TextIOWrapper)
Q(write)

/* os */
Q(os)
Q(exit)
Q(exec)
Q(listdir)
Q(unlink)

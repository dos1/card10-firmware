import os
import display
import leds
import utime
import buttons
import max30001
import math
import struct
import itertools
from ecg.settings import *

config = ecg_settings()
WIDTH = 160
HEIGHT = 80
OFFSET_Y = 49
HISTORY_MAX = WIDTH * 4
DRAW_AFTER_SAMPLES = 5
SCALE_FACTOR = 30
MODE_USB = "USB"
MODE_FINGER = "Finger"
FILEBUFFERBLOCK = 4096
COLOR_BACKGROUND = [0, 0, 0]
COLOR_LINE = [255, 255, 255]
COLOR_TEXT = [255, 255, 255]
COLOR_MODE_FINGER = [0, 255, 0]
COLOR_MODE_USB = [0, 0, 255]
COLOR_WRITE_FG = [255, 255, 255]
COLOR_WRITE_BG = [255, 0, 0]

history = []
filebuffer = bytearray()
write = 0
update_screen = 0
pause_screen = 0
pause_histogram = False
histogram_offset = 0
sensor = 0
disp = display.open()
last_sample_count = 1

leds.dim_top(1)
COLORS = [((23 + (15 * i)) % 360, 1.0, 1.0) for i in range(11)]

# variables for high-pass filter
# note: corresponds to 1st order hpf with -3dB at ~18.7Hz
# general formula: f(-3dB)=-(sample_rate/tau)*ln(1-betadash)
moving_average = 0
alpha = 2
beta = 3
betadash = beta / (alpha + beta)


def update_history(datasets):
    global history, moving_average, alpha, beta, last_sample_count
    last_sample_count = len(datasets)
    for val in datasets:
        if "HP" in config.get_option("Filter"):
            history.append(val - moving_average)
            moving_average += betadash * (val - moving_average)
            # identical to: moving_average = (alpha * moving_average + beta * val) / (alpha + beta)
        else:
            history.append(val)

    # trim old elements
    history = history[-HISTORY_MAX:]


# variables for pulse detection
pulse = -1
samples_since_last_pulse = 0
last_pulse_blink = 0
q_threshold = -1
r_threshold = 1
q_spike = -500 #just needs to be long ago


def neighbours(n, lst):
    """
    neighbours(2, "ABCDE") = ("AB", "BC", "CD", "DE")
    neighbours(3, "ABCDE") = ("ABC", "BCD", "CDE")
    """

    for i in range(len(lst) - (n - 1)):
        yield lst[i : i + n]


def detect_pulse(num_new_samples):
    global history, pulse, samples_since_last_pulse, q_threshold, r_threshold, q_spike

    # look at 3 consecutive samples, starting 2 samples before the samples that were just added, e.g.:
    # existing samples: "ABCD"
    # new samples: "EF" => "ABCDEF"
    # consider ["CDE", "DEF"]
    # new samples: "GHI" => "ABCDEFGHI"
    # consider ["EFG", "FGH", "GHI"]
    ecg_rate = config.get_option("Rate")

    for [prev, cur, next_] in neighbours(3, history[-(num_new_samples + 2) :]):
        samples_since_last_pulse += 1

        if prev > cur < next_ and cur < q_threshold:
            q_spike = samples_since_last_pulse
            # we expect the next q-spike to be at least 60% as high as this one
            q_threshold = (cur * 3) // 5
        elif (
            prev < cur > next_
            and cur > r_threshold
            and samples_since_last_pulse - q_spike < ecg_rate // 10
        ):
            # the full QRS complex is < 0.1s long, so the q and r spike in particular cannot be more than ecg_rate//10 samples apart
            pulse = 60 * ecg_rate // samples_since_last_pulse
            samples_since_last_pulse = 0
            q_spike = -ecg_rate
            if pulse < 30 or pulse > 210:
                pulse = -1
            elif write > 0 and "pulse" in config.get_option("Log"):
                write_pulse()
            # we expect the next r-spike to be at least 60% as high as this one
            r_threshold = (cur * 3) // 5
        elif samples_since_last_pulse > 2 * ecg_rate:
            q_threshold = -1
            r_threshold = 1
            pulse = -1


def callback_ecg(datasets):
    global update_screen, history, filebuffer, write
    update_screen += len(datasets)

    # update histogram datalist
    if not pause_histogram:
        update_history(datasets)
        detect_pulse(len(datasets))

    # buffer for writes
    if write > 0 and "graph" in config.get_option("Log"):
        for value in datasets:
            filebuffer.extend(struct.pack("h", value))
            if len(filebuffer) >= FILEBUFFERBLOCK:
                write_filebuffer()

    # don't update on every callback
    if update_screen >= DRAW_AFTER_SAMPLES:
        draw_histogram()

def write_pulse():
    global write, pause_screen
    # write to file
    lt = utime.localtime(write)
    filename = "/pulse-{:04d}-{:02d}-{:02d}_{:02d}{:02d}{:02d}.log".format(*lt)

    # write stuff to disk
    try:
        f = open(filename, "ab")
        print(utime.time(), pulse)
        f.write(struct.pack("h", utime.time(), pulse))
        f.close()
    except OSError as e:
        print("Please check the file or filesystem", e)
        write = 0
        pause_screen = -1
        disp.clear(COLOR_BACKGROUND)
        disp.print("IO Error", posy=0, fg=COLOR_TEXT)
        disp.print("Please check", posy=20, fg=COLOR_TEXT)
        disp.print("your", posy=40, fg=COLOR_TEXT)
        disp.print("filesystem", posy=60, fg=COLOR_TEXT)
        disp.update()
        close_sensor()
    except:
        print("Unexpected error, stop writing logfile")
        write = 0

def write_filebuffer():
    global write, filebuffer, pause_screen
    # write to file
    lt = utime.localtime(write)
    filename = "/ecg-{:04d}-{:02d}-{:02d}_{:02d}{:02d}{:02d}.log".format(*lt)

    # write stuff to disk
    try:
        f = open(filename, "ab")
        f.write(filebuffer)
        f.close()
    except OSError as e:
        print("Please check the file or filesystem", e)
        write = 0
        pause_screen = -1
        disp.clear(COLOR_BACKGROUND)
        disp.print("IO Error", posy=0, fg=COLOR_TEXT)
        disp.print("Please check", posy=20, fg=COLOR_TEXT)
        disp.print("your", posy=40, fg=COLOR_TEXT)
        disp.print("filesystem", posy=60, fg=COLOR_TEXT)
        disp.update()
        close_sensor()
    except:
        print("Unexpected error, stop writing logfile")
        write = 0

    filebuffer = bytearray()


def open_sensor():
    global sensor
    sensor = max30001.MAX30001(
        usb=(config.get_option("Mode") == "USB"),
        bias=config.get_option("Bias"),
        sample_rate=config.get_option("Rate"),
        callback=callback_ecg,
    )


def close_sensor():
    global sensor
    sensor.close()


def toggle_write():
    global write, disp, pause_screen
    pause_screen = utime.time_ms() + 1000
    disp.clear(COLOR_BACKGROUND)
    if write > 0:
        write_filebuffer()
        write = 0
        disp.print("Stop", posx=50, posy=20, fg=COLOR_TEXT)
        disp.print("logging", posx=30, posy=40, fg=COLOR_TEXT)
    else:
        filebuffer = bytearray()
        write = utime.time()
        disp.print("Start", posx=45, posy=20, fg=COLOR_TEXT)
        disp.print("logging", posx=30, posy=40, fg=COLOR_TEXT)

    disp.update()


def toggle_pause():
    global pause_histogram, histogram_offset, history, leds
    if pause_histogram:
        pause_histogram = False
        history = []
    else:
        pause_histogram = True
    histogram_offset = 0
    leds.clear()



def draw_leds(vmin, vmax):
    # vmin should be in [0, -1]
    # vmax should be in [0, 1]
    global pulse, samples_since_last_pulse, last_pulse_blink

    led_mode = config.get_option("LEDs")

    # stop blinking
    if not bool(led_mode):
        return

    # update led bar
    if "bar" in led_mode:
        for i in reversed(range(6)):
            leds.prep_hsv(
                5 + i, COLORS[5 + i] if vmin <= 0 and i <= vmin * -6 else (0, 0, 0)
            )
        for i in reversed(range(6)):
            leds.prep_hsv(
                i, COLORS[i] if vmax >= 0 and 5 - i <= vmax * 6 else (0, 0, 0)
            )

    # blink red on pulse
    if (
        "pulse" in led_mode
        and pulse > 0
        and samples_since_last_pulse < last_pulse_blink
    ):
        for i in range(4):
            leds.prep(11 + i, (255, 0, 0))
    elif "pulse" in led_mode:
        for i in range(4):
            leds.prep(11 + i, (0, 0, 0))
    last_pulse_blink = samples_since_last_pulse

    leds.update()


def draw_histogram():
    global disp, history, write, pause_screen, update_screen

    # skip rendering due to message beeing shown
    if pause_screen == -1:
        return
    elif pause_screen > 0:
        t = utime.time_ms()
        if t > pause_screen:
            pause_screen = 0
        else:
            return

    disp.clear(COLOR_BACKGROUND)

    # offset in pause_histogram mode
    timeWindow = config.get_option("Window")
    window_end = int(len(history) - histogram_offset)
    s_end = max(0, window_end)
    s_start = max(0, s_end - WIDTH*timeWindow)

    # get max value and calc scale
    value_max = max(abs(x) for x in history[s_start:s_end])
    scale = SCALE_FACTOR / (value_max if value_max > 0 else 1)

    # draw histogram
    # values need to be inverted so high values are drawn with low pixel coordinates (at the top of the screen)
    draw_points = (int(-x * scale + OFFSET_Y) for x in history[s_start:s_end])

    prev = next(draw_points)
    for x, value in enumerate(draw_points):
        disp.line(x//timeWindow, prev, (x + 1)//timeWindow, value, col=COLOR_LINE)
        prev = value

    # draw text: mode/bias/write
    if pause_histogram:
        disp.print(
            "Pause"
            + (
                " -{:0.1f}s".format(histogram_offset / config.get_option("Rate"))
                if histogram_offset > 0
                else ""
            ),
            posx=0,
            posy=0,
            fg=COLOR_TEXT,
        )
    else:
        led_range = last_sample_count if last_sample_count > 5 else 5
        draw_leds(
            min(history[-led_range:]) / value_max, max(history[-led_range:]) / value_max
        )
        if pulse < 0:
            disp.print(
                config.get_option("Mode") + ("+Bias" if config.get_option("Bias") else ""),
                posx=0,
                posy=0,
                fg=(
                    COLOR_MODE_FINGER if config.get_option("Mode") == MODE_FINGER else COLOR_MODE_USB
                ),
            )
        else:
            disp.print(
                "BPM: {}".format(pulse),
                posx=0,
                posy=0,
                fg=(
                    COLOR_MODE_FINGER if config.get_option("Mode") == MODE_FINGER else COLOR_MODE_USB
                ),
            )

    # announce writing ecg log
    if write > 0:
        t = utime.time()
        if write > 0 and t % 2 == 0:
            disp.print("LOG", posx=0, posy=60, fg=COLOR_WRITE_FG, bg=COLOR_WRITE_BG)

    disp.update()
    update_screen = 0


def main():
    global pause_histogram, histogram_offset, pause_screen

    # show button layout
    disp.clear(COLOR_BACKGROUND)
    disp.print("  BUTTONS ", posx=0, posy=0, fg=COLOR_TEXT, font=display.FONT20)
    disp.line(0, 20, 159, 20, col=COLOR_LINE)
    disp.print(
        "       Pause >", posx=0, posy=28, fg=COLOR_MODE_FINGER, font=display.FONT16
    )
    disp.print(
        "    Settings >", posx=0, posy=44, fg=COLOR_MODE_USB, font=display.FONT16
    )
    disp.print(
        "< WriteLog    ", posx=0, posy=64, fg=COLOR_WRITE_BG, font=display.FONT16
    )
    disp.update()
    utime.sleep(3)

    # start ecg
    open_sensor()
    while True:
        button_pressed = {"BOTTOM_LEFT": 0, "BOTTOM_RIGHT": 0, "TOP_RIGHT": 0}
        while True:
            v = buttons.read(
                buttons.BOTTOM_LEFT | buttons.BOTTOM_RIGHT | buttons.TOP_RIGHT
            )

            # TOP RIGHT
            #
            # pause

            # down
            if button_pressed["TOP_RIGHT"] == 0 and v & buttons.TOP_RIGHT != 0:
                button_pressed["TOP_RIGHT"] = 1
                toggle_pause()
                

            # up
            if button_pressed["TOP_RIGHT"] > 0 and v & buttons.TOP_RIGHT == 0:
                button_pressed["TOP_RIGHT"] = 0

            # BOTTOM LEFT
            #
            # on pause = shift view left
            # else = toggle write

            # down
            if button_pressed["BOTTOM_LEFT"] == 0 and v & buttons.BOTTOM_LEFT != 0:
                button_pressed["BOTTOM_LEFT"] = 1
                if pause_histogram:
                    l = len(history)
                    histogram_offset += config.get_option("Rate") / 2
                    if l - histogram_offset < WIDTH*config.get_option("Window"):
                        histogram_offset = l - WIDTH*config.get_option("Window")
                else:
                    toggle_write()
                
            # up
            if button_pressed["BOTTOM_LEFT"] > 0 and v & buttons.BOTTOM_LEFT == 0:
                button_pressed["BOTTOM_LEFT"] = 0

            # BOTTOM RIGHT
            #
            # on pause = shift view right
            # else = show settings

            # down
            if button_pressed["BOTTOM_RIGHT"] == 0 and v & buttons.BOTTOM_RIGHT != 0:
                button_pressed["BOTTOM_RIGHT"] = 1
                if pause_histogram:
                    histogram_offset -= config.get_option("Rate") / 2
                    histogram_offset -= histogram_offset % (config.get_option("Rate") / 2)
                    if histogram_offset < 0:
                        histogram_offset = 0
                else:
                    pause_screen = -1 # hide graph
                    leds.clear() # disable all LEDs
                    config.run() # show config menu
                    close_sensor() # reset sensor in case mode or bias was changed TODO do not close sensor otherwise?
                    open_sensor()
                    pause_screen = 0 # start plotting graph again
                    button_pressed["TOP_RIGHT"] = 1 # returning from menu was by pressing the TOP_RIGHT button
                
            # up
            if button_pressed["BOTTOM_RIGHT"] > 0 and v & buttons.BOTTOM_RIGHT == 0:
                button_pressed["BOTTOM_RIGHT"] = 0


if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt as e:
        sensor.close()
        raise e

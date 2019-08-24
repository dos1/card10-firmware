import leds
import utime
import buttons
import display
import os

TRANSMISSION_SEND_TXT = "blinki-transmission.txt"
TRANSMISSION_CALIBRATE = [1, 0, 1, 0, 1, 0, 1, 0]
TRANSMISSION_START_SIGNATURE = [0, 0, 0, 0, 0, 1, 0, 1]
TRANSMISSION_END_SIGNATURE = [0, 0, 0, 0, 0, 0, 0, 0]
#TRANSMISSION_END_SIGNATURE = [1, 1, 1, 1, 1, 1, 1, 1]
TRANSMISSION_TIMING_SIGNATURE = [1, 0, 1, 1]
TIMING = 150


def init():
    global input_string
    if TRANSMISSION_SEND_TXT not in os.listdir("/"):
        with open("/" + TRANSMISSION_SEND_TXT, "w") as f:
            f.write("hello world!")
    with open("/" + TRANSMISSION_SEND_TXT, "r") as f:
        input_string = f.readlines()[0]


def triangle(disp, x, y, left):
    yf = 1 if left else -1
    scale = 6
    disp.line(x - scale * yf, int(y + scale / 2), x, y)
    disp.line(x, y, x, y + scale)
    disp.line(x, y + scale, x - scale * yf, y + int(scale / 2))


def get_transmission():
    str_to_byte_list = []
    for character in input_string:
        str_to_byte_list_dirty = " ".join(map(bin, bytearray(character, "utf-8")))
        while len(str_to_byte_list_dirty) <= 8:
            str_to_byte_list_dirty = "0" + str_to_byte_list_dirty
        for item in str_to_byte_list_dirty:
            if item == "1" or item == "0":
                str_to_byte_list.append(int(item))
    transmission = []
    transmission.extend(TRANSMISSION_START_SIGNATURE)
    transmission.extend(TRANSMISSION_TIMING_SIGNATURE)
    while len(str_to_byte_list) // 8 > 0:
        transmission.extend(str_to_byte_list[:8])
        del str_to_byte_list[:8]
        transmission.extend(TRANSMISSION_TIMING_SIGNATURE)
    transmission.extend(str_to_byte_list)
    transmission.extend(TRANSMISSION_END_SIGNATURE)
    return transmission


def send_stream():
    global timing_test_list
    global transmission
    global app_state
    if transmission[0] == 1:
        print(transmission[1])
        leds.set(13, [255, 255, 255])
    elif transmission[0] == 0:
        print(transmission[0])
        leds.set(13, [0, 0, 0])
    if len(transmission) == 1:
        app_state = 3
    del transmission[0]


def send_calibration():
    leds.set(13, [255, 255, 255])
    utime.sleep_ms(int(TIMING/2))
    leds.set(13, [0, 0, 0])


def menu():
    if app_state == 0:
        disp.print("start", posy=0, fg=[255, 255, 255])
        disp.print("transmit", posy=20, fg=[255, 255, 255])
        disp.print("calibration", posy=40, fg=[255, 255, 255])
        disp.print("hold", posx=85, posy=60, fg=[0, 255, 255])
        triangle(disp, 150, 66, False)
    if app_state == 1:
        disp.print("calibrating", posy=0, fg=[0, 255, 255])
        disp.print("release", posx=36, posy=40, fg=[255, 255, 255])
        disp.print("second", posx=36, posy=60, fg=[255, 255, 255])
    if app_state == 2:
        disp.print(str("sending"), posy=20, fg=[255, 255, 255])
        disp.print("reset", posx=70, posy=60, fg=[0, 255, 255])
        triangle(disp, 150, 66, False)
    if app_state == 3:
        disp.print(str("done"), posy=20, fg=[255, 255, 255])


app_state = 0
input_string = ""
button_pressed = False
disp = display.open()
counter = 0
calibration_started = 0
counter = 0
start_time = 0
start_time_set = 0
cycle_time = 0
cycle_start_time = 0

init()
transmission = get_transmission()

while True:
    if app_state == 2:
        counter += 1
    if app_state == 2 & start_time_set == 0:
        start_time = utime.time()
        start_time_set = 1
    disp.clear()
    v = buttons.read(buttons.BOTTOM_LEFT | buttons.BOTTOM_RIGHT | buttons.TOP_RIGHT)
    if v == 0:
        button_pressed = False
    if not button_pressed and v & buttons.BOTTOM_RIGHT != 0:
        button_pressed = True
        app_state += 1
    if app_state == 0:
        if v == 4:
            calibration_started = 1
            app_state = 1
            cycle_start_time = utime.time_ms()
    if app_state == 1:
        send_calibration()
        if v == 0 and calibration_started == 1:
            calibration_started = 0
            app_state = 2
    if app_state == 2:
        send_stream()
    if app_state == 3:
        transmission = get_transmission()
        app_state = 0
        cycle_time = 0
    menu()
    disp.update()
    if app_state == 2:
        end_time = utime.time()
        print(counter)
        print(end_time-start_time)
    if app_state == 3:
        print(counter)
        print(end_time-start_time)
    if app_state == 1 or app_state == 2:
        while utime.time_ms() - cycle_start_time <= cycle_time:
            wait = True
            print("wait")
            print(utime.time_ms())
            print(cycle_start_time)
            print(cycle_time)
            print("wait-end")
        cycle_time += TIMING


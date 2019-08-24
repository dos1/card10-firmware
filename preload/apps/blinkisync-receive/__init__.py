import display
import light_sensor
import utime
import buttons
import os

TRANSMISSION_RECEIVED_TXT = "blinki-received.txt"
TRANSMISSION_CALIBRATE = [1, 0, 1, 0, 1, 0, 1, 0]
TRANSMISSION_START_SIGNATURE = [0, 0, 0, 0, 0, 1, 0, 1]
TRANSMISSION_END_SIGNATURE = [0, 0, 0, 0, 0, 0, 0, 0]
TRANSMISSION_TIMING_SIGNATURE = [1, 0, 1, 1]
TIMING = 75
SCROLLSPEED = 20


def init():
    if TRANSMISSION_RECEIVED_TXT not in os.listdir("/"):
        with open("/" + TRANSMISSION_RECEIVED_TXT, "w") as f:
            f.write("blinki transmissions")


def triangle(disp, x, y, left):
    yf = 1 if left else -1
    scale = 6
    disp.line(x - scale * yf, int(y + scale / 2), x, y)
    disp.line(x, y, x, y + scale)
    disp.line(x, y + scale, x - scale * yf, y + int(scale / 2))


def save_transmission():
    with open(TRANSMISSION_RECEIVED_TXT, "a") as myfile:
        myfile.write("\n" + decoded_string)


def transmission_to_list(length=0):
    global read_values_list
    global read_values_current
    read_values_current = light_sensor.read()
    if length != 0:
        while len(read_values_list) >= length:
            read_values_list.pop(0)
    read_values_list.append(read_values_current)


def transmission_list_to_byte_list():
    global read_values_list_binary
    global read_values_list_copy
    global read_values_list
    if read_values_list[0] <= read_values_mean:
        read_values_list_binary.append(0)
        read_values_list_copy.append(0)
    else:
        read_values_list_binary.append(1)
        read_values_list_copy.append(1)
    del read_values_list[:1]
    if len(read_values_list_binary) == 2:
        if read_values_list_binary[:2] == [0, 0]:
            read_values_list_binary_filtered.append(0)
            del read_values_list_binary[:2]
        if read_values_list_binary[:2] == [1, 1]:
            read_values_list_binary_filtered.append(1)
            del read_values_list_binary[:2]
        if read_values_list_binary[:2] == [0, 1]:
            read_values_list_binary_filtered.append(0)
            del read_values_list_binary[:1]
        if read_values_list_binary[:2] == [1, 0]:
            read_values_list_binary_filtered.append(1)
            del read_values_list_binary[:1]


def get_transmission_mean():
    global read_values_mean
    transmission_to_list(8)
    read_values_mean = sum(read_values_list) / len(read_values_list)


def detect_transmission_start():
    global read_values_list_binary_filtered
    global transmission_started
    #print(read_values_list_binary_filtered)
    if read_values_list_binary_filtered[-8:] == TRANSMISSION_START_SIGNATURE:
        transmission_started = 1


def detect_transmission_end():
    global transmission_ended
    if read_values_list_binary_filtered[-8:] == TRANSMISSION_END_SIGNATURE:
        transmission_ended = 1


def strip_timecode():
    global read_values_list_binary_filtered
    if read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)] == TRANSMISSION_TIMING_SIGNATURE:
        del read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)]
    elif read_values_list_binary_filtered[1:len(TRANSMISSION_TIMING_SIGNATURE)+1] == TRANSMISSION_TIMING_SIGNATURE:
        del read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)+1]
    elif read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)-1] == TRANSMISSION_TIMING_SIGNATURE[1:len(TRANSMISSION_TIMING_SIGNATURE)]:
        del read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)-1]
    else:
        del read_values_list_binary_filtered[:len(TRANSMISSION_TIMING_SIGNATURE)-1]
    if read_values_list_binary_filtered[0] == 1:
        del read_values_list_binary_filtered[:1]


def decode_byte():
    global read_values_list_binary_filtered
    global decoded_string
    byte_string = ""
    for item in read_values_list_binary_filtered[:8]:
        byte_string += str(item)
    del read_values_list_binary_filtered[:8]
    decoded_string += chr(int(byte_string, 2))


def decode_transmission():
    global decoded_string
    del read_values_list_binary_filtered[0]
    while len(read_values_list_binary_filtered) >= 8+len(TRANSMISSION_TIMING_SIGNATURE):
        decode_byte()
        strip_timecode()


def menu():
    if app_state == 0:
        disp.print("start", posy=0, fg=[255, 255, 255])
        disp.print("receive", posy=20, fg=[255, 255, 255])
        disp.print("calibration", posy=40, fg=[255, 255, 255])
        disp.print("hold", posx=85, posy=60, fg=[0, 255, 255])
        triangle(disp, 150, 66, False)
    if app_state == 2:
        disp.print("calibrating", posy=0, fg=[0, 255, 255])
        disp.print(str(min(read_values_list)), posx=0, posy=20, fg=[255, 0, 0])
        #disp.print(str(round(read_values_mean, 2)), posx=46, posy=40, fg=[0, 255, 255])
        disp.print(str(max(read_values_list)), posx=50, posy=20, fg=[0, 255, 0])
        disp.print("release", posx=36, posy=40, fg=[255, 255, 255])
        disp.print("first", posx=36, posy=60, fg=[255, 255, 255])
        triangle(disp, 150, 66, False)
    if app_state == 1:
        disp.print("waiting for", posy=0, fg=[255, 255, 255])
        disp.print("transmission", posy=20, fg=[255, 255, 255])
        printlist = ''.join(str(e) for e in read_values_list_binary_filtered[-22:])
        disp.print(printlist, posx=0, posy=40, fg=[0, 255, 0])
        disp.print("reset", posx=12, posy=60, fg=[255, 0, 0])
        triangle(disp, 10, 66, True)
    if app_state == 3:
        disp.print("transmission", posy=0, fg=[255, 255, 255])
        disp.print("started", posy=20, fg=[255, 255, 255])
        printlist = ''.join(str(e) for e in read_values_list_binary_filtered[-22:])
        disp.print(printlist, posx=0, posy=40, fg=[0, 255, 0])
    if app_state == 5:
        global offset_counter
        linelength = len(decoded_string)
        maxchars = 11
        if linelength > maxchars:
            offset = offset_counter//SCROLLSPEED
            #print(offset)
            #print(offset_counter)
            disp.print(decoded_string[offset:maxchars+offset], posx=0, posy=20, fg=[0, 255, 0])
            offset_counter += 1
            if maxchars + offset > linelength:
                offset_counter = 0
        else:
            disp.print(decoded_string, posx=0, posy=20, fg=[0, 255, 0])
        disp.print("ended", posy=0, fg=[255, 255, 255])
        disp.print("save", posx=85, posy=60, fg=[255, 255, 255])
        disp.print("back", posx=15, posy=60, fg=[255, 255, 255])
        triangle(disp, 10, 66, True)
        triangle(disp, 150, 66, False)

def reset():
    global app_state
    global offset_counter
    global read_values_current
    global read_values_list
    global read_values_list_binary
    global read_values_list_binary_filtered
    global read_values_list_binary_copy
    global read_values_list_copy
    global cycle_start_time
    global cycle_time
    global transmission_decoded
    global transmission_started
    global transmission_ended
    global mean_calculation_started
    global counter
    global decoded_string
    global synchronized
    app_state = 0
    offset_counter = 0
    read_values_current = 0
    read_values_list = [0]
    read_values_list_binary = [0]
    read_values_list_binary_filtered = []
    read_values_list_binary_copy = []
    read_values_list_copy = []
    cycle_start_time = 0
    cycle_time = 0
    transmission_decoded = ""
    transmission_started = 0
    transmission_ended = 0
    mean_calculation_started = 0
    counter = 0
    decoded_string = ""
    synchronized = 0

app_state = 0
offset_counter = 0
read_values_current = 0
read_values_list = [0]
read_values_list_binary = [0]
read_values_list_binary_filtered = []
read_values_list_binary_copy = []
read_values_list_copy = []
cycle_start_time = 0
cycle_time = 0
transmission_decoded = ""
read_values_mean = sum(read_values_list) / len(read_values_list)
transmission_started = 0
transmission_ended = 0
mean_calculation_started = 0
button_pressed = False
disp = display.open()
counter = 0
decoded_string = ""
synchronized = 0

init()

while True:
    v = buttons.read(buttons.BOTTOM_LEFT | buttons.BOTTOM_RIGHT | buttons.TOP_RIGHT)
    if v == 0:
        button_pressed = False
    disp.clear()
    if app_state == 0:
        if v == 4:
            mean_calculation_started = 1
            app_state = 2
            cycle_start_time = utime.time_ms()
            decoded_string = ""
    if app_state == 2:
        get_transmission_mean()
        if v == 0 and mean_calculation_started == 1: 
            app_state = 1
    if app_state == 1:
        mean_calculation_started = 0
        transmission_to_list()
        transmission_list_to_byte_list()
        detect_transmission_start()
        if not button_pressed and v & buttons.BOTTOM_LEFT != 0:
            button_pressed = True
            reset()
        #print(read_values_list)
        #print(read_values_list_binary)
        #print(read_values_list_binary_filtered)
        if transmission_started == 1:
            read_values_list = []
            read_values_list_copy = []
            read_values_list_binary_copy = []
            read_values_list_binary = []
            read_values_list_binary_filtered = []
            app_state = 3
    if app_state == 3:
        transmission_to_list()
        transmission_list_to_byte_list()
        #filter_binary_list()
        detect_transmission_end()
        if transmission_ended == 1:
            app_state = 4
    if app_state == 4:
        decode_transmission()
        app_state = 5
    if app_state == 5:
        if not button_pressed and v & buttons.BOTTOM_RIGHT != 0:
            button_pressed = True
            print(decoded_string)
            save_transmission()
            reset()
        if not button_pressed and v & buttons.BOTTOM_LEFT != 0:
            button_pressed = True
            reset()            
    menu()
    disp.update()
    if app_state == 1 or app_state == 2 or app_state == 3:
        while utime.time_ms() - cycle_start_time <= cycle_time:
            wait = True
            #print("wait")
            #print(utime.time_ms())
            #print(cycle_start_time)
            #print(cycle_time)
            #print("wait-end")
        cycle_time += TIMING


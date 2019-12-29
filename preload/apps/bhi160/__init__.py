import bhi160
import buttons
import color
import contextlib
import display
import itertools
import simple_menu

STATUS_COLORS = [
    color.RED,
    # Orange
    color.RED * 0.5 + color.YELLOW * 0.5,
    color.YELLOW,
    color.GREEN,
]


def sensors(**args):
    while True:
        with bhi160.BHI160Orientation(**args) as s:
            yield s, "Orientation"
        with bhi160.BHI160Accelerometer(**args) as s:
            yield s, "Accel"
        with bhi160.BHI160Gyroscope(**args) as s:
            yield s, "Gyro"
        with bhi160.BHI160Magnetometer(**args) as s:
            yield s, "Magnetic"


with display.open() as disp:
    args = {"sample_rate": 10, "sample_buffer_len": 20}

    sensor_iter = sensors(**args)
    sensor, sensor_name = next(sensor_iter)

    for ev in simple_menu.button_events(timeout=0.1):
        # Pressing the bottom right button cycles through sensors
        if ev == buttons.BOTTOM_RIGHT:
            sensor, sensor_name = next(sensor_iter)

        samples = sensor.read()
        if not samples:
            continue

        sample = samples[-1]
        col = STATUS_COLORS[sample.status]

        disp.clear()
        disp.print("{:^11s}".format(sensor_name), posy=0, posx=3)
        disp.print("X:{: 9.4f}".format(sample.x), posy=20, fg=col)
        disp.print("Y:{: 9.4f}".format(sample.y), posy=40, fg=col)
        disp.print("Z:{: 9.4f}".format(sample.z), posy=60, fg=col)
        disp.update()

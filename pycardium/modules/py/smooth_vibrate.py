import vibra
import utime


def vibrate(times=None, duration=None, sleep=None):
    """
    trigger vibration motor with three values
    :param int times: how often we should trigger
    :param int duration: how long should we trigger for each iteration (ms)
    :param int sleep: how long should we wait in between (ms)

    combination out of this creates a fake PWM
    that enables easy and smooth vibrations
    """
    times = times or 1

    for i in range(times):
        vibra.vibrate(duration)
        utime.sleep_ms(sleep)


def simple():
    """
    simple() is a short way of vibration
    for example for button push/change status suitable
    """
    vibrate(5, 5, 10)


def select():
    """
    select() is a little longer, but smoother
    good for communicating something got selected
    """
    vibrate(10, 5, 20)


def pattern(n=None):
    """
    :param int n: how often to repeat the pattern
    n = 2 and n = 1 feels the same

    very distinctive but long pattern
    feels like duk-duk-duk
    """
    n = n or 3
    vibrate(n, 20, 100)

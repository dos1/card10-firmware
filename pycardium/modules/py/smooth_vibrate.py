import vibra
import utime


def vibrate(times=None, dur=None, sle=None):
    """
    trigger vibration motor with three values
    times => how often we should triger
    dur => how long should we trigger for each iteration
    sle => how long should we wait in between

    combination out of this creates a fake PWM
    that enables easy and smooth vibrations
    """
    times = times or 1

    for i in range(times):
        vibra.vibrate(dur)
        utime.sleep_ms(sle)


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
    very distinctive but long pattern
    feels like duk-duk-duk
    n = 2 and n = 1 feels the same
    """
    n = n or 3
    vibrate(n, 20, 100)

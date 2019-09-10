import vibra
import utime


def vi(dur, sle):
    """
    helper for repeater()
    see repeater() params
    """
    vibra.vibrate(dur)
    utime.sleep_ms(sle)


def repeater(times, dur, sle):
    """
    trigger vibration motor with three values
    times => how often we should triger
    dur => how long should we trigger for each iteration
    sle => how long should we wait in between

    combination out of this creates a fake PWM
    that enables easy and smooth vibrations
    """
    for i in range(times):
        vi(dur, sle)


def simple():
    """
    simple() is a short way of vibration
    for example for button push/change status suitable
    """
    repeater(5, 5, 10)


def select():
    """
    select() is a little longer, but smoother
    good for communicating something got selected
    """
    repeater(10, 5, 20)


def pattern(n=None):
    """
    very distinctive but long pattern
    feels like duk-duk-duk
    n = 2 and n = 1 feels the same
    """
    n = n or 3
    repeater(n, 20, 100)

import color
import simple_menu
import itertools


class Settings(simple_menu.Menu):
    color_1 = color.CAMPGREEN
    color_2 = color.CAMPGREEN_DARK

    options = {}

    def __init__(self):
        super().__init__([("return", False)])
        self.config_path = "/".join(__file__.split("/")[0:-1])

    def on_select(self, value, index):
        if index == 0:
            self.exit()
        else:
            self.options[value[0]] = next(value[1])

    def entry2name(self, value):
        if value[0] == "return":
            return value[0]
        else:
            return "{}: {}".format(value[0], self.options[value[0]][0])

    def add_option(self, option):
        self.entries.append(option)
        self.options[option[0]] = next(option[1])

    def get_option(self, name):
        return self.options[name][1]


def ecg_settings():
    config = Settings()
    config.add_option(
        (
            "LEDs",
            itertools.cycle(
                [
                    ("off", {}),
                    ("Pulse", {"pulse"}),
                    ("Bar", {"bar"}),
                    ("Full", {"pulse", "bar"}),
                ]
            ),
        )
    )
    config.add_option(("Mode", itertools.cycle([("Finger", "Finger"), ("USB", "USB")])))
    config.add_option(("Bias", itertools.cycle([("on", True), ("off", False)])))
    config.add_option(("Filter", itertools.cycle([("HP", {"HP"}), ("off", {})])))
    config.add_option(("Rate", itertools.cycle([("128Hz", 128), ("256Hz", 256)])))
    config.add_option(("Window", itertools.cycle([("1x", 1), ("2x", 2), ("3x", 3)])))
    config.add_option(
        (
            "Log",
            itertools.cycle(
                [
                    ("graph", {"graph"}),
                    ("pulse", {"pulse"}),
                    ("full", {"graph", "pulse"}),
                ]
            ),
        )
    )

    return config

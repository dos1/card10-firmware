"""
Menu Script
===========
You can customize this script however you want :)  If you want to go back to
the default version, just delete this file; the firmware will recreate it on
next run.
"""
import buttons
import color
import display
import os


FACTORY_RESET_CMD = "! RESET !"
STATE_MAIN_MENU = "MAIN"
STATE_RESET = "RESET"
STATE_RESET_ARMED = "RESET_ARMED"


def list_apps():
    """Create a list of available apps."""
    apps = sorted(os.listdir("."))

    # Filter for apps
    apps = [app for app in apps if app.endswith(".elf") or app.endswith(".py")]

    if "menu.py" in apps:
        apps.remove("menu.py")

    return apps


def list_extra_entries():
    return [FACTORY_RESET_CMD]


def activate_menu_entry(entryy):
    if entryy == FACTORY_RESET_CMD:
        return STATE_RESET

    disp.clear().update()
    disp.close()

    try:
        os.exec(entryy)
    except OSError as e:
        print("Loading failed: ", e)
        os.exit(1)

    return STATE_MAIN_MENU


def button_events():
    """Iterate over button presses (event-loop)."""
    yield 0
    button_pressed = False
    while True:
        v = buttons.read(buttons.BOTTOM_LEFT | buttons.BOTTOM_RIGHT | buttons.TOP_RIGHT)

        if v == 0:
            button_pressed = False

        if not button_pressed and v & buttons.BOTTOM_LEFT != 0:
            button_pressed = True
            yield buttons.BOTTOM_LEFT

        if not button_pressed and v & buttons.BOTTOM_RIGHT != 0:
            button_pressed = True
            yield buttons.BOTTOM_RIGHT

        if not button_pressed and v & buttons.TOP_RIGHT != 0:
            button_pressed = True
            yield buttons.TOP_RIGHT


COLOR1, COLOR2 = (color.CHAOSBLUE_DARK, color.CHAOSBLUE)


def draw_menu(disp, menulist, idx, offset):
    disp.clear()

    # Wrap around the app-list and draw entries from idx - 3 to idx + 4
    for y, i in enumerate(range(len(menulist) + idx - 3, len(menulist) + idx + 4)):
        disp.print(
            " " + menulist[i % len(menulist)] + "      ",
            posy=offset + y * 20 - 40,
            bg=COLOR1 if i % 2 == 0 else COLOR2,
        )

    disp.print(">", posy=20, fg=color.COMMYELLOW, bg=COLOR2 if idx % 2 == 0 else COLOR1)
    disp.update()


def draw_reset(disp, armed):
    disp.clear()
    
    button = "LEFT" if not armed else "RIGHT" 

    disp.print("Press %s"%button, posy=0)       
    disp.print("to perform", posy=20)
    disp.print("factory", posy=40)
    disp.print("reset", posy=60)

    disp.update()


def main():
    disp = display.open()
    menulist = list_apps()
    menulist += list_extra_entries()
    numentries = len(menulist)
    current = 0
    state = STATE_MAIN_MENU 
    for ev in button_events():
        # Button handling
        if state == STATE_MAIN_MENU:
            if ev == buttons.BOTTOM_RIGHT:
                # Scroll down
                draw_menu(disp, menulist, current, -8)
                current = (current + 1) % numentries
            elif ev == buttons.BOTTOM_LEFT:
                # Scroll up
                draw_menu(disp, menulist, current, 8)
                current = (current + numentries - 1) % numentries
            elif ev == buttons.TOP_RIGHT:
                # Select & start
                state = activate_menu_entry(menulist[current])
        elif state == STATE_RESET:
            if ev == buttons.BOTTOM_LEFT:
                state = STATE_RESET_ARMED
            else:
                state = STATE_MAIN_MENU
        elif state == STATE_RESET_ARMED:
            if ev == buttons.BOTTOM_RIGHT:
                files = os.listdir(".")
                for f in files:
                    os.unlink(f)
                os.exit(0)
            else:
                state = STATE_MAIN_MENU

        # Output handling
        if state == STATE_MAIN_MENU: 
            draw_menu(disp, menulist, current, 0)
        elif state == STATE_RESET:
            draw_reset(disp, False)
        elif state == STATE_RESET_ARMED:
            draw_reset(disp, True)


if __name__ == "__main__":
    main()

# Changelog
All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/).

## [Unreleased]
### Added
- Scripts for profiling card10 (`tools/poor-profiler`)
- `tools/ecg2wav.py` script for displaying ECG logs in audio programs like
  Audacity.

### Changed
- Ported hardware-locks & bhi160 to new mutex API
- The menu now tries to display apps without a `metadata.json` as well, if
  possible.

### Fixed
- Fixed an unguarded i2c bus transaction which caused strange issues all
  around.
- Fixed copying large files freezing card10.
- Fixed BHI160 initialization interrupt behavior.
- Properly disable BHI160 if an error occurs during init.
- Fixed bhi160 app overflowing sensor queues.
- Fixed neopixel driver not properly writing the first pixel the first
  time.
- Fixed some l0dables crashing because the SysTick timer interrupt was not
  disabled.


## [v1.13] - 2019-12-09 - [Mushroom]
[Mushroom]: https://card10.badge.events.ccc.de/release/card10-v1.13-Mushroom.zip

### Added
- ECG plotter tool (for desktop machines) which can plot ECG logs taken with card10.
- The `input()` Python function.
- Enabled the MicroPython `framebuf` module for a Pycardium-only framebuffer
  implementation.
- Added the `utime.ticks_us()` and `utime.ticks_ms()` functions for very
  accurate timing of MicroPython code.
- Added an option to use the right buttons for scrolling and the left one for
  selecting.  This will be made configurable in a future release.
- Made timezone configurable with a new `timezone` option in `card10.cfg`.
- Added a setting-menu to the ECG App.

### Changed
- Changed default timezone to CET.
- Made a few library functions callable without any parameters so they are
  easier to use.
- Refactored the `card10.cfg` config parser.

### Fixed
- Fixed the Pycardium delay implementation in preparation for features like
  button-interrupts.  Should also be more accurate now.
- Fixed the filter which is used by the ECG app.
- Fixed the display staying off while printing the sleep-messages.
- Improved the USB-Storage mode in the menu app.
- Fixed GPIO module not properly configuring a pin if both IN and ADC are given.
- Added missing documentation for `os.mkdir()` and `os.rename()`.
- Fixed all `-Wextra` warnings, including a few bugs.  Warnings exist for a reason!

### Removed
- Removed unnecessary out-of-bounds checks in display module.  Drawing outside
  the display is now perfectly fine and the pixels will silently be ignored.


## [v1.12] - 2019-10-19 - [Leek]
[Leek]: https://card10.badge.events.ccc.de/release/card10-v1.12-Leek.zip

### Added
- **USB Storage mode**!  You can now select 'USB Storage' in the menu and
  access card10's filesystem via USB.  No more rebooting into bootloader!
- LED feedback on boot.  If your display is broken, you can still see it doing
  something now.
- `./tools/pycard10.py --set-time` to set card10's system time from your host.
- 4 new functions in `utime` modules:
  * `set_time_ms()`
  * `set_unix_time_ms()`
  * `unix_time()`
  * `unix_time_ms()`

### Changed
- Updated BLE stack
- Refactored gfx API for drawing images (internal).
- Draw partially clipped primitives in all cases (Fixes menu scrolling
  animation).
- Fatal errors are now handled in a central 'panic' module.

### Fixed
- Make BLE interrupts higher priority than anything else to hopefully increase
  stability.
- Turn off BLE encryption after closing a connection.
- Fixed mainline bootloader being broken.
- Fixed menu entries being ordered by path instead of name.
- Fixed menu crashing without a message.
- Fixed QSTR build-system.


## [v1.11] - 2019-09-24 - [Karotte]
[Karotte]: https://card10.badge.events.ccc.de/release/card10-v1.11-Karotte.zip

### Added
- **Support for sleep-mode instead of full power-off.  This means the RTC now
  retains its state!**
- For debugger users: A GDB macro `task_backtrace` which allows to view
  backtraces of tasks which are currently swapped out.  Use like
  ```text
  (gdb) task_backtrace serial_task_id
  ...
  (gdb) task_backtrace dispatcher_task_id
  ...
  (gdb) task_backtrace ble_task_id
  ```
- BHI160 magnetometer sensor
- ESB API in Pycardium.
- Monotonic clock API
- New FOSS font ...

### Changed
- `Display.print()` uses a transparent background when printing with `bg == fg`.
- Try different crc16 module during build because different environments might
  have different ones installed.
- Improved ECG app, it can now blink on pulse and more!
- Improved BHI160 and BME680 apps.

### Fixed
- Fixed a regression which made it impossible to turn off the flashlight.
- Fixed CRT for l0dables not allowing to overwrite interrupt handlers.
- Fixed ECG App not closing the sensor on `KeyboardInterrupt`.
- Fixed a bug which made the power-button unresponsive when pressed during boot
  (Interrupts were getting ignored).
- Fixed `simple_menu.Menu.exit()` not actually working.
- Added a few missing locks in `leds` module.
- Added a workaround for BHI160 axis mapping not being applied in some cases.
- Added a critical-section in BLE stack initialization to prevent weird lock-ups.
- Fixed vibra module crashing when calling `vibra.vibrate()` while already running.
- Fixed sensor-sample overflow leading to I2C bus lockup.


## [v1.10] - 2019-09-05 21:42 - [JerusalemArtichoke]
[JerusalemArtichoke]: https://card10.badge.events.ccc.de/release/card10-v1.10-JerusalemArtichoke.zip

### Added
- **ws2812**: Connect Neopixels to the wristband GPIOs and make your card10
  even more colorful!
- DigiClk is now in the default prelude!
- High-pass filter and pulse detection in default ECG app.
- Actually added `uuid` module - it was not built into the firmware before,
  by accident.
- `leds.get_rgb()`: Get the current color of an LED.
- `leds.get_rocket()`: Get the current brightness of one of the rockets.
- `micropython.mem_use()` function.
- The analog-clock can now also set the time using the buttons.

### Changed
- **Pycardium**: Switched from `long-long` to `mpz` integer representation.
  This should resolve any issues with large numbers which had popped up so far.
- Refactored BME680 sensor interface.
- Made OpenOCD scripts work with more debuggers out of the box.
- Internal changes in preparation for button-interrupts.

### Fixed
- Backlight and Vibration motor were not reset when switching apps.
- Mismatch in default settings of the *Card10 Nickname* app.
- Fixed the PMIC ADC muxer not being properly reset to neutral after a
  measurement.
- Fixed wrong timezone offset calculation in `utime.time_ms()`.
- Fixed bug where `\` characters were not parsed as path separators.
- Fixed the alignment request check in our ELF l0der.
- Fixed a buffer-overflow in the config-parser.


## [v1.9] - 2019-08-28 23:23 - [IcebergLettuce]
[IcebergLettuce]: https://card10.badge.events.ccc.de/release/card10-v1.9-IcebergLettuce.zip

### Added
- `tools/pycard10.py`: Tool to interact with card10's serial connection and
  upload files directly:
  ```bash
  ./tools/pycard10.py path/to/python-script.py
  ```
- `epic_disp_print_adv` & `Display.print(font=...)`: Print with different
  fonts!  The following fonts are supported: `8px`, `12px`, `16px`, `20px`,
  and `24px`.
- **pycardium**: Support for RAW REPL mode.
- **bhi160**: Function to disable all sensors (`bhi160.disable_all_sensors()`).
- `ls_cmsis_dap`: A tool to enumerate CMSIS-DAP debuggers.
- Tons of new features to `simple_menu`: Timeout, scrolling of long texts,
  robustness against crashes, and proper exiting.
- `card10.cfg` config file which allows enabling *ELF* files.
- Analog read for wristband GPIOs.

### Changed
- Refactored *menu* and *personal-state* apps.
- `main.py` was moved into an app to allow easier reconfiguration of the
  default app.  The new `main.py` points to the "old" one so behavior is not
  changed.
- After a timeout, the menu will close and `main.py` will run again.
- BLE security updates.
- More detailed battery state display in nickname app.
- Improved ECG app.

### Removed
- Some unused font files.

### Fixed
- Fixed a regression which made the ECG app no longer work.
- Fixed card10 advertising support for AT-commands.
- Rectangles being one pixel too small.



## [v1.8] - 2019-08-27 11:38 - [HabaneroChilli]
[HabaneroChilli]: https://card10.badge.events.ccc.de/release/card10-v1.8-HabaneroChilli.zip

### Added
- API-call for direct light-sensor readout: `epic_light_sensor_read`.
- Pause mode in ECG-App.
- `bin` field in metatdata for an alternate entrypoint.
- `shell.nix`: Nix-Shell which installs patched OpenOCD and dependencies.
- Cool LED animation in default ECG app.

### Changed
- No longer require locking the display for setting the backlight.


## [v1.7] - 2019-08-24 21:48 - [Garlic]
[Garlic]: https://card10.badge.events.ccc.de/release/card10-v1.7-Garlic.zip

### Added
- **ESB**: Epic Serial Bus (Better than USB!), stability improvements of the
  USB module.  Preparation for mass-storage access in the Firmware.
- Enabled the Hardware Watchdog;  Card10 will reset itself if the firmware crashes
- Log messages when BLE is pairing / connected.
- The name of the offending app is printed to the serial console, if an app
  crashes the metatdata parser.

### Changed
- Improved log messages in cases of lock-contention.
- Menu will show an error message if a crash occurs.

### Fixed
- Fixed race-conditions in serial writes by using a queue.
- "Card10 Nickname" crashing if only `nickname.txt` exists.
- Lockup when debug prints are enabled.
- Delayed BHI160 startup a bit so the PMIC task can check the battery first.
- Relaxed the PMIC lock-timeouts so other task can take a little more time.
- Fixed off-by-one error in `gfx_line()`.
- Fixed the API interrupts sometimes getting stuck.
- Fixed binary building on MacOS.
- Fixed race-conditions in serial console prints by introducing a queue.
- Fixed API & MAX30001 mutexes being initialized late sometimes.
- Fixed wrong stripe width in bi flag.


## [v1.6] - 2019-08-23 20:30 - [Fennel]
[Fennel]: https://card10.badge.events.ccc.de/release/card10-v1.6-Fennel.zip

- Maxim BLE SDK update

### Added
- **BLE**: Added personal state API to card10 SVC.
- **ECG**: Support for ECG + Python app
- **BLE**: Characteristic to read the time

### Changed
- Improved performance of circle-drawing algorithm.

### Fixed
- Removed a debug print in the `bhi160` module.


## [v1.5] - 2019-08-23 00:18 - [Eggppppplant]
[Eggppppplant]: https://card10.badge.events.ccc.de/release/card10-v1.5-Eggppppplant.zip

### Added
- **bootloader**: Add an error message when flashing fails.
- **display**: Option to set backlight from Python
- **utime**: Function to read time in ms from Python

### Changed
- **gpio**: Rename constants for consistency.
- **ble**: Storing pairings outside BLE stack context
- **security**: Disable ELFs by default, prevent access to some more files

### Fixed
- **gpio**: Fix field-setting in `gpio_cfg_t`.


## [v1.4] - 2019-08-22 19:43 - [DaikonRadish]
[DaikonRadish]: https://card10.badge.events.ccc.de/release/card10-v1.4-DaikonRadish.zip

### Added
- Support for the `bme680` environmental sensor.
- Support for the `bhi160` sensor fusion.
- `simple_menu` module for creating simple menus in Python.
- `power` module to access the voltage and current measurements from the PMIC.
- Support for color themes in the default clock script:
  Color themes are read from a json file, so people can customize their clock.
  Last selected theme is saved in the `clock.json` so it is persistent.

### Changed
- Refactored BLE card10 service.
- Improved BLE file-transfer (added security).
- Replaced dynamic attribute creation with static attributes.

### Fixed
- Fixed menu listing files starting with `.`.
- Fixed `utime.set_time()` applying the timezone offset in the wrong direction.
- Fixed the PMIC driver not releasing some locks properly.


## [v1.3] - 2019-08-22 00:12 - [CCCauliflower]
[CCCauliflower]: https://card10.badge.events.ccc.de/release/card10-v1.3-cccauliflower.zip

### Added
- A splashscreen in Epicardium showing the version number.
- `os.urandom()` function.

### Changed
- BLE file-transfers now create missing folders.

### Fixed
- **gfx**: Add a linebreak before character, not after.  This prevent the last
  character being cut off.
- Fixed serial task overflowing because it had a too small stack size.
- Removed confusing MAXUSB messages.


## [v1.2] - 2019-08-21 18:18 - [Broccoli]
[Broccoli]: https://card10.badge.events.ccc.de/release/card10-v1.2-broccoli.zip

```text
8e8d8614 feat(apps): Add scope to preload
e1a7684a fix(cdcacm): Disable before printing error
4c74f061 fix(utime.c): set_time should operate in local tz
e0824843 feat(pmic): Switch off if battery is low
46ef3985 feat(pmic): Add API-call to read battery voltage
79e2fb15 feat(epicardium): Periodically check the battery voltage
5da9e074 feat(pmic): Implement AMUX reading
8c59935e py: timezone workaround
c7f59d3e fix(text_reader): Convert to unix line-endings
78a7a7f4 docs: Fix underlines in ble/card10
15649293 feat(app): Add some preloads
b12e4ef9 chore(docs): Fix utime module docs
3efbab13 feat(utime.c): add python functions to set time
38f83243 chore(docs): Fix color documentation
a966e221 chore(docs): Fix python-directives with double-module
66cd10d4 docs: Document os.reset()
5fe5fe31 docs: Document pride module
338132e5 apped apps folder to search module search path
cda91555 rename Main Clock to Home
c2935c8c fixed syntax
3017591a Rename preloaded apps to make use of hatchery folder structure
842e9ad8 feat(menu.py): support scrolling for long menu entries
fbf7c8c0 fix(menu.py) Refactored menu.py based on !138
8aa8c31f feat(ble): Store bondings
5e5c7a4f fix(menu.py): Fix color-mismatch of selector background
```

## [v1.1] - 2019-08-21 03:14 - Asparagus
### Added
- Seed ``urandom`` PRNG with ``TRNG`` peripheral.
- Show linenumbers in MicroPython tracebacks.

### Fixed
- **buttons**: Acquire lock before accessing I2C.
- **rtc**: Fix RTC getting stuck because of improper initialization.
- Make lifecycle task more important than dispatcher.

## [v1.0] - 2019-08-21 00:50
Initial release.

[Unreleased]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.13...master
[v1.13]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.12...v1.13
[v1.12]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.11...v1.12
[v1.11]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.10...v1.11
[v1.10]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.9...v1.10
[v1.9]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.8...v1.9
[v1.8]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.7...v1.8
[v1.7]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.6...v1.7
[v1.6]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.5...v1.6
[v1.5]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.4...v1.5
[v1.4]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.3...v1.4
[v1.3]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.2...v1.3
[v1.2]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.1...v1.2
[v1.1]: https://git.card10.badge.events.ccc.de/card10/firmware/compare/v1.0...v1.1
[v1.0]: https://git.card10.badge.events.ccc.de/card10/firmware/-/tags/v1.0

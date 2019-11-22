NimBLE
======

On the card10 the ARM Cordio-B50 stack is used, which is in a very early experimental state and has some incompatibilities with some smartphones.
Therefore some alternative stacks are evaluated, which meight be used as a replacement in the long term.


Here a stack called NimBLE is presented, which claims to be feature complete. Originally it has been developed for Mynewt, an open source embedded operating system by Apache (https://mynewt.apache.org/).


There is a working port for the ESP32 espressif ESP-IDF framework.
Like Epicardium, ESP-IDF is based on FreeRTOS. Therefore it can be used for evaluation purposes.

Getting NimBLE run on the ESP32
-------------------------------

Install required packages:

Ubuntu:

.. code-block:: shell-session

  sudo apt install git virtualenv python2.7 cmake

Arch:

.. code-block:: shell-session

  sudo pacman -S git python2 python2-virtualenv cmake

Download and extract xtensa ESP32 compiler:

.. code-block:: shell-session

  wget https://dl.espressif.com/dl/xtensa-esp32-elf-gcc8_2_0-esp32-2018r1-linux-amd64.tar.xz
  tar -xf xtensa-esp32-elf-gcc8_2_0-esp32-2018r1-linux-amd64.tar.xz


Clone esp-idf:

.. code-block:: shell-session

    git clone https://github.com/espressif/esp-idf.git

Add xtensa and ESP-IDF path to $PATH:

bash shell:

.. code-block:: shell-session

  export IDF_PATH=$PWD/esp-idf
  export PATH=${PATH}:$PWD/xtensa-esp32-elf/bin:$PWD/esp-idf/tools

fish shell:

.. code-block:: shell-session

  set -gx IDF_PATH $PWD/esp-idf
  set -gx PATH $PWD/xtensa-esp32-elf/bin/ $PWD/esp-idf/tools $PATH

Create a python2.7 virtualenv:

.. code-block:: shell-session

  cd esp-idf
  virtualenv -p /usr/bin/python2.7 venv

Enter the virtualenv:

bash shell:

.. code-block:: shell-session

  . venv/bin/activate

fish shell:

.. code-block:: shell-session

  . venv/bin/activate.fish

Init git submodules and install all required Python packages:

.. code-block:: shell-session

  git submodule update --init --recursive
  pip install -r requirements.txt


Now you are ready to build!

The following steps assume that your ESP32 is connected via USB and
is accessible via /dev/ttyUSB0. This meight be different on your system.

There are a few NimbLE examples which can be used for playing around:

Build a BLE server example (host mode):
---------------------------------------
.. code-block:: shell-session

  cd examples/bluetooth/nimble/bleprph
  idf.py -p /dev/ttyUSB0 flash monitor

This will build and flash the example to the ESP32 and instantly listens on /dev/ttyUSB0 serial port.
After the flashing process the ESP32 will anounce itself as **nimble-bleprph** device via BLE.

Build a BLE client example (central mode):
------------------------------------------
.. code-block:: shell-session

  cd examples/bluetooth/nimble/blecent
  idf.py -p /dev/ttyUSB0 flash monitor

This will build and flash the example to the ESP32 and instantly listens on /dev/ttyUSB0 serial port.
After the flashing process the ESP32 creates a GATT client and performs passive scan, it then connects to peripheral device if the device advertises connectability and the device advertises support for the Alert Notification service (0x1811) as primary service UUID.

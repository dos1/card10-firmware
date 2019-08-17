How To Build
============
If you just want to write MicroPython code for card10, you probably **won't**
need to build the firmware yourself.  This page is for people who want to work
on the underlying firmware itself.

Dependencies
------------
* **gcc**, **binutils** & **newlib** for ``arm-none-eabi``:  The packages have
  slightly different names on different distros.

  - Ubuntu / Debian:

    .. code-block:: shell-session

       apt install gcc-arm-none-eabi binutils-arm-none-eabi libnewlib-arm-none-eabi

  - Arch:

    .. code-block:: shell-session

       pacman -S arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib
    
  - Fedora
  
    .. code-block:: shell-session
        
        dnf install arm-none-eabi-gcc arm-none-eabi-binutils arm-none-eabi-newlib

  - Alternative: Download `ARM's GNU toolchain`_.  **TODO**
* **python3**:  For meson and various scripts needed for building.
* **meson** (>0.43.0) & **ninja**:  Unfortunately most distros only have very old versions
  of meson in their repositories.  Instead, you'll probably save yourself a lot
  of headaches by installing meson from *pip*.

   - Ubuntu / Debian:

    .. code-block:: shell-session

       apt install ninja-build
       pip3 install --user meson

   - Arch (has latest *meson* in the repos):

    .. code-block:: shell-session

       pacman -S meson

* **python3-crc16**: Install with ``pip3 install --user crc16``.
* **python3-pillow**: Python Image Library ``pip3 install --user pillow``.

.. _ARM's GNU toolchain: https://developer.arm.com/tools-and-software/open-source-software/developer-tools/gnu-toolchain/gnu-rm/downloads

Cloning
-------
Clone the ``master`` branch of the firmware repository:

.. code-block:: shell-session

   $ git clone https://git.card10.badge.events.ccc.de/card10/firmware.git

Build Configuration
-------------------
Initialize the build-system using

.. code-block:: shell-session

   $ ./bootstrap.sh

Additional arguments to ``bootstrap.sh`` will be passed to *meson*.  You can
use this to for example, to enable one or more of the following optional
firmware features:

- ``-Ddebug_prints=true``: Print more verbose debugging log messages
- ``-Dble_trace=true``: Enable BLE tracing.  This will output lots of status
  info related to BLE.
- ``-Ddebug_core1=true``: Enable the core 1 SWD lines which are exposed on the
  SAO connector.  Only use this if you have a debugger which is modified for core 1.

.. warning::

   Our build-system contains a few workarounds around short-comings in meson.
   These workarounds might break on some setups which we did not yet test.  If
   this is the case for you, please open an issue in our `issue tracker`_!

.. _issue tracker: https://git.card10.badge.events.ccc.de/card10/firmware/issues

Building
--------
Build using *ninja*:

.. code-block:: shell-session

   $ ninja -C build/

If ninja succeeds, the resulting binaries are in ``build/``.  They are
available in two formats:  As an ``.elf`` which can be flashed using a debugger
and as a ``.bin`` which can be loaded using the provided bootloader.  Here is a
list of the binaries:

- ``build/bootloader/bootloader.elf``: Our bootloader.  It should already be on
  your card10.  The bootloader can only be flashed using a debugger.
- ``build/pycardium/pycardium_epicardium.bin``: The entire firmware in one ``.bin``.
- ``build/epicardium/epicardium.elf``: The core 0 part of the firmware, called Epicardium.
- ``build/pycardium/pycardium.elf``: Our MicroPython port, the core 1 part of the firmware.

In order to do a rebuild you can issue a clean command to ninja via

.. code-block:: shell-session

  $ ninja -C build/ -t clean

Otherwise, rerunning ``./bootstrap.sh`` will also clean the build-directory.

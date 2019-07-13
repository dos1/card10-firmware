################################################################################
 # Copyright (C) 2016 Maxim Integrated Products, Inc., All Rights Reserved.
 #
 # Permission is hereby granted, free of charge, to any person obtaining a
 # copy of this software and associated documentation files (the "Software"),
 # to deal in the Software without restriction, including without limitation
 # the rights to use, copy, modify, merge, publish, distribute, sublicense,
 # and/or sell copies of the Software, and to permit persons to whom the
 # Software is furnished to do so, subject to the following conditions:
 #
 # The above copyright notice and this permission notice shall be included
 # in all copies or substantial portions of the Software.
 #
 # THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 # OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 # MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 # IN NO EVENT SHALL MAXIM INTEGRATED BE LIABLE FOR ANY CLAIM, DAMAGES
 # OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
 # ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 # OTHER DEALINGS IN THE SOFTWARE.
 #
 # Except as contained in this notice, the name of Maxim Integrated
 # Products, Inc. shall not be used except as stated in the Maxim Integrated
 # Products, Inc. Branding Policy.
 #
 # The mere transfer of this software does not imply any licenses
 # of trade secrets, proprietary technology, copyrights, patents,
 # trademarks, maskwork rights, or any other form of intellectual
 # property whatsoever. Maxim Integrated Products, Inc. retains all
 # ownership rights.
 #
 # $Date: 2019-06-20 15:52:01 -0500 (Thu, 20 Jun 2019) $ 
 # $Revision: 44115 $
 #
 ###############################################################################

ifeq "$(CMSIS_ROOT)" ""
$(error CMSIS_ROOT must be specified)
endif

CMSIS_ROOT := $(abspath $(CMSIS_ROOT))

# The build directory
ifeq "$(BUILD_DIR)" ""
BUILD_DIR=$(CURDIR)/build
endif

ifeq "$(STARTUPFILE)" ""
STARTUPFILE=max32665_startup.asm
endif

ifeq "$(ENTRY)" ""
ENTRY=entry
endif

ifeq "$(SDMA_LEN)" ""
# 256kB
# SRAM_ADDR    = 0x20040440
# SRAM_LEN    = 0x3fbc0
# OTP_ADDR    = 0x20000000
# SHARED_ADDR = 0x20000040
# SDMA_ADDR   = 0x20000440
SDMA_LEN    = 0x40000
endif

OTP_ADDR = 0x20000000

# Calculate the memory addresses for the different sections
OTP_ADDR_DEC=$(shell printf "%d" $(OTP_ADDR))
SDMA_LEN_DEC=$(shell printf "%d" $(SDMA_LEN))
SHARED_ADDR_DEC=$(shell expr $(OTP_ADDR_DEC) \+ 64)
MEMSTART_DEC=$(shell expr $(SHARED_ADDR_DEC) \+ 1024)
MEMEND_DEC=$(shell expr $(MEMSTART_DEC) \+ $(SDMA_LEN_DEC) \- 1)
STACK_START_DEC=$(shell expr $(MEMEND_DEC) \+ 1)

# Convert addresses back to hex
SHARED_ADDR=$(shell printf "0x%x" $(SHARED_ADDR_DEC))
MEMSTART=$(shell printf "0x%x" $(MEMSTART_DEC))
MEMEND=$(shell printf "0x%x" $(MEMEND_DEC))
STACK_START=$(shell printf "0x%x" $(STACK_START_DEC))

MEMRANGE=$(MEMSTART)-$(MEMEND)

# Default TARGET_REVISION
# "A1" in ASCII
ifeq "$(TARGET_REV)" ""
TARGET_REV=0x4131
endif

# Add target specific CMSIS source files
ifneq (${MAKECMDGOALS},lib)
SRCS += ${STARTUPFILE}
SRCS += max32665_sdma.c
endif

# Add target specific CMSIS source directories
VPATH+=$(CMSIS_ROOT)/Device/Maxim/MAX32665/Source/RCS

# Add target specific CMSIS include directories
IPATH+=$(CMSIS_ROOT)/Device/Maxim/MAX32665/Include
IPATH+=$(CMSIS_ROOT)/Device/Maxim/MAX32665/Source/RCS

LIBPATH=$(CMSIS_ROOT)/Lib/RCS

# Include the rules and goals for building
include $(CMSIS_ROOT)/Device/Maxim/MAX32665/Source/RCS/rcs.mk

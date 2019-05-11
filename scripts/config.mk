PROJNAME := iotos

ARCH := arm

SUBARCH := armv7m

BOARD := stm32f429discovery

PLATFORM := stm32f4

TOOLCHAIN ?= arm-none-eabi-

QEMU? := $(CONFIG_QEMU)

CC := $(TOOLCHAIN)gcc
export CC

AS := $(TOOLCHAIN)gcc
export AS

AR := $(TOOLCHAIN)ar
export AR

LD := $(TOOLCHAIN)ld
export LD

OBJCOPY := $(TOOLCHAIN)objcopy

NM := $(TOOLCHAIN)nm

SIZE := $(TOOLCHAIN)size

CFLAGS := -Wall \
          -ggdb \
		  -gdwarf \
          -std=c99 \
          -mlittle-endian \
          -mthumb \
          -mcpu=cortex-m4 \
          -mfloat-abi=hard \
          -mfpu=fpv4-sp-d16 \
          -Wdouble-promotion \
          -finline \
          -ffreestanding \
          -MT -MP -MD \
		  -fno-builtin

ifeq ($(CONFIG_COMPILE_OPTIMISE_LEVEL),0)
	CFLAGS += -O0
else ifeq ($(CONFIG_COMPILE_OPTIMIZE_LEVEL),1)
	CFLAGS += -O1
else ifeq ($(CONFIG_COMPILE_OPTIMISE_LEVEL),2)
	CFLAGS += -O2
else ifeq ($(CONFIG_COMPILE_OPTIMISE_LEVEL),3)
	CFLAGS += -O3
else
	CFLAGS += -Os
endif

ARFLAGS := rcs

LDFLAGS := -nostartfiles \
	       -nodefaultlibs

LINKER_FILE := $(ROOTDIR)/board/$(BOARD)/src/stm32_flash.ld


ST_FLASH := st-flash
FLASH_BASEADDR := 0x8000000


PROJNAME := iotos

ARCH := cortex-m4

CHIP := stm32f4

BOARD := stm32f429discovery

TOOLCHAIN ?= arm-none-eabi-

QEMU? := $(CONFIG_QEMU)

CC := $(TOOLCHAIN)gcc
export CC

AS := $(TOOLCHAIN)as
export AS

AR := $(TOOLCHAIN)ar
export AR

LD := $(TOOLCHAIN)ld
export LD

OBJCOPY := $(TOOLCHAIN)objcopy

NM := $(TOOLCHAIN)nm

SIZE := $(TOOLCHAIN)size

CFLAGS := -Wall \
          -g \
          -std=c99 \
          -mlittle-endian \
          -mthumb \
          -mcpu=cortex-m4 \
          -mfloat-abi=hard \
          -mfpu=fpv4-sp-d16 \
          -ffreestanding \
          -Wdouble-promotion \
          -finline \
          -MT -MP -MD \

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
           -nodefaultlibs \
		   -nostdlib


LINKER_FILE := $(ROOTDIR)/arch/boot/src/stm32_flash.ld


ST_FLASH := st-flash
FLASH_BASEADDR := 0x8000000



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

CFLAGS := -Wall \
          -O2 \
          -g \
          -std=c99 \
          -mlittle-endian \
          -mthumb \
          -mthumb-interwork \
          -mcpu=cortex-m4 \
          -ffreestanding \
          -fsingle-precision-constant \
          -Wdouble-promotion \
          -mfpu=fpv4-sp-d16 \
          -mfloat-abi=hard \
          -finline \
          -MT -MP -MD \
          -nostdlib

ARFLAGS := rcs

LDFLAGS := -nostartfiles \
           -Wl,--gc-seticons \
           -mthumb \
           -mcpu=cortex-m4


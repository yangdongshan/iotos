
TOOLCHAIN ?= $(CONFIG_TOOLCHAIN)

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
		  -Wextra \
		  -O2 \
		  -g \
		  -finline \
		  -fno-common \
		  -fasynchronous-unwind-tables \
		  -gdwarf-2 \
		  -fno-pic \
		  -fno-stack-protector \
		  -mcmodel=kernel \
		  -mno-red-zone \
		  -MT -MP -MD \
		  -nostdlib

ARFLAGS := rcs

LDFLAGS := -z max-page-size=4096



ifeq ($(V),)
	Q = @
else
	Q :=
endif
export Q

ROOTDIR = $(shell pwd)
export ROOTDIR

SCRIPTSDIR = $(ROOTDIR)/scripts
export SCRIPTSDIR

# MAKEFLAGS += --no-print-directory

# include system .config which is generated by make menuconfig
-include .config

# include config
-include $(SCRIPTSDIR)/config.mk

KCONFIGDIR = $(ROOTDIR)/kconfig
KCONFIGFILE = $(ROOTDIR)/Kconfig


CFLAGS += -include $(ROOTDIR)/include/generated/autoconf.h

ARFLAGS +=

LDFLAGS +=

# library directory
LIBDIR := arch/$(ARCH)/$(SUBARCH) \
          board/$(BOARD) \
		  platform/$(PLATFORM) \
		  libc \
		  init \
		  modules \
		  app \
		  test

LIBDIR += kernel

LIBS = $(addprefix $(ROOTDIR)/,$(foreach dir, $(LIBDIR), $(dir)/lib$(dir).a))
export LIBS

LDDIR += $(addprefix -L$(ROOTDIR)/,$(LIBDIR))
LDLIB += $(addprefix -l,$(foreach dir, $(LIBDIR), $(shell basename $(dir))))

CFLAGS += -I$(ROOTDIR)/arch/$(ARCH)/$(SUBARCH) \
		  -I$(ROOTDIR)/arch/$(ARCH)/common \
		  -I$(ROOTDIR)/platform/$(PLATFORM)/peripherals/include \
		  -I$(ROOTDIR)/platform/$(PLATFORM)/include \
		  -I$(ROOTDIR)/board/$(BOARD)/include \
          -I$(ROOTDIR)/kernel/include \
          -I$(ROOTDIR)/include \
          -I$(ROOTDIR)/libc/include \
		  -I$(ROOTDIR)/test/include \
		  -I$(ROOTDIR)/modules/ringbuf \


export LDDIR
export LDLIB

export CFLAGS
export ARFLAGS
export LDFLAGS

ELF=$(ROOTDIR)/board/$(BOARD)/$(PROJNAME).elf
HEX=$(ROOTDIR)/board/$(BOARD)/$(PROJNAME).hex
BIN=$(ROOTDIR)/board/$(BOARD)/$(PROJNAME).bin
SREC=$(ROOTDIR)/board/$(BOARD)/$(PROJNAME).srec
MAP=$(ROOTDIR)/board/$(BOARD)/$(PROJNAME).map

all: $(ELF)

$(ELF): lib
	$(Q) $(MAKE) -C board/$(BOARD) exe elf=$@ linker_file=$(LINKER_FILE)
	$(Q) echo "OBJCOPY $(HEX)"
	$(Q) $(OBJCOPY) -O ihex $@ $(HEX)
	$(Q) echo "OBJCOPY $(BIN)"
	$(Q) $(OBJCOPY) -O binary $@ $(BIN)
	$(Q) echo "OBJCOPY $(SREC)"
	$(Q) $(OBJCOPY) -O srec $@ $(SREC)
	$(Q) echo "NM $(MAP)"
	$(Q) $(NM) -s -S $(ELF) > $(MAP)
	$(Q) echo "================================================"
	$(Q) $(SIZE) -t $(ELF)
	$(Q) echo "================================================"


lib: $(LIBDIR)
	$(Q) $(foreach dir, $(LIBDIR), \
		$(MAKE) -C $(dir) obj || exit "$$?";\
		$(MAKE) -C $(dir) lib libname=lib$(shell basename $(dir)).a || exit "$$?";)


.PHONY: menuconfig distclean silentoldconfig clean launch_qemu download

menuconfig: $(KCONFIGDIR)/mconf $(KCONFIGDIR)/conf
	$(Q) $< -s $(KCONFIGFILE)
	$(Q) $(MAKE) silentoldconfig

$(KCONFIGDIR)/mconf:
	$(Q) $(MAKE) -C $(KCONFIGDIR)

silentoldconfig: $(KCONFIGDIR)/conf
	$(Q) mkdir -p include/generated include/config
	$(Q) $< -s --silentoldconfig $(KCONFIGFILE)

clean:
	$(Q) $(foreach dir, $(BOOT_DIR) $(LIBDIR), $(MAKE) -C $(dir) clean;)
	$(Q) -rm -f $(ELF) $(HEX) $(BIN) $(SREC) $(MAP)

distclean: clean
	$(Q) $(MAKE) -C $(KCONFIGDIR) clean
	$(Q) -rm -rf include/generated include/config .config

launch_qemu: $(ISO)
	$(Q) $(QEMU) -cdrom $(ISO) -nographic #-enable-kvm

download:
	$(Q) st-flash --reset  write $(BIN) 0x8000000


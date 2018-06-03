

CUR_DIR=$(shell pwd)

COBJ=$(patsubst %.c,%.o,$(CSRC))
ASMOBJ=$(patsubst %.S,%.o,$(ASMSRC))
DEPOBJ=$(patsubst %.o,%.d,$(COBJ) $(ASMOBJ))

obj: SUBMODULE $(COBJ) $(ASMOBJ)

SUBMODULE:$(MODULE)
	$(Q) $(foreach dir,$(MODULE),$(MAKE) -C ./$(dir) obj || exit "$$?";)

%.o:%.c
	$(Q) echo "CC -c $^ -o $@"
	$(Q) $(CC) $(CFLAGS) -c $^ -o $@

%.o:%.S
	$(Q) echo "CC -c $< -o $@"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean lib exe

LIB_OBJS=$(shell find $(CUR_DIR) -name "*.o")

lib:
	$(Q) echo "AR $(libname)"
	$(Q) $(AR) $(ARFLAGS) $(libname) $(LIB_OBJS)

exe: obj $(LIB_OBJS)
	$(Q) echo "Linking $(elf) ..."
	$(Q) $(LD) -T $(linker_file) $(LDFLAGS) $(LDDIR) -o $(elf) $(LIB_OBJS) --start-group $(LDLIB) --end-group

clean:
	$(Q) $(foreach dir,$(MODULE),$(MAKE) -C ./$(dir) clean || exit "$$?";)
	$(Q) rm -rf $(COBJ) $(ASMOBJ) $(DEPOBJ) *.a


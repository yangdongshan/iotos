

CUR_DIR=$(shell pwd)

COBJ=$(patsubst %.c,%.o,$(CSRC))
ASMOBJ=$(patsubst %.S,%.o,$(ASMSRC))
DEPOBJ=$(patsubst %.o,%.d,$(COBJ) $(ASMOBJ))

obj: SUBMODULE $(COBJ) $(ASMOBJ)

SUBMODULE:$(MODULE)
	$(Q) $(foreach dir,$(MODULE),$(MAKE) -C ./$(dir) obj || exit "$$?";)

%.o:%.c %.d
	$(Q) echo "CC -c $^ -o $@"
	$(Q) $(CC) $(CFLAGS) -c $^ -o $@

%.o:%.S %.d
	$(Q) echo "CC -c $< -o $@"
	$(Q) $(CC) $(CFLAGS) -c $< -o $@

.PHONY: clean lib binary

LIB_OBJS=$(shell find $(CUR_DIR) -name "*.o")

lib:
	$(Q) echo "AR $(libname) $(LIB_OBJS)"
	$(Q) $(AR) $(ARFLAGS) $(libname) $(LIB_OBJS)

binary: obj $(LIB_OBJS)
	$(Q) echo "Linking $(elf)..."
	$(Q) $(LD) -n -d -T $(linker_file) $(LDFLAGS) $(LDDIR) -o $(elf) $(LIB_OBJS) --start-group $(LDLIB) --end-group

clean:
	$(Q) $(foreach dir,$(MODULE),$(MAKE) -C ./$(dir) clean || exit "$$?";)
	$(Q) rm -rf $(COBJ) $(ASMOBJ) $(DEPOBJ) *.a


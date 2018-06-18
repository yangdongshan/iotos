# add extra CFLAGS
CFLAGS +=

# add .c files
CSRC:= irq.c \
	   sched.c \
	   thread.c \
	   timer.c \
	   time.c \
	   semphore.c \
	   mutex.c \
	   signal.c \
	   mailbox.c \
	   watchdog.c \
	   kernel.c \

# add .S files
ASMSRC:=

# add sub-directories
MODULE := mm
MODULE += kdebug

# include build.mk
include $(SCRIPTSDIR)/build.mk


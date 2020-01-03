CFLAGS ?= -Wall
LDFLAGS ?=
INC = ./include
OBJECTS = mixer.o pcm.o
LIB = libtinyalsa.a
SHLIB = libtinyalsa.so
CROSS_COMPILE = mips-linux-gnu-
CC := gcc
STRIP = strip

all: $(LIB) $(SHLIB) tinyplay tinycap tinymix tinypcminfo

tinyplay: $(SHLIB) tinyplay.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinyplay.o -L. -ltinyalsa -o tinyplay
	$(CROSS_COMPILE)$(STRIP) $@

tinycap: $(SHLIB) tinycap.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinycap.o -L. -ltinyalsa -o tinycap
	$(CROSS_COMPILE)$(STRIP) $@

tinymix: $(SHLIB) tinymix.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinymix.o -L. -ltinyalsa -o tinymix
	$(CROSS_COMPILE)$(STRIP) $@

tinypcminfo: $(SHLIB) tinypcminfo.o
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) tinypcminfo.o -L. -ltinyalsa -o tinypcminfo
	$(CROSS_COMPILE)$(STRIP) $@

$(SHLIB): $(OBJECTS)
	$(CROSS_COMPILE)$(CC) $(LDFLAGS) -shared $(OBJECTS) -o $(SHLIB)
	$(CROSS_COMPILE)$(STRIP) $@

$(LIB): $(OBJECTS)
	$(CROSS_COMPILE)$(AR) rcs $@ $^
	$(CROSS_COMPILE)$(STRIP) $@

%.o: %.c
	$(CROSS_COMPILE)$(CC) $(CFLAGS) -fPIC -c $^ -I$(INC) -o $@


clean:
	-rm $(LIB) $(SHLIB) $(OBJECTS) tinyplay.o tinyplay tinycap.o tinycap \
	tinymix.o tinymix tinypcminfo.o tinypcminfo

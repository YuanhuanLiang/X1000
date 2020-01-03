#
# Cross compiler
CROSS_COMPILE ?= mips-linux-gnu-
CC := $(CROSS_COMPILE)gcc
STRIP := $(CROSS_COMPILE)strip
export CC STRIP

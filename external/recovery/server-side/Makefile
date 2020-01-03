TOPDIR := $(shell pwd)/
OUTDIR := $(TOPDIR)/out
IMAGE_SRC ?= $(TOPDIR)/image
RES_SRC := $(TOPDIR)/../resource
SECURITY := $(RES_SRC)/security
KEYNAME := testkey
SOURCE := pack.sh

all: $(SOURCE)
	@./$(SOURCE) $(OUTDIR) $(IMAGE_SRC) $(SECURITY) $(KEYNAME)

clean:
	@rm -rf $(OUTDIR)

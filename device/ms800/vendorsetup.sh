#!/bin/sh

# This file is executed by build/envsetup.sh, and can use anything
# defined in envsetup.sh.
#
# In particular, you can add lunch options with the add_lunch_combo
# function: add_lunch_combo generic-nand-eng


add_lunch_combo ms800-v10-norflash-eng
add_lunch_combo ms800-v10-spinand-eng


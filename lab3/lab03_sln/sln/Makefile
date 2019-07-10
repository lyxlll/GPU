################################################################################
#
# Recursive Makefile script for COM4521/COM6521
#
# Authors : Dr Mozhgan Kabiri Chimeh
#
# NOTICE TO USER:
#
# This is an example recursive Makefile for building a all exercises for a lab 
# class in COM4521/COM6521.
#
#
# Makefile only supported on Linux Platforms.
# 
# IMPORTANT: To use this Makefile on ShARC you MUST ensure you have loaded the 
# appropriate modules. e.g.
#
# module load dev/gcc/4.9.4
# module load libs/CUDA
#
# You do NOT need to be on a GPU node in order to build GPU code but you must be 
# on a GPU node to run the code you have built. See the module website for more
# information.
#
################################################################################

# Project folders
PROJECTS=Lab03_Exercise01 Lab03_Exercise02 Lab03_Exercise03 Lab03_Exercise04 Lab03_Exercise05 Lab03_Exercise06

.PHONY : all $(PROJECTS)
all: $(PROJECTS)

$(PROJECTS):
	$(MAKE) -C $@

# Clean
SUBCLEAN = $(addsuffix .clean,$(PROJECTS))

# PHONEY prevents make from doing something with a filename called clean
.PHONY : clean
clean: $(SUBCLEAN)

$(SUBCLEAN): %.clean :
	$(MAKE) -C $* clean

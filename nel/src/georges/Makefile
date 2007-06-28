#############################################################################
# Simple make file for compiling admin modules

#############################################################################
# Setting up the compiler settings...

# Global variables
include ../Variables.mk

# The flags for the C++ compiler
CXXFLAGS     = $(FLAGS_CMN) $(FLAGS_DBG_$(DBG)) \
		-I /usr/include/stlport \
		-I /usr/include/libxml2 \
		-I ../../include \
		-I.. \

# The flags for the linker
LDFLAGS      = 


#############################################################################
# The bit that changes each time we cut paste and hack this file :o)

# The list of targets to build
TARGETS		= libnelgeorges.a
DSP_TARGET	= georges.dsp

# The default build rule
all:			check-deps $(TARGETS)

libnelgeorges.a: $(OBJS)
	ar r ../../libnelgeorges.a $(OBJS)

#############################################################################
# Rules

include ../Rules.mk


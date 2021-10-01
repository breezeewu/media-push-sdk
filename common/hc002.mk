SHAWN_TOOLCHAIN=/home/zwu/sdk/mips-gcc540-glibc222-64bit-r3.3.0
CXX = $(SHAWN_TOOLCHAIN)/bin/mips-linux-uclibc-gnu-g++
CC = $(SHAWN_TOOLCHAIN)/bin/mips-linux-uclibc-gnu-gcc
AR = $(SHAWN_TOOLCHAIN)/bin/mips-linux-uclibc-gnu-ar
LD = $(SHAWN_TOOLCHAIN)/mips-linux-uclibc-gnu-ld
#CXX = $(SHAWN_TOOLCHAIN)/bin/mips-linux-gnu-g++
#CC = $(SHAWN_TOOLCHAIN)/bin/mips-linux-gnu-gcc
#AR = $(SHAWN_TOOLCHAIN)/bin/mips-linux-gnu-ar
#LD = $(SHAWN_TOOLCHAIN)/mips-linux-gnu-ld
PLATFORM = mips_hc002
#HC002 toolchain
SHAWN_TOOLCHAIN=/home/zwu/sdk/OpenWrt/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2
CXX = $(SHAWN_TOOLCHAIN)/bin/mipsel-openwrt-linux-uclibc-g++
CC = $(SHAWN_TOOLCHAIN)/bin/mipsel-openwrt-linux-uclibc-g++
AR = $(SHAWN_TOOLCHAIN)//bin/mipsel-openwrt-linux-uclibc-gcc-ar

PLATFORM = owt_hs004
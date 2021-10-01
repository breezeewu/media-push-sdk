SHAWN_TOOLCHAIN=/home/zwu/sdk/openwrt-musl/toolchain-mipsel_24kc_gcc-7.5.0_musl
CXX = $(SHAWN_TOOLCHAIN)/bin/mipsel-openwrt-linux-musl-g++
CC = $(SHAWN_TOOLCHAIN)/bin/mipsel-openwrt-linux-musl-g++
AR = $(SHAWN_TOOLCHAIN)//bin/mipsel-openwrt-linux-musl-ar

PLATFORM = openwrt_musl
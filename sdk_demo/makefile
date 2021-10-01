include config.mk

define make_path
		if [ ! -d $(1) ]; then\
			mkdir -p $(1); \
		fi
endef
#PLATFORM = mips_hs00x
$(warning  $(PLATFORM)) 
#include ../common/$(PLATFORM).mk
ifneq ($(PLATFORM), linux)
	LDFLAGS += -L./openssl/$(PLATFORM)/lib -lssl -lcrypto
else
	LDFLAGS += -lssl -lcrypto
	#LIBDIR = ../thirdpart/openssl/$(PLATFORM)/lib
endif
OBJDIR = ./obj/$(PLATFORM)
EXE_OBJS = $(OBJDIR)/demo.o
LDFLAGS += -L../lib -lsvpush -lstdc++  -lpthread -lrt -ldl -lm
CFLAGS := -Wall -fPIC -g -O0 -D__STDC_FORMAT_MACROS
#需要编译的exe名称
TARGET_NAME=demo
EXE_NAME = $(TARGET_NAME)
EXE_TARGET = ./$(EXE_NAME)
$(warning  $(EXE_TARGET))
$(warning  $(LDFLAGS))
$(warning  $(EXE_OBJS))
#第一个命令，makefile的终极目标，须使用tab键开始，此处将产生两个目标
all : $(EXE_TARGET)

$(EXE_TARGET) : $(EXE_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

#.o文件依赖的cpp，此处使用模式匹配%.o表示以.o结尾的任意文件
$(OBJDIR)/%.o:%.cpp
	$(call make_path, $(OBJDIR))
	#if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi;\
	$(CC) $(CFLAGS) -o $@ -c $<

.PHONY:clean
clean:
	rm -f $(EXE_TARGET) $(EXE_TARGET) $(OBJS)
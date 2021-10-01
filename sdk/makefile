include ../common/func.mk
ifeq ($(env), )
    env = lnx
else
endif

include ../common/$(env).mk

ifneq ($(PLATFORM), linux)
	INCDIR += ../thirdpart/openssl/$(PLATFORM)
	LIBDIR = ../thirdpart/openssl/$(PLATFORM)/lib
endif

INSTALLDIR = ../build/$(PLATFORM)_push_sdk
LDLIB := rtmp
#LDSO= -lssl -lcrypto -lpthread -lstdc++ -lrt -ldl -lm
LD_SO  := ssl crypto pthread stdc++ rt dl m
LIBDIR += $(INSTALLDIR)/lib ../thirdpart/librtmp/$(PLATFORM)/lib
SRCDIR = ./
INCDIR += ../common ../librtmp/
OBJDIR = ../obj/$(PLATFORM)
LD_LIB = $(foreach ldlib, $(LDLIB), -l$(ldlib))
LDSO = $(foreach so, $(LD_SO), -l$(so))
# CFLAGS中-I后跟的只能是包含头文件, -fPIC:告诉编译器产生与位置无关代码，-Wall，显示所有警告
INC_PATH = $(foreach incdir, $(INCDIR), -I$(incdir))
CFLAGS += -Wall -fPIC -g -O0 -D__STDC_FORMAT_MACROS -D__STDC_LIMIT_MACROS -std=c++11 $(INC_PATH)
LIB_PATH = $(foreach lib_dir, $(LIBDIR), -L$(lib_dir))
#-shared：编译动态库，-fPIC：告诉编译器产生与位置无关代码
LDFLAGS = $(LIB_PATH) $(LD_LIB) $(LDSO)
#设置源代码索-目录
VPATH = $(SRCDIR)
#需要编译的.o文件列表
PROTOCOL_OBJS = $(OBJDIR)/amf0any.o $(OBJDIR)/rawstream.o $(OBJDIR)/filewriter.o $(OBJDIR)/rtmpmsg.o $(OBJDIR)/rtmppacket.o $(OBJDIR)/rtmpprotocol.o
JSON_OBJS = $(OBJDIR)/json_reader.o $(OBJDIR)/json_value.o $(OBJDIR)/json_writer.o
#需要编译的.o文件列表
OBJS = $(OBJDIR)/basepush.o $(OBJDIR)/rtmppush.o $(OBJDIR)/pushmgr.o $(OBJDIR)/isvpush.o
LIB_OBJS=$(PROTOCOL_OBJS) $(OBJS) $(JSON_OBJS)
EXE_OBJS = $(OBJS) $(OBJDIR)/demo.o
#PROTOCOL_OBJS = $(OBJDIR)/amf0any.o $(OBJDIR)/bitstream.o $(OBJDIR)/rawstream.o $(OBJDIR)/rtmpmsg.o $(OBJDIR)/rtmppacket.o $(OBJDIR)/tcpsocket.o $(OBJDIR)/rtmpprotocol.o 
#JSON_OBJS = $(OBJDIR)/json_reader.o $(OBJDIR)/json_value.o $(OBJDIR)/json_writer.o
TARGET_NAME=svpush
#需要编译的lib库名称
LIB_NAME = lib$(TARGET_NAME).a
LIB_TARGET = $(INSTALLDIR)/lib/$(LIB_NAME)
#需要编译的so库名称
ifneq ($(PLATFORM), linux)
	SO_NAME = lib$(TARGET_NAME).so
	SO_TARGET = $(INSTALLDIR)/so/$(SO_NAME)
endif
#需要编译的exe名称
EXE_NAME = $(TARGET_NAME)
EXE_TARGET = ../$(EXE_NAME)
$(warning  $(LIB_OBJS))
#第一个命令，makefile的终极目标，须使用tab键开始，此处将产生两个目标
all : $(SO_TARGET) $(LIB_TARGET) $(EXE_TARGET)
#生成lib库
$(LIB_TARGET):$(LIB_OBJS)
	$(call make_path, $(INSTALLDIR)/lib/)
	#if [ ! -d $(INSTALLDIR)/lib/ ]; then mkdir -p $(INSTALLDIR)/lib/; fi;\
	$(AR) cr $@ $(OBJS)
	$(call make_path, $(INSTALLDIR)/include)
	cp ../common/isvpush.h $(INSTALLDIR)/include/
	$(AR) r $@ $^
	#$(AR) $@ $(OBJS)  -L../thirdpart/librtmp/$(PLATFORM)/lib/librtmp.a
#$(AR) rcs $(LIB_TARGET) $(LIB_TARGET)
# -lssl -lcrypto -lpthread -lrtmpprotocol 链接的lib库一定要放在-o$@ $^后面，否则编译的so库不会链接lib库，而是把它当作动态库进行链接
$(SO_TARGET):$(OBJS)
	$(call make_path, $(INSTALLDIR)/so/)
	$(CC) $(LDFLAGS) -shared -o $@ $^ $(LD_LIB)
#生产动态库，$@:目标集合, $^:所有依赖目标的集合, 会去除重复的依赖目标
$(EXE_TARGET) : $(EXE_OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)
#.o文件依赖的cpp，此处使用模式匹配%.o表示以.o结尾的任意文件
$(OBJDIR)/%.o:%.cpp
	$(call make_path, $(OBJDIR))
	#if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi;\
	$(CC) $(CFLAGS) -o $@ -c $<

#.PHONY是一个伪目标，可以防止在Makefile中定义的执行命令的目标和工作目录下的实际文件出现名字冲突，另一种是提高执行makefile时的效率
.PHONY:clean
clean:
	rm -f $(LIB_TARGET) $(SO_TARGET) $(EXE_TARGET) $(OBJS)
	#rm -f $(INSTALLDIR)/include/*.h $(INSTALLDIR)/lib/*.a $(INSTALLDIR)/lib/*.so

#安装
.PHONY:install
install:
	$(call make_path, $(INSTALLDIR)/demo)
	#@if test ! -d $(INSTALLDIR)/demo   ; then mkdir -p $(INSTALLDIR)/demo; fi
	cp ../common/$(env).mk $(INSTALLDIR)/demo/config.mk
	cp ./RecordMuxer.h $(INSTALLDIR)/demo/
	cp ../sdk_demo/demo.cpp $(INSTALLDIR)/demo/
	cp ../sdk_demo/makefile $(INSTALLDIR)/demo/
	cp ../data/demo.data $(INSTALLDIR)/demo/
	$(call make_path, $(INSTALLDIR)/demo/openssl/$(PLATFORM)/lib)
	#$(call make_path, $(INSTALLDIR)/demo/openssl/lib) 
	cp ../thirdpart/openssl/$(PLATFORM)/lib/*.so $(INSTALLDIR)/demo/openssl/$(PLATFORM)/lib/
	@if test ! -d $(INSTALLDIR)/doc   ; then mkdir -p $(INSTALLDIR)/doc; fi
	cp ../doc/Sunvalley云存储推流SDK接口文档V2.00.pdf $(INSTALLDIR)/doc/
include ../common/func.mk
include ../common/$(env).mk
#ifeq ($(env), hs004)
#	PLATFORM := owt_hs004
#else ifeq ($(env), owtm)
#	PLATFORM := openwrt_musl
#else ifeq ($(env), hmx)
#	PLATFORM := arm_himix200
#else ifeq ($(env), mips_hs005)
#	PLATFORM := mips_hs005
#else ifeq ($(env), mips_hs006)
#	PLATFORM := mips_hs006
#else ifeq ($(env), mips_hc002)
#	PLATFORM := mips_hc002
#else 
#	PLATFORM := linux
#endif

#	$(warning "begin: $(PLATFORM)")
INSTALLDIR = ../build/$(PLATFORM)
LIBDIR = $(INSTALLDIR)/lib  ../thirdpart/librtmp/$(PLATFORM)/lib/
# ../thirdpart/openssl/$(PLATFORM)/lib
SRCDIR = ./
INCDIR += ../common ../librtmp
# ../thirdpart/openssl/$(PLATFORM)/
OBJDIR = ../obj/$(PLATFORM)
LDSO= -lssl -lcrypto -lpthread -lstdc++ -lrt -ldl -lm
# CFLAGS中-I后跟的只能是包含头文件, -fPIC:告诉编译器产生与位置无关代码，-Wall，显示所有警告
INC_PATH = $(foreach incdir, $(INCDIR), -I$(incdir))
SO_DIR= $(foreach sodir, $(SODIR), -L$(sodir))
CFLAGS += -Wall -fPIC $(INC_PATH) -std=c++11 -g -o0
LIB_PATH = $(foreach libdir, $(LIBDIR), -L$(libdir))
LDLIB = -lsvpush -lrtmp
#-shared：编译动态库，-fPIC：告诉编译器产生与位置无关代码
LDFLAGS = $(SO_DIR) $(LDSO)
#设置源代码索-目录
VPATH = $(SRCDIR)
#需要编译的.o文件列表
OBJS = $(OBJDIR)/cloud_sunvalley.o $(OBJDIR)/aesencrypt.o $(OBJDIR)/demo_main.o $(OBJDIR)/cputotol.o $(OBJDIR)/rec_demux.o

TARGET_NAME=Ppcs_clouddemo
#需要编译的lib库名称
LIB_NAME = $(TARGET_NAME).a
LIB_TARGET = ./lib/$(LIB_NAME)
#需要编译的so库名称
SO_NAME = $(TARGET_NAME).so
SO_TARGET = ./lib/$(TARGETNAME)

#需要编译的exe名称
EXE_NAME = $(TARGET_NAME)
EXE_TARGET = ../$(EXE_NAME)
#第一个命令，makefile的终极目标，须使用tab键开始，此处将产生两个目标
all : $(EXE_TARGET)
#生产动态库，$@:目标集合, $^:所有依赖目标的集合, 会去除重复的依赖目标
$(EXE_TARGET) : $(OBJS)
	$(CC) $(LDFLAGS) -o $@ $^ $(LIB_PATH) $(LDLIB)
#-lssl -lcrypto -lpthread -L../SDK/ -lsvpushlnx -lstdc++ -lrt -ldl -lm
#生成lib库
$(LIB_TARGET):$(OBJS)
	$(AR) cr $@ $(OBJS)

#.o文件依赖的cpp，此处使用模式匹配%.o表示以.o结尾的任意文件
$(OBJDIR)/%.o:%.c
	$(call make_path, $(OBJDIR))
	#if [ ! -d $(OBJDIR) ]; then mkdir -p $(OBJDIR); fi;\
	$(CC) $(CFLAGS) -o $@ -c $<

#.PHONY是一个伪目标，可以防止在Makefile中定义的执行命令的目标和工作目录下的实际文件出现名字冲突，另一种是提高执行makefile时的效率
.PHONY:clean
clean:
	rm -f $(TARGET) $(LIBTARGET) $(OBJDIR)/*.o $(OBJDIR)/*.d $(OBJDIR)/*.d.*
#	rm -f $(INSTALLDIR)/include/*.h $(INSTALLDIR)/lib/*.a $(INSTALLDIR)/lib/*.so

#安装
.PHONY:install
install:
	@if test ! -d $(INSTALLDIR)   ; then mkdir -p $(INSTALLDIR); fi
	@if test ! -d $(INSTALLDIR)/include   ; then mkdir -p $(INSTALLDIR)/include; fi
	@if test ! -d $(INSTALLDIR)/lib   ; then mkdir -p $(INSTALLDIR)/lib; fi
	cp $(INCDIR)/BPMDetect.h $(INSTALLDIR)/include/
	cp $(INCDIR)/FIFOSampleBuffer.h $(INSTALLDIR)/include/
	cp $(INCDIR)/FIFOSamplePipe.h $(INSTALLDIR)/include/
	cp $(INCDIR)/SoundTouch.h $(INSTALLDIR)/include/
	cp $(INCDIR)/soundtouch_config.h $(INSTALLDIR)/include/
	cp $(INCDIR)/STTypes.h $(INSTALLDIR)/include/
	cp $(LIBTARGET)  $(INSTALLDIR)/lib/
	cp $(TARGET)  $(INSTALLDIR)/lib/

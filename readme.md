# rtmp push sdk

## 1.sdk简介：
	
	sunvalley云存储嵌入式操作系统下的rtmp推流sdk，用于将设备端的媒体数据流以rtmp协议推流到流媒体服务器

## 2.编译流程

### 1.编译linux下的sdk和Ppcs_clouddemo

./build_linux.sh

编译完成之后生成的Ppcs_clouddemo在当前目录，下，使用./data/目录下的record模拟摄像头移动侦测触发进行推流

### 2.编译交叉编译的so库

在conf中准备交叉编译链配置文件$env.mk

编译： sh build_sdk.sh hs004（对应conf中的hs004.mk文件的交叉编译链）

编译完成后将在build文件夹下生成如下文件：

./build/$env/so 目录下生成libsvpush.so

./build/$env/include 目录下生成isvpush.h

./build/$env/lib 目录下生成libsvpush.a

./build/$env/demo/ 目录下生成交叉编译链对应的推流接口调用示例demo例码

./build/$env/doc 推流SDK接口相关的说明文档

脚本执行到最后要求输出root密码，执行拷贝so库的操作，可以忽略！

## 3.编译运行demo

进入./build/$env/demo目录，编译demo
make
编译完成后，生成一个名为demo的可执行程序，运行demo，即可进行连接推流
./demo


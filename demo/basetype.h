#ifndef _BASE_TYPE_H_
#define _BASE_TYPE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdarg.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <netdb.h>
#include <termios.h> 
#include <semaphore.h>
#include <syslog.h>
#include <pthread.h>
#include <netdb.h>
#include <setjmp.h>
#include <sys/msg.h> 
#include <sys/ioctl.h>
#include <sys/ipc.h>   
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <sys/vfs.h>
#include <sys/reboot.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <netinet/ip_icmp.h>
#include <net/if.h>
#include <net/if_arp.h>
#include <arpa/inet.h>
#include <asm/types.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <linux/sysinfo.h>
#include <linux/netlink.h>

#include "isvpush.h"

#define STACK_SIZE			50000
//#define MAX_CHANNEL_NUM		100
extern int	g_cur_push_channel_num;
#define AV_BUFF_SIZE		300000

#define RTMP_PUSH_STREAM_BY_DEVICE_SN
#define RTMP_PUSH_URL			"rtmp://127.0.0.1:19350/live"
#define TOKEN_SERVER_URL		"https://storage-sit.sunvalleycloud.com/ipc/connection/token/get"
//#define TOKEN_SERVER_URL		"http://10.30.0.206:2004/ipc/connection/token/get"
//#define TOKEN_SERVER_URL		"http://10.30.0.206:2118/connection/token/get"
//#define TOKEN_SERVER_URL "http://10.30.0.201:2118/connection/token/get"
//pair info
typedef struct _pairinfo{
	int nstat;						//
	int lock;						//
	int ipaddr;						// IP addr
	char mac[18];					// mac addr
	char sn[32];					// IPC camera sn
	char token[33];					// IPC camera token
}VAVA_PairInfo;

//record frame info
typedef struct _rechead{
	unsigned int tag;				// magic number, 0xEB0000AA
	unsigned int size;				// frame size
	unsigned int type;				// 0:P frame, 1:I frame, 8:audio frame
	unsigned int fps;				// frame per second
	unsigned int time_sec;			// frame pts, second
	unsigned int time_usec;			// frame pts, microsecond
}VAVA_RecHead;

// record main info
typedef struct _recinfo{
	char tag;						//magic number
	char v_encode;					//video encoder type
	char a_encode;					// audio encoder type
	char res;						// video resolution
	char fps;						// frame per second
	char encrypt;					// encrypt flag
	unsigned short vframe;			// video frame count
	int size;						// record size
	int time;						// record duration
}VAVA_RecInfo;

extern int g_running;

#endif


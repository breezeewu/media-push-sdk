#include "basetype.h"
#include "cputotol.h"

#define VMRSS_LINE 			17
#define VMSIZE_LINE 		13
#define PROCESS_ITEM 		14

typedef struct {
	unsigned long user;
	unsigned long nice;
	unsigned long system;
	unsigned long idle;
}Total_Cpu_Occupy_t;

typedef struct {
	unsigned int pid;
	unsigned long utime;  //user time
	unsigned long stime;  //kernel time
	unsigned long cutime; //all user time
        unsigned long cstime; //all dead time
}Proc_Cpu_Occupy_t;
#define COMPARE_STRING(str1, str2)		0 ==  memcmp(str1, str2, strlen(str2))
#define VMPEAK	  	"VmPeak"
#define VMSIZE   	"VmSize"
#define VMHWM	  	"VmHWM"
#define VMRSS     	"VmRSS"
#define VMSTK		"VmStk"
#define VMEXE		"VmExe"
#define VMLIB		"VmLib"
#define VMDATA		"VmData"
unsigned int get_proc_mem(int pid, int *vmpeak, int *vmem, int *mempeak, int *mem, int *mdata, int *stk, int *exe, int *lib)
{
	FILE *fd = NULL;
	//int line = 0;
	char name[64];
	char file_name[64];
	char line_buff[512];
	
	sprintf(file_name, "/proc/%d/status", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
		printf("open %s failed!", file_name);
		return 0;
	}

	while(1)
	{
		//line++;
		
		memset(line_buff, 0, 512);
		if(fgets(line_buff, 512, fd) == NULL)
		{
			break;
		}
		if(vmpeak && COMPARE_STRING(line_buff, VMPEAK))
		{
			
			sscanf(line_buff, "%s %d", name, vmpeak);
			//printf("%s:%d", VMPEAK, *vmpeak);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMSIZE))
		{
			sscanf(line_buff, "%s %d", name, vmem);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMHWM))
		{
			sscanf(line_buff, "%s %d", name, mempeak);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMRSS))
		{
			sscanf(line_buff, "%s %d", name, mem);
		}
		else if(mdata && COMPARE_STRING(line_buff, VMDATA))
		{
			sscanf(line_buff, "%s %d", name, mdata);
		}
		else if(stk && COMPARE_STRING(line_buff, VMSTK))
		{
			sscanf(line_buff, "%s %d", name, stk);
		}
		else if(vmem && COMPARE_STRING(line_buff, VMEXE))
		{
			sscanf(line_buff, "%s %d", name, exe);
		}
		else if(lib && COMPARE_STRING(line_buff, VMLIB))
		{
			sscanf(line_buff, "%s %d", name, lib);
		}
		
		/*switch(line)
		{
			case VMPEAK_LINE:
				sscanf(line_buff, "%s %d", name, vmemmax);
				break;
			case VMSIZE_LINE:
				sscanf(line_buff, "%s %d", name, vmem);
				break;
			case VMHWM_LINE:
				sscanf(line_buff, "%s %d", name, memmax);
				break;
			case VMRSS_LINE:
				sscanf(line_buff, "%s %d", name, mem);
				break;
			case VMDATA_LINE:
				sscanf(line_buff, "%s %d", name, mdata);
				break;
			case VMSTK_LINE:
				sscanf(line_buff, "%s %d", name, stk);
				break;
			default:
				break;
		}

		if(line >= VMSTK_LINE)
		{
			break;
		}*/
	}

	fclose(fd);
	fd = NULL;
	
	return 0;
}
/*
//��ȡ����ռ�������ڴ�
unsigned int get_proc_virtualmem(unsigned int pid)
{
	FILE *fd = NULL;
	int i;
	int vmsize;
	char file_name[64];
	char line_buff[512];
	char name[64];

	memset(file_name, 0, 64);
	sprintf(file_name, "/proc/%d/status", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
		return 0;
	}
	do
	{
		memset(line_buff, 0, 512);
		if(!fgets(line_buff, 512, fd))
		{
			fclose(fd);
			return -1;
		}
		if(0 == memcmp(line_buff, VMSIZE, strlen(VMSIZE)))
		{
			break;
		}
	}
	while(1);

	//memset(line_buff, 0, 512);
	//fgets(line_buff, 512, fd);
	fclose(fd);
	
	sscanf(line_buff, "%s %d", name, &vmsize);
	
	return vmsize;
}

//��ȡ����ռ���ڴ�
unsigned int get_proc_mem(unsigned int pid)
{
	FILE *fd = NULL;
	int i;
	int vmrss;
	char name[64];
	char file_name[64];
	char line_buff[512];
	
	sprintf(file_name, "/proc/%d/status", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
		return -1;
	}
	do
	{
		memset(line_buff, 0, 512);
		if(!fgets(line_buff, 512, fd))
		{
			fclose(fd);
			return -1;
		}
		if(0 == memcmp(line_buff, VMRSS, strlen(VMRSS)))
		{
			break;
		}
	}
	while(1);
	fclose(fd);
	sscanf(line_buff, "%s %d", name, &vmrss);

	return vmrss;
}*/

//��ȡ�ܵ�CPUʱ��
unsigned long get_cpu_total_occupy()
{
	FILE *fd = NULL;
	char buff[1024];
	char name[64];
	Total_Cpu_Occupy_t t;
 
	fd = fopen("/proc/stat" , "r");
	if(fd == NULL)
	{
		return 0;
	}

	memset(buff, 0, 1024);
	fgets(buff, sizeof(buff), fd);
	fclose(fd);
	
	memset(name, 0, 64);
	sscanf(buff, "%s %ld %ld %ld %ld", name, &t.user, &t.nice, &t.system, &t.idle);

	return (t.user + t.nice + t.system + t.idle);
}

//��ȡ��N�ʼ��ָ��
const char* get_items(const char*buffer, unsigned int item)
{
	const char *p = buffer;
 	int i;
	int len; 
	int count = 0;

	len= strlen(buffer);
	
	for(i = 0; i < len; i++)
	{
		if(' ' == *p)
		{
			count ++;
			if(count == item -1)
			{
				p++;
				break;
			}
		}
		
		p++;
	}
 
	return p;
}

//��ȡ���̵�CPUʱ��
unsigned long get_cpu_proc_occupy(unsigned int pid)
{
	char file_name[64];
	Proc_Cpu_Occupy_t t;
	FILE *fd = NULL;
	const char *q = NULL;
	char line_buff[1024];
	
	sprintf(file_name, "/proc/%d/stat", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
		return 0;
	}

	fgets(line_buff,sizeof(line_buff),fd);
	fclose(fd);
	
	sscanf(line_buff, "%u", &t.pid);
	q = get_items(line_buff, PROCESS_ITEM);
	sscanf(q, "%ld %ld %ld %ld", &t.utime, &t.stime, &t.cutime, &t.cstime);

	return (t.utime + t.stime + t.cutime + t.cstime);
}

float get_proc_cpu(unsigned int pid)
{
	float cpu = 0.0;
	
	unsigned long totalcputime1;
	unsigned long totalcputime2;
	unsigned long procputime1;
	unsigned long procputime2;

	totalcputime1 = get_cpu_total_occupy();
	procputime1 = get_cpu_proc_occupy(pid);

	sleep(1);
	//usleep(200000);

	totalcputime2 = get_cpu_total_occupy();
	procputime2 = get_cpu_proc_occupy(pid);

	if(0 != totalcputime2-totalcputime1)
	{ 
		cpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
	}
	
	return cpu;

}

int get_pid(char *name)
{
	FILE *pstr = NULL;
	char cmd[256];
	char buf[128];

	memset(cmd, 0, 256);
	sprintf(cmd, "pgrep %s", name);

	pstr = popen(cmd, "r");	
	if(pstr == NULL)
	{
		return 0;
	}

	memset(buf, 0, 128);
	fgets(buf, 128, pstr);
	pclose(pstr);

	return atoi(buf);
}

void *Cputotol_pth(void *data)
{
	unsigned int pid = 0;
	return NULL;
	pid = getpid();//get_pid("Ppcs_clouddemo");

	while(g_running)
	{
		sleep(1);
		//usleep(300000);
		int vmpeak = 0, vmmem = 0, peakmem = 0, mem = 0, datamem = 0, stackmem = 0, exemem = 0, libmem = 0;
		get_proc_mem(pid, &vmpeak, &vmmem, &peakmem, &mem, &datamem, &stackmem, &exemem, &libmem);
		printf("pid:%d, cpu = %.2f%%, vmmem:%d[%d], mem:%d[%d], data:%d, stk:%d\n", pid, get_proc_cpu(pid), vmmem, vmpeak, mem, peakmem, datamem, stackmem);
	}
	
	return NULL;
}

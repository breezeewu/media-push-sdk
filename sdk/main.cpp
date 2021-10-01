#include <stdio.h>
#include <vector>
#include <signal.h>
#include "push_task.hpp"
#include "lbsc_util_conf.h"


#define PUSH_LOG_PATH				"log_file"
#define PUSH_LOG_LEVEL				"log_level"
#define PUSH_LOG_OUTPUT_MODE		"log_output_mode"
#define	TOKEN_SERVER_URL			"token_server"
#define RECORD_PATH					"record_path"
#define DEVICE_LIST_FILE			"device_list_file"
#define MAX_CFG_CHANNEL_NUM			"max_channel_num"
#define TIGGER_RANDOM_TIME			"tigger_random_time"
#define PUSH_SERVER_URL				"push_server_url"
#define SEND_FRAME_INTERVAl			"send_frame_interval"
#define TIGGER_TYPE					"tigger_type"
typedef struct push_config_context
{
    char*       plog_file_path;
    char*       ptoken_server_url;
    char*       pdefault_rec_path;
    char*       pdevice_list_file;
	char*		ppush_server_url;
    int         nlog_level;
    int         nlog_output_mode;
    int         nmax_channel_num;
    int         ntigger_random_sec;
	int			nsend_frame_interval;
	int			ntigger_type;

}   push_cfg;

//int     g_ncur_channel_num(0);

typedef struct _pairinfo{
	int nstat;						//
	int lock;						//
	int ipaddr;						// IP addr
	char mac[18];					// mac addr
	char sn[32];					// IPC camera sn
}VAVA_PairInfo;
struct push_info
{
	int npush_num;
	char* ppush_conf;
	int ntigger_type;
	int ntigger_random_time;
};
push_info* g_ppush_info = NULL;

push_config_context *g_ppush_cfg(NULL);
bool g_brunning = false;
VAVA_PairInfo*  g_ppair_info(NULL);

int on_read_conf(void* powner, const char* ptag, const char* pvalue)
{
	if(NULL == powner || NULL == ptag || NULL == pvalue)
	{
		lberror("Invalid parameter, powner:%p, ptag:%p, pvalue:%p\n", powner, ptag, pvalue);
		return -1;
	}
	lbtrace("on_read_conf(powner:%p, ptag:%s, pvalue:%s)\n", powner, ptag, pvalue);
	int len = strlen(ptag);
	int val_len = strlen(pvalue) + 1;

	push_config_context* pcfg = (push_config_context*)powner;
	if(memcmp(ptag, PUSH_LOG_PATH, len) == 0)
	{
		pcfg->plog_file_path = (char*)malloc(val_len);
		memcpy(pcfg->plog_file_path, pvalue, val_len);
	}
	else if(memcmp(ptag, PUSH_LOG_LEVEL, len) == 0)
	{
		pcfg->nlog_level = atoi(pvalue);
	}
	else if(memcmp(ptag, PUSH_LOG_OUTPUT_MODE, len) == 0)
	{
		pcfg->nlog_output_mode = atoi(pvalue);
	}
	else if(memcmp(ptag, TOKEN_SERVER_URL, len) == 0)
	{
		pcfg->ptoken_server_url = (char*)malloc(val_len);
		memcpy(pcfg->ptoken_server_url, pvalue, val_len);
	}
	else if(memcmp(ptag, RECORD_PATH, len) == 0)
	{
		pcfg->pdefault_rec_path = (char*)malloc(val_len);
		memcpy(pcfg->pdefault_rec_path, pvalue, val_len);
	}
	else if(memcmp(ptag, DEVICE_LIST_FILE, len) == 0)
	{
		pcfg->pdevice_list_file = (char*)malloc(val_len);
		memcpy(pcfg->pdevice_list_file, pvalue, val_len);
		printf("pcfg->pdevice_list_file:%s\n", pcfg->pdevice_list_file);
	}
	else if(memcmp(ptag, MAX_CFG_CHANNEL_NUM, len) == 0)
	{
		pcfg->nmax_channel_num = atoi(pvalue);
	}
	else if(memcmp(ptag, TIGGER_RANDOM_TIME, len) == 0)
	{
		pcfg->ntigger_random_sec = atoi(pvalue);
	}
	else if(memcmp(ptag, PUSH_SERVER_URL, len) == 0)
	{
		pcfg->ppush_server_url = (char*)malloc(val_len);
		memcpy(pcfg->ppush_server_url, pvalue, val_len);
	}
	else if(memcmp(ptag, SEND_FRAME_INTERVAl, len) == 0)
	{
		pcfg->nsend_frame_interval = atoi(pvalue);
	}
	else if(memcmp(ptag, TIGGER_TYPE, len) == 0)
	{
		pcfg->ntigger_type = atoi(pvalue);
	}
    else
    {
        printf("unknown ptag:%s, value:%s\n", ptag, pvalue);
        return 0;
    }
    
	return 0;
}

int init_pair_from_sn_file(const char* psnpath)
{
	if(NULL == psnpath)
	{
		printf("Invalid sn file path:%s\n", psnpath);
		return -1;
	}
	int i = 0;
	FILE* fd = fopen(psnpath, "rb");
	printf("fd:%p = fopen(psnpath:%s, rb)\n", fd, psnpath);
	if(fd <= 0)
	{
		printf("open sn file failed, fd:%d, psnpath:%s\n", fd, psnpath);
		return -1;

	}
    if(NULL == g_ppair_info)
    {
        g_ppair_info = new VAVA_PairInfo[g_ppush_cfg->nmax_channel_num];
    }
	memset(g_ppair_info, 0, sizeof(VAVA_PairInfo) * g_ppush_cfg->nmax_channel_num);
	while(1)
	{
		char line_buff[1024];
		printf("while begin\n");
		if(fgets(line_buff, 512, fd) == NULL)
		{
			g_ppush_info->npush_num = i;
			printf("fgets(line_buff:%s, 512, fd:%p) == NULL\n", line_buff, fd);
			break;
		}
		if(i >= g_ppush_info->npush_num)
		{
			printf("i:%d >= g_ppush_info->npush_num:%d\n", i, g_ppush_info->npush_num);
			break;
		}
		g_ppair_info[i].nstat = 1;
		int len = strlen(line_buff);
		char* pent = strstr(line_buff, "\r\n");
		memset(pent, 0, sizeof("\r\n"));
		strcpy(g_ppair_info[i++].sn, line_buff);
		printf("sn:%d:%s\n", i, line_buff);
	};
	
	printf("total read %d sn from file!", i);
	if(fd)
	{
		fclose(fd);
		fd = 0;
	}
}

int load_push_config(const char* pcfg_path)
{
    //assert(0);
	printf("load_push_config:%s\n", pcfg_path);
	if(NULL == pcfg_path)
	{
		printf("Invalid config path:%p\n", pcfg_path);
		return -1;
	}
	g_ppush_cfg = (push_cfg*)malloc(sizeof(push_cfg));
	memset(g_ppush_cfg, 0, sizeof(push_cfg));
	g_ppush_cfg->nmax_channel_num = 100;
	g_ppush_cfg->ntigger_random_sec = 50;
	g_ppush_cfg->nlog_level = 3;
	g_ppush_cfg->nlog_output_mode = 3;
	int ret = load_config(pcfg_path, g_ppush_cfg, on_read_conf);
    //assert(0);
    printf("log_path:%s log_level:%d, log_output_mode:%d\n", g_ppush_cfg->plog_file_path, g_ppush_cfg->nlog_level, g_ppush_cfg->nlog_output_mode);
    return ret;
}
void sig_stop(int signo)
{
	printf("sig_stop: signo = %d\n", signo);
	g_brunning = false;
	return;
}
CCriSec   CPushTask::m_crisec;

std::map<long, CPushTask*>   CPushTask::m_mconnect_task_list;


void usage()
{
    printf("usage:\n");
    printf("-n: push channel count\n");
    printf("-c: push.conf path\n");
    printf("-r: tigger random time\n");
    printf("-t: tigger type\n");
    printf("-v: help, show usage\n");
}

int on_parser_option(void* powner, const char* ptag, const char* popt_val)
{
	printf("powner:%p, ptag:%s, opt_val:%s\n", powner, ptag, popt_val);
	if(NULL == ptag || NULL == popt_val || NULL == powner)
	{
		printf("Invalid args paramter ptag:%p, popt_val:%p, powner:%p\n", ptag, popt_val, powner);
		return -1;
	}
	push_info* ppc = (push_info*)powner;
	// if ch is upper char, convert to lower char
	char ch = ptag[1];
	if(ch >= 65 && ch <= 90)
	{
		ch += 32;
	}

	switch(ch)
	{
		case 'n':
			ppc->npush_num = atoi(popt_val);
			printf("ppc->npush_num:%d\n", ppc->npush_num);
		break;
		case 'c':
		{
			int len = strlen(popt_val)+1;
			ppc->ppush_conf = (char*)malloc(len);
			memcpy(ppc->ppush_conf, popt_val, len);
			printf("ppc->ppush_conf:%d\n", ppc->ppush_conf);
		}
		break;
		case 'r':
			ppc->ntigger_type = atoi(popt_val);
			printf("ppc->ntigger_type:%d\n", ppc->ntigger_type);
		break;
		case 't':
			ppc->ntigger_random_time = atoi(popt_val);
			printf("ppc->ntigger_random_time:%d\n", ppc->ntigger_random_time);
		break;
		case 'h':
			usage();
		break;
		default:
			return -1;
		break;
	}

	return 0;
}
/*int parser_args(char** args, int argc, push_info* ppi)
{
    int ret = 0;
    assert(ppi);
    memset(ppi, 0, sizeof(push_info));
    int i;
    for(i = 1; i < argc; i++)
    {
		if('-' == args[i][0] && i + 1 < argc)
		{
			parser_option(args[i][1], args[i+1], ppi);
			i++;
		}
    }

    return ret;
}*/

int main(int argc, char** args)
{
	g_ppush_info = (push_info*)malloc(sizeof(push_info));
	parser_args(args, argc, g_ppush_info, on_parser_option);
	if(NULL == g_ppush_info->ppush_conf)
	{
		printf("please specify push conf file!\n");
		return -1;
	}


    std::vector<CPushTask*> push_task_list;
    char ver[256] = {0};
    SVPush_API_Version(ver, 256);
    printf(ver);
    printf("\n task number:%d\n", g_ppush_info->npush_num);
    load_push_config("push.conf");
    //return 0;
    if(NULL == g_ppush_cfg)
    {
        assert(0);
        return -1;
    }

    init_pair_from_sn_file(g_ppush_cfg->pdevice_list_file);
	printf("init_pair_from_sn_file(g_ppush_cfg->pdevice_list_file:%s)\n", g_ppush_cfg->pdevice_list_file);
    if(NULL == g_ppair_info)
    {
        assert(0);
        return 0;
    }
    int ret = SVPush_API_Initialize(1024*1024, g_ppush_cfg->nmax_channel_num);
    CHECK_RESULT(ret);

    ret = SVPush_API_Init_log((E_SV_LOG_LEVEL)g_ppush_cfg->nlog_level, g_ppush_cfg->nlog_output_mode, g_ppush_cfg->plog_file_path);
    CHECK_RESULT(ret);

    ret = SVPush_API_Set_Token_Server_Interface(g_ppush_cfg->ptoken_server_url);
    CHECK_RESULT(ret);
    g_brunning = true;
    signal(SIGINT,  sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);
	printf("g_ppush_info->npush_num:%d\n", g_ppush_info->npush_num);
    for(int i = 0; i < g_ppush_info->npush_num; i++)
    {
        CPushTask* ptask = new CPushTask();
        char url[256] = {0};
        printf(url, "%s%s", g_ppush_cfg->ppush_server_url, g_ppair_info[i].sn);
        ret = ptask->set_device_sn(g_ppair_info[i].sn);
		printf("ret:%d = ptask->set_device_sn(sn:%s)\n", ret, g_ppair_info[i].sn);
        ret = ptask->open_camera(g_ppush_cfg->pdefault_rec_path, g_ppush_cfg->ntigger_type, 0, g_ppush_cfg->nsend_frame_interval);
		printf("ret:%d = ptask->open_camera(g_ppush_cfg->pdefault_rec_path:%s, g_ppush_cfg->ntigger_type:%d, 0, g_ppush_cfg->nsend_frame_interval:%d)\n",  ret, g_ppush_cfg->pdefault_rec_path, g_ppush_cfg->ntigger_type, g_ppush_cfg->nsend_frame_interval);
        ret = ptask->start(0);
		printf("ret:%d = ptask->start(0)\n", ret);
        push_task_list.push_back(ptask);
    }

    do
    {
       usleep(200000);
    } while (g_brunning);
	printf("after while\n");
    for(int i = 0; i < g_ppush_info->npush_num; i++)
    {
        CPushTask* ptask = push_task_list[i];
        ptask->stop();
        delete ptask;
    }
    push_task_list.clear();
    SVPush_API_UnInitialize();
    free(g_ppush_cfg);
    g_ppush_cfg = NULL;
    return 0;
}
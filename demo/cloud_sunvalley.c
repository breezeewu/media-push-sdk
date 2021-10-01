//#include "basetype.h"
#include "basetype.h"
#include "cloud_sunvalley.h"
#include "lbsc_util_conf.h"
#include "rec_demux.h"


#define PUSH_LOG_PATH				"log_file"
#define PUSH_LOG_LEVEL				"log_level"
#define PUSH_LOG_OUTPUT_MODE		"log_output_mode"
#define	TOKEN_SERVER_URL			"token_server"
#define LIVE_TOKEN_SERVER_URL		"live_token_server"
#define	PUSH_SERVER_URL				"push_server_url"
#define RECORD_PATH					"record_path"
#define DEVICE_LIST_FILE			"device_list_file"
#define TOKEN_LIST_FILE				"token_list_file"
#define MAX_CFG_CHANNEL_NUM			"max_channel_num"
#define TIGGER_RANDOM_TIME			"tigger_random_time"
#define TIGGER_TYPE					"tigger_type"
#define ENCRYPT_FLAG				"encrypt_flag"

#define AUTH_LOGIN_SERVER_URL			"login_server"
#define AUTH_CLIENT_ID					"client_id"
#define AUTH_CLIENT_SECRET				"client_secret"
#define AUTH_SCOPE						"scope"
#define AUTH_GRANT_TYPE					"grant_type"
#define AUTH_TYPE						"auth_type"
#include <assert.h>

int on_read_conf(void* powner, const char* ptag, const char* pvalue)
{
	if(NULL == powner || NULL == ptag || NULL == pvalue)
	{
		lberror("Invalid parameter, powner:%p, ptag:%p, pvalue:%p\n", powner, ptag, pvalue);
		return -1;
	}
	//printf("on_read_conf(powner:%p, ptag:%s, pvalue:%s)\n", powner, ptag, pvalue);
	int len = strlen(ptag);
	int val_len = strlen(pvalue) + 1;

	push_cfg* pcfg = (push_cfg*)powner;
	if(memcmp(ptag, PUSH_LOG_PATH, len) == 0)
	{
		//int i = 0;
		pcfg->plog_file_path = (char*)malloc(val_len);
		memcpy(pcfg->plog_file_path, pvalue, val_len);
		/*printf("after memcpy pcfg->plog_file_path:%s\n", pcfg->plog_file_path);
		FILE* pfile = fopen(pcfg->plog_file_path, "wb");
		printf("pfile:%p = fopen(pcfg->plog_file_path:%s, wb", pfile, pcfg->plog_file_path);
		for(i = 0; pcfg->plog_file_path[i] != 0; i++)
		{
			printf("%2x", pcfg->plog_file_path[i]);
		}
		assert(0);*/
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
	else if(memcmp(ptag, PUSH_SERVER_URL, len) == 0)
	{
		pcfg->ppush_server_url = (char*)malloc(val_len);
		memcpy(pcfg->ppush_server_url, pvalue, val_len);
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
		printf("device list file:%s\n", pcfg->pdevice_list_file);
	}
	else if(memcmp(ptag, LIVE_TOKEN_SERVER_URL, len) == 0)
	{
		pcfg->plive_token_server_url = (char*)malloc(val_len);
		memcpy(pcfg->plive_token_server_url, pvalue, val_len);
		printf("device list file:%s\n", pcfg->plive_token_server_url);
	}
	else if(memcmp(ptag, TOKEN_LIST_FILE, len) == 0)
	{
		pcfg->ptoken_list_file = (char*)malloc(val_len);
		memcpy(pcfg->ptoken_list_file, pvalue, val_len);
		printf("token list file:%s\n", pcfg->ptoken_list_file);
	}
	else if(memcmp(ptag, MAX_CFG_CHANNEL_NUM, len) == 0)
	{
		pcfg->nmax_channel_num = atoi(pvalue);
	}
	else if(memcmp(ptag, TIGGER_RANDOM_TIME, len) == 0)
	{
		pcfg->ntigger_random_sec = atoi(pvalue);
	}
	else if(memcmp(ptag, TIGGER_TYPE, len) == 0)
	{
		pcfg->ntigger_type = atoi(pvalue);
	}
	else if(memcmp(ptag, AUTH_LOGIN_SERVER_URL, len) == 0)
	{
		if(NULL == pcfg->pai)
		{
			pcfg->pai = (struct auth_info*)malloc(sizeof(struct auth_info));
			memset(pcfg->pai, 0, sizeof(struct auth_info));
		}
		pcfg->pai->pauth_url = (char*)malloc(val_len);
		memcpy(pcfg->pai->pauth_url, pvalue, val_len);
		printf("pauth_url url:%s\n", pcfg->pai->pauth_url);
	}
	else if(memcmp(ptag, AUTH_CLIENT_ID, len) == 0 && pcfg->pai)
	{
		pcfg->pai->pclient_id = (char*)malloc(val_len);
		memcpy(pcfg->pai->pclient_id, pvalue, val_len);
		printf("client_id:%s\n", pcfg->pai->pclient_id);
	}
	else if(memcmp(ptag, AUTH_CLIENT_SECRET, len) == 0 && pcfg->pai)
	{
		pcfg->pai->pclient_secret = (char*)malloc(val_len);
		memcpy(pcfg->pai->pclient_secret, pvalue, val_len);
		printf("client_secret url:%s\n", pcfg->pai->pclient_secret);
	}
	else if(memcmp(ptag, AUTH_SCOPE, len) == 0 && pcfg->pai)
	{
		pcfg->pai->pscope = (char*)malloc(val_len);
		memcpy(pcfg->pai->pscope, pvalue, val_len);
		printf("scope:%s\n", pcfg->pai->pscope);
	}
	else if(memcmp(ptag, AUTH_GRANT_TYPE, len) == 0 && pcfg->pai)
	{
		pcfg->pai->pgrant_type = (char*)malloc(val_len);
		memcpy(pcfg->pai->pgrant_type, pvalue, val_len);
		printf("grant_type url:%s\n", pcfg->pai->pgrant_type);
	}
	else if(memcmp(ptag, AUTH_TYPE, len) == 0 && pcfg->pai)
	{
		pcfg->pai->pauth_type = (char*)malloc(val_len);
		memcpy(pcfg->pai->pauth_type, pvalue, val_len);
		printf("auth_type:%s\n", pcfg->pai->pauth_type);
	}
	else if(memcmp(ptag, ENCRYPT_FLAG, len) == 0)
	{
		printf("%d = memcmp(pvalue:%s, off, val_len:%d)\n", memcmp(pvalue, "off", val_len), pvalue,  val_len);
		if(memcmp(pvalue, "OFF", val_len) == 0 || memcmp(pvalue, "off", val_len) == 0)
		{
			pcfg->enc_flag = 0;
		}
		else
		{
			pcfg->enc_flag = 1;
		}
		printf("on_read_conf pcfg->enc_flag:%d\n", pcfg->enc_flag);
	}

	return 0;
}
int load_push_config(const char* pcfg_path)
{
	if(NULL == pcfg_path)
	{
		lberror("Invalid config path:%p\n", pcfg_path);
		return -1;
	}
	lbdebug("load_push_config(%s)\n", pcfg_path);
	g_ppush_cfg = (push_cfg*)malloc(sizeof(push_cfg));
	memset(g_ppush_cfg, 0, sizeof(push_cfg));
	g_ppush_cfg->nmax_channel_num = 100;
	g_ppush_cfg->ntigger_random_sec = 50;
	g_ppush_cfg->nlog_level = 3;
	g_ppush_cfg->nlog_output_mode = 3;
	g_ppush_cfg->ntigger_type = E_IPC_TIGGER_TYPE_DEFAULT;
	g_ppush_cfg->enc_flag = 1;
	int ret = load_config(pcfg_path, g_ppush_cfg, on_read_conf);
	lbdebug("g_ppush_cfg->pdevice_list_file:%s", g_ppush_cfg->pdevice_list_file);
	return ret;
}
/*int read_from_conf_tag(const char* pconf_file, const char* ptag, char* pvalue, int len)
{
	int val_len = 0;
	FILE* pfile = fopen(pconf_file, "rb");
	if(pfile)
	{
		do
		{
			char line[512] = {0};
			int ret = fgets(line, 512, pfile);
			if(!ret)
			{
				break;
			}
			int i = 0;
			while(line[i] == ' ' || line[i] == '\t')i++;
			
			if(line[i] == '#')
			{
				continue;
			}

			if(memcmp(&line[i], ptag, strlen(ptag)) == 0)
			{
				i += strlen(ptag);
				while(line[i] == ' ' || line[i] == '\t')i++;
				if(strlen(&line[i]) <= 0)
				{
					printf("tag %s has not value", ptag);
					break;
					//return 0;
				}

				val_len = len > strlen(&line[i]) ? strlen(&line[i]) : len;
				memcpy(pvalue, &line[i], val_len);
				break;
				//return len;
			}
		}while(1);
		
	fclose(pfile);
	pfile = NULL;
	}

	return val_len;
}*/

int ZBCloud_callback(long nUserID, long eHeaderEventCode, long wparam, long lparam)
{
	int channel;
	
	printf("ZBCloud_callback: nUserID = %ld, eHeaderEventCode = 0x%x, wparam = %ld, lparam = %ld\n",
		                      nUserID, (int)eHeaderEventCode, wparam, lparam);

	switch(eHeaderEventCode)
	{
		case E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE:
		case E_SV_EVENT_TYPE_RTMP_INVALID_URL:
	    case E_SV_EVENT_TYPE_RTMP_CONNECT_DNS_RESOVLE_FAILED:
	    case E_SV_EVENT_TYPE_RTMP_CONNECT_HANDSHAKE_FAILED:
	    case E_SV_EVENT_TYPE_RTMP_CONNECT_APP_FAILED:
	    case E_SV_EVENT_TYPE_RTMP_PUBLISH_STREAM_FAILED:
		case E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR:
			for(channel = 0; channel < g_cur_push_channel_num; channel++)
			{
				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] == nUserID)
				{
					g_ppush_channel_ctx->pcloudrecheck[channel] = 1;
					g_ppush_channel_ctx->pcloudflag[channel] = 0;
					g_ppush_channel_ctx->pcloudctrl[channel] = 0;
					break;
				}
			}
			break;
		case E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH:
			for(channel = 0; channel < g_ppush_cfg->nmax_channel_num; channel++)
			{
				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] == nUserID)
				{
					g_ppush_channel_ctx->pcloudctrl[channel] = 0;
					break;
				}
			}
			break;
		case E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH:
			for(channel = 0; channel < g_ppush_cfg->nmax_channel_num; channel++)
			{
				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] == nUserID)
				{
					g_ppush_channel_ctx->pcloudctrl[channel] = 1;
					break;
				}
			}
			break;
		default:
			break;
	}
	
	return 0;
}

int cloud_readdomain(char *domain, char *logpath, int *logtype, int *loglevel)
{
	FILE *fd = NULL;
	printf("domain:%s, logpath:%s, logtype:%p, loglevel:%p\n", domain, logpath, logtype, loglevel);
	char str[256];
	char key[32];
	char val[128];
	int i;
	int j;

	char tmpstr[8];
	int tmptype;

	strcpy(domain, "13.52.112.219:19350");
	strcpy(logpath, g_ppush_cfg->plog_file_path);

	*logtype = 3;

	fd = fopen("/tmp/config/cloud_domain", "r");
	if(fd == NULL)
	{
		return 0;
	}

	*logtype = 3;
	*loglevel = 3;

	while(g_running)
	{
		if(feof(fd))
		{
			break;
		}

		memset(str, 0, 256);
		memset(key, 0, 32);
		memset(val, 0, 128);

		if(fgets(str, 256, fd) == NULL)
		{
			break;
		}

		for(i = 0, j = 0; i < strlen(str); i++)
		{
			if(str[i] == '=')
			{
				break;
			}

			if(str[i] != ' ')
			{
				key[j] = str[i];
				j++;
			}
		}

		key[j] = '\0';
		i++;

		for(j = 0; i < strlen(str); i++)
		{
			if(str[i] == '\r' || str[i] == '\n' || str[i] == '\0')
			{
				break;
			}

			if(str[i] != ' ')
			{
				val[j] = str[i];
				j++;
			}
		}

		val[j] = '\0';

		if(strcmp(key, "domain") == 0)
		{
			memset(domain, 0, 32);
			strcpy(domain, val);
		}
		else if(strcmp(key, "path") == 0)
		{
			memset(logpath, 0, 128);
			strcpy(logpath, val);
		}
		else if(strcmp(key, "type") == 0)
		{
			memset(tmpstr, 0, 8);
			strcpy(tmpstr, val);
			tmptype = atoi(tmpstr);
			
			if(tmptype < 1 || tmptype > 3)
			{
				*logtype = 1;
			}
			else 
			{
				*logtype = tmptype;
			}
		}
		else if(strcmp(key, "level") == 0)
		{
			memset(tmpstr, 0, 8);
			strcpy(tmpstr, val);
			tmptype = atoi(tmpstr);

			if(tmptype < 1 || tmptype > 6)
			{
				*loglevel = 3;
			}
			else 
			{
				*loglevel = tmptype;
			}
		}
	}

	return 0;
}

/*int read_from_conf_tag(const char* pconf_file, const char* ptag, char* pvalue, int len)
{
	int val_len = 0;
	FILE* pfile = fopen(pconf_file, "rb");
	if(pfile)
	{
		do
		{
			char line[512] = {0};
			int ret = fgets(line, 512, pfile);
			if(!ret)
			{
				break;
			}
			int i = 0;
			while(line[i] == ' ' || line[i] == '\t')i++;
			
			if(line[i] == '#')
			{
				continue;
			}

			if(memcmp(&line[i], ptag, strlen(ptag)) == 0)
			{
				i += strlen(ptag);
				while(line[i] == ' ' || line[i] == '\t')i++;
				if(strlen(&line[i]) <= 0)
				{
					printf("tag %s has not value", ptag);
					break;
					//return 0;
				}

				val_len = len > strlen(&line[i]) ? strlen(&line[i]) : len;
				memcpy(pvalue, &line[i], val_len);
				break;
				//return len;
			}
		}while(1);
		
	fclose(pfile);
	pfile = NULL;
	}

	return val_len;
}*/
void *cloud_suvalley_pth(void *data)
{
	int channel;
	int waitcount = 0;

	int * lasttime = (int*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(int));
	
	struct timeval t_current;

	char cloudver[512];
	//char cloudurl[1024];

	char domain[32];
	char logpath[128];
	int logtype = 3;
	int loglevel = 3;
	
	memset(domain, 0, 32);
	memset(logpath, 0, 128);
	cloud_readdomain(domain, logpath, &logtype, &loglevel);

	printf("cloud_readdomain: domain = %s\n", domain);
	printf("cloud_readdomain: logpath = %s\n", logpath);
	printf("cloud_readdomain: logtype = %d\n", logtype);
	printf("cloud_readdomain: loglevel = %d\n", loglevel);

	for(channel = 0; channel < g_ppush_cfg->nmax_channel_num; channel++)
	{
		g_ppush_channel_ctx->psunvalley_cloud_id[channel] = -1;
		g_ppush_channel_ctx->pcloudflag[channel] = 0;
		g_ppush_channel_ctx->pcloudctrl[channel] = 0;
		lasttime[channel] = 0;
	}

	SVPush_API_Initialize(1024*1024, g_ppush_cfg->nmax_channel_num);

	SVPush_API_Init_log((E_SV_LOG_LEVEL)g_ppush_cfg->nlog_level, g_ppush_cfg->nlog_output_mode, g_ppush_cfg->plog_file_path);

	memset(cloudver, 0, 512);
	SVPush_API_Version(cloudver, 512);

	//char token_sever_url[256] = {0};
	//int token_url_len = read_from_conf_tag("push.conf", "token_server", token_sever_url, 512);
	//printf("url_len:%d, token_sever_url:%s\n", token_url_len, token_sever_url);
	if(g_ppush_cfg->ptoken_server_url)
	{
		SVPush_API_Set_Token_Server_Interface(g_ppush_cfg->ptoken_server_url);
	}
	
	printf("cloud_suvalley_pth: cloudver = %s\n", cloudver);
	#if 0
	SVPush_API_Connect("rtmp://13.52.67.204:19350/live/roby", NULL, ZBCloud_callback);
	g_ppush_channel_ctx->psunvalley_cloud_id[0] = 1;
	#endif

	printf("--------------- SUNVALLEY CLOUD OPEN -----------------\n");

	while(g_running)
	{
		sleep(1);
		
		if(waitcount++ < 2)
		{
			continue;
		}
		else
		{
			waitcount = 0;
		}

		gettimeofday(&t_current, NULL);

		for(channel = 0; channel < g_ppush_cfg->nmax_channel_num; channel++)
		{
			if(g_ppush_channel_ctx->ppair[channel].nstat == 0)
			{
				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] != -1)
				{
					SVPush_API_Close(g_ppush_channel_ctx->psunvalley_cloud_id[channel]);
					g_ppush_channel_ctx->psunvalley_cloud_id[channel]= -1;
				}
				
				continue;
			}

			if(g_ppush_channel_ctx->pcloudflag[channel] == 1)
			{
				continue;
			}

			if(t_current.tv_sec - lasttime[channel] >= 60 || g_ppush_channel_ctx->pcloudrecheck[channel] == 1)
			{
				lasttime[channel] = t_current.tv_sec;
				g_ppush_channel_ctx->pcloudrecheck[channel] = 0;

				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] != -1)
				{
					printf("cloud_suvalley_pth: AVBaseMedia_API_RTMPAVClose, id = %ld\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel]);
					
					SVPush_API_Close(g_ppush_channel_ctx->psunvalley_cloud_id[channel]);
					g_ppush_channel_ctx->psunvalley_cloud_id[channel] = -1;
					sleep(1);
				}

				//g_ppush_channel_ctx->psunvalley_cloud_id[channel] = AVBaseMedia_API_RTMPAVConnect("rtmp://13.52.67.204:19350/live/roby", ZBCloud_callback);
				//g_ppush_channel_ctx->psunvalley_cloud_id[channel] = AVBaseMedia_API_RTMPAVConnect("rtmp://13.52.112.219:19350/live/dann", ZBCloud_callback);
				//memset(cloudurl, 0, 1024);
				//sprintf(cloudurl, "rtmp://13.52.112.219:19350/live/%s", g_ppush_channel_ctx->ppair[channel].id);
				//sprintf(cloudurl, "rtmp://%s/live/%s", domain, g_ppush_channel_ctx->ppair[channel].id);

				//#ifdef DEBUG_LEVER_DEBUG
				//printf("cloud_suvalley_pth: url = [%s]\n", cloudurl);
				//#endif
				printf("g_ppush_cfg->ptoken_server_url:%s, g_ppush_cfg->ppush_server_url:%s, channel:%d, sn:%s\n", g_ppush_cfg->ptoken_server_url, g_ppush_cfg->ppush_server_url, channel, g_ppush_channel_ctx->ppair[channel].sn);
				if(g_ppush_cfg->ntigger_type >= E_IPC_TIGGER_TYPE_LIVE && g_ppush_cfg->plive_token_server_url)
				{
					g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Live_Connect(g_ppush_cfg->plive_token_server_url/*"third-api-dev.sunvalleycloud.com"*/, (char *)(g_ppush_channel_ctx->ppair[channel].sn), ZBCloud_callback);
				}
				else if(g_ppush_cfg->ptoken_server_url && g_ppush_cfg->pdevice_list_file)
				{
					g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Connect_By_DeviceSN(g_ppush_cfg->pai, (char *)(g_ppush_channel_ctx->ppair[channel].sn), ZBCloud_callback);	
				}
				else if(g_ppush_cfg->ptoken_server_url && g_ppush_cfg->ptoken_list_file)
				{
					g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Connect_By_Device_Token((char *)(g_ppush_channel_ctx->ppair[channel].token), ZBCloud_callback);
					printf("g_ppush_channel_ctx->psunvalley_cloud_id[channel]:%ld = SVPush_API_Connect_By_Device_Token((char *)(g_ppush_channel_ctx->ppair[channel].token%s), ZBCloud_callback)\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel], g_ppush_channel_ctx->ppair[channel].token);
				}
				else
				{
					if(g_ppush_cfg->ppush_server_url[strlen(g_ppush_cfg->ppush_server_url) - 1] != '/')
					{
						g_ppush_cfg->ppush_server_url[strlen(g_ppush_cfg->ppush_server_url)] = '/';
					}
					char url[256] = {0};
					sprintf(url, "%s%s", g_ppush_cfg->ppush_server_url, g_ppush_channel_ctx->ppair[channel].sn);
					printf("g_ppush_cfg->ppush_server_url:%s\n url:%s\n", g_ppush_cfg->ppush_server_url, url);
					if(g_ppush_cfg->ntigger_type >= E_IPC_TIGGER_TYPE_LIVE)
					{
						g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Live_Connect(url, NULL, ZBCloud_callback);
						printf("%ld = SVPush_API_Live_Connect(url:%s, NULL, ZBCloud_callback)\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel], url);
					}
					else
					{
						g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Connect(url, NULL, ZBCloud_callback);
						printf("%ld = SVPush_API_Connect(url:%s, NULL, ZBCloud_callback)\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel], url);
					}
				}
	
				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] < 0)
				{
					printf("connect rtmp server failed, g_ppush_channel_ctx->psunvalley_cloud_id[channel]:%ld\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel]);
					sleep(1);
					break;
				}
				/*#ifdef RTMP_PUSH_STREAM_BY_DEVICE_SN
				g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Connect_By_DeviceSN("live", NULL, (char *)(g_ppush_channel_ctx->ppair[channel].sn), ZBCloud_callback);	
				#else
				char url[256] = {0};
				sprintf(url, "%s/%s", RTMP_PUSH_URL, g_ppush_channel_ctx->ppair[channel].sn);
				g_ppush_channel_ctx->psunvalley_cloud_id[channel] = SVPush_API_Connect(url, NULL, ZBCloud_callback);				
				#endif*/
				
				printf("cloud_suvalley_pth: AVBaseMedia_API_RTMPAVConnect id = %ld\n", g_ppush_channel_ctx->psunvalley_cloud_id[channel]);

				if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] < 0)
				{
					 g_ppush_channel_ctx->pcloudrecheck[channel] = 1;
				}
				else
				{
					g_ppush_channel_ctx->pcloudflag[channel] = 1;
					g_ppush_channel_ctx->pcloudctrl[channel] = 0;
					
					printf("\n\n------------------------------------------\n");
					printf("------- CLOUD SUPPORT CHANNEL - %d -------\n", channel);
					printf("------------------------------------------\n\n");
				}
			}
		}
	}

	for(channel = 0; channel < g_ppush_cfg->nmax_channel_num; channel++)
	{
		if(g_ppush_channel_ctx->psunvalley_cloud_id[channel] >= 0)
		{
			SVPush_API_Close(g_ppush_channel_ctx->psunvalley_cloud_id[channel]);
		}
	}
	printf("cloud_suvalley_pth after close\n");
	free(lasttime);
	SVPush_API_UnInitialize();
	printf("cloud_suvalley_pth exit\n");
	return NULL;
}

/*
 * 函数名: get_timezone
 * 函数功能: 读取时区偏移值(夏令时)
 * 返回值：时区值
 * 修改:
 * 备注:
 */
/*int get_timezone()
{
  int time_zone;
  int tmp_zone;
  time_t time_utc;
  struct tm tm_gmt;
  
  // Get the UTC time
  time(&time_utc);
  struct tm tm_local;

  // Get the local time
  // Use localtime_r for threads safe
  localtime_r(&time_utc, &tm_local);

  // Change tm to time_t 
  time_t local_timestamp = mktime(&tm_local);

  // Change it to GMT tm
  gmtime_r(&time_utc, &tm_gmt);

  time_zone = (tm_local.tm_hour * 3600 + tm_local.tm_min * 60 + tm_local.tm_sec) 
            - (tm_gmt.tm_hour * 3600 + tm_gmt.tm_min * 60 + tm_gmt.tm_sec);

  if(tm_local.tm_mday != tm_gmt.tm_mday)
  {
    tmp_zone = tm_local.tm_mday - tm_gmt.tm_mday;
    if(tmp_zone == 1 || tmp_zone < -25)
    {
      time_zone += 24 * 3600;
    }
    else if(tmp_zone == -1 || tmp_zone > 25)
    {
      time_zone -= 24 * 3600;
    }
  }

  return time_zone;
}

int64_t get_local_timestamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	int64_t cur_time =  tv.tv_sec;
	int tz = get_timezone();
	cur_time = tz * 1000 + cur_time * 1000 + tv.tv_usec/1000;
	printf("tz:%d, cur_time:%ld\n", tz, cur_time);
	return cur_time;
}*/

int64_t get_local_timestamp()
{
    int time_zone;
    int tmp_zone;
    time_t time_utc;
    struct tm tm_gmt;
    struct timeval tv;
    
    // Get the UTC time
    time(&time_utc);
    struct tm tm_local;

    // Get the local time
    // Use localtime_r for threads safe
    localtime_r(&time_utc, &tm_local);

    // Change tm to time_t 
    time_t local_timestamp = mktime(&tm_local);

    // Change it to GMT tm
    gmtime_r(&time_utc, &tm_gmt);

    time_zone = (tm_local.tm_hour * 3600 + tm_local.tm_min * 60 + tm_local.tm_sec) 
                - (tm_gmt.tm_hour * 3600 + tm_gmt.tm_min * 60 + tm_gmt.tm_sec);
	printf("tm_local.tm_mday:%d != tm_gmt.tm_mday:%d, tm_local.tm_hour:%d, tm_gmt.tm_hour:%d, time_zone:%d\n", tm_local.tm_mday, tm_gmt.tm_mday, tm_local.tm_hour, tm_gmt.tm_hour, time_zone);
    if(tm_local.tm_mday != tm_gmt.tm_mday)
    {
        tmp_zone = tm_local.tm_mday - tm_gmt.tm_mday;
        if(tmp_zone == 1 || tmp_zone < -25)
        {
        	time_zone += 24 * 3600;
        }
        else if(tmp_zone == -1 || tmp_zone > 25)
        {
        	time_zone -= 24 * 3600;
        }
    }
    
	gettimeofday(&tv, NULL);
	int64_t cur_time =  tv.tv_sec;
	cur_time = time_zone * 1000 + cur_time * 1000 + tv.tv_usec/1000;
    return cur_time;
}

int64_t get_timestamp()
{
	struct timeval curtime;
	gettimeofday(&curtime, NULL);
	int64_t cur_time =  curtime.tv_sec;
	cur_time =  cur_time * 1000 + curtime.tv_usec/1000;
	return cur_time;
}

uint8_t adts_hdr[] =
{0xff, 0xf9, 0x60, 0x40, 0x20, 0x1f, 0xfc};
void *Rec_Cloud_pth(void *data)
{
	int channel = *(int *)data;
	char recname[256];
	char *readbuff = NULL;
	char* pkeystring = "vavalic2";

	int ret;
	int start = 0;
	int lostvideo = 0;
	int lostaudio = 0;
	int a_first = 0;
	int timeout;
#ifdef ENABLE_HS004_RECORD
	VAVA_HS_REC_CTX* prec_ctx = NULL;
	//IPC_PACKET_HDR	pkt_hdr;
#else
	FILE *fd = NULL;
	VAVA_RecHead av_head;
	VAVA_RecInfo recinfo;
#endif
	
	struct timeval t_start;
	struct timeval t_end;
	unsigned int usedtime;
	unsigned long long ntsamp;
	long long cloud_ntsamp;

	if(channel < 0 || channel >= g_ppush_cfg->nmax_channel_num)
	{
		printf("Rec_Cloud_pth: check channel fail, channel = %d\n", channel);
		g_running = 0;
		return NULL;
	}

	g_ppush_channel_ctx->precstart[channel] = 0;

	readbuff = (char*)malloc(AV_BUFF_SIZE);
	if(readbuff == NULL)
	{
		printf("Rec_Cloud_pth: malloc readbuff fail, channel = %d\n", channel);
		g_running = 0;
		return NULL;
	}

	

	memset(recname, 0, sizeof(recname));
	strcpy(recname, g_ppush_cfg->pdefault_rec_path);
	printf("Rec_Cloud_pth: pth start, channel = %d, recname:%s\n", channel, recname);
	/*switch(channel)
	{
		case 0:
			//strcpy(recname, "/mnt/sd0/P020201000401010000000147/20190916/161724_1_0");
			sprintf(recname, "%s%s", g_ppush_cfg->pdefault_rec_path, "161724_1_0");
			//strcpy(recname, "/mnt/sd0/rec/161724_1_0");
			//strcpy(recname, "/mnt/sd0/P020201000201190813000001/20190916/P020101000201190813000006/115521_U_0");
			break;
		case 1:
			//strcpy(recname, "/mnt/sd0/P020201000401010000000147/20190916/161724_1_1");
			//strcpy(recname, "/mnt/sd0/P020201000201190813000001/20190916/P020101000201190813000006/110718_U_0");
			sprintf(recname, "%s%s", g_ppush_cfg->pdefault_rec_path, "161724_1_1");
			//strcpy(recname, "/mnt/sd0/rec/161724_1_1");
			break;
		case 2:
			//strcpy(recname, "/mnt/sd0/P020201000401010000000147/20190916/153831_1_2");
			sprintf(recname, "%s%s", g_ppush_cfg->pdefault_rec_path, "153831_1_2");
			//strcpy(recname, "/mnt/sd0/rec/153831_1_2");
			//strcpy(recname, "/mnt/sd0/P020201000201190813000001/20190916/P020101000201190813000006/111421_U_0");
			break;
		case 3:
			//strcpy(recname, "/mnt/sd0/P020201000401010000000147/20190916/161735_1_3");
			sprintf(recname, "%s%s", g_ppush_cfg->pdefault_rec_path, "161735_1_3");
			//strcpy(recname, "/mnt/sd0/rec/161735_1_3");
			//strcpy(recname, "/mnt/sd0/P020201000201190813000001/20190916/P020101000201190813000006/115605_U_0");
			break;
		default:
			sprintf(recname, "%s%s", g_ppush_cfg->pdefault_rec_path, "161724_1_0");
			break;
	}*/

	while(g_running)
	{
		if(g_ppush_channel_ctx->pcloudflag[channel] == 0 || g_ppush_channel_ctx->precstart[channel] == 0)
		{
			sleep(1);
			continue;
		}

#ifdef ENABLE_HS004_RECORD
		VAVA_HS_REC_CTX* prec_ctx = vava_hs_open_record(recname);
		if(NULL == prec_ctx)
		{
			lberror("prec_ctx:%p = vava_hs_open_record(recname:%s) failed\n", prec_ctx, recname);
			sleep(2);
			continue;
		}

		//vava_hs_read_packet(prec_ctx);
#else
		fd = fopen(recname, "r");
		if(fd == NULL)
		{
			printf("Rec_Cloud_pth: open %s fail\n", recname);
			break;
		}

		ret = fread(&recinfo, sizeof(VAVA_RecInfo), 1, fd);
		if(ret <= 0)
		{
			printf("Rec_Cloud_pth: read recinfo fail\n");
			break;
		}
#endif
		printf("Rec_Cloud_pth: channel = %d, push cloud start, g_ppush_cfg->enc_flag:%d\n", channel, g_ppush_cfg->enc_flag);

		//gettimeofday(&t_start, NULL);
		//ntsamp = (unsigned long long)t_start.tv_sec * 1000 + t_start.tv_usec / 1000;
		ntsamp = get_local_timestamp();//((unsigned long long)t_start.tv_sec /*+3600*16*/) * 1000 + t_start.tv_usec / 1000;
		SVPush_API_Record_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], "./test.rec");
		if(g_ppush_cfg->enc_flag)
		{
			SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_VIDEO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME, (long)(pkeystring));
			SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_AUDIO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME, (long)(pkeystring));
		}

		SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_STREAM_START, g_ppush_cfg->ntigger_type, (long)(&ntsamp));
		//printf("send spspps:%p, size:%d\n", g_ppush_channel_ctx->pspsppsbuff, g_ppush_channel_ctx->nspsppssize);
		char logbuf[256] = {0};
		int i = 0;
		for(i = 0; i < g_ppush_channel_ctx->nspsppssize && (int)strlen(logbuf) < 256; i++)
		{
			int val = *(g_ppush_channel_ctx->pspsppsbuff + i);
			sprintf(logbuf + strlen(logbuf), "%02x", val);
		}
		//printf("spspps buffer:%s\n", logbuf);
		E_SV_MEDIA_TYPE vcodec_id = prec_ctx->rec_hdr.vcodec_id == 0 ? E_SV_MEDIA_TYPE_H264 : E_SV_MEDIA_TYPE_H265;
		SVPush_API_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], vcodec_id, (char *)g_ppush_channel_ctx->pspsppsbuff, g_ppush_channel_ctx->nspsppssize, 0, 1, 0);
		
		start = 1;
		a_first = 0;
		i = 0;
		int64_t pts_offset = 0;
		int64_t llpts_start = INT64_MIN;
		prec_ctx->pkt_hdr.size = 0;
		int64_t begin_time = get_timestamp();
		while(g_running && g_ppush_channel_ctx->pcloudflag[channel])
		{
			if(g_ppush_channel_ctx->pcloudctrl[channel] == 1)
			{
				sleep(1);
			}
			if(prec_ctx->pkt_hdr.size <= 0)
			{
#ifdef ENABLE_HS004_RECORD
				ret = vava_hs_read_packet(prec_ctx, &prec_ctx->pkt_hdr, readbuff, AV_BUFF_SIZE);
				if(ret != 0)
				{
					if(g_ppush_cfg->ntigger_type >= E_IPC_TIGGER_TYPE_LIVE)
					{
						vava_hs_seek_record(prec_ctx, 0);
						pts_offset += cloud_ntsamp + 67;
						llpts_start = INT64_MIN;
						prec_ctx->pkt_hdr.size = 0;
						printf("read record reash end!\n");
						continue;
					}

					lberror("ret:%d = vava_hs_read_packet(prec_ctx:%p, &prec_ctx->pkt_hdr, readbuff, AV_BUFF_SIZE)\n", ret, prec_ctx);
					break;
				}
#else
				ret = fread(&av_head, sizeof(VAVA_RecHead), 1, fd);
				if(ret <= 0)
				{
					if(g_ppush_cfg->ntigger_type >= E_IPC_TIGGER_TYPE_LIVE)
					{
						fseek(fd, sizeof(VAVA_RecInfo), SEEK_SET);
						pts_offset += cloud_ntsamp;
						printf("read record reash end!\n");
						continue;
					}
					break;
				}
				
				if(av_head.size <= 0 || av_head.size >= AV_BUFF_SIZE)
				{
					printf("Rec_Cloud_pth: [Waring] check size fail, size = %d, channel = %d\n", av_head.size, channel);
				}

				memset(readbuff, 0, AV_BUFF_SIZE);
				ret = fread(readbuff, av_head.size, 1, fd);
				if(ret <= 0)
				{
					if(g_ppush_cfg->ntigger_type >= E_IPC_TIGGER_TYPE_LIVE)
					{
						fseek(fd, sizeof(VAVA_RecInfo), SEEK_SET);
						pts_offset += cloud_ntsamp;
						printf("read record reash end!\n");
						continue;
					}
					break;
				}
#endif
				if(INT64_MIN == llpts_start)
				{
					llpts_start = prec_ctx->pkt_hdr.pts;
				}

				prec_ctx->pkt_hdr.pts -= llpts_start;
			}

			
			if(get_timestamp() - begin_time < prec_ctx->pkt_hdr.pts + pts_offset + 20)
			{
				usleep(20000);
				continue;
			}
			//printf("get_timestamp() - begin_time = %lld < prec_ctx->pkt_hdr.pts:%lld + pts_offset:%lld + 20 = %lld\n", get_timestamp() - begin_time, prec_ctx->pkt_hdr.pts, pts_offset, prec_ctx->pkt_hdr.pts + pts_offset + 20);
			if(start == 2)
			{
				if(0 == prec_ctx->pkt_hdr.mt && prec_ctx->pkt_hdr.keyflag)
				{
					start = 1;

					//printf("Rec_Cloud_pth[%d]: lost video = %d frame, lost audio = %d frame\n", channel, lostvideo, lostaudio);
					SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_STREAM_DROP_FRAME, (long)(&lostvideo), (long)(&lostaudio));

					lostvideo = 0;
					lostaudio = 0;
				}
				else
				{
					if(0 == prec_ctx->pkt_hdr.mt && !prec_ctx->pkt_hdr.keyflag)
					{
						lostvideo++;
					}
					else if(1 == prec_ctx->pkt_hdr.mt)
					{
						lostaudio++;
					}
				}
			}

			if(start == 1)
			{
				timeout = 0;
				while(g_ppush_channel_ctx->pcloudctrl[channel] == 1) //��ͣ
				{
					if(timeout++ >= 15) //3�볬ʱ
					{
						printf("Rec_Cloud_pth[%d]: [WARING] cloud update timeout(3s)\n", channel);
						break;
					}
						
					usleep(200000);	
				}

				if(timeout >= 15)
				{
					start = 2;

					lostvideo = 0;
					lostaudio = 0;
				}
				else
				{
					int encrypt = (prec_ctx->rec_hdr.encrypt && prec_ctx->pkt_hdr.keyflag) ? 1 : 0;
					cloud_ntsamp = (long long)prec_ctx->pkt_hdr.pts; //(av_head.time_sec * 1000 + av_head.time_usec);

					switch(prec_ctx->pkt_hdr.codec_id)
					{
						case 0:
							//printf("send avc packet, size:%d", av_head.size);
							/*SVPush_API_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], vcodec_id, readbuff, prec_ctx->pkt_hdr.size, pts_offset + cloud_ntsamp, 0, encrypt);
							if(i++ > 30)
							{
								usleep(60000);
							}
							else
							{
								usleep(10000);
							}
							break;*/
						case 1:
							//printf("send video packet, pts:%lld, size:%d", cloud_ntsamp, prec_ctx->pkt_hdr.size);
							SVPush_API_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], vcodec_id, readbuff, prec_ctx->pkt_hdr.size, cloud_ntsamp + pts_offset, 1, encrypt);
							//if(i++ > 30)
							{
								usleep(60000);
							}
							break;
						case 3:
						case 8:
							// send aac packet
							if(a_first == 0)
							{
								a_first = 1;
								SVPush_API_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_MEDIA_TYPE_AAC, (char *)adts_hdr, sizeof(adts_hdr)/*g_ppush_channel_ctx->padtshead, g_ppush_channel_ctx->nadtssize*/, cloud_ntsamp, 0, 0);
							}
							SVPush_API_Send_Packet(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_MEDIA_TYPE_AAC, readbuff, prec_ctx->pkt_hdr.size, cloud_ntsamp + pts_offset, 0, encrypt);
							break;
						default:
							break;
					}
					prec_ctx->pkt_hdr.size = 0;
				}
			}
		}
		gettimeofday(&t_start, NULL);
		ntsamp = ((unsigned long long)t_start.tv_sec /*+3600*16*/) * 1000 + t_start.tv_usec / 1000;
		if(g_ppush_cfg->ntigger_type < E_IPC_TIGGER_TYPE_LIVE)
		{
			SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_TIGGER_TYPE_CHANGE, g_ppush_cfg->ntigger_type, E_IPC_TIGGER_TYPE_FACE);
			printf("E_SV_METADATA_TYPE_TIGGER_TYPE_CHANGE, g_ppush_cfg->ntigger_type:%d, E_IPC_TIGGER_TYPE_FACE\n", g_ppush_cfg->ntigger_type);
		}
		SVPush_API_Send_Metadata(g_ppush_channel_ctx->psunvalley_cloud_id[channel], E_SV_METADATA_TYPE_STREAM_END, /*av_head.time_sec*1000+av_head.time_usec*/ prec_ctx->pkt_hdr.pts, (long)(&ntsamp));

		g_ppush_channel_ctx->precstart[channel] = 0;

		gettimeofday(&t_end, NULL);

		usedtime = (unsigned int)(t_end.tv_sec - t_start.tv_sec);
		printf("Rec_Cloud_pth: channel = %d, push cloud end, push time = %d s\n", channel, usedtime);
		if(prec_ctx)
		{
			vava_hs_close_record(&prec_ctx);
		}
	}

	if(readbuff != NULL)
	{
		free(readbuff);
	}

	if(prec_ctx)
	{
		vava_hs_close_record(&prec_ctx);
	}
	g_running = 0;

	printf("Rec_Cloud_pth: pth exit, channel = %d\n", channel);

	return NULL;
}


#include "basetype.h"
#include "cloud_sunvalley.h"
#include "aesencrypt.h"
#include "cputotol.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "lbsc_util_conf.h"
#define BASE_STATION_COM_4
int 	g_running = 1;
int	g_cur_push_channel_num = 4;
push_channel_ctx*		g_ppush_channel_ctx = NULL;
push_cfg*				g_ppush_cfg = NULL;
//int g_ppush_channel_ctx->pchannel[MAX_CHANNEL_NUM];
//VAVA_PairInfo g_ppush_channel_ctx->ppair[MAX_CHANNEL_NUM];

/*unsigned char g_spsppsbuff[64];
unsigned int g_spsppssize = 0;
unsigned char g_adtshead[8];
unsigned int g_adtssize = 0;*/

void sig_stop(int signo)
{
	printf("sig_stop: signo = %d\n", signo);
	g_running = 0;
	return;
}

void sig_continue(int signo)
{
	printf("pipe, continue...\n");
}

int init_channel_context(const char* ppush_conf)
{
	load_push_config(ppush_conf);
	g_ppush_channel_ctx = (push_channel_ctx*)malloc(sizeof(push_channel_ctx));
	memset(g_ppush_channel_ctx, 0, sizeof(push_channel_ctx));

	//init channel flag array
	g_ppush_channel_ctx->psunvalley_cloud_id = (long*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(long));
	memset((void*)g_ppush_channel_ctx->psunvalley_cloud_id, 0, g_ppush_cfg->nmax_channel_num*sizeof(long));

	g_ppush_channel_ctx->pcloudflag = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->pcloudflag, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->pcloudsend = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->pcloudsend, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->pcloudupcheck = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->pcloudupcheck, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->pcloudrecheck = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->pcloudrecheck, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->pcloudctrl = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->pcloudctrl, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->precstart = (unsigned char*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));
	memset((void*)g_ppush_channel_ctx->precstart, 0, g_ppush_cfg->nmax_channel_num*sizeof(unsigned char));

	g_ppush_channel_ctx->pchannel = (int*)malloc(g_ppush_cfg->nmax_channel_num * sizeof(int));
	memset(g_ppush_channel_ctx->pchannel, 0, g_ppush_cfg->nmax_channel_num*sizeof(int));

	g_ppush_channel_ctx->ppair = (VAVA_PairInfo*)malloc(g_ppush_cfg->nmax_channel_num*sizeof(VAVA_PairInfo));
	memset(g_ppush_channel_ctx->ppair, 0, g_ppush_cfg->nmax_channel_num*sizeof(VAVA_PairInfo));
	return 0;
}

int init_channel()
{
	int channel;
	
	for(channel = 0; channel < g_cur_push_channel_num; channel++)
	{
		g_ppush_channel_ctx->pchannel[channel] = channel;
	}

	return 0;
}

int init_pair()
{
	memset(g_ppush_channel_ctx->ppair, 0, sizeof(VAVA_PairInfo) * g_ppush_cfg->nmax_channel_num);
/*#ifdef PUSH_SDK_LINUX
	g_ppush_channel_ctx->ppair[0].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[0].sn, "P020101000201190813000105"); 

	g_ppush_channel_ctx->ppair[1].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[1].sn, "P020101000201190813000106"); 

	g_ppush_channel_ctx->ppair[2].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[2].sn, "P020101000201190813000107"); 

	g_ppush_channel_ctx->ppair[3].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[3].sn, "P020101000201190813000108");
#else*/
/*#ifdef BASE_STATION_366
	g_ppush_channel_ctx->ppair[0].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[0].sn, "P020101000201190813000105"); 

	g_ppush_channel_ctx->ppair[1].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[1].sn, "P020101000201190813000106"); 

	g_ppush_channel_ctx->ppair[2].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[2].sn, "P020101000201190813000107"); 

	g_ppush_channel_ctx->ppair[3].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[3].sn, "P020101000201190813000108");
#else*/
#ifdef BASE_STATION_COM_4
	g_ppush_channel_ctx->ppair[0].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[0].sn, "P020101000201190813000097"); 

	g_ppush_channel_ctx->ppair[1].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[1].sn, "P020101000201190813000098"); 

	g_ppush_channel_ctx->ppair[2].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[2].sn, "P020101000201190813000099"); 

	g_ppush_channel_ctx->ppair[3].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[3].sn, "P020101000201190813000100");
#else
	g_ppush_channel_ctx->ppair[0].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[0].sn, "P020101000201190813000101"); 

	g_ppush_channel_ctx->ppair[1].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[1].sn, "P020101000201190813000102"); 

	g_ppush_channel_ctx->ppair[2].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[2].sn, "P020101000201190813000103"); 

	g_ppush_channel_ctx->ppair[3].nstat = 1;
	strcpy(g_ppush_channel_ctx->ppair[3].sn, "P020101000201190813000104");
//#endif
#endif

	SSL_library_init();
	ERR_load_crypto_strings();
	SSL_load_error_strings();
	OpenSSL_add_all_algorithms();
//#endif
	return 0;
}

int init_pair_from_sn_file(const char* psnpath)
{
	printf("init_pair_from_sn_file(%s)\n\n", psnpath);
	if(NULL == psnpath)
	{
		printf("Invalid sn file path:%s\n", psnpath);
		return -1;
	}
	int i = 0;
	FILE* fd = fopen(psnpath, "rb");
	//printf("\nfd:%p\n\n", fd);
	printf("\n\nfd:%p = fopen(psnpath:%s, rb)\n\n", fd, psnpath);
	if(fd == NULL)
	{
		printf("\nopen sn file failed, fd:%p, psnpath:%s, reason:%s\n\n", fd, psnpath, strerror(errno));
		return -1;

	}
	memset(g_ppush_channel_ctx->ppair, 0, sizeof(VAVA_PairInfo) * g_ppush_cfg->nmax_channel_num);
	while(1)
	{
		char line_buff[1024];
		if(fgets(line_buff, 512, fd) == NULL)
		{
			g_cur_push_channel_num = i;
			break;
		}
		if(i >= g_cur_push_channel_num)
		{
			break;
		}
		g_ppush_channel_ctx->ppair[i].nstat = 1;
		int len = strlen(line_buff);
		//char* pent = strstr(line_buff, "\r\n");
		if(len > 0 && '\n' == line_buff[len-1])
		{
			line_buff[len-1] = 0;
			len--;
		}
		if(len > 0 && '\r' == line_buff[len-1])
		{
			line_buff[len-1] = 0;
			len--;
		}
		/*char* pent = strstr(line_buff, "\r\n");
		memset(pent, 0, sizeof("\r\n"));*/
		strcpy(g_ppush_channel_ctx->ppair[i++].sn, line_buff);
		printf("read sn:%s\n", line_buff);
	};
	
	printf("total read %d sn from file!", i);
	if(fd)
	{
		fclose(fd);
		fd = 0;
	}
	
	return 0;
}

int init_pair_from_token_file(const char* ptoken_path)
{
	int i = 0;
	FILE* fd = fopen(ptoken_path, "rb");
	if(NULL == ptoken_path)
	{
		printf("Invalid token file path:%s\n", ptoken_path);
		return -1;
	}
	if(fd <= 0)
	{
		printf("open sn file failed, fd:%p, ptoken_path:%s\n", fd, ptoken_path);
		return -1;

	}
	memset(g_ppush_channel_ctx->ppair, 0, sizeof(VAVA_PairInfo) * g_ppush_cfg->nmax_channel_num);
	while(1)
	{
		char line_buff[1024];
		if(fgets(line_buff, 512, fd) == NULL)
		{
			g_cur_push_channel_num = i;
			break;
		}
		if(i >= g_cur_push_channel_num)
		{
			break;
		}
		g_ppush_channel_ctx->ppair[i].nstat = 1;
		int len = strlen(line_buff);
		//char* pent = strstr(line_buff, "\r\n");
		if(len > 0 && '\n' == line_buff[len-1])
		{
			line_buff[len-1] = 0;
			len--;
		}
		if(len > 0 && '\r' == line_buff[len-1])
		{
			line_buff[len-1] = 0;
			len--;
		}

		strcpy(g_ppush_channel_ctx->ppair[i++].token, line_buff);
	};
	
	printf("total read %d sn from file!", i);
	if(fd)
	{
		fclose(fd);
		fd = 0;
	}
	return 0;
}

int init_cloud_sunvalley()
{
	int ret;
	pthread_t pth_id;
	pthread_attr_t attr; 

	pthread_attr_init(&attr); 
    pthread_attr_setdetachstate(&attr, 1);
	pthread_attr_setstacksize(&attr, STACK_SIZE);
	ret = pthread_create(&pth_id, &attr, cloud_suvalley_pth, NULL);
	pthread_attr_destroy(&attr);
	
	if(ret != 0)
	{
		printf("init_cloud_sunvalley: create pth err, ret = %d\n", ret);
	}
	
	return ret;
}

int VAVAHAL_GetSpsPps(unsigned char *buff, unsigned char *spsbuff, unsigned char *ppsbuff, 
	                  int *spssize, int *ppssize)
{
	int i;

	for(i = 0; i < 100; i++)
	{
		printf("%2x", buff[i]);
		if(buff[i] == 0 && buff[i + 1] == 0 && buff[i + 2] == 0 && buff[i + 3] == 1 && buff[i + 4] == 0x68)	
		{
			
			*spssize = i;
			printf("\n*spssize:%d\n", *spssize);
			memcpy(spsbuff, buff, *spssize);
			break;
		}
	}

	for(i = *spssize + 4; i < 100; i++)
	{
		printf("%2x",buff[i]);
		if(buff[i] == 0 && buff[i + 1] == 0 && buff[i + 2] == 0 && buff[i + 3] == 1 && buff[i + 4] == 0x6)
		{
			*ppssize = i - *spssize;
			printf("\n*ppssize:%d\n", *ppssize);
			memcpy(ppsbuff, buff + *spssize, *ppssize);
			break;
		}
	}

	return 0;
}
int init_sps_pps()
{
	char recname[256];
	int ret = 0;
	int i = 0;
	if(NULL == g_ppush_cfg->pdefault_rec_path)
	{
		printf("g_ppush_cfg->pdefault_rec_path == NULL, init_sps_pps failed\n");
		return -1;
	}
	memset(recname, 0, sizeof(recname));
	sprintf(recname, "%s", g_ppush_cfg->pdefault_rec_path);
	printf("init_spspps recname：%s\n", recname);
	VAVA_HS_REC_CTX* prec_ctx = vava_hs_open_record(recname);
	lbtrace("prec_ctx:%p = vava_hs_open_record(recname)\n", prec_ctx);
	if(NULL == prec_ctx)
	{
		lbtrace("prec_ctx = vava_hs_open_record(recname) failed\n", prec_ctx, recname);
		return -1;
	}

	ret = vava_hs_parser_sequence_hdr(prec_ctx);
	lbtrace("ret:%d = vava_hs_parser_sequence_hdr(prec_ctx)\n", ret);
	if(ret < 0)
	{
		lbtrace("ret:%d = vava_hs_parser_sequence_hdr(prec_ctx:%p) failed\n", ret, prec_ctx);
		return ret;
	}

	g_ppush_channel_ctx->pspsppsbuff = (unsigned char*)malloc(prec_ctx->nvsh);
	//printf("memcpy(g_ppush_channel_ctx->pspsppsbuff:%p, spsbuff:%p, spssize:%d)\n", g_ppush_channel_ctx->pspsppsbuff, spsbuff, spssize);
	memcpy(g_ppush_channel_ctx->pspsppsbuff, prec_ctx->pvsh, prec_ctx->nvsh);
	g_ppush_channel_ctx->nspsppssize = prec_ctx->nvsh;
	lbtrace("memcpy(g_ppush_channel_ctx->pspsppsbuff:%p, prec_ctx->pvsh:%p, prec_ctx->nvsh:%d)\n", g_ppush_channel_ctx->pspsppsbuff, prec_ctx->pvsh, prec_ctx->nvsh);
	g_ppush_channel_ctx->padtshead = (unsigned char*)malloc(prec_ctx->nadts_hdr_len);
	memcpy(g_ppush_channel_ctx->padtshead, prec_ctx->padts_hdr, prec_ctx->nadts_hdr_len);
	g_ppush_channel_ctx->nadtssize = prec_ctx->nadts_hdr_len;
	printf("============================= SPS PPS [size - %d] =======================\n", g_ppush_channel_ctx->nspsppssize);
	for(i = 0; i < g_ppush_channel_ctx->nspsppssize; i++)
	{
		printf("%02x ", g_ppush_channel_ctx->pspsppsbuff[i]);
	}
	printf("\n");
	printf("=================================adts header [size - %d] ====================================\n", g_ppush_channel_ctx->nadtssize);
	for(i = 0; i < g_ppush_channel_ctx->nadtssize; i++)
	{
		printf("%02x ", g_ppush_channel_ctx->padtshead[i]);
	}
	printf("\n");
	printf("========================================================================\n");
	vava_hs_close_record(&prec_ctx);
	return 0;
}
int init_spspps()
{
	FILE *fd = NULL;
	char recname[256];
	VAVA_RecHead av_head;
	VAVA_RecInfo recinfo;
	int i;
	int ret;
	int iframe = 0;
	int aframe = 0;
	char *readbuff = NULL;
	unsigned char spsbuff[32];
	unsigned char ppsbuff[32];
	int spssize;
	int ppssize;

	memset(recname, 0, sizeof(recname));
	sprintf(recname, "%s", g_ppush_cfg->pdefault_rec_path);
	printf("init_spspps recname：%s\n", recname);
	//strcpy(recname, "/mnt/sd0/rec/161724_1_0");

	readbuff = (char*)malloc(AV_BUFF_SIZE);
	if(readbuff == NULL)
	{
		printf("init_spspps: malloc readbuff fail\n");
		return -1;
	}

	fd = fopen(recname, "r");
	if(fd == NULL)
	{
		free(readbuff);
		return -1;
	}

	fread(&recinfo, sizeof(VAVA_RecInfo), 1, fd);

	while(1)
	{
		if(iframe == 1 && aframe == 1)
		{
			break;
		}

		ret = fread(&av_head, sizeof(VAVA_RecHead), 1, fd);
		if(ret <= 0)
		{
			break;
		}

		memset(readbuff, 0, AV_BUFF_SIZE);
		ret = fread(readbuff, av_head.size, 1, fd);
		if(ret <= 0)
		{
			break;
		}

		switch(av_head.type)
		{
			case 0:
				break;
			case 1:
				if(iframe == 0)
				{
					if(recinfo.encrypt)
					{
						VAVA_Aes_Decrypt(readbuff, readbuff, av_head.size);
					}
					
					memset(spsbuff, 0, 32);
					memset(ppsbuff, 0, 32);
					spssize = 0;
					ppssize = 0;
					VAVAHAL_GetSpsPps((unsigned char *)readbuff, spsbuff, ppsbuff, &spssize, &ppssize);
					printf("spsbuff:%p, ppsbuff:%p, &spssize:%d, &ppssize:%d\n", spsbuff, ppsbuff, spssize, ppssize);
					if(g_ppush_channel_ctx->pspsppsbuff)
					{
						free(g_ppush_channel_ctx->pspsppsbuff);
						g_ppush_channel_ctx->pspsppsbuff = NULL;
					}
					g_ppush_channel_ctx->pspsppsbuff = (unsigned char*)malloc(spssize + ppssize);
					printf("memcpy(g_ppush_channel_ctx->pspsppsbuff:%p, spsbuff:%p, spssize:%d)\n", g_ppush_channel_ctx->pspsppsbuff, spsbuff, spssize);
					memcpy(g_ppush_channel_ctx->pspsppsbuff, spsbuff, spssize);
					memcpy(g_ppush_channel_ctx->pspsppsbuff + spssize, ppsbuff, ppssize);
					g_ppush_channel_ctx->nspsppssize = spssize + ppssize;
					//memset((char *)g_spsppsbuff, 0, 64);
					/*memcpy((char *)g_spsppsbuff, (char *)spsbuff, spssize);
					memcpy((char *)(g_spsppsbuff + spssize), (char *)ppsbuff, ppssize);
					g_spsppssize = spssize + ppssize;*/

					printf("============================= SPS PPS [size - %d] =======================\n", g_ppush_channel_ctx->nspsppssize);
					for(i = 0; i < g_ppush_channel_ctx->nspsppssize; i++)
					{
						printf("%02x ", g_ppush_channel_ctx->pspsppsbuff[i]);
					}
					printf("\n");
					printf("==========================================================================\n");

					iframe = 1;
				}
				break;
			case 8:
				if(aframe == 0)
				{
					if(recinfo.encrypt)
					{
						VAVA_Aes_Decrypt(readbuff + 1, readbuff + 1, av_head.size - 1);
					}
					
					readbuff[0] = 0xFF;
					if(g_ppush_channel_ctx->padtshead)
					{
						free(g_ppush_channel_ctx->padtshead);
					}
					g_ppush_channel_ctx->padtshead = (unsigned char*)malloc(8);
					memset(g_ppush_channel_ctx->padtshead, 0,  8);
					memcpy(g_ppush_channel_ctx->padtshead, readbuff, 7);
					g_ppush_channel_ctx->nadtssize = 7;
					//memset(g_adtshead, 0, 8);
					//memcpy(g_adtshead, readbuff, 7);
					//g_ppush_channel_ctx->nadtssize = 7;

					printf("============================= ADTS Head [size - %d] =======================\n", g_ppush_channel_ctx->nadtssize);
					for(i = 0; i < g_ppush_channel_ctx->nadtssize; i++)
					{
						printf("%02x ", g_ppush_channel_ctx->padtshead[i]);
					}
					printf("\n");
					printf("==========================================================================\n");

					aframe = 1;
				}
				break;
			default:
				break;
		}
	}

	if(readbuff != NULL)
	{
		free(readbuff);
	}

	if(fd != NULL)
	{
		fclose(fd);
	}

	if(iframe == 1 && aframe == 1)
	{
		return 0;
	}

	return -1;
}

int init_cloud_rec_service(int channel)
{
	int ret;
	pthread_t pth_id;
	pthread_attr_t attr; 

	pthread_attr_init(&attr); 
    pthread_attr_setdetachstate(&attr, 1);
	pthread_attr_setstacksize(&attr, STACK_SIZE);
	ret = pthread_create(&pth_id, &attr, Rec_Cloud_pth, &g_ppush_channel_ctx->pchannel[channel]);
	pthread_attr_destroy(&attr);
	
	if(ret != 0)
	{
		printf("init_cloud_rec_service: create pth err, ret = %d\n", ret);
	}
	
	return ret;
}

void *Random_pth(void *data)
{
	int channel = *(int *)data;
	int random;
	struct timeval randomtime; 

	while(g_running)
	{
		sleep(1);
		//continue;
		
		if(g_ppush_channel_ctx->precstart[channel] == 0 && g_ppush_channel_ctx->pcloudflag[channel] == 1)
		{
			gettimeofday(&randomtime, NULL);
			srand((unsigned int)randomtime.tv_usec); //��ʱ�����֣�ÿ�β����������һ��
			random = rand() % (g_ppush_cfg->ntigger_random_sec + 1); //����1-100�����

			printf("Random_pth: channel - %d, random = %d\n", channel, random);

			sleep(random);
			g_ppush_channel_ctx->precstart[channel] = 1;
		}
	}
	printf("main exit");
	return NULL;
}

int init_random(int channel)
{
	int ret;
	pthread_t pth_id;
	pthread_attr_t attr; 

	pthread_attr_init(&attr); 
    pthread_attr_setdetachstate(&attr, 1);
	pthread_attr_setstacksize(&attr, STACK_SIZE);
	pthread_attr_setdetachstate(&attr,1); 
	//pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	ret = pthread_create(&pth_id, &attr, Random_pth, &g_ppush_channel_ctx->pchannel[channel]);
	pthread_attr_destroy(&attr);
	
	if(ret != 0)
	{
		printf("init_cloud_rec_service: create pth err, ret = %d\n", ret);
	}
	
	return ret;
}

int init_cputotol()
{
	int ret;
	pthread_t pth_id;
	pthread_attr_t attr; 

	pthread_attr_init(&attr); 
    pthread_attr_setdetachstate(&attr, 1);
	pthread_attr_setstacksize(&attr, STACK_SIZE);
	ret = pthread_create(&pth_id, &attr, Cputotol_pth, NULL);
	pthread_attr_destroy(&attr);
	
	if(ret != 0)
	{
		printf("init_cloud_rec_service: create pth err, ret = %d\n", ret);
	}
	
	return ret;
}

typedef struct
{
	int npush_num;
	char* ppush_conf;
	int ntigger_type;
	int ntigger_random_time;
} push_ctx;

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
	push_ctx* ppc = (push_ctx*)powner;
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
			printf("ppc->ppush_conf:%s\n", ppc->ppush_conf);
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
/*int parser_args(char** args, int argc, void* powner)
{
    int ret = 0;
    assert(ppi);
    memset(ppi, 0, sizeof(push_ctx));
    int i;
    for(i = 1; i < argc; i++)
    {
		if('-' == args[i][0] && i + 1 < argc)
		{
			on_parser_option(args[i][1], args[i+1], powner);
			i++;
		}
    }

    return ret;
}*/
push_ctx* g_ppush_info = NULL;
void destroy_demo()
{
	if(g_ppush_info)
	{
		if(g_ppush_info->ppush_conf)
		{
			free(g_ppush_info->ppush_conf);
			g_ppush_info->ppush_conf = NULL;
		}

		free(g_ppush_info);
		g_ppush_info = NULL;
	}

	if(g_ppush_cfg)
	{
		if(g_ppush_cfg->pdefault_rec_path)
		{
			free(g_ppush_cfg->pdefault_rec_path);
			g_ppush_cfg->pdefault_rec_path = NULL;
		}

		if(g_ppush_cfg->pdevice_list_file)
		{
			free(g_ppush_cfg->pdevice_list_file);
			g_ppush_cfg->pdevice_list_file = NULL;
		}

		if(g_ppush_cfg->plog_file_path)
		{
			free(g_ppush_cfg->plog_file_path);
			g_ppush_cfg->plog_file_path = NULL;
		}

		if(g_ppush_cfg->ptoken_list_file)
		{
			free(g_ppush_cfg->ptoken_list_file);
			g_ppush_cfg->ptoken_list_file = NULL;
		}

		if(g_ppush_cfg->ptoken_server_url)
		{
			free(g_ppush_cfg->ptoken_server_url);
			g_ppush_cfg->ptoken_server_url = NULL;
		}
		if(g_ppush_cfg->plive_token_server_url)
		{
			free(g_ppush_cfg->plive_token_server_url);
			g_ppush_cfg->plive_token_server_url = NULL;
		}
		free(g_ppush_cfg);
		g_ppush_cfg = NULL;
	}

	if(g_ppush_channel_ctx)
	{
		if(g_ppush_channel_ctx->padtshead)
		{
			free(g_ppush_channel_ctx->padtshead);
			g_ppush_channel_ctx->padtshead = NULL;
		}

		if(g_ppush_channel_ctx->pchannel)
		{
			free(g_ppush_channel_ctx->pchannel);
			g_ppush_channel_ctx->pchannel = NULL;
		}

		if(g_ppush_channel_ctx->pcloudctrl)
		{
			free((void*)g_ppush_channel_ctx->pcloudctrl);
			g_ppush_channel_ctx->pcloudctrl = NULL;
		}

		if(g_ppush_channel_ctx->pcloudflag)
		{
			free((void*)g_ppush_channel_ctx->pcloudflag);
			g_ppush_channel_ctx->pcloudflag = NULL;
		}

		if(g_ppush_channel_ctx->pcloudrecheck)
		{
			free((void*)g_ppush_channel_ctx->pcloudrecheck);
			g_ppush_channel_ctx->pcloudrecheck = NULL;
		}

		if(g_ppush_channel_ctx->pcloudsend)
		{
			free((void*)g_ppush_channel_ctx->pcloudsend);
			g_ppush_channel_ctx->pcloudsend = NULL;
		}

		if(g_ppush_channel_ctx->pcloudupcheck)
		{
			free((void*)g_ppush_channel_ctx->pcloudupcheck);
			g_ppush_channel_ctx->pcloudupcheck = NULL;
		}

		if(g_ppush_channel_ctx->ppair)
		{
			free((void*)g_ppush_channel_ctx->ppair);
			g_ppush_channel_ctx->ppair = NULL;
		}

		if(g_ppush_channel_ctx->precstart)
		{
			free((void*)g_ppush_channel_ctx->precstart);
			g_ppush_channel_ctx->precstart = NULL;
		}

		if(g_ppush_channel_ctx->pspsppsbuff)
		{
			free((void*)g_ppush_channel_ctx->pspsppsbuff);
			g_ppush_channel_ctx->pspsppsbuff = NULL;
		}

		if(g_ppush_channel_ctx->psunvalley_cloud_id)
		{
			free((void*)g_ppush_channel_ctx->psunvalley_cloud_id);
			g_ppush_channel_ctx->psunvalley_cloud_id = NULL;
		}

		free(g_ppush_channel_ctx);
		g_ppush_channel_ctx = NULL;
	}
}

int main(int argc, char *argv[])
{
	int ret;
	int channel;
	g_ppush_info = (push_ctx*)malloc(sizeof(push_ctx));
	memset(g_ppush_info, 0, sizeof(push_ctx));
	parser_args(argv, argc, g_ppush_info, on_parser_option);
	if(NULL == g_ppush_info->ppush_conf)
	{
		printf("please specify push conf file!\n");
		return -1;
	}
	if(g_ppush_info->npush_num <= 0)
	{
		g_ppush_info->npush_num = 1;
	}
	printf("g_ppush_info->npush_num:%d, conf_file:%s\n", g_ppush_info->npush_num, g_ppush_info->ppush_conf);
	init_channel_context(g_ppush_info->ppush_conf);

	g_cur_push_channel_num = g_ppush_info->npush_num;
	if(g_ppush_info->ntigger_type > 0)
	{
		g_ppush_cfg->ntigger_type = g_ppush_info->ntigger_type;
	}

	if(g_ppush_info->ntigger_random_time > 0)
	{
		g_ppush_cfg->ntigger_random_sec = g_ppush_info->ntigger_random_time;
	}
	/*if(argc >= 2)
	{
		g_cur_push_channel_num = atoi(argv[1]);
		g_cur_push_channel_num = g_cur_push_channel_num > g_ppush_cfg->nmax_channel_num ? g_ppush_cfg->nmax_channel_num : g_cur_push_channel_num;
		//printf("");
	}*/
	printf("-----------------------------------\n");
	printf("VAVA CloudDemo v1.0 @2018 Sunvalley\n");
	printf("-----------------------------------\n");
	init_channel();
	signal(SIGINT,  sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);
	signal(SIGPIPE, sig_continue);
	//load_push_config(g_ppush_info->ppush_conf);

	//printf("g_ppush_cfg->pdevice_list_file:%s, g_ppush_cfg->ptoken_list_file:%s\n", g_ppush_cfg->pdevice_list_file, g_ppush_cfg->ptoken_list_file);
	if(g_ppush_cfg->pdevice_list_file)
	{
		ret = init_pair_from_sn_file(g_ppush_cfg->pdevice_list_file);//("./sn.txt");
		//lbtrace("ret:%d = init_pair_from_sn_file(%s)\n", ret, g_ppush_cfg->pdevice_list_file);
		if(ret < 0)
		{
			printf("ret:%d = init_pair_from_sn_file(%s) failed\n", ret, g_ppush_cfg->pdevice_list_file);
			return 0;
		}
	}
	else if(g_ppush_cfg->ptoken_list_file)
	{
		ret = init_pair_from_token_file(g_ppush_cfg->ptoken_list_file);
		lbtrace("ret:%d = init_pair_from_token_file(%s)\n", ret, g_ppush_cfg->ptoken_list_file);
		if(ret < 0)
		{
			printf("ret:%d = init_pair_from_token_file(%s) failed\n", ret, g_ppush_cfg->ptoken_list_file);
			return 0;
		}
	}
	
	//init_pair();
	printf("g_cur_push_channel_num:%d\n", g_cur_push_channel_num);
	ret = init_cloud_sunvalley();
	if(ret != 0)
	{
		printf("init cloud server fail\n");
		return -1;
	}
	printf("before init_sps_pps\n");
	ret = init_sps_pps();//init_spspps();
	lbtrace("ret:%d = init_sps_pps()\n", ret);
	if(ret != 0)
	{
		printf("init sps pps fail\n");
		return -1;
	}

	for(channel = 0; channel < g_cur_push_channel_num; channel++)
	{
		ret = init_cloud_rec_service(channel);
		if(ret != 0)
		{
			printf("init rec server fail, channel = %d\n", channel);
			return -1;
		}
	}

	for(channel = 0; channel < g_cur_push_channel_num; channel++)
	{
		ret = init_random(channel);
		if(ret != 0)
		{
			printf("init random server fail, channel = %d\n", channel);
			return -1;
		}
	}

	ret = init_cputotol();
	if(ret != 0)
	{
		printf("init cputotol server fail, channel = %d\n", channel);
		return -1;
	}

	while(g_running)
	//while(g_running)
	{
		/*ret = system("ls /tmp/log");
		if(ret < 0)
		{
			printf("call system failed ret:%d,errno:%d, reason:%s", ret, errno, strerror(errno));
		}*/
		usleep(100000);
	}
/*#else
	// m_fd = ::socket(AF_INET, SOCK_STREAM, 0);
	int fd = -1;
	if(ret < 0)
	{
		printf("connenct failed, ret:%d", ret);
		return -1;
	}
	char buf[4096] = {'a'};
	while(g_running)
	//while(g_running)
	{
		if(fd < 0)
		{
			sleep(1);
			fd = connect_socket();
			continue;
		}
		//sleep(1);
		ret = ::send(fd, buf, 4096, 0);
		if(ret < 0)
		{
			int vmpeak = 0, vmmem = 0, peakmem = 0, mem = 0, datamem = 0, stackmem = 0, exemem = 0, libmem = 0;
			int pid = getpid();
			get_proc_mem(pid, &vmpeak, &vmmem, &peakmem, &mem, &datamem, &stackmem, &exemem, &libmem);
			printf("pid:%d, cpu = %.2f%%, vmpeak:%dKB, vmmem:%dKB, peakmem:%dKB, mem:%dKB, datamem:%dKB, stackmem:%dKB, exemem:%dKB, libmem:%dKB\n", pid, get_proc_cpu(pid), vmpeak, vmmem, peakmem, mem, datamem, stackmem, exemem, libmem);
			printf("send socket data failed!, ret:%d", ret);
			close(fd);
			fd = -1;
		}
		usleep(20000);
	};
#endif*/
	g_running = 0;
	//usleep(100000);
	sleep(1);
	SVPush_API_UnInitialize();
	destroy_demo();
	//pthread_detach();
	return 0;
}

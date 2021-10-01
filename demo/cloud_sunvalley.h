#ifndef _CLOUD_FSK_H_
#define _CLOUD_FSK_H_
#include "basetype.h"
#include "rec_demux.h"
#include "isvpush.h"
/*#ifdef PUSH_SDK_LINUX
#define rec_file_dir "./data/"
#else
#define rec_file_dir "/mnt/sd0/rec/"
#endif*/
#define ENABLE_HS004_RECORD
typedef struct push_config_context
{
    char*       plog_file_path;
    char*       ptoken_server_url;
	char*		plive_token_server_url;
    char*       pdefault_rec_path;
    char*       pdevice_list_file;
	char* 		ptoken_list_file;
	char*		ppush_server_url;

	struct auth_info*	pai;
	// auth info
	char*		plogin_server;
	char*		pclient_id;
	char*		pclient_secret;
	char*		pscope;
	char*		pgrant_type;
	char*		pauth_type;
	int			enc_flag;


    int         nlog_level;
    int         nlog_output_mode;
    int         nmax_channel_num;
    int         ntigger_random_sec;
	int			ntigger_type;

}push_cfg;

typedef struct push_channel_contex
{
	int*						pchannel;
	VAVA_PairInfo*				ppair;
	volatile long* 				psunvalley_cloud_id;
	volatile unsigned char* 	pcloudflag;
	volatile unsigned char* 	pcloudsend;
	volatile unsigned char* 	pcloudupcheck;
	volatile unsigned char* 	pcloudrecheck;
	volatile unsigned char* 	pcloudctrl;
	volatile unsigned char* 	precstart;
    unsigned char* 				pspsppsbuff;
	unsigned int 				nspsppssize;
	unsigned char*				padtshead;
	int							nadtssize;
}push_channel_ctx;

extern int g_running;
extern push_channel_ctx*		g_ppush_channel_ctx;
extern push_cfg*				g_ppush_cfg;

int load_push_config(const char* pcfg_path);

void *cloud_suvalley_pth(void *data);
void *Rec_Cloud_pth(void *data);

#endif

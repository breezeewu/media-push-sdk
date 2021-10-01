#include <stdio.h>
#include <vector>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "RecordMuxer.h"
#include "../include/isvpush.h"
#ifndef INT64_MIN
#define INT64_MIN        (-9223372036854775807 - 1)
#endif
#define MAX_CHANNEL_NUM			20
#define TOKEN_SERVER_URL	"https://storage-test.sunvalleycloud.com/ipc/connection/token/get"
#define CHECK_RESULT(ret) if(ret < 0) {assert(0); return ret;}
//P020101000101191216300001
#define TEST_DEVICE_SN		"P020101000101191216300001"
#define READ_AND_SEND_H264_RAW_STREAM
bool g_brunning = false;
void sig_stop(int signo)
{
	printf("sig_stop: signo = %d\n", signo);
	g_brunning = false;
	return;
}

int vava_event_callback(long lCID, long eventType, long wparam, long lparam)
{
        switch(eventType)
        {
            case E_SV_EVENT_TYPE_RTMP_INVALID_URL:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_DNS_RESOVLE_FAILED:
            case E_SV_EVENT_TYPE_RTMP_SOCKET_CONNECT_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_HANDSHAKE_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_APP_FAILED:
            case E_SV_EVENT_TYPE_RTMP_PUBLISH_STREAM_FAILED:
            case E_SV_EVENT_TYPE_RTMP_CONNECT_SET_TIMEOUT_FAILED:
            case E_SV_EVENT_TYPE_RTMP_INVALID_PARAMETER:
            {
                printf("lcid:%ld connect error, errorcode:%ld\n", lCID, eventType);
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE:
            {
                printf("lcid:%ld connection close\n", lCID);
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_SUCCESS:
            {
                printf("lcid:%ld connection success\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_AUDIO_ADTS_DATA_ERROR:
            case E_SV_EVENT_TYPE_RTMP_VIDEO_START_CODE_ERROR:
            case E_SV_EVENT_TYPE_RTMP_RECV_AUDIO_DATA_ERROR:
            case E_SV_EVENT_TYPE_RTMP_SEND_VIDEO_NO_SPS_PPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS_PPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_SPS:
            case E_SV_EVENT_TYPE_RTMP_SEND_ERROR_PPS:
            {
                printf("lcid:%ld connection error, errorcode:%ld\n", lCID, eventType);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH:
            {
                printf("lcid:%ld connection resume media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH:
            {
                printf("lcid:%ld connection pause media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR:
            {
                printf("lcid:%ld connection error, push thread exit\n", lCID);
                g_brunning = false;
                //ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_OPEN:
            {
                g_brunning = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_CLOSE:
            {
                g_brunning = false;
                return 0;
            }
            default:
            {
                 printf("lcid:%ld connection unknown error,  errorcode:%ld\n", lCID, eventType);
                return 0;
            }

        }
}

E_SV_MEDIA_TYPE code_id_2_media_type(int codec_id)
{
    switch(codec_id)
    {
        case codec_id_none:
        {
            return E_SV_MEDIA_TYPE_UNKNOW;
        }
        case codec_id_h264:
        {
            return E_SV_MEDIA_TYPE_H264;
        }
        case codec_id_h265:
        {
            return E_SV_MEDIA_TYPE_H265;
        }
        case codec_id_mp3:
        {
            return E_SV_MEDIA_TYPE_MP3;
        }
        case codec_id_aac:
        {
            return E_SV_MEDIA_TYPE_AAC;
        }
        default:
        {
            return E_SV_MEDIA_TYPE_UNKNOW;
        }
    }
}

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
    //time_t local_time = mktime(&tm_local);

    // Change it to GMT tm
    gmtime_r(&time_utc, &tm_gmt);
    //time_t gmt_time = mktime(&tm_gmt);
    //printf("local_time:%lld, gmt_time:%lld\n", local_time, gmt_time);
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
    
	gettimeofday(&tv, NULL);
	int64_t cur_time =  tv.tv_sec;
	cur_time = time_zone * 1000 + cur_time * 1000 + tv.tv_usec/1000;
    return cur_time;
}

int64_t get_current_time()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    int64_t curtime = (int64_t)tv.tv_sec*1000 + tv.tv_usec/1000;
    return curtime;
}

#ifdef READ_AND_SEND_H264_RAW_STREAM
FILE* g_pfile = NULL;
int g_nframe_num = 0;
int read_h264_raw_packet(ipc_packet_header* piph, char* pbuf, int size)
{
    if(NULL == g_pfile)
    {
        g_pfile = fopen("../../H264_data2.h264", "wb");
    }

    if(g_pfile)
    {
        int frame_size = 0;
        int read_len = fread(&frame_size, 4, 1, g_pfile);
        if(read_len <= 0)
        {
            printf("read_len:%d = fread(&frame_size, 4, 1, g_pfile) failed\n", read_len);
            return -1;
        }
        piph->mt = 0;
        piph->codec_id = 0;
        piph->keyflag = 0;
        piph->frame_num = g_nframe_num;
        piph->pts = g_nframe_num * 66;
        
        read_len = fread(pbuf, 1, frame_size, g_pfile);
        if(read_len <= 0)
        {
            printf("read_len:%d = fread(pbuf, 1, size, g_pfile) failed\n", read_len);
            return -1;
        }
        piph->size = read_len;
        return read_len;
    }

    return 0;
}
#endif

int main(int argc, char** args)
{
	char ver[256];
	struct timeval tv;
	int ret = 0;
	long lcid = -1;
	const char* pkeystring = "vavakey";

	// get sdk version
    SVPush_API_Version(ver, 256);
    printf(ver);
    printf("\n sunvalley cloud storage push sdk example\n");

	//sdk initialize
    ret = SVPush_API_Initialize(1024*1024, MAX_CHANNEL_NUM);
    CHECK_RESULT(ret);

    ret = SVPush_API_Init_log((E_SV_LOG_LEVEL)E_LOG_LEVEL_INFO, SV_LOG_OUTPUT_MODE_CONSOLE | SV_LOG_OUTPUT_MODE_FILE, "./log/");
    CHECK_RESULT(ret);

    ret = SVPush_API_Set_Token_Server_Interface(TOKEN_SERVER_URL);
    CHECK_RESULT(ret);
    RecordDemux recdemux;
    signal(SIGINT,  sig_stop);
	signal(SIGQUIT, sig_stop);
	signal(SIGTERM, sig_stop);
	// connect to media server
    ipc_record_info rec_info;
    ipc_packet_header iph;
    memset(&iph, 0, sizeof(iph));
    const char* purl = "demo.data";
    ret = recdemux.open(purl, &rec_info);
    if(ret < 0)
    {
        printf("ret:%d = recdemux.open(purl:%s, &rec_info) failed, Invalid record url\n", ret, purl);
        return -1;
    }
	lcid = SVPush_API_Connect_By_DeviceSN(NULL, TEST_DEVICE_SN, vava_event_callback);
	if(lcid < 0)
	{
		printf("push sdk connect media server failed, lcid:%ld, TEST_DEVICE_SN:%s\n", lcid, TEST_DEVICE_SN);
		assert(0);
		return -1;
	}

	//gettimeofday(&tv, NULL);
    //printf("minute:%ld, tz_dsttime:%ld", tz.tz_minuteswest, tz.tz_dsttime);
	unsigned long long untsamp = get_local_timestamp();//((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_VIDEO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME, (long)(pkeystring));
	CHECK_RESULT(ret);
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_AUDIO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME, (long)(pkeystring));
	CHECK_RESULT(ret);
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_STREAM_START, E_IPC_TIGGER_TYPE_FACE, (long)(&untsamp));
	CHECK_RESULT(ret);
	g_brunning = true;
    char* pbuf = (char*)malloc(1024*0124);
    const int max_buf_len = 1024*0124;
    int64_t lstart_time = INT64_MIN;
    int64_t lstart_pts = 0;
    do
    {
        if(iph.size <= 0)
        {
#ifdef READ_AND_SEND_H264_RAW_STREAM
            ret = read_h264_raw_packet(&iph, pbuf, max_buf_len);
#else
             ret = recdemux.readframe(&iph, pbuf, max_buf_len);
#endif
            if(ret < 0)
            {
                printf("ret:%d = recdemux.readframe(&iph, pbuf, max_buf_len) failed\n", ret);
                break;
            }
            if(INT64_MIN == lstart_time)
            {
                lstart_time = get_current_time();
                lstart_pts = iph.pts;
            }
        }

        if(get_current_time() - lstart_time < iph.pts - 20 - lstart_pts)
        {
            usleep(20000);
            continue;
        }
		//encflag = 0;
        ret = SVPush_API_Send_Packet(lcid, (E_SV_MEDIA_TYPE)iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag, 0);
        printf("send packet pts:%" PRId64 ", size:%d\n", iph.pts, iph.size);
	    if(ret < 0)
        {
            printf("ret:%d = SVPush_API_Send_Packet(lcid:%ld, media_type:%d, pbuf:%p, iph.size:%d, iph.pts:%" PRId64 ", iph.keyflag:%d, 0)\n", ret, lcid, iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag);
            break;
        }
        iph.size = 0;
    } while (g_brunning);
	printf("after while, g_brunning:%d\n", (int)g_brunning);
	gettimeofday(&tv, NULL);
	untsamp = ((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
	// segment end alarm time 10000ms
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_STREAM_END, 10000, (long)(&untsamp));
	CHECK_RESULT(ret);
    if(pbuf)
    {
        free(pbuf);
        pbuf = NULL;
    }
    SVPush_API_UnInitialize();

    return 0;
}
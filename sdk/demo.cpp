#include <stdio.h>
#include <vector>
#include <signal.h>
#include <assert.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include "RecordMuxer.h"
#include "../common/isvpush.h"
#define MAX_CHANNEL_NUM			20
#define TOKEN_SERVER_URL		"http://10.30.0.200:2118/connection/token/get"
#define CHECK_RESULT(ret) if(ret < 0) {assert(0); return ret;}
#define TEST_DEVICE_SN		"P020101000101200707400001"
bool g_brunning = false;
void sig_stop(int signo)
{
	printf("sig_stop: signo = %d\n", signo);
	g_brunning = false;
	return;
}

int vava_event_callback(long lCID, long eventType, long wparam, long lparam)
{
        //CAutoLock lock(m_crisec);

        /*std::map<long, CPushTask*>::iterator it = m_mconnect_task_list.find(lCID);
        if(it == m_mconnect_task_list.end())
        {
            printf("Invalid connect task id:%ld eventType:%ld, wparam:%ld, lparam:%ld\n", lCID, eventType, wparam, lparam);
            //assert(0);
            return -1;
        }*/

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
    //time_t local_timestamp = mktime(&tm_local);

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
    
	gettimeofday(&tv, NULL);
	int64_t cur_time =  tv.tv_sec;
	cur_time = time_zone * 1000 + cur_time * 1000 + tv.tv_usec/1000;
    return cur_time;
}

#define DEFAULT_RECORD_URL          "./data/154632_1_0_d"
int main(int argc, char** args)
{
    const char* purl = DEFAULT_RECORD_URL;
    if(argc <= 1)
    {
        purl = args[1];
    }
	char ver[256];
	struct timeval tv;
	int ret = 0;
	long lcid = -1;
	const char* pkeystring = "vavakey";
	//char* pvideodata = NULL;
	//int video_data_len = 0;
	//char* paudiodata = NULL;
	//int audio_data_len = 0;
	//int64_t pts = 0;
	//int keyflag = 0;
	int encflag = 0;
	// get sdk version
    SVPush_API_Version(ver, 256);
    printf(ver);
    printf("\n sunvalley cloud storage push sdk example\n");

	//sdk initialize
    ret = SVPush_API_Initialize(1024*1024, MAX_CHANNEL_NUM);
    CHECK_RESULT(ret);

    ret = SVPush_API_Init_log((E_SV_LOG_LEVEL)E_LOG_LEVEL_MAIN, SV_LOG_OUTPUT_MODE_CONSOLE | SV_LOG_OUTPUT_MODE_FILE, "./log/");
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
    //har* purl = ;
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

	unsigned long long untsamp = get_local_timestamp();
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_VIDEO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME, (long)(pkeystring));
	CHECK_RESULT(ret);
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_AUDIO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME, (long)(pkeystring));
	CHECK_RESULT(ret);
	ret = SVPush_API_Send_Metadata(lcid, E_SV_METADATA_TYPE_STREAM_START, E_IPC_TIGGER_TYPE_FACE, (long)(&untsamp));
	CHECK_RESULT(ret);
	g_brunning = true;
    char* pbuf = (char*)malloc(1024*0124);
    const int max_buf_len = 1024*0124;
    do
    {
        ret = recdemux.readframe(&iph, pbuf, max_buf_len);
        if(ret < 0)
        {
            printf("ret:%d = recdemux.readframe(&iph, pbuf, max_buf_len) failed\n", ret);
            break;
        }
		encflag = 0;
        ret = SVPush_API_Send_Packet(lcid, (E_SV_MEDIA_TYPE)iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag, encflag);
        printf("ret:%d = SVPush_API_Send_Packet(lcid:%ld, media_type:%d, pbuf:%p, iph.size:%d, iph.pts:%" PRId64 ", iph.keyflag:%d, 0)\n", ret, lcid, iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag);
        if(ret < 0)
        {
            break;
        }
        /*if(iph.codec_id == codec_id_h264 || codec_id_h265 == iph.codec_id)
        {
            // read h264 data from file(annexb format)
            // ...
            //send h264 data packet
            ret = SVPush_API_Send_Packet(lcid, iph.codec_id, pbuf, iph.size, iph.pts, iph.keyflag, 0);
        }
		
		// read aac packet data from file
		// ...
		//send aac data packet(with adts header)
		keyflag = 1;
       	ret = SVPush_API_Send_Packet(lcid, E_SV_MEDIA_TYPE_AAC, paudiodata, audio_data_len, pts, keyflag, encflag);*/
		usleep(30000);
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
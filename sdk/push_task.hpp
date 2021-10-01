#include <stdio.h>
#include <stdint.h>
#include "sv_log.h"
#include "lbsp_utility_thread.hpp"
#include "isvpush.h"
#include <string>
#include <map>
#include "lbsp_ipc_record.hpp"
#include "autolock.h"
#define INT64_MIN       -1
using namespace lbsp_util;
class CPushTask:public CLazyThread
{
protected:
    std::string     m_rec_path;
    std::string     m_push_url;
    std::string     m_device_sn;
    RecordDemux     m_record_demux;
    bool            m_binit;
    int             m_ntigger_interval;
    long            m_lcid;
    int             m_ntigger_type;
    int             m_nlive_flag;
    int             m_nsend_interval;
    bool            m_bconnected;
    bool            m_bpuase;
    bool            m_btigger_change;
    unsigned long   m_ullast_timestamp;

    static CCriSec         m_crisec;

    static std::map<long, CPushTask*>   m_mconnect_task_list;
public:
    CPushTask()
    {
        m_binit = false;
        m_ntigger_interval = 0;
        m_lcid = -1;
        m_ntigger_type = E_IPC_TIGGER_TYPE_DEFAULT;
        m_nlive_flag = 0;
        m_bconnected = false;
        m_ullast_timestamp = 0;
        m_btigger_change = false;
        m_nsend_interval = 40;
        
    }

    virtual int open_camera(const char* prec_path, int tigger_type, int tigger_interval, int send_interval)
    {
        lbtrace("(prec_path:%s, tigger_type:%d, tigger_interval:%d)\n", prec_path, tigger_type, tigger_interval);
        m_rec_path = prec_path;
        int ret = m_record_demux.open(prec_path);
        CHECK_RESULT(ret);
        m_binit = true;
        m_ntigger_type = tigger_type;
        m_ntigger_interval = tigger_interval;
        m_nsend_interval = send_interval;
        m_record_demux.close();
        return ret;
    }

    virtual int set_connect_url(const char* purl)
    {
        if(purl)
        {
            m_push_url = purl;
        }
        
        return 0;
    }

    virtual int set_device_sn(const char* pdeviecsn)
    {
        if(pdeviecsn)
        {
            m_device_sn = pdeviecsn;
        }
        
        return 0;
    }

    virtual int on_thread_start()
    {
        if(!m_binit)
        {
            return -1;
        }
        return 0;
    }

    virtual int on_cycle()
    {
        int64_t lstart_pts = INT64_MIN;
        int64_t pts_offset = 0;
        int64_t pts = 0;
        int64_t last_pts = 0;
        int i = 0;
        bool bpause = false;
        ipc_packet_header pkthdr;
        const int data_len = 1024*1024;
        //lbtrace("on_cycle begin 2\n");
        int ret = connect();
        //lbtrace("ret:%d = connect()\n", ret);
        CHECK_RESULT(ret);

        if(!m_record_demux.open(m_rec_path.c_str()))
        {
            lberror("ret:%d = m_record_demux.open(%s) failed\n", ret, m_rec_path.c_str());
            return -1;
        }
        if(m_ntigger_interval > 0)
        {
            m_ullast_timestamp = GetSysTime();
            lbtrace("on_cycle m_ullast_timestamp:%d\n", m_ullast_timestamp);
        }
        else if(m_ntigger_interval == 0)
        {
            ret = on_segment_begin();
            CHECK_RESULT(ret);
            lbtrace("on_cycle begin ret:%d = on_segment_begin()\n", ret);
        }
        char* pdata = new char[data_len];
        
        while(m_brun && m_bconnected)
        {
            if(i++%500 == 0)
            {
                lbtrace("m_ntigger_interval:%d, m_ullast_timestamp:%lu, GetSysTime():%lu, dur:%lu\n", m_ntigger_interval, m_ullast_timestamp, GetSysTime(), GetSysTime() - m_ullast_timestamp);
            }
            if(m_ntigger_interval < 0)
            {
                CLazyThread::lazy_sleep(20);
                continue;
            }
            if(m_ullast_timestamp > 0 && GetSysTime() - m_ullast_timestamp < m_ntigger_interval)
            {
                CLazyThread::lazy_sleep(20);
                continue;
            }
            else if(m_ullast_timestamp > 0 && GetSysTime() - m_ullast_timestamp >= m_ntigger_interval)
            {
                lbtrace("on_segment_begin(), m_ullast_timestamp:%lu\n", m_ullast_timestamp);
                 m_ullast_timestamp = 0;
                 ret = m_record_demux.open(m_rec_path.c_str());
                 CHECK_RESULT(ret);
                 ret = on_segment_begin();
                 CHECK_RESULT(ret);
            }

            if(m_btigger_change)
            {
                m_btigger_change = false;
                break;
            }
           
            int pktsize = m_record_demux.readframe(&pkthdr, pdata, data_len);
            //lbtrace("pktsize:%d = m_record_demux.readframe(&pkthdr, pdata:%p, data_len:%d, &pts:%" PRId64 ")", pktsize, pdata, data_len, pts);
            if(pktsize <= 0)
            {
                lbtrace("pktsize:%d = m_record_demux.readframe(&pkthdr, pdata, data_len, &pts)", pktsize);
                m_bpuase = true;
                if(m_ntigger_interval > 0 && !m_nlive_flag)
                {
                    ret = on_segment_end();
                    CHECK_RESULT(ret);
                    m_ullast_timestamp = GetSysTime();
                    pts_offset = 0;
                    lbtrace("on_segment_end end becase eof\n");
                }
                else
                {
                    m_record_demux.close();
                    ret = m_record_demux.open(m_rec_path.c_str());
                    CHECK_RESULT(ret);
                    pts_offset += last_pts;
                    lstart_pts = INT64_MIN;
                    lbtrace("while eof, continue push without new segment， pts_offset:%" PRId64 ", pts:%" PRId64 "\n", pts_offset, pkthdr.pts);
                }
                continue;
            }
            if(INT64_MIN == lstart_pts)
            {
                lstart_pts = pkthdr.pts;
            }
            pkthdr.pts -= lstart_pts;
            last_pts = pkthdr.pts;
            //E_SV_MEDIA_TYPE mt = pkthdr.type == 8 ? E_SV_MEDIA_TYPE_AAC:E_SV_MEDIA_TYPE_H264;
            //int keyflag = pkthdr.type == 0 ? 0 : 1;
            ret = SVPush_API_Send_Packet(m_lcid, (E_SV_MEDIA_TYPE)pkthdr.codec_id, pdata, pktsize, pts_offset + pkthdr.pts, pkthdr.keyflag, 0);
            //lbtrace("ret:%d = SVPush_API_Send_Packet(m_lcid:%lu, codec_id:%d, pdata:%p, pktsize:%d, pts_offset + pts:%" PRId64 ", pkthdr.keyflag:%d, 0), pts_offset:%" PRId64 "\n", ret, m_lcid, pkthdr.codec_id, pdata, pktsize, pts_offset + pkthdr.pts, pkthdr.keyflag, pts_offset);
            if(ret < 0)
            {
                lberror("ret:%d = SVPush_API_Send_Packet(m_lcid:%ld, codec_id:%d, pdata:%p, pktsize:%d, %" PRId64 ", %d, 0) failed\n", ret, m_lcid, pkthdr.codec_id, pdata, pktsize, pts_offset + pkthdr.pts, pkthdr.keyflag);
                break;
            }
            usleep(m_nsend_interval*1000);
            //lbtrace("send interval %d\n", m_nsend_interval);
            //CLazyThread::lazy_sleep(35);
        };
        lbtrace("while end\n");
        ret = on_segment_end();
        CHECK_RESULT(ret);
        close();
        m_record_demux.close();
        if(pdata)
        {
            delete[] pdata;
            pdata = NULL;
        }
        lbtrace("exit do cycyle, ret:%d\n", ret);
        return ret;
    }

    /*virtual int recodec_codec_type_to_media_type(int type)
    {
        switch(type)
        {
            case 
        }
    }*/
    /*virtual int thread_proc()
    {
        int ret = 0;
        ret = on_thread_start();
        if(ret < 0)
        {
            return ret;
        }

        while(m_brun && m_bconnected)
        {
            
        }

        ret = on_thread_stop();
        
        return ret;
    }*/

    virtual int on_thread_stop()
    {
        return 0;
    }

protected:

    virtual int on_segment_begin()
    {
        struct timeval tv;
        int ret = 0;
        char* pkeystring = "vavalic2";
        if(m_lcid < 0)
        {
            assert(0);
            return -1;
        }

        gettimeofday(&tv, NULL);
		unsigned long long untsamp = ((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
        ret = SVPush_API_Send_Metadata(m_lcid, E_SV_METADATA_TYPE_VIDEO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_KEY_FRAME, (long)(pkeystring));
        CHECK_RESULT(ret);
		ret = SVPush_API_Send_Metadata(m_lcid, E_SV_METADATA_TYPE_AUDIO_ENCRYPT, E_SV_MEDIA_ENCRYPT_AES_ALL_FRAME, (long)(pkeystring));
        CHECK_RESULT(ret);
		ret = SVPush_API_Send_Metadata(m_lcid, E_SV_METADATA_TYPE_STREAM_START, m_ntigger_type, (long)(&untsamp));
        CHECK_RESULT(ret);

        return ret;
    }

    virtual int on_segment_end()
    {
        struct timeval tv;
        int ret = 0;
        if(m_lcid < 0)
        {
            lberror("m_lcid:%ld error\n", m_lcid);
            assert(0);
            return -1;
        }

        gettimeofday(&tv, NULL);
		unsigned long long untsamp = ((unsigned long long)tv.tv_sec /*+3600*16*/) * 1000 + tv.tv_usec / 1000;
        ret = SVPush_API_Send_Metadata(m_lcid, E_SV_METADATA_TYPE_STREAM_END, 1, (long)(&untsamp));
        CHECK_RESULT(ret);

        return ret;
    }

    virtual int connect()
    {
        int ret = 0;
        printf("connect begin, m_lcid:%ld, m_device_sn:%s\n", m_lcid, m_device_sn.c_str());
        if(m_lcid >= 0)
        {
            lbtrace("m_lcid:%ld has already connect\n", m_lcid);
            return -1;
        }
        if(!m_push_url.empty())
        {
            m_lcid = SVPush_API_Connect(m_push_url.c_str(), NULL, CPushTask::event_callback);
            if(m_lcid < 0)
            {
                lberror("m_lcid:%ld = SVPush_API_Connect(m_push_url:%s, NULL, CPushTask::event_callback) failed\n", m_lcid, m_push_url.c_str());
                return m_lcid;
            }
        }
        else if(!m_device_sn.empty())
        {
            m_lcid = SVPush_API_Connect_By_DeviceSN(NULL, NULL, m_device_sn.c_str(), CPushTask::event_callback);
            if(m_lcid < 0)
            {
                lberror("m_lcid:%ld = SVPush_API_Connect_By_DeviceSN(NULL, NULL, %s, CPushTask::event_callback) failed\n", m_lcid, m_device_sn.c_str());
                return m_lcid;
            }
        }
        m_bconnected = true;
        m_mconnect_task_list[m_lcid] = this;
        lbtrace("m_lcid:%ld\n", m_lcid);
        return 0;
    }

    virtual void close()
    {
        if(m_lcid < 0)
        {
            return ;
        }
        SVPush_API_Close(m_lcid);
        CAutoLock lock(m_crisec);

        std::map<long, CPushTask*>::iterator it = m_mconnect_task_list.find(m_lcid);
        if(it == m_mconnect_task_list.end())
        {
            lberror("Invalid connect task id:%ld\n", m_lcid);
            assert(0);
            return ;
        }
        else
        {
            m_mconnect_task_list.erase(it);
        }
        m_bconnected = false;
        m_lcid = -1;
    }

    virtual int notify_live_status(int opflag, int tigger_type)
    {
        m_nlive_flag = opflag;
        m_ntigger_type = tigger_type;
        m_btigger_change = true;

        return 0;
    }

    static int event_callback(long lCID, long eventType, long wparam, long lparam)
    {
        CAutoLock lock(m_crisec);

        std::map<long, CPushTask*>::iterator it = m_mconnect_task_list.find(lCID);
        if(it == m_mconnect_task_list.end())
        {
            lberror("Invalid connect task id:%ld eventType:%ld, wparam:%ld, lparam:%ld\n", lCID, eventType, wparam, lparam);
            //assert(0);
            return -1;
        }

        CPushTask* ptask = it->second;
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
                lberror("lcid:%ld connect error, errorcode:%d\n", lCID, eventType);
                ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE:
            {
                lbtrace("lcid:%ld connection close\n", lCID);
                ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_CONNECT_SUCCESS:
            {
                lbtrace("lcid:%ld connection success\n", lCID);
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
                lberror("lcid:%ld connection error, errorcode:%d\n", lCID, eventType);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH:
            {
                lbtrace("lcid:%ld connection resume media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH:
            {
                lbtrace("lcid:%ld connection pause media push\n", lCID);
                return 0;
            }
            case E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR:
            {
                lberror("lcid:%ld connection error, push thread exit\n", lCID);
                ptask->m_bconnected = false;
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_OPEN:
            {
                lbtrace("lcid:%ld connection echoshow live open\n", lCID);
                ptask->notify_live_status(1, E_IPC_TIGGER_TYPE_ALEXA);
                return 0;
            }
            case E_SV_EVENT_TYPE_ECHOSHOW_NOTIFY_LIVE_CLOSE:
            {
                lbtrace("lcid:%ld connection echoshow live close\n", lCID);
                ptask->notify_live_status(0, E_IPC_TIGGER_TYPE_ALEXA);
                return 0;
            }
            default:
            {
                 lberror("lcid:%ld connection unknown error,  errorcode:%ld\n", lCID, eventType);
                return 0;
            }

        }
    }

};

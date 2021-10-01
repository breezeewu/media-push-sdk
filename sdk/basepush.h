/***************************************************************************************************************
 * filename     basepush.h
 * describe     Sunvalley cloud storage push sdk push base protocol class definition
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 **************************************************************************************************************/
#ifndef _BASE_PUSH_H_
#define _BASE_PUSH_H_
#include "config.h"
#include "irtmpprotocol.h"
#include "sv_thread.h"
#include "framequeue.h"
#include <string>
#include "ipush.h"
#define ENABLE_DUMP_SYS_INFO
extern int g_nMemoryPoolSize;
extern int g_nMaxConnectionCount;

#define ENABLE_GEN_URL_BY_TOKEN
class IPushProtocol;
class CPushCallback:public IEventNotify
{
public:
    CPushCallback()
    {
        m_pEventcb = NULL;
        m_pEventNotify = NULL;
        m_lcid = -1;
    }
    
    void SetCallBack(long lcid, event_callback peventcallback)
    {
        m_lcid = lcid;
        m_pEventcb = peventcallback;
    }
    
    void SetCallBack(IEventNotify* pcbnotify)
    {
        m_pEventNotify = pcbnotify;
    }

    long OnEventNotify(int EventType, long wparam, long lparam)
    {
        if(m_pEventcb)
        {
            return m_pEventcb(m_lcid, (E_SV_EVENT_TYPE)EventType, wparam, lparam);
        }
        else if(m_pEventNotify)
        {
            return m_pEventNotify->OnEventNotify((E_SV_EVENT_TYPE)EventType, wparam, lparam);
        }
        return -1;
    }
    
private:
    event_callback      m_pEventcb;
    IEventNotify*       m_pEventNotify;
    long                m_lcid;
};

class CBasePush:public CLazyThread, public ISVPush
{
public:
    CBasePush();
    virtual ~CBasePush();

    virtual int SetTokenFetctInterface(const char* ptoken_server_url);

    virtual int Connect(const char* purl, const char* ptoken, IEventNotify* pevenhandle);
    
    virtual int Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun);

    virtual int RecordSendPacket(const char* precord_url);

#ifdef ENABLE_GEN_URL_BY_TOKEN
    virtual int ConnectByApp(auth_info* pai, const char* pappstreamname, const char* ptoken, long lcid, event_callback pcbfun);
#endif

    virtual int SendEncryptPacket(int eMediaType, char* pData, unsigned int uSize, int keyframe, int64_t lltimestamp);

    virtual int SendPacket(int eMediaType, char* pData, unsigned int nSize, int64_t llTimeStamp, int keyframe, int encflag);

    virtual int SendMetadata(int eMetadataType, long wParam, long lParam);

    virtual int IsConnected();

    virtual int Close();

protected:
    // connect routine
    virtual int ResolveUrl(const char* pUrl);

    virtual int ConnectServer();

    virtual int ShakeHand();

    virtual int Authorize();

    virtual int ConnectApp();

    virtual int Publish();

    virtual int OnConnect();

    virtual int OnSendPacket(int eMediaType, char* pData, unsigned int uSize, int flag, int64_t llTimeStamp, CFrameBuffer* pframe);

    //virtual int OnSendPacket(int eMediaType, char* pInData, int nInSize, unsigned int nTimeStamp, CFrameBuffer* pframe);

    virtual int OnSendMetadata(int eMetadataType, long wParam, long lParam, CFrameBuffer* pframe);

    //virtual int OnSendMetadata(int eMetadataType, long wParam, long lParam);

    virtual int Unpublish();
    
    virtual void OnRawPacket(int& eMediaType, char*& pData,unsigned int& uSize, int64_t& llTimeStamp);
    
    virtual void OnRtmpMsg(int eMediaType, CFrameBuffer* pframe);

    virtual THREAD_RETURN ThreadProc(); 

    virtual int OnCallbackNotify(E_SV_EVENT_TYPE eEventType, long wParam, long lParam);

    virtual std::string RequestUrl(auth_info* pai, const char* pdeviceSN, const char* ptoken);

#ifdef ENABLE_GEN_URL_BY_TOKEN
    virtual std::string RequestUrlByToken(const char* ptoken);
#endif

    virtual int SendHeartBeat();

    virtual int dump_memory_info();

    virtual int MuxerProc();

    virtual int Start();

    virtual int Stop();


protected:
    bool check_frame(int eMediaType, char* pData, unsigned int nSize, int64_t pts, int& keyframe, int encflag);

    int OnSegmentDeliverStart(int64_t media_timestamp);

    int OnSegmentDeliverEnd();

    int OnSegmentSendStart();

    int OnSegmentSendEnd();

#ifdef ENABLE_DUMP_SYS_INFO
    int get_pro_memory(int pid, int* pmemory, int* pvrtualmemory);

    unsigned long get_cpu_total_occupy();

    const char* get_items(const char*buffer, unsigned int item);

    unsigned long get_cpu_proc_occupy(unsigned int pid);

    float get_proc_cpu(unsigned int pid);
#endif

    void on_recv_packet(int eMediaType, char* pData, unsigned int nSize, int64_t pts, int keyframe, int encflag);
    //virtual std::string RequestUrl(const char* pappkey, const char* pappsecret, const char* pappstreamname);//(std::string& appKey, std::string appSecret, std::string& deviceSN);
protected:
    IEventNotify*   m_pEventNotify;
    CFrameQueue     m_FrameList;
    CCriSec         m_CriSec;
    CFrameBuffer*   m_pFrame;
    CFrameBuffer*   m_pHeartBeatFrame;
    char*           m_pPacketBuffer;
    int             m_nPacketBufLen;
    int             m_bConnected;
    int             m_bDisContinue;
    int             m_nDeliverVideoBytes;
    int             m_nSendVideoBytes;
    int             m_nDeliverAudioBytes;
    int             m_nSendAudioBytes;
    int             m_nVideoDropBytes;
    int             m_nAlarmTime;
    int             m_nstreamType;
    bool            m_bStreamDeliverStart;
    bool            m_bStreamSendStart;
    std::string     m_sToken;
    std::string     m_sappKeyName;
    std::string     m_sappStreamName;
    std::string     m_sGetTokenUrl;
    std::string     m_sURL;
    std::string     m_sDeviceSN;
    std::string     m_sDataTime;
    char*           m_pTag;
    char*           m_pTokenServerUrl;

    CPushCallback*  m_pPushCb;
    bool            m_bBufferFull;
    int64_t         m_llintervalmsgtime;
    int64_t         m_llStartPts;
    int64_t         m_llLastSendTime;
    int64_t         m_llLastPts;
    int64_t         m_llMediaTimeStamp;
    int64_t         m_llLastVideoPts;
    int64_t         m_llLastAudioPts;
    int64_t         m_llLastDeliverVPts;
    int64_t         m_llLastDeliverAPts;
    int64_t         m_llLastSendVPts;
    int64_t         m_llLastSendAPts;
#ifdef WRITE_AVC_DATA_FILE
    m_pavc_file     = NULL;
#endif
    unsigned long   m_lLastFreeMemoryBytes;
    IPushProtocol*  m_pPushProtocol;
#ifdef READ_IPC_RECORD_FILE
    class CRecordReader*    m_pRecordReader;
#endif
    CLazyThreadEx<CBasePush>   m_MuxerThread;
    CFrameQueue     m_MuxerQueue;
    class IRecordMuxer*  m_pRecMuxer;
    class RecordMuxer*  m_pRtmpMuxer;

    char* m_precord_url;


};
#endif


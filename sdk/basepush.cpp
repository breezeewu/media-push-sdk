/****************************************************************************************************************
 * filename     basepush.cpp
 * describe     Sunvalley cloud storage push sdk push base push class implement
 * author       Created by dawson on 2019/04/26
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ****************************************************************************************************************/
#include "basepush.h"
#include "irtmpprotocol.h"
#include "sv_log.h"
#include "lbsp_ipc_record.hpp"
#include <sys/sysinfo.h>
#ifdef READ_IPC_RECORD_FILE
#include "RecordReader.h" 
#endif


CBasePush::CBasePush():m_sGetTokenUrl(GET_TEST_TOKEN_SERVER_URL)
{
    //dump_memory_info();
    m_pEventNotify  = NULL;
    m_pPacketBuffer = NULL;
    m_nPacketBufLen = 0;
    m_bConnected    = 0;
    m_pPushCb       = NULL;
    m_pPushProtocol = NULL;
    m_pFrame        = NULL;
    m_bBufferFull   = false;
    m_llintervalmsgtime = 2000;
    m_llStartPts    = -1;
    m_llLastPts     = INT64_MIN;
    m_bDisContinue  = 0;
    m_pTag          = NULL;//new char[5];
    m_pTokenServerUrl       = NULL;
    m_pHeartBeatFrame       = NULL;
    m_nDeliverVideoBytes    = 0;
    m_nSendVideoBytes       = 0;
    m_nDeliverAudioBytes    = 0;
    m_nSendAudioBytes       = 0;
    m_nVideoDropBytes       = 0;
    m_bStreamDeliverStart   = false;
    m_bStreamSendStart      = false;
    m_pRecMuxer     = NULL;
    m_pRtmpMuxer    = NULL;
    m_llMediaTimeStamp = -1;
    m_llLastVideoPts   = INT64_MIN;
    m_llLastAudioPts   = INT64_MIN;
    m_llLastSendTime   = INT64_MIN;

    m_llLastDeliverVPts = INT64_MIN;
    m_llLastDeliverAPts = INT64_MIN;
    m_llLastSendVPts = INT64_MIN;
    m_llLastSendAPts = INT64_MIN;
    m_nAlarmTime        = 0;
    m_nstreamType   = -1;
    m_precord_url = NULL;//"/tmp/test.rec";
    //strcpy(m_pTag, "462");
#ifdef READ_IPC_RECORD_FILE
    m_pRecordReader = NULL;
    m_pRecordReader = new CRecordReader();
    m_pRecordReader->Open("/mnt/sd0/data/154632_1_0_d");
#endif

#ifdef WRITE_AVC_DATA_FILE
    m_pavc_file     = NULL;
#endif
    
#ifdef WRITE_AAC_DATA_FILE
    m_paac_file     = NULL;
#endif
}

CBasePush::~CBasePush()
{
    //Close();
    if(m_pPushCb)
    {
        delete m_pPushCb;
        m_pPushCb = NULL;
    }
    
    if(m_pPushProtocol)
    {
        delete m_pPushProtocol;
        m_pPushProtocol = NULL;
    }
    
    if(m_pFrame)
    {
        delete m_pFrame;
        m_pFrame = NULL;
    }

    if(m_pTag)
    {
        delete[] m_pTag;
        m_pTag = NULL;
    }

    if(m_pHeartBeatFrame)
    {
        delete m_pHeartBeatFrame;
        m_pHeartBeatFrame = NULL;
    }
}
int CBasePush::SetTokenFetctInterface(const char* ptoken_server_url)
{
    if(m_pTokenServerUrl)
    {
        delete[] m_pTokenServerUrl;
        m_pTokenServerUrl = NULL;
    }
    
    if(ptoken_server_url && strlen(ptoken_server_url) > 0)
    {
        m_sGetTokenUrl = ptoken_server_url;
        sv_trace("set token url success %s\n", m_sGetTokenUrl.c_str());
        return 0;
    }
    sv_error("set token url failed, ptoken_server_url:%s\n", ptoken_server_url);
    return -1;
}

int CBasePush::Connect(const char* purl, const char* ptoken, IEventNotify* peventhandle)
{
    return ERROR_FAIL;
}

int CBasePush::Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun)
{
    return ERROR_FAIL;
}

int CBasePush::RecordSendPacket(const char* precord_url)
{
    if(m_precord_url)
    {
        free(m_precord_url);
        m_precord_url = NULL;
    }

    if(precord_url)
    {
        m_precord_url = (char*)malloc(strlen(precord_url) + 1);
        memcpy(m_precord_url, precord_url, strlen(precord_url) + 1);
    }

    svt_trace("RecordSendPacket(precord_url:%s)\n", precord_url);
    return 0;
}

#ifdef ENABLE_GEN_URL_BY_TOKEN
int CBasePush::ConnectByApp(auth_info* pai, const char* pappstreamname, const char* ptoken, long lcid, event_callback pcbfun)
{
    svt_trace(m_pPushProtocol->LogTag(), "ConnectByApp(pai:%p, pappstreamname:%s, ptoken:%s, lcid:%ld, pcbfun:%p)\n", pai, pappstreamname, ptoken, lcid, pcbfun);
    if(pappstreamname)
    {
        m_sDeviceSN = pappstreamname;
    }
    
    m_FrameList.init_queue(MAX_RTMP_CONNECTION_BLOCK_SIZE/2, g_nMemoryPoolSize);
    //sv_debug("before new CPushCallback(), m_pTag:%s", m_pTag);
    //svt_trace(m_pTag, "pappkey:%s, pappsecret:%s, pappstreamname:%s, lcid:%ld, pcbFun:%p, m_pTag:%s)", pappkey, pappsecret, pappstreamname, lcid, pcbfun, m_pTag);
    //sv_debug("before new CPushCallback()");
    m_pPushCb = new CPushCallback();
    m_pPushCb->SetCallBack(lcid, pcbfun);
    
    //sv_debug("after new CPushCallback()");
    
    //int ret = Connect(purl, ptoken, m_pPushCb);
    std::string url = RequestUrl(pai, pappstreamname, ptoken);
    //sv_debug("url:%s = RequestUrl(pappkey, pappsecret, pappstreamname, ptoken)\n", url.c_str());
    if(url.empty())
    {
        svt_error(m_pPushProtocol->LogTag(), "Request token failed, url is empyt");
        Close();
        return ERROR_FAIL;
    }

    if(ptoken && m_pPushProtocol)
    {
        if(m_pPushProtocol->GetdeviceSN())
        {
            m_sDeviceSN = m_pPushProtocol->GetdeviceSN();
        }
        
        if(m_pPushProtocol->GetToken())
        {
            m_sToken = m_pPushProtocol->GetToken();
        }
    }

    if(!m_sDeviceSN.empty() && m_sDeviceSN.length() > 3)
    {
        if(m_pTag)
        {
            delete[] m_pTag;
            m_pTag = NULL;
        }
        std::string sTag = m_sDeviceSN.substr(m_sDeviceSN.length() - 3, m_sDeviceSN.npos);
        m_pTag = new char[sTag.length() + 1];
        strcpy(m_pTag, sTag.c_str());
        svt_trace(m_pPushProtocol->LogTag(), "device tag:%s", m_pTag);
        //m_pPushProtocol->SetLogTag(m_pTag);
    }
    int ret = Connect(url.c_str(), m_sToken.c_str(), m_pPushCb);
    //dump_memory_info();
    return ret;
    //return ERROR_FAIL;
}
#endif

int CBasePush::SendEncryptPacket(int eMediaType, char* pData, unsigned int uSize, int keyframe, int64_t lltimestamp)
{
    //sv_memory(3, pData, 4);
    if(!m_bConnected)
    {
        svt_error(m_pPushProtocol->LogTag(), "(eMediaType:%d, pData:%p, uSize:%d, keyframe:%d, llTimeStamp:%d), deliver packet failed, push has disconnect!", eMediaType, pData, uSize, keyframe, (int)lltimestamp);
        return ERROR_RTMP_CONNECTION_LOSS;
    }

    if(E_SV_MEDIA_TYPE_H264 == eMediaType || E_SV_MEDIA_TYPE_H265 == eMediaType)
    {
        m_nDeliverVideoBytes += uSize;
        //sv_trace("m_nDeliverVideoBytes:%d += uSize:%d", m_nDeliverVideoBytes, uSize);
        if(INT64_MIN == m_llLastVideoPts)
        {
            m_llLastVideoPts = lltimestamp;
        }
        else if(lltimestamp - m_llLastVideoPts > 500)
        {
            svt_error(m_pPushProtocol->LogTag(), "Invalid video pts increase, lltimestamp:%" PRId64 " - m_llLastVideoPts:%" PRId64 " = %" PRId64 " > 500", lltimestamp, m_llLastVideoPts,  lltimestamp - m_llLastVideoPts);
        }

        m_llLastVideoPts = lltimestamp;
    }
    else if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        m_nDeliverAudioBytes += uSize;
        //sv_trace("m_nDeliverAudioBytes:%d += uSize:%d", m_nDeliverAudioBytes, uSize);
        if(INT64_MIN == m_llLastAudioPts)
        {
            m_llLastAudioPts = lltimestamp;
        }
        else if(lltimestamp - m_llLastAudioPts > 500)
        {
            svt_error(m_pPushProtocol->LogTag(), "Invalid audio pts increase, llTimeStamp:%" PRId64 " - m_llLastAudioPts:%" PRId64 " = %" PRId64 " > 500", lltimestamp, m_llLastAudioPts,  lltimestamp - m_llLastAudioPts);
            
        }
        m_llLastAudioPts = lltimestamp;
    }

    /*if(m_MuxerQueue.get_cur_frame_size() > g_nMemoryPoolSize * 4/ 5)
    {
        svt_error(m_pTag, "memory allmost overflow, notify drop frame!, m_MuxerQueue.get_cur_frame_size():%d", m_MuxerQueue.get_cur_frame_size());
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
        m_bBufferFull = true;
        //m_pPushProtocol->OnPacketDiscontinue();
        return ERROR_FAIL;
    }*/
    CFrameBuffer* pframe = m_MuxerQueue.get_frame_buffer();
    if(NULL == pframe)
    {
        svt_error(m_pPushProtocol->LogTag(), "Get SendEncryptPacketmuxer frame buffer failed!");
        dump_memory_info();
        return 0;
    }

    pframe->Reset();
    pframe->ReAllocBuffer(uSize);

    if(keyframe)
    {
        pframe->m_nflag = pframe->m_nflag | KEY_FRAME_FLAG;
    }

    pframe->m_nflag = pframe->m_nflag | AES_ENCRYPT_FLAG;
    memcpy(pframe->m_pdata, pData, uSize);
    pframe->m_nsize = uSize;
    pframe->m_llpts = lltimestamp;
    pframe->m_lldts = lltimestamp;
    pframe->m_nmsgtype = eMediaType;
    int ret = m_MuxerQueue.push_frame(pframe);
#ifdef LARGE_MEMORY_MANAGER
    if(sv_get_free_memory(m_pPushProtocol) <  SV_MIN_FREE_BUFFER_SIZE)
#else
    if(m_MuxerQueue.get_cur_frame_size() > g_nMemoryPoolSize/2)
#endif
    {
        svt_error(m_pPushProtocol->LogTag(), "memory allmost overflow, notify drop frame!, free memory size:%d, queue bytes:%d", sv_get_free_memory(m_pPushProtocol), m_MuxerQueue.get_cur_frame_size());
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
        m_bBufferFull = true;
        //m_pPushProtocol->OnPacketDiscontinue();
        //return ERROR_FAIL;
    }
    //sv_trace("ret:%d, pts:%d, uSize:%d, m_MuxerQueue.size():%d, queue size:%d, g_nMemoryPoolSize:%d, free memory:%d", ret, (int)lltimestamp, uSize, m_MuxerQueue.size(), m_MuxerQueue.get_cur_frame_size(), g_nMemoryPoolSize, sv_get_free_memory(m_pPushProtocol));
    //sv_trace("ret:%d = m_MuxerQueue.push_frame(pframe), m_MuxerQueue.size():%d", ret, m_MuxerQueue.size());
    return ret;
}
#ifdef ENABLE_BUFFER_MEDIA_DATA
int CBasePush::SendPacket(int eMediaType, char* pData, unsigned int uSize, int64_t llTimeStamp, int keyframe, int encflag)
{
    sv_verb("eMediaType:%d, pData:%p, uSize:%u, llTimeStamp:%" PRId64 ", keyframe:%d, encflag:%d\n", eMediaType, pData, uSize, llTimeStamp, keyframe, encflag);
    if(!m_bConnected)
    {
        svt_error(m_pTag, "(eMediaType:%d, pData:%p, uSize:%d, llTimeStamp:%d, keyframe:%d, encflag:%d), deliver packet failed, push has disconnect!", eMediaType, pData, uSize, (int)llTimeStamp, keyframe, encflag);
        return ERROR_RTMP_CONNECTION_LOSS;
    }
    //void on_recv_packet(int eMediaType, char* pData, unsigned int nSize, int64_t pts, int keyframe, int encflag);

    on_recv_packet(eMediaType, pData, uSize, llTimeStamp, keyframe, encflag);
#ifdef WRITE_INPUT_VIDEO_RECORD
    //sv_trace("sendpacket m_pRecMuxer:%p", m_pRecMuxer);
    if(m_pRecMuxer)
    {
        codec_id codecid = eMediaType == E_SV_MEDIA_TYPE_H264 ? codec_id_h264 : codec_id_aac;
        int ret = m_pRecMuxer->writeframe(codecid, pData, uSize, llTimeStamp);
        //sv_trace("ret:%d = m_pRecMuxer->writeframe(codecid:%d, pData, uSize:%d, llTimeStamp:%d)", ret, codecid, uSize, llTimeStamp);
    }
#endif
    if(encflag)
    {
        return SendEncryptPacket(eMediaType, pData, uSize, keyframe, llTimeStamp);
    }
    if(E_SV_MEDIA_TYPE_H264 == eMediaType || E_SV_MEDIA_TYPE_H265 == eMediaType)
    {
        m_nDeliverVideoBytes += uSize;
        //sv_trace("m_nDeliverVideoBytes:%d += uSize:%d", m_nDeliverVideoBytes, uSize);
        //sv_trace("m_llLastVideoPts:%" PRId64 ", llTimeStamp:%" PRId64 "", m_llLastVideoPts, llTimeStamp);
        if(INT64_MIN == m_llLastVideoPts)
        {
            m_llLastVideoPts = llTimeStamp;
        }
        else if(llTimeStamp - m_llLastVideoPts > 500)
        {
            sv_warn("Invalid video pts increase, pts:%d - lastvpts:%d = %d > 500", (int)llTimeStamp, (int)m_llLastVideoPts,  (int)(llTimeStamp - m_llLastVideoPts));
        }
        
        m_llLastVideoPts = llTimeStamp;
    }
    else if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        m_nDeliverAudioBytes += uSize;
        //sv_trace("m_nDeliverAudioBytes:%d += uSize:%d", m_nDeliverAudioBytes, uSize);
        //sv_trace("m_llLastAudioPts:%" PRId64 ", llTimeStamp:%" PRId64 "", m_llLastAudioPts, llTimeStamp);
        if(INT64_MIN == m_llLastAudioPts)
        {
            m_llLastAudioPts = llTimeStamp;
        }
        else if(llTimeStamp - m_llLastAudioPts > 500)
        {
            //llTimeStamp = 0;
            sv_warn("Invalid audio pts increase, apts:%d - lastapts:%d = %d > 500", (int)llTimeStamp, (int)m_llLastAudioPts,  (int)(llTimeStamp - m_llLastAudioPts));
        }
        m_llLastAudioPts = llTimeStamp;
    }
    else
    {
        svt_error(m_pPushProtocol->LogTag(), "Invalid Media type %d", eMediaType);
    }
    

#ifdef CACHE_MEDIA_DATA_IMMEDIATELY
    if(!check_frame(eMediaType, pData, uSize, llTimeStamp, keyframe, encflag))
    {
        m_nVideoDropBytes += uSize;
        //svt_error(m_pTag, "check_frame(eMediaType:%d, pData:%p, uSize:%d, llTimeStamp:%" PRId64 ") failed", eMediaType, pData, uSize, llTimeStamp); 
        return ERROR_FAIL;
    }
    
    CFrameBuffer* pframe = m_MuxerQueue.get_frame_buffer();
    if(NULL == pframe)
    {
        svt_error(m_pPushProtocol->LogTag(), "Get muxer frame buffer failed!");
        dump_memory_info();
        return 0;
    }
    pframe->Reset();
    pframe->ReAllocBuffer(uSize);
    memcpy(pframe->m_pdata, pData, uSize);
    pframe->m_nsize = uSize;
    pframe->m_llpts = llTimeStamp;
    pframe->m_lldts = llTimeStamp;
    pframe->m_nmsgtype = eMediaType;
    if(keyframe)
    {
	    pframe->m_nflag |= SV_PACKET_TYPE_KEY_FRAME;
    }
    if(encflag)
    {
	    pframe->m_nflag |= SV_PACKET_TYPE_AES_ENCRYPT;
    }
    //pframe->m_nflag = flag;
    int ret = m_MuxerQueue.push_frame(pframe);
    //sv_info("ret:%d = m_MuxerQueue.push_frame(pframe:%p)\n", ret, pframe);
#ifdef LARGE_MEMORY_MANAGER
    if(sv_get_free_memory(m_pPushProtocol) <  SV_MIN_FREE_BUFFER_SIZE)
#else
    if(m_MuxerQueue.get_cur_frame_size() > g_nMemoryPoolSize/2)
#endif
    {
        svt_error(m_pPushProtocol->LogTag(), "memory allmost overflow, notify drop frame!, free memory size %d, queue bytes:%d", sv_get_free_memory(m_pPushProtocol), m_MuxerQueue.get_cur_frame_size());
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
        m_bBufferFull = true;
        //m_pPushProtocol->OnPacketDiscontinue();
        //return ERROR_FAIL;
    }
    //sv_trace("ret:%d, pts:%d, uSize:%d, memory size:%d, g_nMemoryPoolSize:%d", ret, (int)llTimeStamp, uSize, sv_get_free_memory(m_pPushProtocol), g_nMemoryPoolSize);
    return ret;
#else
    CFrameBuffer* pframe = m_FrameList.get_frame_buffer();//new CFrameBuffer();
    CheckPtr(pframe, ERROR_OUT_OF_MEMORY);
    int ret = OnSendPacket(eMediaType, pData, uSize, llTimeStamp, pframe);
    if(CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "ret:%d = OnSendPacket(eMediaType:%d, pData:%p, uSize:%d, llTimeStamp:%" PRId64 ", pframe:%p) faield", ret, eMediaType, pData, uSize, llTimeStamp, pframe);
        m_FrameList.free_frame_buffer(&pframe);
        //delete pframe;
        return ret;
    }
    //pframe->m_nmsgtype = eMediaType;

    //pframe->m_lldts = llTimeStamp;
    //pframe->m_llpts = llTimeStamp;
    if(pframe->m_pdata && pframe->m_ncurpos > 0)
    {
#ifdef SEND_PACKET_DIRECTORY
        m_pPushProtocol->SendFrame(pframe);
		m_FrameList.free_frame_buffer(&pframe);
#else
	    sv_verb("OnSendPacket size:%d, pts:%" PRId64 ", free_memory_size:%" PRId64 "", pframe->m_nsize, pframe->m_llpts, CMemoryMgr::get_inst()->get_free_memory_size());
        ret = m_FrameList.push_frame(pframe);
        if(ret < 0)
        {
            svt_trace(mm_pPushProtocol->LogTag(), "ret:%d = push frame failed, pframe:%p", ret, pframe);
            //delete pframe;
            m_FrameList.free_frame_buffer(&pframe);
            //pframe = NULL;
        }
	//sv_verb("ret:%d = m_FrameList.push_frame(pframe):%d, uSize:%d,uTimeStamp:%d, curpos:%d, size:%d\n", ret, m_FrameList.size(), uSize, uTimeStamp, pframe->m_ncurpos, pframe->m_nsize);
        if(m_FrameList.is_full())
        {
            OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
            m_bBufferFull = true;
            m_pPushProtocol->OnPacketDiscontinue();
            svt_error(m_pPushProtocol->LogTag(), "m_pPushProtocol->OnPacketDiscontinue(), m_FrameList.size:%d, m_FrameList.get_cur_frame_size:%d", m_FrameList.size(), m_FrameList.get_cur_frame_size());
        }
#endif
    }
    else
    {
		sv_info("pframe->m_pdata:%p && pframe->m_ncurpos:%d, pframe->m_nsize:%d", pframe->m_pdata, pframe->m_ncurpos, pframe->m_nsize);
        
		m_FrameList.free_frame_buffer(&pframe);
    }
    CheckResult(ret);
    return ret;
#endif
}
#else
//int SendPacket(int eMediaType, char* pData,unsigned int nSize, int64_t llTimeStamp, int keyframe, int encflag)
int CBasePush::SendPacket(int eMediaType, char* pData, unsigned int uSize, int64_t llTimeStamp, int keyframe, int encflag)
{
    int ret = 0;
    //sv_trace("SendPacket\n");
    
    if(!m_bConnected)
    {
        svt_error(m_pPushProtocol->LogTag(), "(eMediaType:%d, pData:%p, uSize:%d, llTimeStamp:%d, keyframe:%d, encflag:%d), deliver packet failed, push has disconnect!", eMediaType, pData, uSize, (int)llTimeStamp, keyframe, encflag);
        return ERROR_RTMP_CONNECTION_LOSS;
    }
    
#ifdef WRITE_INPUT_VIDEO_RECORD
    if(m_pRecMuxer)
    {
        codec_id codecid = eMediaType == E_SV_MEDIA_TYPE_H264 ? codec_id_h264 : codec_id_aac;
        m_pRecMuxer->writeframe(codecid, pData, uSize, llTimeStamp);
    }
#endif

    if(E_SV_MEDIA_TYPE_H264 == eMediaType || E_SV_MEDIA_TYPE_H265 == eMediaType)
    {
        m_nDeliverVideoBytes += uSize;
        if(INT64_MIN == m_llLastVideoPts)
        {
            m_llLastVideoPts = llTimeStamp;
        }
        else if(llTimeStamp - m_llLastVideoPts > 500)
        {
            svt_error(m_pPushProtocol->LogTag(), "Invalid video pts increase, pts:%d - lastvpts:%d = %d > 500", (int)llTimeStamp, (int)m_llLastVideoPts,  (int)(llTimeStamp - m_llLastVideoPts));
        }
        
        m_llLastVideoPts = llTimeStamp;
    }
    else if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        m_nDeliverAudioBytes += uSize;
        if(INT64_MIN == m_llLastAudioPts)
        {
            m_llLastAudioPts = llTimeStamp;
        }
        else if(llTimeStamp - m_llLastAudioPts > 500)
        {
            //llTimeStamp = 0;
            svt_error(m_pPushProtocol->LogTag(), "Invalid audio pts increase, apts:%d - lastapts:%d = %d > 500", (int)llTimeStamp, (int)m_llLastAudioPts,  (int)(llTimeStamp - m_llLastAudioPts));
        }
        m_llLastAudioPts = llTimeStamp;
    }
    else
    {
        svt_error(m_pPushProtocol->LogTag(), "Invalid Media type %d", eMediaType);
    }
    

//#ifdef CACHE_MEDIA_DATA_IMMEDIATELY
    if(!check_frame(eMediaType, pData, uSize, llTimeStamp, keyframe, encflag))
    {
        m_nVideoDropBytes += uSize;
        //svt_error(m_pTag, "check_frame(eMediaType:%d, pData:%p, uSize:%d, llTimeStamp:%" PRId64 ") failed", eMediaType, pData, uSize, llTimeStamp); 
        return ERROR_FAIL;
    }
    
    CFrameBuffer* pframe = m_MuxerQueue.get_frame_buffer();
    if(NULL == pframe)
    {
        svt_error(m_pPushProtocol->LogTag(), "Get muxer frame buffer failed!");
        dump_memory_info();
        return 0;
    }

    pframe->Reset();
    pframe->ReAllocBuffer(uSize);
#ifdef MUXER_RTMP_PACKET_DIRECTORY
    int flag = 0;
    if(keyframe)
    {
	    flag |= SV_PACKET_TYPE_KEY_FRAME;
    }
    if(encflag)
    {
	    flag |= SV_PACKET_TYPE_AES_ENCRYPT;
    }
    
    ret = OnSendPacket(eMediaType, pData, uSize, flag, llTimeStamp, pframe);

#else
    
    memcpy(pframe->m_pdata, pData, uSize);
    pframe->m_nsize = uSize;
    pframe->m_llpts = llTimeStamp;
    pframe->m_lldts = llTimeStamp;
    pframe->m_nmsgtype = eMediaType;
    if(keyframe)
    {
	    pframe->m_nflag |= SV_PACKET_TYPE_KEY_FRAME;
    }
    if(encflag)
    {
	    pframe->m_nflag |= SV_PACKET_TYPE_AES_ENCRYPT;
    }
    
#endif
    ret = m_MuxerQueue.push_frame(pframe);
#ifdef LARGE_MEMORY_MANAGER
    if(sv_get_free_memory(m_pPushProtocol) <  SV_MIN_FREE_BUFFER_SIZE)
#else
    if(m_MuxerQueue.get_cur_frame_size() > g_nMemoryPoolSize/2)
#endif
    //if(sv_get_free_memory(m_pPushProtocol) <  SV_MIN_FREE_BUFFER_SIZE)
    {
        svt_error(m_pPushProtocol->LogTag(), "memory allmost overflow, notify drop frame!, free memory size %d, queue bytes:%d", sv_get_free_memory(m_pPushProtocol), m_MuxerQueue.get_cur_frame_size());
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
        m_bBufferFull = true;
    }
    //sv_trace("ret:%d, pts:%d, uSize:%d, free memory size:%d, SV_MIN_FREE_BUFFER_SIZE:%d, g_nMemoryPoolSize:%d", ret, (int)llTimeStamp, uSize, sv_get_free_memory(m_pPushProtocol), SV_MIN_FREE_BUFFER_SIZE, g_nMemoryPoolSize);
    return ret;
}
#endif
int CBasePush::SendMetadata(int eMetadataType, long wParam, long lParam)
{
#ifndef ENABLE_ENCRYPTION
    if(2 == eMetadataType || 3 == eMetadataType)
    {
        return 0;
    }
#endif

#ifdef CACHE_MEDIA_DATA_IMMEDIATELY
    if(E_SV_METADATA_TYPE_STREAM_START == eMetadataType)
    {
        m_llMediaTimeStamp =  *(uint64_t*)lParam;
        OnSegmentDeliverStart(m_llMediaTimeStamp);
    }
    else if(E_SV_METADATA_TYPE_STREAM_END == eMetadataType)
    {
        m_nAlarmTime = wParam;
        OnSegmentDeliverEnd();
    }
    
    CFrameBuffer* pframe = m_MuxerQueue.get_frame_buffer();//new CFrameBuffer();
    int ret = OnSendMetadata(eMetadataType, wParam, lParam, pframe);
    if(CheckFailed(ret))
    {
        svt_error(m_pTag, "ret:%d = SendMetadata(eMetadataType:%d, wParam:%ld, lParam:%ld, pframe:%p) faield", ret, eMetadataType, wParam, lParam, pframe);
        m_MuxerQueue.free_frame_buffer(&pframe);
        return -1;
    }
    pframe->m_nmsgtype = E_SV_MEDIA_TYPE_METADATA;
    pframe->m_nmsgid = eMetadataType;
    ret = m_MuxerQueue.push_frame(pframe);
    if(ret < 0)
    {
        m_MuxerQueue.free_frame_buffer(&pframe);
        svt_error(m_pTag, "push frame to list failed, ret:%d", ret);
    }
    
    return ret;
#else
    CFrameBuffer* pframe = m_FrameList.get_frame_buffer();//new CFrameBuffer();
    int ret = OnSendMetadata(eMetadataType, wParam, lParam, pframe);
    if(CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "ret:%d = SendMetadata(eMetadataType:%d, wParam:%ld, lParam:%ld, pframe:%p) faield", ret, eMetadataType, wParam, lParam, pframe);
        m_FrameList.free_frame_buffer(&pframe);
        return 0;
    }
    ret = m_FrameList.push_frame(pframe);
    if(ret < 0)
    {
        m_FrameList.free_frame_buffer(&pframe);
        svt_error(m_pPushProtocol->LogTag(), "push frame to list failed, ret:%d", ret);
    }

    return ret;
#endif
}

int CBasePush::IsConnected()
{
    return m_bConnected;
}

int CBasePush::Close()
{
    //sv_trace("close begin");
    //dump_memory_info();
    Stop();
    CAutoLock lock(m_CriSec);
    //int ret = Unpublish();
    m_bConnected = 0;
    if(m_pPushProtocol)
    {
	    m_pPushProtocol->Close();
        //sv_trace("before notify close");
        //OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE, 0, 0);
        //sv_trace("after notify close");
    }
    m_FrameList.deinit_queue();
#ifdef READ_IPC_RECORD_FILE
    if(m_pRecordReader)
    {
        m_pRecordReader->Close();
        delete m_pRecordReader;
        m_pRecordReader = NULL;
    }
    //m_pRecordReader = NULL;
    //m_pRecordReader->Open("/mnt/sd0/data/154632_1_0_d");
#endif

    if(m_pRecMuxer)
    {
        m_pRecMuxer->close();
        delete m_pRecMuxer;
        m_pRecMuxer = NULL;
    }

    if(m_pRtmpMuxer)
    {
        delete m_pRtmpMuxer;
        m_pRtmpMuxer = NULL;
    }

    if(m_pPushCb)
    {
        delete m_pPushCb;
        m_pPushCb = NULL;
    }
    if(m_pTag)
    {
        delete[] m_pTag;
        m_pTag = NULL;
    }
    //sv_trace("close end");
    return 0;
}

int CBasePush::ResolveUrl(const char* pUrl)
{
    /*int ret = Resolve(pUrl);
    if(CheckFailed(ret))
    {
        svt_error(m_pTag, "ret:%d = Resolve(pUrl:%s) failed", ret, pUrl);
        return ret;
    }

    return ret;*/
    return ERROR_OK;
}

int CBasePush::ConnectServer()
{
    return ERROR_OK;
}

int CBasePush::ShakeHand()
{
    return 0;
}

int CBasePush::Authorize()
{
    return 0;
}

int CBasePush::ConnectApp()
{
    return 0;
}

int CBasePush::Publish()
{
    return 0;
}

int CBasePush::OnConnect()
{
    m_bConnected = 1;
    return 0;
}

int CBasePush::OnSendPacket(int eMediaType, char* pInData, unsigned int uInSize, int flag, int64_t llTimeStamp, CFrameBuffer* pframe)
{
    sv_verb("(eMediaType:%d, pInData:%p, nInSize:%d, flag:%d, llTimeStamp:%" PRId64 ", pframe:%p)", eMediaType, pInData, uInSize, flag, llTimeStamp, pframe);
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
#if ENABLE_DEBUG
    OnRawPacket(eMediaType, pInData, uInSize, llTimeStamp);
#endif

    int ret = m_pPushProtocol->OnSendPacket(eMediaType, pInData, uInSize, flag, llTimeStamp, pframe);
    if (CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "ret:%d = m_pPushProtocol->OnSendPacket(eMediaType:%d, pInData:%p, nInSize:%d, flag:%d, llTimeStamp:%" PRId64 ", pframe:%p) failed", ret, eMediaType, pInData, uInSize, flag, llTimeStamp, pframe);
    }
#if ENABLE_DEBUG
    OnRtmpMsg(eMediaType, pframe);
#endif
    return ret;
}

int CBasePush::OnSendMetadata(int eMetadataType, long wParam, long lParam, CFrameBuffer* pframe)
{
    //sv_trace("(eMetadataType:%d, wParam:%ld, lParam:%ld, pframe:%p)", eMetadataType, wParam, lParam, pframe);
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
#ifdef READ_IPC_RECORD_FILE
    if(NULL == m_pRecordReader)
    {
        m_pRecordReader = new CRecordReader();
    }
    if(eMetadataType == 0 && m_pRecordReader)
    {
        m_pRecordReader->Close();
        m_pRecordReader->Open("/mnt/sd0/data/154632_1_0_d");
    }
#endif
    /*if(0 == eMetadataType || 1 == eMetadataType)
    {
        dump_memory_info();
    }*/
    
    int ret = m_pPushProtocol->OnSendMetadata(eMetadataType, wParam, lParam, pframe);
    if (CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "ret:%d = m_pPushProtocol->OnSendMetadata(eMetadataType:%d, wParam:%ld, lParam:%ld, pframe:%p) failed", ret, eMetadataType, wParam, lParam, pframe);
    }
    if(E_SV_METADATA_TYPE_STREAM_END == eMetadataType || E_SV_METADATA_TYPE_STREAM_START == eMetadataType)
    {
#ifdef WRITE_AVC_DATA_FILE
        if(m_pavc_file)
        {
            sv_trace("OnSendMetadata m_pavc_file:%p, eMetadataType:%d\n", m_pavc_file, eMetadataType);
            fclose(m_pavc_file);
            m_pavc_file = NULL;
        }
#endif
#ifdef WRITE_AAC_DATA_FILE
        if(m_paac_file)
        {
            sv_trace("OnSendMetadata m_paac_file:%p, eMetadataType:%d\n", m_paac_file, eMetadataType);
            fclose(m_paac_file);
            m_paac_file = NULL;
        }
#endif
    }
    
    return ret;
}

int CBasePush::Unpublish()
{
    //sv_trace("Unpublish begin");
    CAutoLock lock(m_CriSec);
    m_bConnected = 0;
    int ret = 0;
    if(m_pPushProtocol)
    {
    	ret = m_pPushProtocol->Unpublish();
    }
    m_llStartPts = -1;
    //svt_trace(m_pTag, "Unpublish end ret:%d", ret);
    return ret;
}

THREAD_RETURN CBasePush::ThreadProc()
{
    CheckPtr(m_pPushProtocol, NULL);
    int ret  = 0;
    CFrameBuffer* pframe = NULL;
    int64_t lllastsendts = GetSysTime();
    int64_t llstarttime = GetSysTime();

    svt_trace(m_pPushProtocol->LogTag(), "protocol thread begin, m_bRun:%d", m_bRun);
    while(m_bRun)
    {
        if(m_FrameList.size() <= 0)
        {
            if((int64_t)GetSysTime() - lllastsendts > m_llintervalmsgtime)
            {
                //svt_trace(m_pTag, "duration:%lu, m_llintervalmsgtime:%" PRId64 "", GetSysTime() - lllastsendts, m_llintervalmsgtime);
                ret = SendHeartBeat();
                if(CheckFailed(ret))
                {
                    svt_error(m_pPushProtocol->LogTag(), "break, SendHeartBeat failed, ret:%d", ret);
                    //OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0);
                    //assert(0);
                  	break;
                }
                lllastsendts = GetSysTime();
            }

            usleep(1000);
            continue;
        }

        // get frame
        pframe = m_FrameList.pop_frame();
		//sv_trace("pframe:%p = m_FrameList.pop_frame", pframe);
        if(NULL == pframe)
        {
            svt_error(m_pPushProtocol->LogTag(), "pframe:%p = m_FrameList.pop_frame(), ingore!", pframe);
            continue;
        }

        if(m_bBufferFull && sv_get_free_memory(m_pPushProtocol) < g_nMemoryPoolSize*1/3)
        {
            svt_trace(m_pPushProtocol->LogTag(), "resume media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
            //sv_trace("free memory size:%d, g_nMemoryPoolSize:%d", sv_get_free_memory(m_pPushProtocol), g_nMemoryPoolSize);
            OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH, ret, 0);
            m_bBufferFull = false;
        }
        // lock and write protocol data to socket
        {
	        lllastsendts = GetSysTime();
	    
            ret = m_pPushProtocol->SendFrame(pframe);
	    //sv_info("ret:%d = m_pPushProtocol->SendFrame(pframe:%p)", ret, pframe);
            //sv_trace("ret:%d = m_pPushProtocol->SendFrame(pframe), size:%d, pts:%" PRId64 "", ret, pframe->m_nsize, pframe->m_llpts);
            if(GetSysTime() - lllastsendts > 1000)
            {
                svt_trace(m_pPushProtocol->LogTag(), "m_pPushProtocol->SendFrame(pframe), size:%d, pts:%" PRId64 ", senddur:%" PRId64 ", m_FrameList.size():%d, lastsenddur:%d", pframe->m_nsize, pframe->m_llpts, GetSysTime() - lllastsendts, m_FrameList.size(), m_pPushProtocol->GetLastSendDuration());
            }
            else
            {
            //sv_info("m_pPushProtocol->SendFrame(pframe), size:%d, pts:%" PRId64 ", senddur:%" PRId64 ", m_FrameList.size():%d, lastsenddur:%d", pframe->m_nsize, pframe->m_llpts, GetSysTime() - lllastsendts, m_FrameList.size(), m_pPushProtocol->GetLastSendDuration());
            }
            m_FrameList.free_frame_buffer(&pframe);

            if(CheckFailed(ret))
            {
                svt_error(m_pPushProtocol->LogTag(), "break, ret:%d = write(pframe), connectdur:%" PRId64 ", senddur:%" PRId64 "", ret, GetSysTime() - llstarttime, GetSysTime() - lllastsendts);
                break;
            }
            else
            {
                lllastsendts = GetSysTime();
            }
            
        }
    }
    if(m_bRun)
    {
        Unpublish();
    }
    
    if(CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "break, ret:%d = write(pframe), connectdur:%" PRId64 ", senddur:%" PRId64 "", ret, GetSysTime() - llstarttime, GetSysTime() - lllastsendts);
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0);
        //assert(0);
        //break;
    }
    svt_trace(m_pPushProtocol->LogTag(), "protocol thread end, m_bRun:%d", m_bRun);

    return NULL;
}

void CBasePush::OnRawPacket(int& eMediaType, char*& pData,unsigned int& uSize, int64_t& llTimeStamp)
{
#ifdef READ_IPC_RECORD_FILE
    if(m_pRecordReader)
    {
        static char* pbuffer = new char[1024*128];
        VAVA_RecHead rechdr;
        int ret = m_pRecordReader->ReadFrame(pbuffer, 1024*128, &rechdr);
        if(ret > 0)
        {
            pData = pbuffer;
            uSize = rechdr.size;
            llTimeStamp = rechdr.time_usec + rechdr.time_sec * 1000;
        }
    }
#endif
#ifdef READ_RAW_MEDIA_FILE
    static FILE* pfile = NULL;
    static char* pbuffer = NULL;
    int size = 0;
    int mt = 0;
    int pts = 0;
    if(!pbuffer)
    {
        pbuffer = new char[1024*128];
    }
    simple_read_file(&pfile, get_log_path(), (char*)"/../data/rawmedia.data", (char*)&mt, sizeof(mt));
    simple_read_file(&pfile, get_log_path(), (char*)"/../data/rawmedia.data", (char*)&size, sizeof(size));
    simple_read_file(&pfile, get_log_path(), (char*)"/../data/rawmedia.data", (char*)&pts, sizeof(pts));
    simple_read_file(&pfile, get_log_path(), (char*)"/../data/rawmedia.data", pbuffer, size);
    pData = pbuffer;
    uSize = size;
    eMediaType = mt;
    llTimeStamp = pts;
#else
#ifdef READ_AVC_DATA_FILE
    if(E_SV_MEDIA_TYPE_H264 == eMediaType)
    {
        static FILE* pavcfile = NULL;
        static char* pavcbuffer = NULL;
        int size = 0;
        if(!pavcbuffer)
        {
            pavcbuffer = new char[1024*128];
        }
        
        simple_read_file(&pavcfile, get_log_path(), (char*)"/../data/avc.data", (char*)&size, sizeof(size));
        simple_read_file(&pavcfile, get_log_path(), (char*)"/../data/avc.data", pavcbuffer, size);
        pData = pavcbuffer;
        uSize = size;
        llTimeStamp = 0;
    }
#endif
#ifdef READ_AAC_DATA_FILE
    if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        static FILE* paacfile = NULL;
        static char* paacbuffer = NULL;
        int size = 0;
        if(!paacbuffer)
        {
            paacbuffer = new char[1024*128];
        }
        
        simple_read_file(&paacfile, get_log_path(), (char*)"/../data/avc.data", (char*)&size, sizeof(size));
        simple_read_file(&paacfile, get_log_path(), (char*)"/../data/avc.data", paacbuffer, size);
        pData = paacbuffer;
        uSize = size;
        llTimeStamp = 0;
    }
#endif
    
#ifdef WRITE_AVC_DATA_FILE
    if(E_SV_MEDIA_TYPE_H264 == eMediaType)
    {
        //static FILE* pavcfile = NULL;
        //simple_write_file(&pavcfile, get_log_path(), (char*)"/../data/avc_v2.data", (char*)&uSize, sizeof(uSize));
#if 0
        if(NULL == m_pavc_file)
        {
            char path[256] = {0};
            struct timeval tv;
            gettimeofday(&tv, NULL);
            sprintf(path, "//mnt/sd0/data/avc_%ld.h264", tv.tv_sec * 1000 + tv.tv_usec/1000);
            m_pavc_file = fopen(path, "wb");
        }

        if(m_pavc_file)
        {
            fwrite(pData, 1, uSize, m_pavc_file);
        }
#else
        char path[256] = {0};
        struct timeval tv;
        gettimeofday(&tv, NULL);
        sprintf(path, "/mnt/sd0/data/avc_%ld.h264", tv.tv_sec * 1000 + tv.tv_usec/1000);
        simple_write_file(&m_pavc_file, NULL, path, pData, uSize);
#endif
    }
#endif
    
#ifdef WRITE_AAC_DATA_FILE
    if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
#if 0
         if(NULL == m_paac_file)
        {
            char path[256] = {0};
            struct timeval tv;
            gettimeofday(&tv, NULL);
            sprintf(path, "//mnt/sd0/data/aac_%ld.h264", tv.tv_sec * 1000 + tv.tv_usec/1000);
            m_paac_file = fopen(path, "wb");
        }

        if(m_paac_file)
        {
            fwrite(pData, 1, uSize, m_paac_file);
        }
#else
        char path[256] = {0};
        struct timeval tv;
        gettimeofday(&tv, NULL);
        sprintf(path, "/mnt/sd0/data/aac_%ld.aac", tv.tv_sec * 1000 + tv.tv_usec/1000);
        simple_write_file(&m_paac_file, NULL, path, pData, uSize);
#endif
        //simple_write_file(&m_paac_file, get_log_path(), (char*)"/../data/aac_v2.data", pData, uSize);
    }
#endif
#endif
}

void CBasePush::OnRtmpMsg(int eMediaType, CFrameBuffer* pframe)
{
    if(pframe->m_ncurpos <= 0)
    {
        return ;
    }
/*
#ifdef READ_RTMP_MSG_FILE
    static FILE* pmsgfile = NULL;
    int size = 0;
    int pts = 0;
    simple_read_file(&pmsgfile, get_log_path(), (char*)"/../data/avcmsgv.data", (char*)&size, sizeof(size));
    simple_read_file(&pmsgfile, get_log_path(), (char*)"/../data/avcmsgv.data", (char*)&pts, sizeof(pts));
    pframe->ReAllocBuffer(size);
    pframe->m_ncurpos = size;
    simple_read_file(&pmsgfile, get_log_path(), (char*)"/../data/rtmpmsg.data", pframe->m_pdata, pframe->m_ncurpos);
    pframe->m_llpts = pts;
    pframe->m_lldts = pts;
#else
    if(E_SV_MEDIA_TYPE_H264 == eMediaType)
    {
        
#ifdef WRITE_AVC_DATA_FILE
        static FILE* pavcfile = NULL;
        simple_write_file(&pavcfile, get_log_path(), (char*)"/../data/avcmsgv2.data", (char*)&pframe->m_ncurpos, sizeof(pframe->m_ncurpos));
        simple_write_file(&pavcfile, get_log_path(), (char*)"/../data/avcmsgv2.data", pframe->m_pdata, pframe->m_ncurpos);
#elif #defined(READ_AVC_MSG_FILE)
        static FILE* pavcfile = NULL;
        int size = 0;
        pframe->Reset();
        simple_read_file(&pavcfile, get_log_path(), (char*)"/../data/avcmsgv2.data", (char*)&sizes, sizeof(size));
        pframe->ReAllocBuffer(size);
        simple_read_file(&pavcfile, get_log_path(), (char*)"/../data/avcmsgv2.data", pframe->m_pdata, size);
#endif
    }
    if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        
#ifdef WRITE_AAC_DATA_FILE
        static FILE* paacfile = NULL;
        simple_write_file(&paacfile, get_log_path(), (char*)"/../data/aacmsgv2.data", (char*)&pframe->m_ncurpos, sizeof(pframe->m_ncurpos));
        simple_write_file(&paacfile, get_log_path(), (char*)"/../data/aacmsgv2.data", pframe->m_pdata, pframe->m_ncurpos);
#elif defined(READ_AAC_MSG_FILE)
        static FILE* pavcfile = NULL;
        int size = 0;
        pframe->Reset();
        simple_read_file(&paacfile, get_log_path(), (char*)"/../data/aacmsgv2.data", (char*)&sizes, sizeof(size));
        pframe->ReAllocBuffer(size);
        simple_read_file(&paacfile, get_log_path(), (char*)"/../data/aacmsgv2.data", pframe->m_pdata, size);
#endif
    }
#ifdef WIRTE_RTMP_MSG_FILE
    static FILE* prtmpfile = NULL;
    int pts = pframe->m_llpts;
    simple_write_file(&prtmpfile, get_log_path(), (char*)"/../data/rtmpmsgv2.data", (char*)&pframe->m_ncurpos, sizeof(pframe->m_ncurpos));
    simple_write_file(&prtmpfile, get_log_path(), (char*)"/../data/rtmpmsgv2.data", (char*)&pts, sizeof(pts));
    simple_write_file(&prtmpfile, get_log_path(), (char*)"/../data/rtmpmsgv2.data", pframe->m_pdata, pframe->m_ncurpos);
#elif #defined(READ_RTMP_MSG_FILE)
    int size = 0;
    pframe->Reset();
    simple_read_file(&paacfile, get_log_path(), (char*)"/../data/rtmpmsg.data", (char*)&sizes, sizeof(size));
    pframe->ReAllocBuffer(size);
    simple_read_file(&paacfile, get_log_path(), (char*)"/../data/rtmpmsgv.data", pframe->m_pdata, size);
#endif
#endif*/
}

int CBasePush::OnCallbackNotify(E_SV_EVENT_TYPE eEventType, long wParam, long lParam)
{
    svt_trace(m_pPushProtocol->LogTag(), "m_pPushCb:%p->OnEventNotify(eEventType:%d, wParam:%ld, lParam:%ld)", m_pPushCb, eEventType, wParam, lParam);
    if(m_pPushCb)
    {
        return m_pPushCb->OnEventNotify(eEventType, wParam, lParam);
    }

    return ERROR_FAIL;
}

std::string CBasePush::RequestUrl(auth_info* pai, const char* pdeviceSN, const char* ptoken)
{
    sv_debug("(pai:%p, pdeviceSN:%s, ptoken:%s)", pai, pdeviceSN, ptoken);
    if(m_pPushProtocol)
    {
        if(pai)
        {
            return m_pPushProtocol->RequestTokenByAuthInfo(m_sGetTokenUrl.c_str(), pai, pdeviceSN);
        }
        else
        {
            return m_pPushProtocol->RequestToken(m_sGetTokenUrl.c_str(), NULL, NULL, pdeviceSN, ptoken);
        }
        
        //equestToken(const char* ptoken_url, const char* pappkey, const char* pappsecret, const char* pdevicesn, const char* ptoken = NULL)
        
    }
    return std::string();
}

int CBasePush::SendHeartBeat()
{
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
    int ret = ERROR_SUCCESS;
    if(NULL == m_pHeartBeatFrame)
    {
        m_pHeartBeatFrame = new CFrameBuffer(m_pPushProtocol);
        //sv_trace("create heartbeat frame");
    }
    //CFrameBuffer* pframe = m_FrameList.get_frame_buffer();
    if(m_pHeartBeatFrame)
    {
        //sv_trace("before SendHeartBeat");
        ret = m_pPushProtocol->SendHeartBeat(m_pHeartBeatFrame);
        //sv_trace("ret:%d = m_pPushProtocol->SendHeartBeat(pframe)", ret);
    }
    else
    {
        svt_trace(m_pPushProtocol->LogTag(), "get pframe buffer failed, m_pHeartBeatFrame:%p", m_pHeartBeatFrame);
    }

    //sv_trace("send heart beat end");
    return ret;
}
#ifdef ENABLE_GEN_URL_BY_TOKEN
std::string CBasePush::RequestUrlByToken(const char* ptoken)
{
    return std::string();
}
#endif
int CBasePush::dump_memory_info()
{
#ifdef ENABLE_DUMP_SYS_INFO
    struct sysinfo info;
    int ret = sysinfo(&info);
    //sv_trace("ret:%d = sysinfo(&info)", ret);
    //int mem = 0;
    //int vmem = 0;

    if(ret == 0)
    {
#if 0
    int pid = getpid();
    get_pro_memory(pid, &mem, &vmem);
    float fcpu = get_proc_cpu(pid);
    svt_trace(m_pTag, "memory info: pid:%d cpu:%f, mem:%dKB, vmem:%dKB, totalram:%lu, freeram:%lu, bufferram:%lu", pid, fcpu, mem, vmem, info.totalram, info.freeram, info.bufferram);
#else
    svt_trace(m_pPushProtocol->LogTag(), "memory info:totalram:%lu, freeram:%lu, bufferram:%lu", info.totalram, info.freeram, info.bufferram);
#endif
    }
    else
    {
        svt_error(m_pPushProtocol->LogTag(), "ret:%d = sysinfo(&info) failed", ret);
    }
    m_lLastFreeMemoryBytes = info.freeram;
#endif
    return 0;
}

#ifdef ENABLE_BUFFER_MEDIA_DATA
int CBasePush::MuxerProc()
{
    int ret = 0;
    m_llStartPts = GetSysTime();
    CFrameBuffer* pdstframe = NULL;
#ifdef MUXER_AND_SEND_FRAME_TOGETHER
    pdstframe = new CFrameBuffer(m_pPushProtocol);
    //bool segmentstart = false;
    //svt_trace(m_pTag, "pdstframe:%p = new CFrameBuffer(m_pPushProtocol)", pdstframe);
#endif
    svt_trace(m_pPushProtocol->LogTag(), "MuxerProc begin");
    m_llLastSendTime = GetSysTime();
    while(m_MuxerThread.IsAlive())
    {
	    //sv_trace("m_MuxerQueue.size():%d", m_MuxerQueue.size());
        if(m_MuxerQueue.size() > 0)
        {
#ifdef LARGE_MEMORY_MANAGER
            if(!m_bBufferFull &&sv_get_free_memory(m_pPushProtocol) < SV_MIN_FREE_BUFFER_SIZE)
            {
                svt_trace(m_pPushProtocol->LogTag(), "pasue media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, ret, 0);
                m_bBufferFull = false;
            }
            else if(m_bBufferFull && sv_get_free_memory(m_pPushProtocol) > g_nMemoryPoolSize / 3)
            {
                svt_trace(m_pPushProtocol->LogTag(), "resume media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH, ret, 0);
                m_bBufferFull = false;
            }
#endif
            /*if(m_MuxerQueue.get_cur_frame_size() > MAX_RTMP_CONNECTION_MEMORY_SIZE / 2)
            {
                sv_error("muxer queue size:%d is almost full!", m_MuxerQueue.get_cur_frame_size());
            }*/
            CFrameBuffer* psrcframe = m_MuxerQueue.pop_frame();
            if(NULL == psrcframe)
            {
                svt_error(m_pPushProtocol->LogTag(), "psrcframe:%p = m_MuxerQueue.pop_frame() is NULL", psrcframe);
                ret = ERROR_FAIL;
                break;
            }
            //CheckPtr(psrcframe, ERROR_FAIL);
#ifndef MUXER_AND_SEND_FRAME_TOGETHER
            CFrameBuffer* pdstframe = m_FrameList.get_frame_buffer();//new CFrameBuffer();
#endif
            if(NULL == pdstframe)
            {
                m_MuxerQueue.free_frame_buffer(&psrcframe);
                svt_error(m_pPushProtocol->LogTag(), "out of memory, pdstframe:%p = m_FrameList.get_frame_buffer() failed", pdstframe);
                ret = ERROR_OUT_OF_MEMORY;
                break;
            }
            
            pdstframe->Reset();
            if(psrcframe->m_nmsgtype >= 0 && psrcframe->m_nmsgtype < E_SV_MEDIA_TYPE_METADATA)
            {
                ret = OnSendPacket(psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_nflag, psrcframe->m_llpts, pdstframe);
            }
            else if(E_SV_MEDIA_TYPE_METADATA == psrcframe->m_nmsgtype)
            {
                psrcframe->Clone(pdstframe);
                if(E_SV_METADATA_TYPE_STREAM_START == pdstframe->m_nmsgid)
                {
                    OnSegmentSendStart();
                }
                else if(E_SV_METADATA_TYPE_STREAM_END == pdstframe->m_nmsgid)
                {
                    OnSegmentSendEnd();
                }
            }
            else
            {
                sv_warn("Invalid message type, psrcframe->m_nmsgtype:%d", psrcframe->m_nmsgtype);
            }
            
            //sv_info("ret:%d = OnSendPacket(psrcframe->m_nmsgtype:%d, psrcframe->m_pdata:%p, psrcframe->m_nsize:%d, psrcframe->m_llpts:%" PRId64 ", pdstframe:%p)", ret, psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_llpts, pdstframe);
            if(CheckFailed(ret))
            {
                svt_error(m_pPushProtocol->LogTag(), "ret:%d = OnSendPacket(m_nmsgtype:%d, m_pdata:%p, m_nsize:%d, pts:%" PRId64 ", dstframe:%p) faield, tag:%s", ret, psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_llpts, pdstframe, m_pTag);
                m_MuxerQueue.free_frame_buffer(&psrcframe);
#ifndef MUXER_AND_SEND_FRAME_TOGETHER
                m_FrameList.free_frame_buffer(&pdstframe);
#endif
                //may be send buffer list is full, begin to drop frame ...
                //assert(0);
                if(ERROR_NO_SPS_AND_PPS == ret)
                {
                    OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_VIDEO_NO_SPS_PPS, 0, 0);
                }
                else if(ERROR_NO_ADTS_HEADER == ret)
                {
                    OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_AUDIO_ADTS_DATA_ERROR, 0, 0);
                }
                continue;
            }
            
            if(pdstframe->m_pdata && pdstframe->m_ncurpos > 0)
            {
#ifdef MUXER_AND_SEND_FRAME_TOGETHER
                ret = m_pPushProtocol->SendFrame(pdstframe);
                //assert(pdstframe->m_ncurpos == pdstframe->m_nsize);
                //svt_trace(m_pTag, "ret:%d = m_pPushProtocol->SendFrame(pdstframe), pdstframe->m_nmsgtype:%d, pdstframe->m_ncurpos:%d == pdstframe->m_nsize:%d, pts:%" PRId64 ", m_llLastSendVPts:%" PRId64 "", ret, pdstframe->m_nmsgtype, pdstframe->m_ncurpos, pdstframe->m_nsize, pdstframe->m_llpts, m_llLastSendVPts);

                if(RTMP_MSG_VideoMessage == pdstframe->m_nmsgtype)
                {
                    m_nSendVideoBytes += pdstframe->m_ncurpos;
                    m_llLastSendVPts = pdstframe->m_llpts;
#ifdef WRITE_OUTPUT_VIDEO_RECORD
                    if(m_pRtmpMuxer)
                    {
                        m_pRtmpMuxer->writeframe(codec_id_h264, pdstframe->m_pdata, pdstframe->m_ncurpos, pdstframe->m_llpts);
                    }
#endif
                }
                else if(RTMP_MSG_AudioMessage == pdstframe->m_nmsgtype)
                {
                    m_nSendAudioBytes += pdstframe->m_ncurpos;
                    m_llLastSendAPts = pdstframe->m_llpts;
#ifdef WRITE_OUTPUT_VIDEO_RECORD
                    if(m_pRtmpMuxer)
                    {
                        m_pRtmpMuxer->writeframe(codec_id_aac, pdstframe->m_pdata, pdstframe->m_ncurpos, pdstframe->m_llpts);
                    }
#endif
                    //sv_trace("send audio packet:");
                    //sv_memory(3, pdstframe->m_pdata, 16);
                }
                else if(E_SV_MEDIA_TYPE_METADATA == psrcframe->m_nmsgtype && E_SV_METADATA_TYPE_STREAM_END == pdstframe->m_nmsgid)
                {
                    //m_nSendVideoBytes, m_nSendAudioBytes, m_llLastSendVPts
                    OnCallbackNotify(E_SV_EVENT_TYPE_PUSH_SEGMENT_END, m_llLastSendVPts, m_llLastSendAPts);
                    svt_trace(m_pPushProtocol->LogTag(), "OnCallbackNotify(E_SV_EVENT_TYPE_PUSH_SEGMENT_END, m_llLastSendVPts:%" PRId64 ", m_llLastSendAPts:%" PRId64 ")\n",  m_llLastSendVPts, m_llLastSendAPts);
                }
                
                m_llLastSendTime = GetSysTime();
                if(CheckFailed(ret))
                {
                    m_MuxerQueue.free_frame_buffer(&psrcframe);
#ifndef MUXER_AND_SEND_FRAME_TOGETHER
                    m_FrameList.free_frame_buffer(&pdstframe);
#endif
                    svt_error(m_pPushProtocol->LogTag(), "ret:%d = m_pPushProtocol->SendFrame(pdstframe) failed, m_pTag:%s\n", ret, m_pTag);
                    break;
                }
                //m_FrameList.free_frame_buffer(&pdstframe);
#else
                //sv_verb("OnSendPacket size:%d, pts:%" PRId64 ", free_memory_size:%" PRId64 "", pdstframe->m_nsize, pdstframe->m_llpts, CMemoryMgr::get_inst()->get_free_memory_size());
                ret = m_FrameList.push_frame(pdstframe);
		//sv_trace("ret:%d = m_FrameList.push_frame(pdstframe:%p), m_FrameList.size():%d", ret, pdstframe, m_FrameList.size());
                if(ret < 0)
                {
                    svt_error(m_pPushProtocol->LogTag(), "ret:%d = push frame failed, pframe:%p", ret, pdstframe);
                    //m_FrameList.free_frame_buffer(&pdstframe);
                    OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PAUSE_MEDIA_PUSH, 0, 0);
                    m_bBufferFull = true;
                    m_pPushProtocol->OnPacketDiscontinue();
                }

#endif
            }
#ifndef MUXER_AND_SEND_FRAME_TOGETHER
            m_FrameList.free_frame_buffer(&pdstframe);
#endif
	        m_MuxerQueue.free_frame_buffer(&psrcframe);

        }
        else
        {
/*    #ifdef LARGE_MEMORY_MANAGER
            if(m_bBufferFull && sv_get_free_memory(m_pPushProtocol) > g_nMemoryPoolSize / 3)
#else*/
            if(m_bBufferFull)
//#endif
            {
                svt_trace(m_pPushProtocol->LogTag(), "resume media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH, ret, 0);
                m_bBufferFull = false;
            }
            /*else if(m_bBufferFull)
            {
                sv_trace("memory overflow, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                assert(0);
            }*/
            if((int64_t)GetSysTime() - m_llLastSendTime > m_llintervalmsgtime)
            {
                ret = SendHeartBeat();
                if(CheckFailed(ret))
                {
                    svt_error(m_pPushProtocol->LogTag(), "break, SendHeartBeat failed, ret:%d, sn:%s", ret, m_pTag);
                    /*int vmpeak = 0, vmmem = 0, peakmem = 0, mem = 0, datamem = 0, stackmem = 0, exemem = 0, libmem = 0;
		            get_proc_mem(getpid(), &vmpeak, &vmmem, &peakmem, &mem, &datamem, &stackmem, &exemem, &libmem);
                    sv_trace("vmpeak:%d, vmmem:%d, peakmem:%d, mem:%d, datamem:%d, stackmem:%d, exemem:%d, libmem:%d)", vmpeak, vmmem, peakmem, mem, datamem, stackmem, exemem, libmem);*/
                  	break;
                }
                m_llLastSendTime = GetSysTime();
            }
            usleep(20000);
        }
        
    }
    if(!m_MuxerThread.IsAlive())
    {
        Unpublish();
    }
    /*if(m_pPushProtocol)
    {
        m_pPushProtocol->Close();
        delete m_pPushProtocol;
        m_pPushProtocol = NULL;
    }*/

#ifdef MUXER_AND_SEND_FRAME_TOGETHER
    if(pdstframe)
    {
        delete pdstframe;
        pdstframe = NULL;
    }
#endif

    if(CheckFailed(ret))
    {
        //svt_error(m_pTag, "break, ret:%d = write(pframe), connectdur:%" PRId64 ", senddur:%" PRId64 "", ret, GetSysTime() - m_llLastSendTime, GetSysTime() - m_llStartPts);
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0);
        //sv_trace("OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0)");
        //assert(0);
        //break;
    }
    else
    {
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_CONNECT_CLOSE, 0, 0);
    }
    /*int vmpeak = 0, vmmem = 0, peakmem = 0, mem = 0, datamem = 0, stackmem = 0, exemem = 0, libmem = 0;
    get_proc_mem(getpid(), &vmpeak, &vmmem, &peakmem, &mem, &datamem, &stackmem, &exemem, &libmem);
    sv_trace("vmpeak:%d, vmmem:%d, peakmem:%d, mem:%d, datamem:%d, stackmem:%d, exemem:%d, libmem:%d)", vmpeak, vmmem, peakmem, mem, datamem, stackmem, exemem, libmem);
    char* pnew = new char[1024*4];
    memset(pnew, 0, 1024*4);
    get_proc_mem(getpid(), &vmpeak, &vmmem, &peakmem, &mem, &datamem, &stackmem, &exemem, &libmem);
    sv_trace("vmpeak:%d, vmmem:%d, peakmem:%d, mem:%d, datamem:%d, stackmem:%d, exemem:%d, libmem:%d)", vmpeak, vmmem, peakmem, mem, datamem, stackmem, exemem, libmem);*/
    //svt_trace(m_pTag, "muxer thread end, m_bRun:%d, ret:%d", m_bRun, ret);
    return ret;
}
#else
/*
int CBasePush::MuxerProc()
{
    int ret = 0;
    m_llStartPts = GetSysTime();
    CFrameBuffer* pdstframe = NULL;
#ifndef MUXER_RTMP_PACKET_DIRECTORY
    pdstframe = new CFrameBuffer(m_pPushProtocol);
#endif
    bool segmentstart = false;
    //svt_trace(m_pTag, "pdstframe:%p = new CFrameBuffer(m_pPushProtocol)", pdstframe);
//#endif
    svt_trace(m_pTag, "MuxerProc begin");
    m_llLastSendTime = GetSysTime();
    while(m_MuxerThread.IsAlive())
    {
	    //sv_trace("m_MuxerQueue.size():%d", m_MuxerQueue.size());
        if(m_MuxerQueue.size() > 0)
        {
            if(m_bBufferFull && sv_get_free_memory(m_pPushProtocol) > g_nMemoryPoolSize / 3)
            {
                sv_trace("resume media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH, ret, 0);
                m_bBufferFull = false;
            }

            CFrameBuffer* psrcframe = m_MuxerQueue.pop_frame();
            if(NULL == psrcframe)
            {
                svt_error(m_pTag, "psrcframe:%p = m_MuxerQueue.pop_frame() is NULL", psrcframe);
                ret = ERROR_FAIL;
                break;
            }
            
            
#ifdef MUXER_RTMP_PACKET_DIRECTORY
            pdstframe = psrcframe;
            //sv_trace("psrcframe->m_nmsgtype:%d, pdstframe->m_nmsgid:%d", psrcframe->m_nmsgtype, pdstframe->m_nmsgid);
            if(E_SV_MEDIA_TYPE_METADATA == pdstframe->m_nmsgtype)
            {
                if(E_SV_METADATA_TYPE_STREAM_START == pdstframe->m_nmsgid)
                {
                    OnSegmentSendStart();
                }
                else if(E_SV_METADATA_TYPE_STREAM_END == pdstframe->m_nmsgid)
                {
                    OnSegmentSendEnd();
                }
            }
            
#else
            pdstframe->Reset();
            if(E_SV_MEDIA_TYPE_H264 == psrcframe->m_nmsgtype || E_SV_MEDIA_TYPE_AAC == psrcframe->m_nmsgtype)
            {
                ret = OnSendPacket(psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_nflag, psrcframe->m_llpts, pdstframe);
            }
            else if(E_SV_MEDIA_TYPE_METADATA == psrcframe->m_nmsgtype)
            {
                psrcframe->Clone(pdstframe);
                if(E_SV_METADATA_TYPE_STREAM_START == pdstframe->m_nmsgid)
                {
                    OnSegmentSendStart();
                }
                else if(E_SV_METADATA_TYPE_STREAM_END == pdstframe->m_nmsgid)
                {
                    OnSegmentSendEnd();
                }
            }
            else
            {
                svt_trace(m_pTag, "Invalid message type, psrcframe->m_nmsgtype:%d", psrcframe->m_nmsgtype);
            }
            
            //sv_info("ret:%d = OnSendPacket(psrcframe->m_nmsgtype:%d, psrcframe->m_pdata:%p, psrcframe->m_nsize:%d, psrcframe->m_llpts:%" PRId64 ", pdstframe:%p)", ret, psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_llpts, pdstframe);
            if(CheckFailed(ret))
            {
                svt_error(m_pTag, "ret:%d = OnSendPacket(m_nmsgtype:%d, m_pdata:%p, m_nsize:%d, pts:%" PRId64 ", dstframe:%p) faield", ret, psrcframe->m_nmsgtype, psrcframe->m_pdata, psrcframe->m_nsize, psrcframe->m_llpts, pdstframe);
                m_MuxerQueue.free_frame_buffer(&psrcframe);
                //may be send buffer list is full, begin to drop frame ...
                assert(0);
                continue;
            }
#endif
            if(pdstframe->m_pdata && pdstframe->m_ncurpos > 0)
            {
                ret = m_pPushProtocol->SendFrame(pdstframe);
                sv_info("ret:%d = m_pPushProtocol->SendFrame(pdstframe), pdstframe->m_nmsgtype:%d, pdstframe->m_ncurpos:%d == pdstframe->m_nsize:%d, pts:%" PRId64 "", ret, pdstframe->m_nmsgtype, pdstframe->m_ncurpos, pdstframe->m_nsize, pdstframe->m_llpts);

                if(RTMP_MSG_VideoMessage == pdstframe->m_nmsgtype)
                {
                    m_nSendVideoBytes += pdstframe->m_ncurpos;
#ifdef WRITE_OUTPUT_VIDEO_RECORD
                    if(m_pRtmpMuxer)
                    {
                        m_pRtmpMuxer->writeframe(codec_id_h264, pdstframe->m_pdata, pdstframe->m_ncurpos, pdstframe->m_llpts);
                    }
#endif
                }
                else if(RTMP_MSG_AudioMessage == pdstframe->m_nmsgtype)
                {
                    m_nSendAudioBytes += pdstframe->m_ncurpos;
#ifdef WRITE_OUTPUT_VIDEO_RECORD
                    if(m_pRtmpMuxer)
                    {
                        m_pRtmpMuxer->writeframe(codec_id_aac, pdstframe->m_pdata, pdstframe->m_ncurpos, pdstframe->m_llpts);
                    }
#endif
                }
                else if(E_SV_MEDIA_TYPE_METADATA == psrcframe->m_nmsgtype && E_SV_METADATA_TYPE_STREAM_END == pdstframe->m_nmsgid)
                {
                    //m_nSendVideoBytes, m_nSendAudioBytes, m_llLastSendVPts
                    OnCallbackNotify(E_SV_EVENT_TYPE_PUSH_SEGMENT_END, m_llLastSendVPts, m_llLastSendAPts);
                    sv_trace("OnCallbackNotify(E_SV_EVENT_TYPE_PUSH_SEGMENT_END, m_llLastSendVPts:%" PRId64 ", m_llLastSendAPts:" PRId64 ")\n",  m_llLastSendVPts, m_llLastSendAPts);
                }
                m_llLastSendTime = GetSysTime();
                if(CheckFailed(ret))
                {
                    m_MuxerQueue.free_frame_buffer(&psrcframe);
                    svt_error(m_pTag, "ret:%d = m_pPushProtocol->SendFrame(pdstframe) failed", ret);
                    break;
                }
            }
            
	        m_MuxerQueue.free_frame_buffer(&psrcframe);

        }
        else
        {
            if(m_bBufferFull && sv_get_free_memory(m_pPushProtocol) > g_nMemoryPoolSize / 3)
            {
                sv_trace("resume media push, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_RESUME_MEDIA_PUSH, ret, 0);
                m_bBufferFull = false;
            }
            else if(m_bBufferFull)
            {
                sv_trace("memory overflow, free memory size:%d", sv_get_free_memory(m_pPushProtocol));
                assert(0);
            }
            if(GetSysTime() - m_llLastSendTime > m_llintervalmsgtime)
            {
                ret = SendHeartBeat();
                if(CheckFailed(ret))
                {
                    svt_error(m_pTag, "break, SendHeartBeat failed, ret:%d", ret);
                  	break;
                }
                m_llLastSendTime = GetSysTime();
            }
            usleep(20000);
        }
        
    }

    Unpublish();

#ifndef MUXER_RTMP_PACKET_DIRECTORY
    if(pdstframe)
    {
        delete pdstframe;
        pdstframe = NULL;
    }
#endif

    if(CheckFailed(ret))
    {
        //svt_error(m_pTag, "break, ret:%d = write(pframe), connectdur:%" PRId64 ", senddur:%" PRId64 "", ret, GetSysTime() - m_llLastSendTime, GetSysTime() - m_llStartPts);
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0);
        sv_trace("OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_SEND_THREAD_EXIT_WITH_ERROR, 0, 0)");
        //assert(0);
        //break;
    }
    svt_trace(m_pTag, "muxer thread end, m_bRun:%d, ret:%d", m_bRun, ret);
    return ret;
}*/
#endif
int CBasePush::Start()
{
    //int ret = Run();
    //svt_trace(m_pTag, "ret:%d = Run()\n", ret);
    m_MuxerThread.init_class_func(this, &CBasePush::MuxerProc);
    //sv_trace("before ret = m_MuxerThread.Run()\n");
    int ret = m_MuxerThread.Run(1);
    sv_info("start end, ret:%d\n", ret);
    return ret;
}

int CBasePush::Stop()
{
    int bcloseskt = 0;
    int loop_num = 0;
    int ret = 0;
    int last_send_duration = m_pPushProtocol->GetLastSendDuration();
    //sv_trace("CBasePush::Stop begin, last_send_duration:%d\n", last_send_duration);
    do{
        if(!bcloseskt && m_pPushProtocol && (last_send_duration > 50 || loop_num >= 1))
        {
            bcloseskt = 1;
            m_pPushProtocol->CloseSocket();
            svt_trace(m_pPushProtocol->LogTag(), "m_pPushProtocol->CloseSocket(), last_send_duration:%d\n", last_send_duration);
        }

        loop_num++;
        if(loop_num++%20 == 0)
        {
            svt_trace(m_pPushProtocol->LogTag(), "waiting for m_MuxerThread stop, loop_num:%d\n", loop_num);
        }
        if(m_MuxerThread.IsAlive())
        {
            m_MuxerThread.Stop();
        }
        if(!m_MuxerThread.IsExit()) usleep(50000);

    }while(!m_MuxerThread.IsExit());
    //sv_trace("stop end, ret:%d\n", ret);
    return ret;
}

bool CBasePush::check_frame(int eMediaType, char* pData, unsigned int nSize, int64_t pts, int& keyframe, int encflag)
{
    bool ret = true;

    if(encflag)
    {
        if(keyframe)
        {
            m_bDisContinue = 0;
        }
        return true;
    }

    if(E_SV_MEDIA_TYPE_H264 == eMediaType)
    {
        unsigned int i = 0;
        while(pData[i] == 0 && i < nSize - 1) i++;
        int nalutype = pData[i+1]&0x1f;
        if((2 == i || 3 == i) && 1 == (unsigned char)pData[i])
        {
            keyframe = nalutype == 5 ?  1 : 0;
        }
        else
        {
            m_bDisContinue = 1;
            m_llLastPts = pts;
            svt_error(m_pPushProtocol->LogTag(), "Invalid H264 start code,i:%d, nalutype:%d", i, nalutype);
            sv_memory(3, pData, 8);
            return false;
        }
        if(m_bDisContinue)
        {
            // if sps pps, idr come
            if(5 == nalutype || 7 == nalutype || 8 == nalutype)
            {
                m_bDisContinue = 0;
                svt_trace(m_pPushProtocol->LogTag(), "key frame come! nalutype:%d", nalutype);
            }
            else
            {
                sv_warn("discontine, drop frame, nalutype:%d\n", nalutype);
                sv_memory(3, pData, 16);
                ret = false;
            }
        }
        m_llLastPts = pts;
        //sv_info("eMediaType:%d check frame ret:%d\n", eMediaType, (int)ret);
        return ret;
    }
    else if(E_SV_MEDIA_TYPE_H265 == eMediaType)
    {
        unsigned int i = 0;
        while(pData[i] == 0 && i < nSize - 1) i++;
        //if(1 == (pData[i]) i++;
        int nalutype = pData[i+1]&0x7e >> 1;
        if((2 == i || 3 == i) && 1 == (unsigned char)pData[i])
        {
            if(nalutype >= 15 && nalutype <= 21)
            {
                keyframe = 1;
            }
        }
        else
        {
            m_bDisContinue = 1;
            m_llLastPts = pts;
            svt_error(m_pPushProtocol->LogTag(), "Invalid hevc start code,i:%d, nalutype:%d", i, nalutype);
            sv_memory(3, pData, 8);
            return false;
        }
        if(m_bDisContinue)
        {
            // if sps pps, idr come
            if(32== nalutype || 33 == nalutype || 34 == nalutype)
            {
                m_bDisContinue = 0;
                svt_trace(m_pPushProtocol->LogTag(), "hevc key frame come! nalutype:%d", nalutype);
            }
            else
            {
                //sv_warn("discontine, drop frame");
                ret = false;
            }
        }
        m_llLastPts = pts;
        return ret;
    }
    else if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        if((unsigned char)0xff != (unsigned char)pData[0])
        {
            svt_error(m_pPushProtocol->LogTag(), "Invliad adts data header:%0x", (unsigned char)pData[0]);
            sv_memory(3, pData, 8);
            return  false;
        }
        return true;
    }

    return true;
}

 int CBasePush::OnSegmentDeliverStart(int64_t media_timestamp)
 {
    if(m_bStreamDeliverStart)
    {
        OnSegmentDeliverEnd();
    }
    m_bStreamDeliverStart = true;
    m_nDeliverVideoBytes = 0;
    m_nDeliverAudioBytes = 0;
    m_nVideoDropBytes    = 0;
    //sv_trace("segment start, m_nDeliverVideoBytes:%d, m_nDeliverVideoBytes:%d", m_nDeliverVideoBytes, m_nDeliverVideoBytes);
    m_llLastVideoPts   = INT64_MIN;
    m_llLastAudioPts   = INT64_MIN;
    time_t t = media_timestamp/1000;
    struct tm *p;
    p=gmtime(&t);
    char datetime[100] = {0};
    strftime(datetime, 100, "%Y%m%d-%H%M%S", p);
    m_sDataTime = datetime;
#ifdef WRITE_INPUT_VIDEO_RECORD
    if(NULL == m_pRecMuxer)
    {
        m_pRecMuxer = new RecordMuxer();
    }
    char url[1024] = {0};
    sprintf(url, "%s/../data/", get_log_path());
    create_if_not_exist(url);
    strcat(url, "%s_raw.data");
    //sv_trace("m_pRecMuxer->open(url:%s, codec_id_h264, codec_id_aac, 0, m_llMediaTimeStamp:%" PRId64 ")", url, media_timestamp);
    int ret = m_pRecMuxer->open(url, codec_id_h264, codec_id_aac, 0, media_timestamp);
    if(CheckFailed(ret))
    {
        svt_error(m_pPushProtocol->LogTag(), "open record muxer failed, url:%s", url);
        return ret;
    }
#endif
    return 0;
 }

int CBasePush::OnSegmentDeliverEnd()
{
    m_bStreamDeliverStart = false;
    m_llLastDeliverVPts = m_llLastVideoPts;
    m_llLastDeliverAPts = m_llLastAudioPts;
    //svt_trace(m_pTag, "segment deliver end, m_nDeliverVideoBytes:%d, m_nDeliverAudioBytes:%d, m_nVideoDropBytes:%d", m_nDeliverVideoBytes, m_nDeliverAudioBytes, m_nVideoDropBytes);
#ifdef WRITE_INPUT_VIDEO_RECORD
    if(m_pRecMuxer)
    {
        m_pRecMuxer->close();
        delete m_pRecMuxer;
        m_pRecMuxer = NULL;
    }
#endif
    return 0;
}

int CBasePush::OnSegmentSendStart()
{
    if(m_bStreamSendStart)
    {
        svt_trace(m_pPushProtocol->LogTag(), "segment start without segment end\n");
        OnSegmentSendEnd();
    }
    m_bStreamSendStart = true;
    m_nSendVideoBytes = 0;
    m_nSendAudioBytes = 0;
    m_llLastSendVPts = INT64_MIN;
    m_llLastSendAPts = INT64_MIN;
    //svt_trace(m_pTag, "m_llLastSendVPts:%" PRId64 ", m_llLastSendAPts:%" PRId64 "", m_llLastSendVPts, m_llLastSendAPts);
#ifdef WRITE_OUTPUT_VIDEO_RECORD
    if(NULL == m_pRtmpMuxer)
    {
        m_pRtmpMuxer = new RecordMuxer();
    }
    char url[1024] = {0};
    sprintf(url, "%s/../data/", get_log_path());
    create_if_not_exist(url);
    strcat(url, "%s_rtmp.data");
    int ret = m_pRtmpMuxer->open(url, codec_id_h264, codec_id_aac, 0, m_llMediaTimeStamp);
    if(CheckFailed(ret))
    {
        svt_error(m_pTag, "open record muxer failed, url:%s", url);
        return ret;
    }
#endif
    return 0;
}

int CBasePush::OnSegmentSendEnd()
{
    m_bStreamSendStart = false;
    svt_trace(m_pPushProtocol->LogTag(), "sn:%s, datetime:%s, send vbytes:%d, send abytes:%d, send vpts:%" PRId64 ", send apts:%" PRId64 ", m_nAlarmTime:%d", m_pTag, m_sDataTime.c_str(), m_nSendVideoBytes, m_nSendAudioBytes, m_llLastSendVPts, m_llLastSendAPts, m_nAlarmTime);
    if(m_nVideoDropBytes > 0 || m_llLastDeliverVPts > m_llLastSendVPts || m_llLastDeliverAPts > m_llLastSendAPts)
    {
        sv_trace("sn:%s, datetime:%s, drop bytes:%d, delive vpts:%" PRId64 " deliver apts:%" PRId64 "", m_pTag, m_sDataTime.c_str(), m_nVideoDropBytes, m_llLastDeliverVPts, m_llLastDeliverAPts);
    }

    if(m_pRecMuxer)
    {
        m_pRecMuxer->close();
        delete m_pRecMuxer;
        m_pRecMuxer = NULL;
    }
    
    //if(m_nDeliverVideoBytes == m_nSendVideoBytes)
    //{
    //sv_trace("sn:%s m_sDataTime:%s, free memory:%d, DVBytes:%d, DABytes:%d, SVBytes:%d, SABytes:%d, VDropBytes:%d, vpts:%" PRId64 ", apts:%" PRId64 ", AlarmTime:%d", m_pTag, m_sDataTime.c_str(), sv_get_free_memory(m_pPushProtocol), m_nDeliverVideoBytes, m_nDeliverAudioBytes, m_nSendVideoBytes, m_nSendAudioBytes, m_nVideoDropBytes, m_llLastVideoPts, m_llLastAudioPts, m_nAlarmTime);
    /*}
    else
    {
        svt_trace(m_pTag, "segment send end, m_nDeliverVideoBytes:%d, m_nSendVideoBytes:%d, m_nSendAudioBytes:%d, m_bStreamSendStart:%d, m_nVideoDropBytes:%d", m_nDeliverVideoBytes, m_nSendVideoBytes, m_nSendAudioBytes, (int)m_bStreamSendStart, m_nVideoDropBytes);
    }*/
#ifdef WRITE_OUTPUT_VIDEO_RECORD
    if(m_pRtmpMuxer)
    {
        m_pRtmpMuxer->close();
        delete m_pRtmpMuxer;
        m_pRtmpMuxer = NULL;
    }
#endif
    return 0;
}
#ifdef ENABLE_DUMP_SYS_INFO
#define VMSIZE    "VmSize"
#define VMRSS     "VmRSS"
#define PROCESS_ITEM    14
int CBasePush::get_pro_memory(int pid, int* pmemory, int* pvirtual_memory)
{
    FILE *fd = NULL;
	int i = 0;
	int vmsize = 0;
	char file_name[64];
	char line_buff[512];
	char name[64];
    bool is_vmrss_read = false;
    bool is_vmsize_read = false;

	memset(file_name, 0, 64);
	sprintf(file_name, "/proc/%d/status", pid);
	
	fd = fopen(file_name, "r");
	if(fd == NULL)
	{
        svt_error(m_pPushProtocol->LogTag(), "open %s failed!", file_name);
		return 0;
	}
	
    do
    {
        memset(line_buff, 0, 512);
		if(fgets(line_buff, 512, fd))
        {
            if(pvirtual_memory && 0 == memcmp(line_buff, VMSIZE, strlen(VMSIZE)))
            {
                //sv_trace("pid:%d, line:%d: %s", pid, i, line_buff);
                sscanf(line_buff, "%s %d", name, pvirtual_memory);
                is_vmsize_read = true;
            }
            else if(pmemory && 0 == memcmp(line_buff, VMRSS, strlen(VMRSS)))
            {
                //sv_trace("pid:%d, line:%d: %s", pid, i, line_buff);
                sscanf(line_buff, "%s %d", name, pmemory);
                is_vmrss_read = true;
            }
            i++;
        }
        else
        {
            break;
        }
        
    }while(!is_vmsize_read || !is_vmrss_read);
	fclose(fd);
	//sscanf(line_buff, "%s %d", name, &vmsize);
	
	return vmsize;
}
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
unsigned long CBasePush::get_cpu_total_occupy()
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
const char* CBasePush::get_items(const char*buffer, unsigned int item)
{
	const char *p = buffer;
 	int i;
	int len; 
	unsigned int count = 0;

	len= strlen(buffer);
	
	for(i = 0; i < len; i++)
	{
		if(' ' == *p)
		{
			count ++;
			if(count == item - 1)
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
unsigned long CBasePush::get_cpu_proc_occupy(unsigned int pid)
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

float CBasePush::get_proc_cpu(unsigned int pid)
{
	float cpu = 0.0;

	unsigned long totalcputime1;
	unsigned long totalcputime2;
	unsigned long procputime1;
	unsigned long procputime2;

	totalcputime1 = get_cpu_total_occupy();
	procputime1 = get_cpu_proc_occupy(pid);

	usleep(200000);

	totalcputime2 = get_cpu_total_occupy();
	procputime2 = get_cpu_proc_occupy(pid);

	if(0 != totalcputime2-totalcputime1)
	{ 
		cpu = 100.0 * (procputime2 - procputime1) / (totalcputime2 - totalcputime1);
	}
	
	return cpu;

}
#endif

void CBasePush::on_recv_packet(int eMediaType, char* pData, unsigned int nSize, int64_t pts, int keyframe, int encflag)
{
    int ret = 0;
    if(NULL == m_pRecMuxer && m_precord_url && (E_SV_MEDIA_TYPE_H264 == eMediaType || E_SV_MEDIA_TYPE_H265 == eMediaType))
    {
        m_pRecMuxer = new HS004RecordMuxer();
        ret = m_pRecMuxer->open(m_precord_url, (codec_id)eMediaType, codec_id_aac, encflag);
        sv_trace("ret:%d = m_pRecMuxer->open(m_precord_url:%s, eMediaType:%d, codec_id_aac:%d, encflag:%d)\n", ret, m_precord_url, eMediaType, codec_id_aac, encflag);
    }
    
    if(m_pRecMuxer)
    {
        int cid = eMediaType;
        /*if(E_SV_MEDIA_TYPE_AAC == cid)
        {
            cid = codec_id_aac;
        }
        else
        {
            cid = eMediaType;
        }*/
        ret = m_pRecMuxer->writeframe((codec_id)cid, pData, nSize, keyframe, pts);
        sv_info("ret:%d = m_pRecMuxer->writeframe(cid:%d, pData:%p, nSize:%d, keyframe:%d, pts:%" PRId64 " )\n", ret, cid, pData, nSize, keyframe, pts);
    }
}
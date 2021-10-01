/*****************************************************************************************************************
 * filename     pushmgr.cpp
 * describe     Sunvalley coud storage push manager implement
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ****************************************************************************************************************/
#include "pushmgr.h"
#include "rtmppush.h"
#include "microdef.h"
#include "irtmpprotocol.h"
log_ctx* g_plog_ctx = NULL;
ISVPushMgr* CSVPushMgr::m_pIPushMgr(NULL);

ISVPushMgr* ISVPushMgr::CreateInst()
{
    ISVPushMgr* ppushmgr = new CSVPushMgr();
    return ppushmgr;
}

void ISVPushMgr::DestroyInst(ISVPushMgr* pinst)
{
    CSVPushMgr* ppushmgr = dynamic_cast<CSVPushMgr*>(pinst);
    if(!ppushmgr)
    {
        sv_error("Invalid PushMgr ptr, org ptr %p, contvert to svpushmgr %p failed", pinst, ppushmgr);
        return ;
    }

    delete ppushmgr;
    ppushmgr = NULL;
}

CSVPushMgr::CSVPushMgr()
{
    //close_log_contex(&g_plog_ctx);
    m_lConnectId = ::abs((long)GetSysTime())%10000;
    m_sToken_Server_Url = GET_TEST_TOKEN_SERVER_URL;

}

CSVPushMgr::~CSVPushMgr()
{
    //CAutoLock lock(m_CriSec, );;
    while(!m_CriSec.trylock())
    {
        sleep(3);
        //sv_trace("try lock failed, m_CriSec.nfun_id:%d\n", m_CriSec.nfun_id);
    }
    while(m_mPushList.size() > 0)
    {
        std::map<long, ISVPush*>::iterator it = m_mPushList.begin();
        if(it->second)
        {
            it->second->Close();
            delete it->second;
            //it->second = NULL;
        }
        m_mPushList.erase(it);
    }
    m_CriSec.unlock();
}

ISVPushMgr*  CSVPushMgr::GetInstace()
{
    if(NULL == m_pIPushMgr)
    {
        m_pIPushMgr = new CSVPushMgr();
    }
    return m_pIPushMgr;
}

int CSVPushMgr::InitLog(int nLogLevel, unsigned int  nLogFlag, const char* pLogPath)
{
    CAutoLock lock(m_CriSec);
    
    if(NULL == pLogPath && (LOG_OUTPUT_FLAG_FILE & nLogFlag))
    {
	    printf("CSVPushMgr::InitLog(pLogPath:%s, nLogFlag:%d) failed\n", pLogPath, nLogFlag);
        return -1;
    }
    sv_init_log(pLogPath, nLogLevel, nLogFlag, SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY);
    printf("after sv_init_log\n");
    return g_plog_ctx ? 0 : -1;
}

int CSVPushMgr::Set_Token_Fetct_Interface(const char* ptoken_server_url)
{
    if(ptoken_server_url && strlen(ptoken_server_url) > 0)
    {
        m_sToken_Server_Url = ptoken_server_url;
         sv_trace("set token server url success %s\n", m_sToken_Server_Url.c_str());
         return 0;
    }
    sv_error("set token server failed, %s\n", ptoken_server_url);
    return -1;
}

long CSVPushMgr::Connect(const char* purl, const char* token, IEventNotify* pEvenHandle, int streamType)
{
    CAutoLock lock(m_CriSec);
    
    sv_trace("begin pUrl:%s, token:%s, pEvenHandle:%p, streamType:%d\n", purl, token, pEvenHandle, streamType);
    if(!purl || !pEvenHandle)
    {
        return INVALID_CONNECTION_ID_VALUE;
    }

    ISVPush* ppush = NULL;
    if(memcmp(purl, "rtmp://", strlen("rtmp://")) == 0)
    {
        // rtmp push
        ppush = new CRtmpPush();
        ppush->SetTokenFetctInterface(m_sToken_Server_Url.c_str());
    }
    else
    {
        sv_error("Invalid url:%s, Connect faild!", purl);
        return INVALID_CONNECTION_ID_VALUE;
    }

    //long lcid = ppush->Connect（pUrl, pEventHandle);
    //ISVPush* ppush = new CSVPush();
    long lcid = GenConnectId();
    if(INVALID_CONNECTION_ID_VALUE == lcid)
    {
        delete ppush;
        ppush = NULL;
        return INVALID_CONNECTION_ID_VALUE;
    }
    ppush->SetStreamType(streamType);
    int ret = ppush->Connect(purl, token, pEvenHandle);
    if(CheckFailed(ret))
    {
        sv_error("ret:%d = ppush->Connect(purl:%s, pEvenHandle:%p) failed", ret, purl, pEvenHandle);
        delete ppush;
        ppush = NULL;
        return INVALID_CONNECTION_ID_VALUE;
    }
    
    //long lcid = m_lConnectId++;
    m_mPushList[lcid] = ppush;
    
    sv_trace("end, lcid:%d", lcid);
    return lcid;
}

long CSVPushMgr::Connect(const char* purl, const char* token, event_callback pcbFun, int streamType)
{
    sv_trace("begin purl:%s, token:%s, pcbFun:%p, streamType:%d", purl, token, pcbFun, streamType);
    if(!purl || !pcbFun)
    {
        sv_error("Invalid purl:%p, pcbFun:%p", purl, pcbFun);
        return INVALID_CONNECTION_ID_VALUE;
    }
    
    ISVPush* ppush = NULL;
    CAutoLock lock(m_CriSec);
    
    if(memcmp(purl, "rtmp://", strlen("rtmp://")) == 0)
    {
        // rtmp push
        ppush = new CRtmpPush();
        ppush->SetTokenFetctInterface(m_sToken_Server_Url.c_str());
    }
    else
    {
        sv_error("Invalid url:%s, Connect faild!", purl);
        return INVALID_CONNECTION_ID_VALUE;
    }
    long lcid = GenConnectId();
    if(INVALID_CONNECTION_ID_VALUE == lcid)
    {
        delete ppush;
        ppush = NULL;
        return lcid;
    }

    ppush->SetStreamType(streamType);

    int ret = ppush->Connect(purl, token, lcid, pcbFun);
    if(CheckFailed(ret))
    {
        delete ppush;
        ppush = NULL;
        sv_error("ret:%d = ppush->Connect(purl:%s, pcbFun:%p) failed", ret, purl, pcbFun);
        return INVALID_CONNECTION_ID_VALUE;
    }

    
    m_mPushList[lcid] = ppush;

    //sv_trace("end, lcid:%d, ppush:%p, m_lConnectId:%ld", lcid, ppush, m_lConnectId);
    return lcid;
}

long CSVPushMgr::ConnectByApp(auth_info* pai, const char* pappstreamname, event_callback pcbfun)
{
    sv_info("begin (pai:%s, pappstreamname:%s, pcbfun:%p)", pai, pappstreamname, pcbfun);
    if(!pappstreamname || !pcbfun)
    {
        sv_error("Invalid parameter, pai:%p, pappstreamname:%p, pcbfun:%p\n", pai, pappstreamname, pcbfun);
        return INVALID_CONNECTION_ID_VALUE;
    }
    
    ISVPush* ppush = NULL;
    CAutoLock lock(m_CriSec);
    ppush = new CRtmpPush();
    ppush->SetTokenFetctInterface(m_sToken_Server_Url.c_str());
    //sv_trace("after ppush->SetTokenFetctInterface(m_sToken_Server_Url.c_str())\n");
    long lcid = GenConnectId();
    if(INVALID_CONNECTION_ID_VALUE == lcid)
    {
        delete ppush;
        ppush = NULL;
        return lcid;
    }
    //sv_trace("lcid:%ld = GenConnectId()\n", lcid);
    int ret = ppush->ConnectByApp(pai, pappstreamname, NULL, lcid, pcbfun);
    if(CheckFailed(ret))
    {
        sv_error("ret:%d = ppush->ConnecByApp(pai:%p, pappstreamname:%p, NULL, lcid:%ld, pcbfun:%p) failed", ret, pai, pappstreamname, lcid, pcbfun);
        ppush->Close();
        delete ppush;
        ppush = NULL;
        return INVALID_CONNECTION_ID_VALUE;
    }

    
    m_mPushList[lcid] = ppush;

    //sv_trace("end, lcid:%d, ppush:%p, m_lConnectId:%ld", lcid, ppush, m_lConnectId);
    return lcid;
}

long CSVPushMgr::ConnectByDeviceToken(const char* pdevice_token, event_callback pcbfun)
{
    if(!pdevice_token || !pcbfun)
    {
        sv_error("Invalid parameter, pdevice_token:%p, pcbfun:%p\n", pdevice_token, pcbfun);
        return INVALID_CONNECTION_ID_VALUE;
    }

    ISVPush* ppush = NULL;
    CAutoLock lock(m_CriSec);
    ppush = new CRtmpPush();
    ppush->SetTokenFetctInterface(m_sToken_Server_Url.c_str());
    long lcid = GenConnectId();
    if(INVALID_CONNECTION_ID_VALUE == lcid)
    {
        delete ppush;
        ppush = NULL;
        return lcid;
    }
    int ret = ppush->ConnectByApp(NULL, NULL, pdevice_token, lcid, pcbfun);
    if(CheckFailed(ret))
    {
        sv_error("ret:%d = ppush->ConnecByApp(NULL, NULL, NULL, pdevice_token:%s, lcid:%ld, pcbfun:%p) failed", ret, pdevice_token, lcid, pcbfun);
        ppush->Close();
        delete ppush;
        ppush = NULL;
        return INVALID_CONNECTION_ID_VALUE;
    }

    
    m_mPushList[lcid] = ppush;

    //sv_trace("end, lcid:%d, ppush:%p, m_lConnectId:%ld", lcid, ppush, m_lConnectId);
    return lcid;
}
#define LIVE_TOKEN_URL "https://third-api-dev.sunvalleycloud.com/connection/token/get"
long CSVPushMgr::Live_Connect(const char* ptoken_url, const char* pdeviceSN, event_callback pcbfun)
{
    if(!ptoken_url || /*!pdeviceSN ||*/ !pcbfun)
    {
        sv_error("Invalid parameter, ptoken_url:%p, pdeviceSN:%p, pcbfun:%p\n", ptoken_url, pdeviceSN, pcbfun);
        return INVALID_CONNECTION_ID_VALUE;
    }

    if(NULL == pdeviceSN)
    {
        return Connect(ptoken_url, pdeviceSN, pcbfun, 1);
    }

    if(memcmp("http", ptoken_url, strlen("http")) != 0)
    {
        ptoken_url = LIVE_TOKEN_URL;
    }
    ISVPush* ppush = NULL;
    CAutoLock lock(m_CriSec);
    ppush = new CRtmpPush();
    ppush->SetTokenFetctInterface(ptoken_url);
    long lcid = GenConnectId();
    if(INVALID_CONNECTION_ID_VALUE == lcid)
    {
        delete ppush;
        ppush = NULL;
        sv_error("get connectid failed, lcid:%ld = GenConnectId()", lcid);
        return lcid;
    }
    //sv_trace("lcid:%ld = GenConnectId()\n", lcid);
    int ret = ppush->ConnectByApp(NULL, pdeviceSN, NULL, lcid, pcbfun);
    if(CheckFailed(ret))
    {
        sv_error("ret:%d = ppush->ConnecByApp(pappkey:%p, pappsecret:%p, pappstreamname:%p, NULL, lcid:%ld, pcbfun:%p) failed", ret, NULL, NULL, pdeviceSN, lcid, pcbfun);
        ppush->Close();
        delete ppush;
        ppush = NULL;
        return INVALID_CONNECTION_ID_VALUE;
    }

    m_mPushList[lcid] = ppush;
    //sv_trace("end, lcid:%d, ppush:%p, m_lConnectId:%ld", lcid, ppush, m_lConnectId);
    return lcid;
}


int CSVPushMgr::RecordSendPacket(long lcid, const char* precord_url)
{
    CAutoLock lock(m_CriSec);
    ISVPush* pipush = GetPushByConnectId(lcid);
    CheckPtr(pipush, ERROR_INVALID_PARAM);

    return pipush->RecordSendPacket(precord_url);
}

int CSVPushMgr::SendPacket(long lcid, int eMediaType, char* pData, unsigned int uSize, int64_t lltimestamp, int keyframe, int encflag)
{
    //sv_trace("(lcid:%p, eMediaType:%d, pData:%p, nSize:%d, llTimeStamp:%" PRId64 ", keyframe:%d, encflag:%d) begin", (void*)lcid, eMediaType, pData, uSize, lltimestamp, keyframe, encflag);
    //sv_memory(3, pData, 4);
    CAutoLock lock(m_CriSec);
    ISVPush* pipush = GetPushByConnectId(lcid);
    CheckPtr(pipush, ERROR_INVALID_PARAM);
    //long begin = GetSysTime();
    int ret = pipush->SendPacket(eMediaType, pData, uSize, lltimestamp, keyframe, encflag);

    /*if(GetSysTime() - begin > 10)
    {
        sv_trace("push mgr send packet ret:%d, time:%ld", ret, GetSysTime() - begin);
    }*/

    return ret;
}

int CSVPushMgr::SendMetadata(long lcid, int emetadatatype, long wParam, long lParam)
{
    //sv_trace("(lcid:%p, emetadatatype:%d, wParam:%ld, lParam:%ld) begin", (void*)lcid, emetadatatype, wParam, lParam);
    CAutoLock lock(m_CriSec);
    ISVPush* pipush = GetPushByConnectId(lcid);
    CheckPtr(pipush, ERROR_INVALID_PARAM);

    
    return pipush->SendMetadata(emetadatatype, wParam, lParam);
}

int CSVPushMgr::IsConnected(long lcid)
{
    CAutoLock lock(m_CriSec);
    ISVPush* pipush = GetPushByConnectId(lcid);
    CheckPtr(pipush, ERROR_INVALID_PARAM);

    
    return pipush->IsConnected();
}

int CSVPushMgr::Close(long lcid)
{
    CAutoLock lock(m_CriSec);
    //sv_trace("(lcid:%ld) begin", lcid);
    ISVPush* pipush = GetPushByConnectId(lcid);
    CheckPtr(pipush, ERROR_INVALID_PARAM);

    int ret = pipush->Close();
    std::map<long, ISVPush*>::iterator it = m_mPushList.find(lcid);
    if(it != m_mPushList.end())
    {
        assert(pipush == it->second);
        m_mPushList.erase(it);
        pipush->Close();
        delete pipush;
        pipush = NULL;
    }
    
    //sv_trace("end, ret:%d", ret);
    return ret;
}

ISVPush* CSVPushMgr::GetPushByConnectId(long lcid)
{
    //CAutoLock lock(m_CriSec);
    
    std::map<long, ISVPush*>::iterator it = m_mPushList.find(lcid);
    if(m_mPushList.end() == it)
    {
        sv_error("convert lcid:%ld to ipush failed\n", lcid);
        return NULL;
    }
    else
    {
        return it->second;
    }
}


long CSVPushMgr::GenConnectId()
{
    //CAutoLock lock(m_CriSec);
    if(m_mPushList.size() >= (size_t)g_nMaxConnectionCount)
    {
        sv_error("current active push connection count %d has reach max push connection count:%d", (int)m_mPushList.size(), g_nMaxConnectionCount);
        return INVALID_CONNECTION_ID_VALUE;
    }

    long lcid = m_lConnectId++;
    return lcid;
}

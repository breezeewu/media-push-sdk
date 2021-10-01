/***********************************************************************************************************************************
 * filename     rtmppush.cpp
 * describe     Sunvalley coud storage push sdk rtmp push class implement
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 **********************************************************************************************************************************/
#include "rtmppush.h"
//#define DISABLE_RTMP_PROTOCOL
#ifndef DISABLE_RTMP_PROTOCOL
#include "irtmpprotocol.h"
#else
#include "irtmpprotocol.h"
#endif

CRtmpPush::CRtmpPush()
{
#ifndef DISABLE_RTMP_PROTOCOL
    m_pPushProtocol = IPushProtocol::CreateRtmpPushProtocol();//new CRtmpProtocol();
    sv_init_pool(m_pPushProtocol, g_nMemoryPoolSize, MAX_RTMP_CONNECTION_BLOCK_SIZE);
    m_FrameList.init(m_pPushProtocol);
    //sv_trace("before m_MuxerQueue.init(m_pPushProtocol)\n");
    m_MuxerQueue.init(m_pPushProtocol);
    //sv_trace("before m_MuxerQueue.init_queue\n");
    m_MuxerQueue.init_queue(MAX_RTMP_CONNECTION_BLOCK_SIZE/2, g_nMemoryPoolSize);
    //sv_trace("after m_MuxerQueue.init_queue\n");
#endif
}

CRtmpPush::~CRtmpPush()
{
    //sv_trace("destroy m_pPushProtocol\n");
    m_FrameList.deinit_queue();
    Close();
    //sv_trace("m_FrameList deinit_queue success");
#ifndef DISABLE_RTMP_PROTOCOL
    if(m_pPushProtocol)
    {
        m_pPushProtocol->Close();
        delete m_pPushProtocol;
        sv_deinit_pool(m_pPushProtocol);
        m_pPushProtocol = NULL;
    }
#endif
}
int CRtmpPush::Connect(const char* purl, const char* ptoken, IEventNotify* pevenhandle)
{
    svt_trace(m_pPushProtocol->LogTag(), "(pUrl:%s, ptoken:%s, pevenhandle:%p) begin", purl, ptoken, pevenhandle);
    int ret = ERROR_OK;
    if(m_bConnected)
    {
        Close();
    }

    do
    {
        CAutoLock lock(m_CriSec);
        if(!purl || !pevenhandle)
        {
            svt_error(m_pPushProtocol->LogTag(), "Invalid parameter, !pUrl:%s || !peventhandle:%p", purl, pevenhandle);
            break;
        }
        m_sURL = purl;
        m_pEventNotify = pevenhandle;
        m_pPushProtocol->SetCallback(pevenhandle);
        ret = ResolveUrl(purl);
        svt_trace(m_pPushProtocol->LogTag(), "ret:%d = ResolveUrl(purl:%s)\n", ret, purl);
        if(CheckFailed(ret))
        {
            break;
        }

        ret = ConnectServer();
        svt_trace(m_pPushProtocol->LogTag(), "ret:%d = ConnectServer()\n", ret);
        if(CheckFailed(ret))
        {
            break;
        }
        //CheckResult(ret);
        //sv_trace("before ShakeHand");
        //sv_info(m_pTag, "before ShakeHand");
        ret = ShakeHand();
        svt_trace(m_pPushProtocol->LogTag(), "ret:%d = ShakeHand()", ret);
        if(CheckFailed(ret))
        {
            break;
        }
        //CheckResult(ret);
        string token;
        if(ptoken)
        {
            m_sToken = ptoken;
        }
        //sv_trace("before Authorize(token:%s)", token.c_str());
        ret = Authorize();
        //sv_trace("ret:%d = Authorize(token)\n", ret);
        //CheckResult(ret);
        if(CheckFailed(ret))
        {
        // ingore ...
        }
        
        ret = ConnectApp();
        svt_trace(m_pPushProtocol->LogTag(), "ret:%d = ConnectApp()\n", ret);
        if(CheckFailed(ret))
        {
            svt_error(m_pTag, "ret:%d = ConnectApp() failed", ret);
            break;
        }
    // CheckResult(ret);

        ret = Publish();
        svt_trace(m_pPushProtocol->LogTag(), "ret:%d = Publish()\n", ret);
        if(CheckFailed(ret))
        {
            svt_error(m_pPushProtocol->LogTag(), "ret:%d = Publish() failed", ret);
            break;
        }
        //CheckResult(ret);

        OnConnect();
        
        Start();
        
        // notify rtmp connect success
        OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_CONNECT_SUCCESS, 0, 0);

        sv_info("connect %s success, sn:%s, ret:%d", purl, m_pTag, ret);
        return ret;
    } while (0);
    
    OnCallbackNotify(E_SV_EVENT_TYPE_RTMP_PUBLISH_STREAM_FAILED, ret, 0);
    svt_error(m_pPushProtocol->LogTag(), "ret:%d =Connect failed", ret);
    Close();
    return ret;
}

int CRtmpPush::Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun)
{
    sv_info("(purl:%s,  ptoken:%p, pcbFun:%p)", purl,  ptoken, pcbfun);
    m_pPushCb = new CPushCallback();
    m_pPushCb->SetCallBack(lcid, pcbfun);
    int ret = Connect(purl, ptoken, m_pPushCb);

    return ret;
}

/*int CRtmpPush::SendPacket(int eMediaType, char* pData,unsigned int nSize, unsigned int nTimeStamp)
{

}

int CRtmpPush::SendMetadata(int eMsgType, long wParam, long lParam)
{

}

int CRtmpPush::IsConnected()
{
    return 0;
}*/

int CRtmpPush::Close()
{
    int ret = CBasePush::Close();
    return ret;
}

int CRtmpPush::SetStreamType(int streamType)
{
    m_nstreamType = streamType;
    //sv_trace("m_nstreamType:%d\n", m_nstreamType);
    return 0;
}

int CRtmpPush::ResolveUrl(const char* pUrl)
{
    CheckPtr(m_pPushProtocol, ERROR_FAIL);
    
    int ret = m_pPushProtocol->ResolveUrl(pUrl);
    return ret;
}

int CRtmpPush::ConnectServer()
{
    CheckPtr(m_pPushProtocol, ERROR_FAIL);
    
    int ret = m_pPushProtocol->ConnectServer();
    return ret;
}

int CRtmpPush::ShakeHand()
{
    CheckPtr(m_pPushProtocol, ERROR_FAIL);

    int ret = m_pPushProtocol->ShakeHand();
    return ret;
}

int CRtmpPush::Authorize()
{
    if(!m_sToken.empty() && !m_sDeviceSN.empty())
    {
        CheckPtr(m_pPushProtocol, ERROR_FAIL);
        //sv_trace("before m_pPushProtocol->Authorize(token)");
        int ret = m_pPushProtocol->Authorize(m_sToken, m_sDeviceSN);
        return ret;
    }
    
    return 0;
}

int CRtmpPush::ConnectApp()
{
    CheckPtr(m_pPushProtocol, ERROR_FAIL);
    int ret = m_pPushProtocol->ConnectApp(m_nstreamType);
    return ret;
}

int CRtmpPush::Publish()
{
    CheckPtr(m_pPushProtocol, ERROR_FAIL);
    int ret = m_pPushProtocol->Publish();
    return ret;
}

/*int CRtmpPush::OnSendPacket(int eMediaType, char* pData, int uSize, unsigned int uTimeStamp, CFrameBuffer* pframe)
{
    int ret = ERROR_OK;
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
    if(E_SV_MEDIA_TYPE_H264 == eMediaType)
    {
        ret = m_pPushProtocol->AVCPacket(pData, uSize, uTimeStamp, pframe);
    }

    else if(E_SV_MEDIA_TYPE_AAC == eMediaType)
    {
        ret = m_pPushProtocol->AACPacket(pData, uSize, uTimeStamp, pframe);
    }
    else
    {
        sv_error("Invalid media type:%d", eMediaType);
        return ERROR_FAIL;
    }

    return ret;
}

int CRtmpPush::OnSendPacket(int eMediaType, char* pInData, int nInSize, char* pOutData, unsigned int* pOutSize, unsigned int nTimeStamp)
{
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
    return ERROR_FAIL;
}

int CRtmpPush::OnSendMetadata(int eMetadataType, long wParam, long lParam, CFrameBuffer* pframe)
{
    CheckPtr(m_pPushProtocol, ERROR_NOT_INIT);
    int ret = ERROR_OK;//m_pPushProtocol->MetadataPacket((E_SV_METADATA_TYPE)eMetadataType, wParam, lParam, pframe);

    return ret;
}

int CRtmpPush::OnSendMetadata(int eMetadataType, long wParam, long lParam)
{
    return ERROR_FAIL;
}*/

int CRtmpPush::Unpublish()
{
    return CBasePush::Unpublish();
}

/*#ifdef ENABLE_GEN_URL_BY_TOKEN
std::string CRtmpPush::RequestUrl(const char* pappkey, const char* pappsecret, const char* pdeviceSN)//(std::string appKey, std::string appSecret, std::string deviceSN)
{
    return m_pPushProtocol->RequestToken(m_sGetTokenUrl.c_str(), pappkey, pappsecret, pdeviceSN);
}
#endif*/

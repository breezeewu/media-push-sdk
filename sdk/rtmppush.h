/****************************************************************************************************************
 * filename     rtmppush.h
 * describe     Sunvalley coud storage push SDK rtmp push class definition
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#ifndef _RTMP_PUSH_H_
#define _RTMP_PUSH_H_
#include "config.h"
#include "basepush.h"

//#include "rtmpprotocol.h"
class CRtmpProtocol;
class CRtmpPush:public CBasePush
{
public:
    CRtmpPush();
    ~CRtmpPush();

    virtual int Connect(const char* purl, const char* ptoken, IEventNotify* pevenhandle);
    virtual int Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun);
#ifdef ENABLE_GEN_URL_BY_TOKEN
    //virtual int Connect(const char* phost, const char* pappkey, const char* pappstreamname, long lcid, event_callback pcbfun);
#endif
    //virtual int Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun);

    //virtual int SendPacket(int eMediaType, char* pData,unsigned int nSize, unsigned int nTimeStamp);

    //virtual int SetMetadata(int eMsgType, long wParam, long lParam);

    //virtual int IsConnected();

    virtual int Close();

    int SetStreamType(int streamType);

protected:
    // connect routine
    virtual int ResolveUrl(const char* pUrl);

    virtual int ConnectServer();

    virtual int ShakeHand();

    virtual int Authorize();

    virtual int ConnectApp();

    virtual int Publish();

    //virtual int OnSendPacket(int eMediaType, char* pData, int nSize, unsigned int nTimeStamp, CFrameBuffer* pframe);

    //virtual int OnSendPacket(int eMediaType, char* pInData, int nInSize, char* pOutData, unsigned int* pOutSize, unsigned int nTimeStamp);

    //virtual int OnSendMetadata(int eMetadataType, long wParam, long lParam, CFrameBuffer* pframe);

    //virtual int OnSendMetadata(int eMetadataType, long wParam, long lParam);

    virtual int Unpublish();

/*#ifdef ENABLE_GEN_URL_BY_TOKEN
virtual std::string RequestUrl(const char* pappkey, const char* pappsecret, const char* pdeviceSN);
//std::string RequestUrlByToken(const char* ptoken);
#endif*/

private:

    int             m_nPort;
    std::string     m_sSchemaName;
    std::string     m_sHostName;
    std::string     m_sDeviceSN;
    std::string     m_sAppName;
    std::string     m_sAppStreamName;
    //CRtmpProtocol*     m_pRtmpProtocol;
};
#endif


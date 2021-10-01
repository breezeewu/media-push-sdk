/*************************************************************************************************************
 * filename     ipush.h
 * describe     Sunvalley cloud storage push sdk ipush interface define
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#ifndef _I_PUSH_H_
#define _I_PUSH_H_
#include "ipushmgr.h"
#include "isvpush.h"

// ipush interface
class ISVPush
{
public:
    virtual ~ISVPush(){}

    virtual int SetTokenFetctInterface(const char* ptoken_server_url) = 0;

    virtual int Connect(const char* purl, const char* token, IEventNotify* pevenhandle) = 0;
    
    virtual int Connect(const char* purl, const char* ptoken, long lcid, event_callback pcbfun) = 0;

    virtual int RecordSendPacket(const char* precord_url) = 0;

    virtual int ConnectByApp(auth_info* pai, const char* pappstreamname, const char* ptoken, long lcid, event_callback pcbfun) = 0;

    //virtual long ConnectByDeviceToken(const char* pdevice_token, event_callback pcbfun) = 0;

    virtual int SendEncryptPacket(int eMediaType, char* pData, unsigned int uSize, int keyframe, int64_t lltimestamp) = 0;

    virtual int SendPacket(int eMediaType, char* pData, unsigned int nSize, int64_t llTimeStamp, int keyframe, int encflag) = 0;

    virtual int SendMetadata(int eMsgType, long wParam, long lParam) = 0;

    virtual int IsConnected() = 0;

    virtual int Close() = 0;

    virtual int SetStreamType(int streamType) = 0;

};
#endif

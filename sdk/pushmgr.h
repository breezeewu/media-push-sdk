/****************************************************************************************************************
 * filename     pushmgr.h
 * describe     Sunvalley coud storage push manager class define
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ****************************************************************************************************************/
#ifndef _SV_PUSH_MGR_H_
#define _SV_PUSH_MGR_H_
#include "ipushmgr.h"
#include "autolock.h"
#include "ipush.h"
#include <map>

class CSVPushMgr:public ISVPushMgr
{
public:
    CSVPushMgr();
    ~CSVPushMgr();

    static ISVPushMgr* GetInstace();
    //static ISVPushMgr* CreateInst();

    static void DestroyInst(ISVPushMgr** ppPushMgr);

    virtual int InitLog(int nLogLevel, unsigned int  nLogFlag, const char* pLogPath);

    virtual int Set_Token_Fetct_Interface(const char* ptoken_server_url);

    virtual long Connect(const char* pUrl, const char* token, IEventNotify* pEvenHandle, int streamType = -1);
    
    virtual long Connect(const char* pUrl, const char* token, event_callback pcbFun, int streamType = -1);

    virtual long ConnectByApp(auth_info* pai, const char* pappstreamname, event_callback pcbfun);

    virtual long ConnectByDeviceToken(const char* pdevice_token, event_callback pcbfun);

    virtual long Live_Connect(const char* ptoken_url, const char* pdeviceSN, event_callback pcbfun);

    virtual int RecordSendPacket(long lcid, const char* precord_url);

    //virtual int SendEncryptPacket(long lcid, int eMediaType, char* pData, unsigned int uSize, int keyframe, int64_t lltimestamp);

    virtual int SendPacket(long lcid, int eMediaType, char* pData, unsigned int uSize, int64_t lltimestamp, int keyframe, int encflag);

    virtual int SendMetadata(long lcid, int emetadatatype, long wparam, long lparam);

    virtual int IsConnected(long lcid);

    virtual int Close(long lcid);

private:
    ISVPush* GetPushByConnectId(long lcid);

    long GenConnectId();
    
private:
    CCriSec                             m_CriSec;
    std::map<long, ISVPush*>            m_mPushList;
    long                                m_lConnectId;
    std::string                         m_sToken_Server_Url;
    static ISVPushMgr*                  m_pIPushMgr;
};
#endif


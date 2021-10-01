/***************************************************************************************************************
 * filename     ipushmgr.h
 * describe     Sunvalley cloud storage push sdk ISVPushMgr define
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#ifndef _I_PUSH_MANAGER_H_
#define _I_PUSH_MANAGER_H_
#include <stdint.h>
//#include "isvpush.h"
#include "irtmpprotocol.h"
// event notify interface
/*class IEventNotify
{
public:
virtual ~IEventNotify(){}*/
/***************************************************************************************************************
 * @describe    cloud push sdk callback funtion
 * @param       EventType : Callback event type
 * @param       wparam : Explain param, such as error code of message
 * @param       lparam : Message value, maybe ptr of special type, this value's type decides to message type
 * @return      Success return 0 , else return -1
 **************************************************************************************************************/
 //   virtual long OnEventNotify(int EventType, long wparam, long lparam) = 0;
//};

// sunvalley push manager interface
class ISVPushMgr
{
public:
    //ISVPushMgr();

    virtual ~ISVPushMgr(){}

    static ISVPushMgr* CreateInst();

    static void DestroyInst(ISVPushMgr* pinst);
/*****************************************************************************************************************
 * @describe    Init Log output mode
 * @param       nLogLevel : 日志输出等级
 * @param       nLogFlag : Log output mode flags, maybe as below:
 *              0: Not output any log
 *              1: Output console log
 *              2: Output file log
 *              3: Output console log and file log
 * @param       pLogPath : Log file path
 * @return      Success return 0 , else return -1
 * @remark      When no file output set, file path can be null.
 ****************************************************************************************************************/
    virtual int InitLog(int nLogLevel, unsigned int  nLogFlag, const char* pLogPath) = 0;

/*****************************************************************************************************************
 * @describe    set token fetch server url
 * @param       ptoken_server_url : token fetch server url in http format
 * @return      Success return 0 , else return -1
 ****************************************************************************************************************/
    virtual int Set_Token_Fetct_Interface(const char* ptoken_server_url) = 0;

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server and read to push media stream
 * @param       pUrl : Sunvalley streaming media server push url
 * @param       token : authorize token
 * @param       pEvenHandle : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
    virtual long Connect(const char* pUrl, const char* token, class IEventNotify* pEvenHandle, int streamType = -1) = 0;

    virtual long Connect(const char* pUrl, const char* token, int (*pcbFun)(long lCID, long eventType, long wparam, long lparam), int streamType = -1) = 0;

    virtual int RecordSendPacket(long lcid, const char* precord_url) = 0;

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server through app key, secret and device sn
 * @param       pai : authorize info
 * @param       pappstreamname:device sn
 * @param       pEvenHandle : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
    virtual long ConnectByApp(struct auth_info* pai, const char* pappstreamname, int (*pcbFun)(long lCID, long eventType, long wparam, long lparam)) = 0;

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server through device token
 * @param       pdevice_token : device token
 * @param       pcbfun : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
    virtual long ConnectByDeviceToken(const char* pdevice_token, int (*pcbFun)(long lCID, long eventType, long wparam, long lparam)) = 0;

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server through device token
 * @param1 ptoken_url : Sunvalley live streaming media server push url
 * @param       pdeviceSN : Sunvalley deivec SN
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
    virtual long Live_Connect(const char* ptoken_url, const char* pdeviceSN, int (*pcbFun)(long lCID, long eventType, long wparam, long lparam)) = 0;

/***************************************************************************************************************
 * @describe    Connect to Sunvalley cloud restore streaming media server by app key and app stream name
 * @param       pappkey : Sunvalley tenancy app key, 
 * @param       pappsecret : Sunvalley tenancy app secret
 * @param       pappstreamname : Sunvalley tenancy app stream name
 * @param       pEvenHandle : SDK callback message event handle, implement see IEventNotify
 * @return      Success return connect id , else return <= 0
 ***************************************************************************************************************/
    //virtual long ConnectByApp(const char* pappkey, const char* pappsecret, const char* pappstreamname, event_callback pcbfun) = 0;

/****************************************************************************************************************
 * @describe    Send media packet data thought sunvalley push sdk
 * @param       lcid  : Connected ID return by ISVPushMgr::Connect
 * @param       eMediaType : Media data type
 * @param       pData : Media data buffer
 * @param       uSize : Media data buffer size
 * @param       llTimeStamp : Pts of media data
 * @return Success return connect id , else return <= 0
 ****************************************************************************************************************/
    //virtual int SendEncryptPacket(long lcid, int eMediaType, char* pData, unsigned int uSize, int64_t lltimestamp) = 0;

/****************************************************************************************************************
 * @describe    Send media packet data thought sunvalley push sdk
 * @param       lcid  : Connected ID return by ISVPushMgr::Connect
 * @param       eMediaType : Media data type
 * @param       pData : Media data buffer
 * @param       uSize : Media data buffer size
 * @param       llTimeStamp : Pts of media data
 * @return Success return connect id , else return <= 0
 ****************************************************************************************************************/
    virtual int SendPacket(long lcid, int eMediaType, char* pData, unsigned int uSize, int64_t lltimestamp, int keyframe, int encflag) = 0;

/******************************************************************************************************************
 * @describe    Send metadata thought sunvalley push sdk
 * @param       lcid : Connected ID return by ISVPushMgr::Connect
 * @param       eMsgType : Message type specify enum E_SV_MESSAGE_TYPE
 * @param       wParam : Explain param, such as error code of message
 * @param       lParam : Message value, maybe ptr of special type, this value's type decides to message type
 * @return      Success : return 0 , else return -1
  ****************************************************************************************************************/
    virtual int SendMetadata(long lcid, int eMsgType, long wParam, long lParam) = 0;

/******************************************************************************************************************
 * @describe    Current rtmp is connected
 * @param       lcid : Connected ID return by ISVPushMgr::Connect
 * @return      Connected return 1 , else return 0
 *****************************************************************************************************************/
    virtual int IsConnected(long lcid) = 0;

/******************************************************************************************************************
 * @describe close push connection
 * @param lcid : connected ID return by ISVPushMgr::Connect
 * @return Success return 0 , else return -1
 *****************************************************************************************************************/
    virtual int Close(long lcid) = 0;

};
#endif

/****************************************************************************************************************
 * filename     ISVCloudPush.cpp
 * describe     Sunvalley coud storage push sdk api inplement
 * author       Created by dawson on 2019/04/25
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ****************************************************************************************************************/
#include <sys/time.h>
#include <stdio.h>
#include <map>
#include "config.h"
#include "isvpush.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <openssl/rsa.h>
//PRId64
//#include <inttypes.h>
int m_fd = 0;
log_ctx* g_plogctx = NULL;
//#define MOUNT_MEMORY_FOR_TEST 0
//#define HEART_BEAT_ONLY
//#define DISABLE_ALL_CPP
#ifndef DISABLE_ALL_CPP
#include "ipushmgr.h"
//#define PUSH_SDK_DEV_VERSION
ISVPushMgr*     g_pPushMgr = NULL;
#else
#endif
char* g_plarge_memory[100] = {0};

int g_nMemoryPoolSize =  1024*1024;
int g_nMaxConnectionCount = 4;

#ifdef NEW_PUSH_SDK_API
int SVPush_API_Initialize(int nMemory_Pool_Size, int nMax_Connection_Count)
{
#ifndef DISABLE_ALL_CPP
    if(NULL == g_pPushMgr)
    {
        g_pPushMgr = ISVPushMgr::CreateInst();//new CSVPushMgr();
    }
    if(nMemory_Pool_Size <= 100*1024 || nMax_Connection_Count <= 1)
    {
        sv_error("Invalid parameter, nMemory_Pool_Size:%d, nMax_Connection_Count:%d", nMemory_Pool_Size, nMax_Connection_Count);
        return ERROR_INVALID_PARAM;
    }
    g_nMemoryPoolSize = nMemory_Pool_Size;
    g_nMaxConnectionCount = nMax_Connection_Count;
    // init a larage memory, all all memory sdk use should be allocate by this memory manager
    //sv_mem_init(NULL, 1024*1024*2, 3000);
    sv_pool_mgr_init(g_nMemoryPoolSize * g_nMaxConnectionCount);

#ifdef MOUNT_MEMORY_FOR_TEST
    printf("SVPush_API_Initialize, malloc mount memory %dM!", MOUNT_MEMORY_FOR_TEST);
    for(int i = 0; i < MOUNT_MEMORY_FOR_TEST; i++)
    {
        g_plarge_memory[i] = new char[1024*1024];
        memset(g_plarge_memory[i], 0, 1024*1024);
        printf("g_plarge_memory[%d]:%p\n", i, g_plarge_memory[i]);
    }
#endif
#else
    g_nMemoryPoolSize = nMemory_Pool_Size;
    g_nMaxConnectionCount = nMax_Connection_Count;
    for(int i = 0; i < MOUNT_MEMORY_FOR_TEST; i++)
    {
		g_plarge_memory[i] = new char[1024*1024];
        memset(g_plarge_memory[i], 0, 1024*1024);
		printf("g_plarge_memory[i:%d] = new char[g_nMaxConnectionCount:%d]", i, 1024*1024);
    }

#endif
	
    return ERROR_OK;
}
#else
int SVPush_API_Initialize()
{
    lbcheck_pointer(g_pPushMgr, -1);
    #ifndef DISABLE_ALL_CPP
    if(NULL == g_pPushMgr)
    {
        g_pPushMgr = ISVPushMgr::CreateInst();//new CSVPushMgr();
    }
    sv_pool_mgr_init(g_nMemoryPoolSize* g_nMaxConnectionCount);
#endif
	
    return ERROR_OK;
}
#endif
int SVPush_API_Init_log(E_SV_LOG_LEVEL eLogLevel, unsigned int  nLogFlag, const char* pLogPath)
{
    //printf("(eLogLevel:%d, nLogFlag:%d, pLogPath:%s)", eLogLevel, nLogFlag, pLogPath);
    //eLogLevel = E_LOG_LEVEL_MAIN;
    sv_trace("(eLogLevel:%d, nLogFlag:%d, pLogPath:%s)", eLogLevel, nLogFlag, pLogPath);
    lbcheck_pointer(g_pPushMgr, -1);
#ifndef DISABLE_ALL_CPP
    if(NULL == g_pPushMgr)
    {
        sv_error("NULL == g_pPushMgr:%p, sdk not initialize!", g_pPushMgr);
        return -1;
    }
    g_pPushMgr->InitLog(eLogLevel, nLogFlag, pLogPath);
#endif
    //printf("SVPush_API_Init_log end\n");
    //ISVPushMgr::GetInstance()->InitLog(eLogLevel, nLogFlag, pLogPath);
    return ERROR_OK;
}

int SVPush_API_Set_Token_Server_Interface(const char* ptoken_server_url)
{
    sv_info("ptoken_server_url:%s)", ptoken_server_url);
    lbcheck_pointer(g_pPushMgr, -1);
#ifndef DISABLE_ALL_CPP
    if(NULL == g_pPushMgr)
    {
        sv_error("NULL == g_pPushMgr:%p, sdk not initialize!", g_pPushMgr);
    }
    int ret = g_pPushMgr->Set_Token_Fetct_Interface(ptoken_server_url);
    //sv_trace("ret:%d = g_pPushMgr->Set_Token_Fetct_Interface(ptoken_server_url:%s)", ret, ptoken_server_url);
    return ret;
#else
    return 0;
#endif
}

int SVPush_API_UnInitialize()
{
    sv_trace("UnInitialize");
    lbcheck_pointer(g_pPushMgr, -1);
#ifndef DISABLE_ALL_CPP
    if(g_pPushMgr)
    {
        ISVPushMgr* ppushmgr = g_pPushMgr;
        g_pPushMgr = NULL;
        ISVPushMgr::DestroyInst(ppushmgr);
        //g_pPushMgr = NULL;
        sv_trace("ISVPushMgr::DestroyInst(g_pPushMgr:%p)\n", ppushmgr);
    }
    
    // destroy memory manager
    //CMemoryMgr::destroy_instance();
    sv_pool_mgr_deinit();
#ifdef MOUNT_MEMORY_FOR_TEST
    printf("free mount memory %dM!", MOUNT_MEMORY_FOR_TEST);
    for(int i = 0; i < MOUNT_MEMORY_FOR_TEST; i++)
    {
        if(g_plarge_memory[i])
        {
            delete[] g_plarge_memory[i];
            g_plarge_memory[i] = NULL;
        }
        
    }
#endif
#else
    for(int i = 0; i < MOUNT_MEMORY_FOR_TEST; i++)
    {
        if(g_plarge_memory[i])
        {
            delete[] g_plarge_memory[i];
            g_plarge_memory[i] = NULL;
        }
    }
#endif
    CRYPTO_cleanup_all_ex_data();
    //sv_trace("after CRYPTO_cleanup_all_ex_data\n");
    close_log_contex(&g_plogctx);
    return 0;
}


long SVPush_API_Connect(const char* purl, const char* token, event_callback pcbfun)
{
    lbcheck_pointer(g_pPushMgr, -1);
    sv_info("(purl:%s, token:%s, pcbfun:%p)", purl, token, pcbfun);
    //printf("conencet end\n");
#ifndef DISABLE_ALL_CPP
    if(NULL == g_pPushMgr)
    {
        sv_error("NULL == g_pPushMgr:%p, sdk not initialize!", g_pPushMgr);
        return -1;
    }
    unsigned long begin = GetSysTime();
    long lcid = g_pPushMgr->Connect(purl, token, pcbfun);
    sv_trace("lcid:%ld = g_pPushMgr->Connect(purl:%s, token, pcbfun), ver:%d.%d.%d.%d, spend time(ms):%lu\n", lcid, purl, SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY, GetSysTime() - begin);
#else
    long lcid = 100;
#endif
    return lcid;
}

long SVPush_API_Connect_By_DeviceSN(auth_info* pai, const char* pdeviceSN, event_callback pcbfun)
//long SVPush_API_Connect_By_App(const char* pappkey, const char* pappsecret, const char* pappstreamname, event_callback pcbfun)
{
    lbcheck_pointer(g_pPushMgr, -1);
    sv_info("pai:%p, pdeviceSN:%s, pcbfun:%p\n", pai, pdeviceSN, pcbfun);
#ifndef DISABLE_ALL_CPP
    if(!pdeviceSN)
    {
        sv_error("Invalid parameter, pai:%s, pdeviceSN:%s\n", pai, pdeviceSN);
        return -1;
    }
    if(NULL == g_pPushMgr)
    {
        sv_error("NULL == g_pPushMgr:%p, sdk not initialize!", g_pPushMgr);
        return -1;
    }
    unsigned long begin = GetSysTime();
#ifdef PUSH_SDK_DEV_VERSION
    const char* purl = "rtmp://52.52.129.160:19350/live/test";
    long lcid = g_pPushMgr->Connect(purl, "nocheck", pcbfun);
#else
    sv_debug("before g_pPushMgr->ConnectByApp\n");
    long lcid = g_pPushMgr->ConnectByApp(pai, pdeviceSN, pcbfun);
    sv_trace("lcid:%ld = ConnectByApp(pai:%p, pdeviceSN:%s, pcbfun:%p); ver:%d.%d.%d.%d, spend time(ms):%lu", lcid, pai, pdeviceSN, pcbfun, SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY, GetSysTime() - begin);
#endif
#else
    long lcid = 100;
#endif
    return lcid;
}


long SVPush_API_Connect_By_Device_Token(const char* pdevice_token, event_callback pcbfun)
{
    lbcheck_pointer(g_pPushMgr, -1);
#ifndef DISABLE_ALL_CPP
    if(!pdevice_token)
    {
        sv_error("Invalid parameter, pdevice_token:%s\n", pdevice_token);
        return -1;
    }
    if(NULL == g_pPushMgr)
    {
        sv_error("NULL == g_pPushMgr:%p, sdk not initialize!", g_pPushMgr);
        return -1;
    }
    unsigned long begin = GetSysTime();
#ifdef PUSH_SDK_DEV_VERSION
    const char* purl = "rtmp://52.52.129.160:19350/live/test";
    long lcid = g_pPushMgr->Connect(purl, "nocheck", pcbfun);
#else
    long lcid = g_pPushMgr->ConnectByDeviceToken(pdevice_token, pcbfun);
    sv_trace("lcid:%ld = ConnectByDeviceToken(pdevice_token:%s, pcbfun:%p); ver:%d.%d.%d.%d, spend time(ms):%lu", lcid, pdevice_token, pcbfun, SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY, GetSysTime() - begin);
#endif
#else
    long lcid = 100;
#endif
    return lcid;
}

long SVPush_API_Live_Connect(const char* ptoken_url, const char* pdeviceSN, event_callback pcbfun)
//long SVPush_API_Live_Connect(const char* ptoken_url, const char* pdeviceSN, event_callback pcbfun)
{
    lbcheck_pointer(g_pPushMgr, -1);
    if(NULL == ptoken_url/*|| NULL == pdeviceSN*/)
    {
        sv_error("Invalid parameter, ptoken_url:%s, pdeviceSN:%s\n", ptoken_url, pdeviceSN, pcbfun);
        return -1;
    }
    unsigned long begin = GetSysTime();// - begin
    long lcid = g_pPushMgr->Live_Connect(ptoken_url, pdeviceSN, pcbfun);
    sv_trace("lcid:%ld = SVPush_API_Live_Connect(ptoken_url:%s, pdeviceSN:%s, pcbfun:%p), ver:%d.%d.%d.%d, spend time:%lu\n", lcid, ptoken_url, pdeviceSN, pcbfun, SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY, GetSysTime() - begin);
    return lcid;
}

int SVPush_API_Record_Send_Packet(long lcid, const char* precord_url)
{
    lbcheck_pointer(g_pPushMgr, -1);

    return g_pPushMgr->RecordSendPacket(lcid, precord_url);
}

int SVPush_API_Send_Packet(long lcid, E_SV_MEDIA_TYPE emediatype, char* pdata, unsigned int usize, int64_t lltimestamp, int keyframe, int encflag)
{
    lbcheck_pointer(g_pPushMgr, -1);
#ifdef HEART_BEAT_ONLY
    return 0;
#endif
#ifdef WRITE_ORG_DATA_ONLY
    if(0  == emediatype)
    {
        static FILE* pavcfile = NULL;
        simple_write_file(&pavcfile, get_log_path(), (char*)"/../data/org_avc.data", pdata, usize);
        return 0;
    }
    else
    {
        static FILE* paacfile = NULL;
        simple_write_file(&paacfile, get_log_path(), (char*)"/../data/org_aac.data", pdata, usize);
        return 0;
    }
#endif
    //sv_memory(LOG_LEVEL_INFO, pdata, 16);
#ifndef DISABLE_ALL_CPP
    int ret = g_pPushMgr->SendPacket(lcid, emediatype,  pdata, usize, lltimestamp, keyframe, encflag);
    sv_verb("ret:%d = g_pPushMgr->SendPacket(lcid:%ld, mt:%d, size:%u, pts:%" PRId64 ", keyframe:%d, encflag:%d)", ret, lcid, emediatype,  usize, lltimestamp, keyframe, encflag);
    //sv_memory(3, pdata, 16);
    CheckResult(ret);
#else
    int ret = ERROR_OK;
#endif
    
	//PRINTF_DUR_TIME(20);
    return ret;
}

int SVPush_API_Send_Metadata(long lcid,E_SV_METADATA_TYPE emetadatatype, long wparam, long lparam)
{
    lbcheck_pointer(g_pPushMgr, -1);
#ifdef HEART_BEAT_ONLY
    return 0;
#endif
    //sv_trace("(lcid:%ld, emetadatatype:%d,  wparam:%ld, lparam:%ld)", lcid, emetadatatype, wparam, lparam);
#ifndef DISABLE_ALL_CPP
    int ret = g_pPushMgr->SendMetadata(lcid, emetadatatype, wparam, lparam);
    CheckResult(ret);
#else
    int ret = ERROR_OK;
#endif
    return ret;
}

int SVPush_API_Is_Connected(long lcid)
{
    lbcheck_pointer(g_pPushMgr, -1);
#ifndef DISABLE_ALL_CPP
    int ret = g_pPushMgr->IsConnected(lcid);
#else
    int ret = ERROR_OK;
#endif
    //sv_trace("(lcid:%ld) ret:%d", lcid, ret);
    return ret;
}

int SVPush_API_Close(long lcid)
{
    lbcheck_pointer(g_pPushMgr, -1);
    unsigned long begin = GetSysTime();
    sv_trace("SVPush_API_Close(lcid:%ld) begin", lcid);
#ifndef DISABLE_ALL_CPP
    int ret = g_pPushMgr->Close(lcid);
    sv_trace("(lcid:%ld) end ret:%d, close time:%lu ms", lcid, ret, GetSysTime() - begin);
#else
    int ret = ERROR_OK;
#endif
    return ret;
}

int SVPush_API_Version(char* szVersionXml, int len)
{
    if (NULL == szVersionXml) {
        sv_error("szVersionXml Empty!\n");
        return -1;
    }
    char VersionId[50] = {0};
    sprintf(VersionId,"%d.%d.%d.%d", SDK_VERSION_MAYJOR, SDK_VERSION_MINOR, SDK_VERSION_MACRO, SDK_VERSION_TINY);

    char Owner[256] = {"Sunvellay All Rights Reserved"};
    char fmt[512] = {0};
    memset(szVersionXml, 0, len);
    sprintf(fmt,"{\"Owner\":\"%s\" ,\"VersionId\":\"%s\", \"VersionDate\":\"%s\", \"VersionTime\":\"%s\"}", Owner, VersionId, __DATE__, __TIME__);
    sv_trace(fmt);
    if((size_t)len < strlen(fmt))
    {
        memset(szVersionXml, 0, len);
        memcpy(szVersionXml, fmt, len -1);
        return 1;
    }
    strcpy(szVersionXml, fmt);
    return 0;
}

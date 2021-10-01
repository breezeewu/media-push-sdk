/***************************************************************************************************************
 * filename     framequeue.h
 * describe     queue buffer of media frame 
 * author       Created by dawson on 2019/04/19
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 **************************************************************************************************************/
#ifndef _FRAME_QUEUE_H_
#define _FRAME_QUEUE_H_
#include <stdint.h>
#include <stdio.h>
#include "autolock.h"
#include <list>
#include <mutex>
#include "sv_log.h"
#include <sys/types.h>
#define ERROR_FRAME_BUFFER_SUCCESS       0
#define ERROR_FRAME_BUFFER_ERROR_PARAM  -1
#define ERROR_FRAME_BUFFER_QUEUE_FULL   -2
#if 1
#define frame_malloc(owner, size)		sv_malloc(owner, size)
#define frame_free(owner, pdata)		sv_free(owner, pdata)
#else
#define frame_malloc(size) new char[size];
#define frame_free(pdata) if(pdata) { delete[] pdata; pdata = NULL; }
#endif
#define KEY_FRAME_FLAG          0x1
#define AES_ENCRYPT_FLAG        0x02
class CFrameBuffer
{
public:
    char*       m_pdata;
    int         m_ncurpos;
    int         m_nsize;
    int         m_nmsgtype;
    int         m_nmsgid;
    int64_t     m_llpts;
    int64_t     m_lldts;
    int         m_nflag;
    bool        m_bdiscontinue;
    void*       m_powner;
    SUN_MUTEX   m_mutex;

public:
    CFrameBuffer(void* powner)
    {
        m_pdata         = NULL;
        m_nsize         = 0;
        m_ncurpos       = 0;
        m_nmsgtype      = -1;
        m_nmsgid        = -1;
        m_llpts         = -1;
        m_lldts         = -1;
        m_nflag         = 0;
        m_bdiscontinue  = false;
        m_powner        = powner;
    }

    ~CFrameBuffer()
    {
        SUN_AUTO_LOCK lock(m_mutex);
        if(m_pdata)
        {
            //delete[] m_pdata;
            frame_free(m_powner, m_pdata);
            m_pdata = NULL;
            m_nsize = 0;
        }
    }

    int ReAllocBuffer(int size)
    {
        SUN_AUTO_LOCK lock(m_mutex);

        if(size <= m_nsize)
        {
            return 0;
        }
        char* pnewbuf = frame_malloc(m_powner, size);
         if(!pnewbuf)
        {
            printf("Fatal error, out of memory, allocate %d bytes failed, pnewbuf:%p", size, pnewbuf);
            return -1;
        }
        memset(pnewbuf, 0, size);
        memcpy(pnewbuf, m_pdata, m_ncurpos);
        if(m_pdata)
        {
            //delete[] m_pdata;
            frame_free(m_powner, m_pdata);
            m_pdata = NULL;
            m_nsize = 0;
        }
        m_pdata = pnewbuf;
        m_nsize = size;
        return 0;
    }

    int Clone(CFrameBuffer* pdstframe)
    {
        SUN_AUTO_LOCK lock(m_mutex);
        if(NULL == pdstframe)
        {
            sv_trace("clone pdstframe is null");
            return -1;
        }

        pdstframe->Reset();
        pdstframe->ReAllocBuffer(m_nsize);
        pdstframe->m_ncurpos = m_ncurpos;
        memcpy(pdstframe->m_pdata, m_pdata, m_nsize);
        pdstframe->m_nmsgtype = m_nmsgtype;
        pdstframe->m_powner = m_powner;
        pdstframe->m_bdiscontinue = m_bdiscontinue;
        pdstframe->m_llpts = m_llpts;
        pdstframe->m_lldts = m_lldts;
        pdstframe->m_nmsgid = m_nmsgid;
        m_nflag  = -1;

        return 0;
    }
    void Reset()
    {
        SUN_AUTO_LOCK lock(m_mutex);

        if(m_pdata)
        {
            //delete[] m_pdata;
            frame_free(m_powner, m_pdata);
            m_pdata = NULL;
        }
        m_nsize         = 0;
        m_ncurpos       = 0;
        m_nmsgtype      = -1;
        m_nmsgid        = -1;
        m_llpts         = -1;
        m_lldts         = -1;
        m_nflag         = 0;
        m_bdiscontinue  = false;
    }
};

class CFrameQueue
{
public:
    CFrameQueue()
    {
	    m_nmax_frame_count      = 1024;
    	m_nmax_frame_size       = 1024*1024;
    	m_ncur_frame_size       = 0;
        m_nfree_frame_count     = 0;
        m_ntotal_frame_count    = 0;
        m_powner                = NULL;
    }
    ~CFrameQueue()
    {
	    deinit_queue();
    }

    void init(void* powner)
    {
        SUN_AUTO_LOCK lock(m_mutex);
        m_powner = powner;
    }

    int init_queue(int maxframecnt = 1024, int maxframesize = 1024*1024)
    {
	    //sv_trace("init_queue begin");
        SUN_AUTO_LOCK lock(m_mutex);
        m_nmax_frame_count = maxframecnt;
        m_nmax_frame_size = maxframesize;
        m_ncur_frame_size = 0;
	    //sv_trace("init_queue end");
        return ERROR_FRAME_BUFFER_SUCCESS;
    }

    void deinit_queue()
    {
        SUN_AUTO_LOCK lock(m_mutex);
        //sv_trace("m_frame_list.size():%d, m_free_frame_list.size():%d, m_ncur_frame_size:%d, m_nfree_frame_count:%d, m_ntotal_frame_count:%d", (int)m_frame_list.size(), (int)m_free_frame_list.size(), m_ncur_frame_size, m_nfree_frame_count, m_ntotal_frame_count);
        while(m_frame_list.size() > 0)
        {
            CFrameBuffer* pframe = pop_frame();
            if(pframe)
            {
                delete pframe;
                pframe = NULL;
            }
        }

        while(m_free_frame_list.size() > 0)
        {
            CFrameBuffer* pframe = m_free_frame_list.front();
            m_free_frame_list.pop_front();
            if(pframe)
            {
                delete pframe;
                pframe = NULL;
            }
        }
        m_ntotal_frame_count = 0;
    }

    int push_frame(CFrameBuffer* pframe)
    {
        SUN_AUTO_LOCK lock(m_mutex);
        if((int)m_frame_list.size() >= m_nmax_frame_count || m_ncur_frame_size + pframe->m_nsize > m_nmax_frame_size)
        {
            sv_error("push_frame m_frame_list.size():%d >= m_nmax_frame_count:%d || m_ncur_frame_size:%d + pframe->m_nsize:%d > m_nmax_frame_size:%d, failed\n", m_frame_list.size(), m_nmax_frame_count, m_ncur_frame_size, pframe->m_nsize, m_nmax_frame_size);
            return ERROR_FRAME_BUFFER_QUEUE_FULL;
        }
        
        m_ncur_frame_size += pframe->m_nsize;
        m_frame_list.push_front(pframe);
	//sv_trace("m_frame_list.size():%d, m_ncur_frame_size:%d, pframe->m_nsize:%d, pframe->m_ncurpos:%d", size(), m_ncur_frame_size, pframe->m_nsize, pframe->m_ncurpos);
        return ERROR_FRAME_BUFFER_SUCCESS;
    }

    CFrameBuffer* pop_frame()
    {
        //sv_trace("begin");
        SUN_AUTO_LOCK lock(m_mutex);
        CFrameBuffer* pframe = NULL;
	//sv_trace("m_ncur_frame_size:%d", m_ncur_frame_size);
        if(m_frame_list.size() > 0)
        {
            pframe = m_frame_list.back();
            m_frame_list.pop_back();
	    //sv_trace("m_ncur_frame_size:%d -= pframe->m_nsize:%d", m_ncur_frame_size, pframe->m_nsize);
            m_ncur_frame_size -= pframe->m_nsize;
            if(0 == m_frame_list.size() && m_ncur_frame_size != 0)
            {
                sv_error("0 == m_frame_list.size():%d, m_ncur_frame_size:%d", m_frame_list.size(), m_ncur_frame_size);
                assert(0);
            }
        }
        //sv_trace("end, pframe:%p", pframe);
        return pframe;
    }
    
    int push_frame(int msg_type, char* pframe, int len, int64_t pts, int64_t dts, bool keyframe, bool discontinue)
    {
        CFrameBuffer* ptmp = NULL;
        if(!pframe || len <= 0)
        {
            sv_error("!pframe:%p || len:%d <= 0 failed!", pframe, len);
            return ERROR_FRAME_BUFFER_ERROR_PARAM;
        }
        SUN_AUTO_LOCK lock(m_mutex);
        //sv_trace("(pframe:%p, len:%d, pts:%ld, dts:%ld, keyframe:%d, discontinue:%d)", pframe, len, (long)pts, (long)dts, (int)keyframe, (int)discontinue);
        {
            ptmp = get_frame_buffer();//new CFrameBuffer();
            CheckPtr(ptmp, ERROR_OUT_OF_MEMORY);
            ptmp->ReAllocBuffer(len);//ptmp->m_pdata = sv_malloc(len);//new char[len];
            memcpy(ptmp->m_pdata, pframe, len);
            ptmp->m_nsize = len;
            ptmp->m_nmsgtype = msg_type;
            ptmp->m_llpts = pts;
            ptmp->m_lldts = dts;
            if(keyframe)
            {
                ptmp->m_nflag |= KEY_FRAME_FLAG;
            }
            //ptmp->m_bkeyframe = keyframe;
            ptmp->m_bdiscontinue =  discontinue;
        }
        int ret = push_frame(ptmp);
        if(ERROR_FRAME_BUFFER_SUCCESS != ret && ptmp)
        {
            delete ptmp;
            ptmp = NULL;
        }
        return ret;
    }

    int pop_frame(char* pframe, int* plen, int64_t* ppts, int64_t* pdts, bool* pkeyframe, bool* pdiscontinue)
    {
        SUN_AUTO_LOCK lock(m_mutex);
        CFrameBuffer* ptmp = pop_frame();
        if(ptmp && pframe && plen)
        {
            //SUN_AUTO_LOCK lock(m_mutex);
            if(*plen < ptmp->m_nsize)
            {
                sv_error("*plen:%d < pframe->m_nsize:%d, memory not enough to copy", *plen, ptmp->m_nsize);
                assert(0);
                return ERROR_FRAME_BUFFER_ERROR_PARAM;
            }

            *plen = ptmp->m_nsize;
            memcpy(pframe, ptmp->m_pdata, ptmp->m_nsize);
            if(ppts)
            {
                *ppts = ptmp->m_llpts;
            }

            if(pdts)
            {
                *pdts = ptmp->m_lldts;
            }

            if(pkeyframe)
            {
                *pkeyframe = ptmp->m_nflag & KEY_FRAME_FLAG;
            }

            if(pdiscontinue)
            {
                *pdiscontinue = ptmp->m_bdiscontinue;
            }
            delete ptmp;
            ptmp = NULL;
            //sv_trace("end");
            return ERROR_FRAME_BUFFER_SUCCESS;
        }
        sv_error("pframe:%p, plen:%p, ppts:%p, pdts:%p", pframe, plen, ppts, pdts);
        return -1;
    }

    int size()
    {
        SUN_AUTO_LOCK lock(m_mutex);
        return m_frame_list.size();
    }

    bool is_full()
    {
        SUN_AUTO_LOCK lock(m_mutex);

        if((int)m_frame_list.size() >= m_nmax_frame_count || m_ncur_frame_size >= m_nmax_frame_size)
        {
	        sv_warn("m_frame_list.size():%d >= m_nmax_frame_count:%d || m_ncur_frame_size:%d >= m_nmax_frame_size:%d", m_frame_list.size(), m_nmax_frame_count, m_ncur_frame_size, m_nmax_frame_size);
            return true;
        }
	    return false;
    }

    int get_cur_frame_size()
    {
        SUN_AUTO_LOCK lock(m_mutex);
	    return m_ncur_frame_size;
    }

    CFrameBuffer* get_frame_buffer()
    {
        //return new CFrameBuffer();
        //sv_trace("get_frame_buffer m_ntotal_frame_count:%d != m_free_frame_list.size():%d + m_frame_list.size():%d", m_ntotal_frame_count, m_free_frame_list.size(), m_frame_list.size());
        SUN_AUTO_LOCK lock(m_mutex);
        if(m_frame_list.size() + m_free_frame_list.size() > (std::size_t)m_nmax_frame_count)
        {
            sv_error("frame buffer overflow, m_frame_list.size():%" PRId64 " + m_free_frame_list.size():%" PRId64 " > m_nmax_frame_count:%d, m_ntotal_frame_count:%d", m_frame_list.size(), m_free_frame_list.size(), m_nmax_frame_count, m_ntotal_frame_count);
            assert(0);
            return NULL;
        }
        if(m_ntotal_frame_count >= m_nmax_frame_count)
        {
            sv_error("fatal error, m_ntotal_frame_count:%d >= m_nmax_frame_count:%d", m_ntotal_frame_count, m_nmax_frame_count);
            assert(0);
        }
        if(m_free_frame_list.size() <= 0)
        {
            m_ntotal_frame_count++;
            return new CFrameBuffer(m_powner);
        }
        else
        {
            CFrameBuffer* pframebuf = m_free_frame_list.front();
            m_free_frame_list.pop_front();
            return pframebuf;
        }
    }

    void free_frame_buffer(CFrameBuffer** ppframebuf)
    {
        SUN_AUTO_LOCK lock(m_mutex);
        if(ppframebuf && *ppframebuf)
        { 
            CFrameBuffer* pframebuf = *ppframebuf;
            *ppframebuf = NULL;
            //delete pframebuf;
            //return ;
           
            pframebuf->Reset();
            m_free_frame_list.push_back(pframebuf);
            //sv_trace("m_ntotal_frame_count:%d != m_free_frame_list.size():%d + m_frame_list.size():%d", m_ntotal_frame_count, m_free_frame_list.size(), m_frame_list.size());
        }
    }

private:

    int         m_nmax_frame_count;
    int         m_nfree_frame_count;
    int         m_nmax_frame_size;
    int         m_ncur_frame_size;
    int         m_ntotal_frame_count;
    void*       m_powner;
    std::list<CFrameBuffer*>    m_frame_list;
    std::list<CFrameBuffer*>    m_free_frame_list;
    SUN_MUTEX   m_mutex;
};

// add end
#endif /* AesCrypt_h */

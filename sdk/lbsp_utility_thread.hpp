/****************************************************************************************************************
 * filename     sv_thread.h
 * describe     thread can be simple create by inherit this class
 * author       Created by dawson on 2019/04/19
 * Copyright    ©2007 - 2029 Sunvally. All Rights Reserved.
 ***************************************************************************************************************/
#pragma once
#include <stdint.h>
#include <stdio.h>
#include <pthread.h>
#include "sv_log.h"
#define THREAD_RETURN void*
typedef THREAD_RETURN (*routine_pt)(void*);
#ifndef sv_trace
#define sv_trace printf
#endif

namespace lbsp_util
{
class ILazyThread
{
public:
    virtual ~ILazyThread(){}


};

class CLazyThread
{
protected:
    int		    	m_brun;
    int         	m_npriority;
    int         	m_bsystem;
    int             m_ninterval;
	pthread_t	    m_tid;

public:
    CLazyThread()
    {
        m_npriority         = 0;
        m_brun              = 0;
        m_tid               = 0;
        m_bsystem           = 0;
    }

    ~CLazyThread()
    {
        stop();
    }

    int init_attribute(int priority = 0, int bsystem = 0)
    {
        m_npriority = priority;
        m_bsystem = bsystem;

        return 0;
    }



    virtual int start(int interval)
    {
        int ret = 0;
        if(m_brun)
        {
            return ret;
        }

        if(0 == m_tid)
        {
            ret = create();
        }
        m_ninterval = interval;

        // wait for thread runing, wait 50ms at most, for stop no block
        for(int i = 0; i < 50; i++)
        {
            if(m_brun)
            {
                break;
            }
            else
            {
                lazy_sleep(1);
            }
        };

        return ret;
    }

    virtual int on_thread_start()
    {
        return 0;
    }

    virtual int on_cycle() = 0;

    virtual int thread_proc()
    {
        int ret = 0;
        sv_trace("thread_proc begin, m_brun:%d, m_tid:%u\n", m_brun, m_tid);
        ret = on_thread_start();
        if(ret < 0)
        {
            return ret;
        }

        while(m_brun)
        {
            if(m_ninterval > 0)
            {
               lazy_sleep(m_ninterval);
            }
            ret = on_cycle();
            if(ret < 0)
            {
                break;
            }
        }

        ret = on_thread_stop();
        sv_trace("thread_proc end, m_brun:%d, ret:%d, m_tid:%u\n", m_brun, ret, m_tid);
        return ret;
    }

    virtual int on_thread_stop()
    {
        return 0;
    }

    virtual int stop()
    { 
        
        if(m_brun)
        {
            m_brun = 0;
            //pthread_detach(m_tid);
            sv_trace("pthread_join(m_tid:%u, NULL)\n", m_tid);
            pthread_join(m_tid, NULL);
        }

        memset(&m_tid, 0, sizeof(m_tid));
        return 0;
    }

    bool IsAlive()
    {
        return m_brun;
    }

    static void lazy_sleep(int ms)
    {
#ifdef _WIN32
        Sleep(ms);
#else
        usleep(ms*1000);
#endif
    }

    static unsigned long get_sys_time()
    {
        struct timeval tv;
        gettimeofday(&tv, NULL);
		unsigned long untsamp = tv.tv_sec * 1000 + tv.tv_usec / 1000;
    }
private:
    int create()
    {
        int res = 0;
        if(0 == m_tid)
        {
            pthread_attr_t tattr;
            res = pthread_attr_init(&tattr);
            
            if (0 != res) {
                /* error */
                //CloseDebug("pthread_attr_init error\n");
                printf("res:%d = pthread_attr_init(&tattr) faild\n", res);
                return res;
            }
            res = pthread_attr_setstacksize(&tattr, 64*1024);
            if (0 != res) {
                /* error */
                //CloseDebug("pthread_attr_init error\n");
                printf("res:%d = pthread_attr_setstacksize(&tattr, 64*1024) faild\n", res);
                return res;
            }
            // 设置为内核级的线程，以获取较高的响应速度
            if(m_bsystem)
            {
                res = pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM);
                //pthread_attr_setdetachstate(&tattr, PTHREAD_CREATE_DETACHED);
                printf("res:%d = pthread_attr_setscope(&tattr, PTHREAD_SCOPE_SYSTEM)\n", res);
            }

            res = pthread_create(&m_tid, &tattr, initial_thread_proc, this);
            sv_trace("res:%d = pthread_create(&m_tid:%u, &tattr, initial_thread_proc, this)\n", res, m_tid);
            if(0 != res)
            {
                printf("res:%d = pthread_create(&m_tid, &tattr, InitialThreadProc, this) failed\n", res);
                return res;
            }

            res = pthread_attr_destroy(&tattr);
            if(0 != res)
            {
                printf("res:%d = pthread_attr_destroy(&tattr) failed\n", res);
                return res;
            }
            return res;
        }
        else
        {
            printf("thread already had been create!, m_tid:%lu\n", m_tid);
        }

        return res;
    }

    static THREAD_RETURN initial_thread_proc(void* arg)
    {
        int ret = NULL;
        CLazyThread* pthread = static_cast<CLazyThread*>(arg);
        if(pthread)
        {
            pthread->m_brun = 1;
            ret = pthread->thread_proc();
            if(pthread->m_brun)
            {
                pthread_detach(pthread->m_tid);
            }
        }
        pthread->m_brun = 0;
        return NULL;
    }

};

template<class T>
class CLazyThreadEx:public CLazyThread
{
public:
	CLazyThreadEx()
	{
		m_powner		= NULL;
		m_pcallback_proc	= NULL;
	}

	~CLazyThreadEx()
	{
	}

	typedef int (T::*callback)();

	virtual int init_class_func(T* powner, callback class_func)
	{
		m_powner = powner;
		m_pcallback_proc = class_func;

		return 0;
	}

	virtual THREAD_RETURN thread_proc()
	{
        int ret = -1;
		sv_trace("CLazyThreadEx::ThreadProc begin, m_brun:%d, m_tid:%u\n", m_brun, m_tid);
		if(m_pcallback_proc && m_powner)
		{
			ret = (m_powner->*m_pcallback_proc)();
		}
        sv_trace("CLazyThreadEx::ThreadProc end, ret:%d, m_brun:%d, m_tid:%u\n", ret, m_brun, m_tid);
		return NULL;
	}

protected:
	callback		m_pcallback_proc;
	T*				m_powner;
};
};

/**
 * Thread.cpp
 * This file is part of the YATE Project http://YATE.null.ro
 *
 * Yet Another Telephony Engine - a fully featured software PBX and IVR
 * Copyright (C) 2004-2006 Null Team
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "stdafx.h"
#include "Yate/yateclass.h"

#include <string.h>

#ifdef _WINDOWS
#include <process.h>
typedef unsigned long HTHREAD;
#else
#include <pthread.h>
typedef pthread_t HTHREAD;
#endif

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 16384
#endif

namespace TelEngine {

class ThreadPrivate : public GenObject {
    friend class Thread;
public:
    ThreadPrivate(Thread* t,const char* name);
    ~ThreadPrivate();
    void run();
    bool cancel(bool hard);
    void cleanup();
    void destroy();
    void pubdestroy();
    static ThreadPrivate* create(Thread* t,const char* name,Thread::Priority prio);
    static void killall();
    static ThreadPrivate* current();
    Thread* m_thread;
    HTHREAD thread;
    bool m_running;
    bool m_started;
    bool m_updest;
    bool m_cancel;
    const char* m_name;
#ifdef _WINDOWS
    static void startFunc(void* arg);
#else
    static void* startFunc(void* arg);
#endif
    static void cleanupFunc(void* arg);
    static void destroyFunc(void* arg);
};

};

using namespace TelEngine;

#define SOFT_KILLS 5
#define HARD_KILLS 5
#define KILL_WAIT 32

#ifdef _WINDOWS
#define TLS_MAGIC 0xfeeb1e
static DWORD tls_index = TLS_MAGIC;
static DWORD getTls()
{
    // this seems unsafe but is not - allocation happens once before
    //  any Thread is actually created
    if (tls_index == TLS_MAGIC)
	tls_index = ::TlsAlloc();
    if (tls_index == (DWORD)-1)
	// if it happened is REALLY BAD so better die quick and clean!
	abort();
    return tls_index;
}
#else
static pthread_key_t current_key;

class ThreadPrivateKeyAlloc
{
public:
    ThreadPrivateKeyAlloc()
    {
	if (::pthread_key_create(&current_key,ThreadPrivate::destroyFunc))
	    Debug(DebugFail,"Failed to create current thread key!");
    }
};

static ThreadPrivateKeyAlloc keyAllocator;
#endif

static TokenDict s_prio[] = {
    { "lowest", Thread::Lowest },
    { "low", Thread::Low },
    { "normal", Thread::Normal },
    { "high", Thread::High },
    { "highest", Thread::Highest },
    { 0, 0 }
};

static ObjList s_threads;
static Mutex s_tmutex(true);

ThreadPrivate* ThreadPrivate::create(Thread* t,const char* name,Thread::Priority prio)
{
    ThreadPrivate *p = new ThreadPrivate(t,name);
    int e = 0;
#ifndef _WINDOWS
    // Set a decent (256K) stack size that won't eat all virtual memory
    pthread_attr_t attr;
    ::pthread_attr_init(&attr);
    ::pthread_attr_setstacksize(&attr, 16*PTHREAD_STACK_MIN);
    if (prio > Thread::Normal) {
	struct sched_param param;
	param.sched_priority = 0;
	int policy = SCHED_OTHER;
	switch (prio) {
	    case Thread::High:
		policy = SCHED_RR;
		param.sched_priority = 1;
		break;
	    case Thread::Highest:
		policy = SCHED_FIFO;
		param.sched_priority = 99;
		break;
	    default:
		break;
	}
	int err = ::pthread_attr_setschedpolicy(&attr,policy);
	if (!err)
	    err = ::pthread_attr_setschedparam(&attr,&param);
	if (err) {
	    DDebug(DebugWarn,"Could not set thread scheduling parameters: %s (%d)",
		strerror(err),err);
	}
    }
#endif

    for (int i=0; i<5; i++) {
#ifdef _WINDOWS
	HTHREAD t = ::_beginthread(startFunc,16*PTHREAD_STACK_MIN,p);
	e = (t == (HTHREAD)-1) ? errno : 0;
	if (!e) {
	    p->thread = t;
	    int pr = THREAD_PRIORITY_NORMAL;
	    switch (prio) {
		case Thread::Lowest:
		    pr = THREAD_PRIORITY_LOWEST;
		    break;
		case Thread::Low:
		    pr = THREAD_PRIORITY_BELOW_NORMAL;
		    break;
		case Thread::High:
		    pr = THREAD_PRIORITY_ABOVE_NORMAL;
		    break;
		case Thread::Highest:
		    pr = THREAD_PRIORITY_HIGHEST;
		    break;
		default:
		    break;
	    }
	    if (pr != THREAD_PRIORITY_NORMAL)
		::SetThreadPriority(reinterpret_cast<HANDLE>(t),pr);
	}
#else
	e = ::pthread_create(&p->thread,&attr,startFunc,p);
#endif
	if (e != EAGAIN)
	    break;
	Thread::usleep(20);
    }
#ifndef _WINDOWS
    ::pthread_attr_destroy(&attr);
#endif
    if (e) {
	Debug(DebugGoOn,"Error %d while creating pthread in '%s' [%p]",e,name,p);
	p->m_thread = 0;
	p->destroy();
	return 0;
    }
    p->m_running = true;
    return p;
}

ThreadPrivate::ThreadPrivate(Thread* t,const char* name)
    : m_thread(t), m_running(false), m_started(false), m_updest(true), m_cancel(false), m_name(name)
{
#ifdef DEBUG
    Debugger debug("ThreadPrivate::ThreadPrivate","(%p,\"%s\") [%p]",t,name,this);
#endif
    Lock lock(s_tmutex);
    s_threads.append(this);
}

ThreadPrivate::~ThreadPrivate()
{
#ifdef DEBUG
    Debugger debug("ThreadPrivate::~ThreadPrivate()"," %p '%s' [%p]",m_thread,m_name,this);
#endif
    m_running = false;
    Lock lock(s_tmutex);
    s_threads.remove(this,false);
    if (m_thread && m_updest) {
	Thread *t = m_thread;
	m_thread = 0;
	// let other threads access the list while we delete our upper layer
	lock.drop();
	delete t;
    }
}

void ThreadPrivate::destroy()
{
    DDebug(DebugAll,"ThreadPrivate::destroy() '%s' [%p]",m_name,this);
    cleanup();
    delete this;
}

void ThreadPrivate::pubdestroy()
{
#ifdef DEBUG
    Debugger debug(DebugAll,"ThreadPrivate::pubdestroy()"," %p '%s' [%p]",m_thread,m_name,this);
#endif
    m_updest = false;
    cleanup();
    m_thread = 0;

    if (current() == this) {
	cancel(true);
	// should never reach here...
	Debug(DebugFail,"ThreadPrivate::pubdestroy() past cancel??? [%p]",this);
    }
    else {
	cancel(false);
	// delay a little so thread has a chance to clean up
	for (int i=0; i<20; i++) {
	    s_tmutex.lock();
	    bool done = !s_threads.find(this);
	    s_tmutex.unlock();
	    if (done)
		return;
	    Thread::msleep(5,false);
	}
	if (m_cancel && !cancel(true))
	    Debug(DebugWarn,"ThreadPrivate::pubdestroy() %p '%s' failed cancel [%p]",m_thread,m_name,this);
    }
}

void ThreadPrivate::run()
{
    DDebug(DebugAll,"ThreadPrivate::run() '%s' [%p]",m_name,this);
#ifdef _WINDOWS
    ::TlsSetValue(getTls(),this);
#else
    ::pthread_setspecific(current_key,this);
    pthread_cleanup_push(cleanupFunc,this);
    ::pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,0);
    ::pthread_detach(::pthread_self());
#endif

    // FIXME: possible race if public object is destroyed during thread startup
    while (!m_started)
	Thread::usleep(10,true);
    if (m_thread)
	m_thread->run();

#ifndef _WINDOWS
    pthread_cleanup_pop(1);
#endif
}

bool ThreadPrivate::cancel(bool hard)
{
    DDebug(DebugAll,"ThreadPrivate::cancel(%s) '%s' [%p]",String::boolText(hard),m_name,this);
    bool ret = true;
    if (m_running) {
	ret = false;
	if (hard) {
	    bool critical = m_thread && m_thread->m_locking;
	    if (critical) {
		// give the thread a chance to cancel without locking a mutex
		Debug(DebugMild,"Hard canceling '%s' while is taking a lock [%p]",m_name,this);
		m_cancel = true;
		for (int i = 0; i < 50; i++) {
		    Thread::msleep(1);
		    if (!m_running)
			return true;
		}
	    }
	    m_running = false;
#ifdef _WINDOWS
	    Debug(DebugGoOn,"ThreadPrivate '%s' terminating win32 thread %lu [%p]",
		m_name,thread,this);
	    ret = ::TerminateThread(reinterpret_cast<HANDLE>(thread),0) != 0;
#else
	    Debug(critical ? DebugInfo : DebugWarn,"ThreadPrivate '%s' terminating pthread %p [%p]",
		m_name,&thread,this);
	    ret = !::pthread_cancel(thread);
#endif
	    if (ret) {
		// hard cancel succeeded - object is unsafe to touch any more
		Thread::msleep(1);
		return true;
	    }
	    // hard cancel failed - set back the running flag
	    m_running = true;
	}
	m_cancel = true;
    }
    return ret;
}

void ThreadPrivate::cleanup()
{
    DDebug(DebugAll,"ThreadPrivate::cleanup() %p '%s' [%p]",m_thread,m_name,this);
    if (m_thread && m_thread->m_private) {
	if (m_thread->m_private == this) {
	    m_thread->m_private = 0;
	    m_thread->cleanup();
	    if (m_thread->locked())
		Debug(DebugFail,"Thread '%s' destroyed with mutex locks (%d held) [%p]",m_name,m_thread->locks(),m_thread);
	}
	else {
	    Debug(DebugFail,"ThreadPrivate::cleanup() %p '%s' mismatching %p [%p]",m_thread,m_name,m_thread->m_private,this);
	    m_thread = 0;
	}
    }
}

ThreadPrivate* ThreadPrivate::current()
{
#ifdef _WINDOWS
    return reinterpret_cast<ThreadPrivate *>(::TlsGetValue(getTls()));
#else
    return reinterpret_cast<ThreadPrivate *>(::pthread_getspecific(current_key));
#endif
}

void ThreadPrivate::killall()
{
    Debugger debug("ThreadPrivate::killall()");
    ThreadPrivate *t;
    bool sledgehammer = false;
    int c = 1;
    s_tmutex.lock();
    ObjList *l = &s_threads;
    while (l && (t = static_cast<ThreadPrivate *>(l->get())) != 0)
    {
	Debug(DebugInfo,"Trying to kill ThreadPrivate '%s' [%p], attempt %d",t->m_name,t,c);
	bool ok = t->cancel(c > SOFT_KILLS);
	if (ok) {
	    int d = 0;
	    // delay a little (exponentially) so threads have a chance to clean up
	    for (int i=1; i<=KILL_WAIT; i<<=1) {
		s_tmutex.unlock();
		Thread::msleep(i-d);
		d = i;
		s_tmutex.lock();
		if (t != l->get())
		    break;
	    }
	}
	if (t != l->get())
	    c = 1;
	else {
	    if (ok) {
		Debug(DebugGoOn,"Could not kill %p but seems OK to delete it (library bug?)",t);
		s_tmutex.unlock();
		t->destroy();
		s_tmutex.lock();
		if (t != l->get())
		    c = 1;
		continue;
	    }
	    Thread::msleep(1);
	    if (++c >= (SOFT_KILLS+HARD_KILLS)) {
		Debug(DebugGoOn,"Could not kill %p, will use sledgehammer later.",t);
		sledgehammer = true;
		t->m_thread = 0;
		l = l->next();
		c = 1;
	    }
	}
    }
    s_tmutex.unlock();
    // last solution - a REALLY BIG tool!
    // usually too big since many libraries have threads of their own...
    if (sledgehammer) {
#ifdef THREAD_KILL
	Debug(DebugGoOn,"Brutally killing remaining threads!");
	::pthread_kill_other_threads_np();
#else
	Debug(DebugGoOn,"Aargh! I cannot kill remaining threads on this platform!");
#endif
    }
}

void ThreadPrivate::destroyFunc(void* arg)
{
#ifdef DEBUG
    Debugger debug("ThreadPrivate::destroyFunc","(%p)",arg);
#endif
    ThreadPrivate *t = reinterpret_cast<ThreadPrivate *>(arg);
    if (t)
	t->destroy();
}

void ThreadPrivate::cleanupFunc(void* arg)
{
    DDebug(DebugAll,"ThreadPrivate::cleanupFunc(%p)",arg);
    ThreadPrivate *t = reinterpret_cast<ThreadPrivate *>(arg);
    if (t)
	t->cleanup();
}

#ifdef _WINDOWS
void ThreadPrivate::startFunc(void* arg)
#else
void* ThreadPrivate::startFunc(void* arg)
#endif
{
    DDebug(DebugAll,"ThreadPrivate::startFunc(%p)",arg);
    ThreadPrivate *t = reinterpret_cast<ThreadPrivate *>(arg);
    t->run();
#ifdef _WINDOWS
    t->m_running = false;
    if (t->m_updest)
	t->destroy();
#else
    return 0;
#endif
}

Runnable::~Runnable()
{
}

Thread::Thread(const char* name, Priority prio)
    : m_private(0), m_locks(0), m_locking(false)
{
#ifdef DEBUG
    Debugger debug("Thread::Thread","(\"%s\",%d) [%p]",name,prio,this);
#endif
    m_private = ThreadPrivate::create(this,name,prio);
}

Thread::Thread(const char *name, const char* prio)
    : m_private(0), m_locks(0), m_locking(false)
{
#ifdef DEBUG
    Debugger debug("Thread::Thread","(\"%s\",\"%s\") [%p]",name,prio,this);
#endif
    m_private = ThreadPrivate::create(this,name,priority(prio));
}

Thread::~Thread()
{
    DDebug(DebugAll,"Thread::~Thread() [%p]",this);
    if (m_private)
	m_private->pubdestroy();
}

bool Thread::error() const
{
    return !m_private;
}

bool Thread::running() const
{
    Lock lock(s_tmutex);
    return m_private ? m_private->m_started : false;
}

const char* Thread::name() const
{
    return m_private ? m_private->m_name : 0;
}

bool Thread::startup()
{
    if (!m_private)
	return false;
    m_private->m_started = true;
    return true;
}

Thread *Thread::current()
{
    ThreadPrivate* t = ThreadPrivate::current();
    return t ? t->m_thread : 0;
}

const char* Thread::currentName()
{
    ThreadPrivate* t = ThreadPrivate::current();
    return t ? t->m_name : 0;
}

int Thread::count()
{
    Lock lock(s_tmutex);
    return s_threads.count();
}

void Thread::cleanup()
{
    DDebug(DebugAll,"Thread::cleanup() [%p]",this);
}

void Thread::killall()
{
    if (!ThreadPrivate::current())
	ThreadPrivate::killall();
}

void Thread::exit()
{
    DDebug(DebugAll,"Thread::exit()");
#ifdef _WINDOWS
    ThreadPrivate* t = ThreadPrivate::current();
    if (t) {
	t->m_running = false;
	t->destroy();
    }
    ::_endthread();
#else
    ::pthread_exit(0);
#endif
}

bool Thread::check(bool exitNow)
{
    ThreadPrivate* t = ThreadPrivate::current();
    if (!(t && t->m_cancel))
	return false;
    if (exitNow)
	exit();
    return true;
}

void Thread::cancel(bool hard)
{
    DDebug(DebugAll,"Thread::cancel() [%p]",this);
    if (m_private)
	m_private->cancel(hard);
}

void Thread::yield(bool exitCheck)
{
#ifdef _WINDOWS
    // zero sleep is bad if we have high priority threads, they
    //  won't relinquish the timeslice for lower priority ones
    ::Sleep(1);
#else
    ::usleep(0);
#endif
    if (exitCheck)
	check();
}

void Thread::sleep(unsigned int sec, bool exitCheck)
{
#ifdef _WINDOWS
    ::Sleep(sec*1000);
#else
    ::sleep(sec);
#endif
    if (exitCheck)
	check();
}

void Thread::msleep(unsigned long msec, bool exitCheck)
{
#ifdef _WINDOWS
    ::Sleep(msec);
#else
    ::usleep(msec*1000L);
#endif
    if (exitCheck)
	check();
}

void Thread::usleep(unsigned long usec, bool exitCheck)
{
#ifdef _WINDOWS
    ::Sleep(usec/1000);
#else
    ::usleep(usec);
#endif
    if (exitCheck)
	check();
}

Thread::Priority Thread::priority(const char* name, Thread::Priority defvalue)
{
    return (Thread::Priority)lookup(name,s_prio,defvalue);
}

const char* Thread::priority(Thread::Priority prio)
{
    return lookup(prio,s_prio);
}

void Thread::preExec()
{
#ifdef THREAD_KILL
    ::pthread_kill_other_threads_np();
#endif
}

/* vi: set ts=8 sw=4 sts=4 noet: */

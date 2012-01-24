/**
 * TelEngine.cpp
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

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>
#include <time.h>


#ifdef _WINDOWS
extern "C" {

static u_int32_t s_randSeed = (u_int32_t)(TelEngine::Time::now() & 0xffffffff);

long int random()
{ return (s_randSeed = (s_randSeed + 1) * 0x8088405) % RAND_MAX; }

void srandom(unsigned int seed)
{ s_randSeed = seed % RAND_MAX; }

}

#ifndef HAVE_GMTIME_S
#include <errno.h>

int _gmtime_s(struct tm* _tm, const time_t* time)
{
    static TelEngine::Mutex m;
    struct tm* tmp;
    if (!_tm)
	return EINVAL;
    _tm->tm_isdst = _tm->tm_yday = _tm->tm_wday = _tm->tm_year = _tm->tm_mon = _tm->tm_mday =
	_tm->tm_hour = _tm->tm_min = _tm->tm_sec = -1;
    if (!time)
	return EINVAL;
    m.lock();
    tmp = gmtime(time);
    if (!tmp) {
	m.unlock();
	return EINVAL;
    }
    *_tm = *tmp;
    m.unlock();
    return 0;
}

#endif

#else
#include <sys/resource.h>
#endif


namespace TelEngine {

#define DebugMin DebugFail
#define DebugMax DebugAll

#define OUT_BUFFER_SIZE 8192

static int s_debug = DebugWarn;
static int s_indent = 0;
static bool s_debugging = true;
static bool s_abort = false;
static u_int64_t s_startTime = 0;
static u_int64_t s_timestamp = 0;
static Debugger::Formatting s_fmtstamp = Debugger::None;

static const char* const s_colors[11] = {
    "\033[5;41;1;33m\033[K",// DebugFail - blinking yellow on red
    "\033[41;1;33m\033[K",  // Unnamed   - yellow on red
    "\033[41;1;37m\033[K",  // DebugGoOn - white on red
    "\033[41;37m\033[K",    // Unnamed   - gray on red
    "\033[40;31m\033[K",    // DebugStub - red on black
    "\033[40;1;31m\033[K",  // DebugWarn - light red on black
    "\033[40;1;33m\033[K",  // DebugMild - yellow on black
    "\033[40;1;37m\033[K",  // DebugCall - white on black
    "\033[40;1;32m\033[K",  // DebugNote - light green on black
    "\033[40;1;36m\033[K",  // DebugInfo - light cyan on black
    "\033[40;36m\033[K"     // DebugAll  - cyan on black
};

static const char* const s_levels[11] = {
    "FAIL",
    "FAIL",
    "GOON",
    "GOON",
    "STUB",
    "WARN",
    "MILD",
    "CALL",
    "NOTE",
    "INFO",
    "ALL",
};

static const char* dbg_level(int level)
{
    if (level < DebugMin)
	level = DebugMin;
    if (level > DebugMax)
	level = DebugMax;
    return s_levels[level];
}

static void dbg_stderr_func(const char* buf, int level)
{
    ::write(2,buf,::strlen(buf));
}

static void dbg_colorize_func(const char* buf, int level)
{
    const char* col = debugColor(level);
    ::write(2,col,::strlen(col));
    ::write(2,buf,::strlen(buf));
    col = debugColor(-2);
    ::write(2,col,::strlen(col));
}

static void (*s_output)(const char*,int) = dbg_stderr_func;
static void (*s_intout)(const char*,int) = 0;

static Mutex out_mux;
static Mutex ind_mux;
static Thread* s_thr = 0;

static bool reentered()
{
    if (!s_thr)
	return false;
    return (Thread::current() == s_thr);
}

static void common_output(int level,char* buf)
{
    if (level < -1)
	level = -1;
    if (level > DebugMax)
	level = DebugMax;
    int n = ::strlen(buf);
    if (n && (buf[n-1] == '\n'))
	    n--;
    buf[n] = '\n';
    buf[n+1] = '\0';
    // serialize the output strings
    out_mux.lock();
    // TODO: detect reentrant calls from foreign threads and main thread
    s_thr = Thread::current();
    if (s_output)
	s_output(buf,level);
    if (s_intout)
	s_intout(buf,level);
    s_thr = 0;
    out_mux.unlock();
}

static void dbg_output(int level,const char* prefix, const char* format, va_list ap)
{
    if (!(s_output || s_intout))
	return;
    char buf[OUT_BUFFER_SIZE];
    unsigned int n = 0;
    if (s_fmtstamp != Debugger::None) {
	u_int64_t t = Time::now();
	if (s_fmtstamp == Debugger::Relative)
	    t -= s_timestamp;
	unsigned int s = (unsigned int)(t / 1000000);
	unsigned int u = (unsigned int)(t % 1000000);
	if (s_fmtstamp == Debugger::Textual) {
	    time_t sec = (time_t)s;
	    struct tm tmp;
#ifdef _WINDOWS
	    _gmtime_s(&tmp,&sec);
#else
	    gmtime_r(&sec,&tmp);
#endif
	    ::sprintf(buf,"%04d%02d%02d%02d%02d%02d.%06u ",
		tmp.tm_year+1900,tmp.tm_mon+1,tmp.tm_mday,
		tmp.tm_hour,tmp.tm_min,tmp.tm_sec,u);
	}
	else
	    ::sprintf(buf,"%07u.%06u ",s,u);
	n = ::strlen(buf);
    }
    unsigned int l = s_indent*2;
    if (l >= sizeof(buf)-n)
	l = sizeof(buf)-n-1;
    ::memset(buf+n,' ',l);
    n += l;
    buf[n] = 0;
    l = sizeof(buf)-n-2;
    if (prefix)
	::strncpy(buf+n,prefix,l);
    n = ::strlen(buf);
    l = sizeof(buf)-n-2;
    if (format) {
	::vsnprintf(buf+n,l,format,ap);
    }
    common_output(level,buf);
}

void Output(const char* format, ...)
{
    char buf[OUT_BUFFER_SIZE];
    if (!((s_output || s_intout) && format && *format))
	return;
    if (reentered())
	return;
    va_list va;
    va_start(va,format);
    ::vsnprintf(buf,sizeof(buf)-2,format,va);
    va_end(va);
    common_output(-1,buf);
}

void Debug(int level, const char* format, ...)
{
    if (!s_debugging)
	return;
    if (level > s_debug)
	return;
    if (reentered())
	return;
    if (!format)
	format = "";
    char buf[32];
    ::sprintf(buf,"<%s> ",dbg_level(level));
    va_list va;
    va_start(va,format);
    ind_mux.lock();
    dbg_output(level,buf,format,va);
    ind_mux.unlock();
    va_end(va);
    if (s_abort && (level == DebugFail))
	abort();
}

void Debug(const char* facility, int level, const char* format, ...)
{
    if (!s_debugging)
	return;
    if (level > s_debug)
	return;
    if (reentered())
	return;
    if (!format)
	format = "";
    char buf[64];
    ::snprintf(buf,sizeof(buf),"<%s:%s> ",facility,dbg_level(level));
    va_list va;
    va_start(va,format);
    ind_mux.lock();
    dbg_output(level,buf,format,va);
    ind_mux.unlock();
    va_end(va);
    if (s_abort && (level == DebugFail))
	abort();
}

void Debug(const DebugEnabler* local, int level, const char* format, ...)
{
    if (!s_debugging)
	return;
    const char* facility = 0;
    if (!local) {
	if (level > s_debug)
	    return;
    }
    else {
	if (!local->debugAt(level))
	    return;
	facility = local->debugName();
    }
    if (reentered())
	return;
    if (!format)
	format = "";
    char buf[64];
    if (facility)
	::snprintf(buf,sizeof(buf),"<%s:%s> ",facility,dbg_level(level));
    else
	::sprintf(buf,"<%s> ",dbg_level(level));
    va_list va;
    va_start(va,format);
    ind_mux.lock();
    dbg_output(level,buf,format,va);
    ind_mux.unlock();
    va_end(va);
    if (s_abort && (level == DebugFail))
	abort();
}

void abortOnBug()
{
    if (s_abort)
	abort();
}

bool abortOnBug(bool doAbort)
{
    bool tmp = s_abort;
    s_abort = doAbort;
    return tmp;
}  

int debugLevel()
{
    return s_debug;
}

int debugLevel(int level)
{
    if (level < DebugMin)
	level = DebugMin;
    if (level > DebugMax)
	level = DebugMax;
    return (s_debug = level);
}

bool debugAt(int level)
{
    return (s_debugging && (level <= s_debug));
}

const char* debugColor(int level)
{
    if (level == -2)
	return "\033[0m\033[K"; // reset to defaults
    if ((level < DebugMin) || (level > DebugMax))
	return "\033[0;40;37m\033[K"; // light gray on black
    return s_colors[level];
}

int DebugEnabler::debugLevel(int level)
{
    if (level < DebugMin)
	level = DebugMin;
    if (level > DebugMax)
	level = DebugMax;
    m_chain = 0;
    return (m_level = level);
}

bool DebugEnabler::debugAt(int level) const
{
    if (m_chain)
	return m_chain->debugAt(level);
    return (m_enabled && (level <= m_level));
}

void DebugEnabler::debugCopy(const DebugEnabler* original)
{
    if (original) {
	m_level = original->debugLevel();
	m_enabled = original->debugEnabled();
    }
    else {
	m_level = TelEngine::debugLevel();
	m_enabled = debugEnabled();
    }
    m_chain = 0;
}

Debugger::Debugger(const char* name, const char* format, ...)
    : m_name(name)
{
    if (s_debugging && m_name && (s_debug >= DebugAll) && !reentered()) {
	char buf[64];
	::snprintf(buf,sizeof(buf),">>> %s",m_name);
	va_list va;
	va_start(va,format);
	ind_mux.lock();
	dbg_output(DebugAll,buf,format,va);
	va_end(va);
	s_indent++;
	ind_mux.unlock();
    }
    else
	m_name = 0;
}

Debugger::Debugger(int level, const char* name, const char* format, ...)
    : m_name(name)
{
    if (s_debugging && m_name && (s_debug >= level) && !reentered()) {
	char buf[64];
	::snprintf(buf,sizeof(buf),">>> %s",m_name);
	va_list va;
	va_start(va,format);
	ind_mux.lock();
	dbg_output(DebugAll,buf,format,va);
	va_end(va);
	s_indent++;
	ind_mux.unlock();
    }
    else
	m_name = 0;
}

static void dbg_dist_helper(const char* buf, const char* fmt, ...)
{
    va_list va;
    va_start(va,fmt);
    dbg_output(DebugAll,buf,fmt,va);
    va_end(va);
}

Debugger::~Debugger()
{
    if (m_name) {
	ind_mux.lock();
	s_indent--;
	if (s_debugging)
	    dbg_dist_helper("<<< ","%s",m_name);
	ind_mux.unlock();
    }
}

void Debugger::setOutput(void (*outFunc)(const char*,int))
{
    out_mux.lock();
    s_output = outFunc ? outFunc : dbg_stderr_func;
    out_mux.unlock();
}

void Debugger::setIntOut(void (*outFunc)(const char*,int))
{
    out_mux.lock();
    s_intout = outFunc;
    out_mux.unlock();
}

void Debugger::enableOutput(bool enable, bool colorize)
{
    s_debugging = enable;
    if (colorize)
	setOutput(dbg_colorize_func);
}

void Debugger::setFormatting(Formatting format)
{
    // start stamp will be rounded to full second
    s_timestamp = (Time::now() / 1000000) * 1000000;
    s_fmtstamp = format;
}



u_int64_t Time::now()
{
#ifdef _WINDOWS
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    // Convert from FILETIME (100 nsec units since January 1, 1601)
    //  to extended time_t (1 usec units since January 1, 1970)
    u_int64_t rval = ((ULARGE_INTEGER*)&ft)->QuadPart / 10;
    rval -= 11644473600000000;
    return rval;
#else
    struct timeval tv;
    return ::gettimeofday(&tv,0) ? 0 : fromTimeval(&tv);
#endif
}

u_int64_t Time::msecNow()
{
    return (u_int64_t)(now() / 1000);
}

u_int32_t Time::secNow()
{
#ifdef _WINDOWS
    return (u_int32_t)(now() / 1000000);
#else
    struct timeval tv;
    return ::gettimeofday(&tv,0) ? 0 : tv.tv_sec;
#endif
}

u_int64_t Time::fromTimeval(const struct timeval* tv)
{
    u_int64_t rval = 0;
    if (tv) {
	// Please keep it this way or the compiler may b0rk
	rval = tv->tv_sec;
	rval *= 1000000;
	rval += tv->tv_usec;
    }
    return rval;
}

void Time::toTimeval(struct timeval* tv, u_int64_t usec)
{
    if (tv) {
	tv->tv_usec = (long)(usec % 1000000);
	tv->tv_sec = (long)(usec / 1000000);
    }
}


bool GenObject::alive() const
{
    return true;
}

void GenObject::destruct()
{
    delete this;
}


static Mutex s_refmutex;

RefObject::~RefObject()
{
    if (m_refcount > 0)
	Debug(DebugFail,"RefObject [%p] destroyed with count=%d",this,m_refcount);
}

bool RefObject::alive() const
{
    return m_refcount > 0;
}

void RefObject::destruct()
{
    deref();
}

bool RefObject::refInternal()
{
    if (m_refcount > 0) {
	++m_refcount;
	return true;
    }
    return false;
}

bool RefObject::ref()
{
    Lock lock(s_refmutex);
    return refInternal();
}

bool RefObject::deref()
{
    bool zeroCall = false;
    s_refmutex.lock();
    int i = m_refcount;
    if (i > 0)
	--m_refcount;
    if (i == 1)
	zeroCall = zeroRefsTest();
    s_refmutex.unlock();
    if (zeroCall)
	zeroRefs();
    else if (i <= 0)
	Debug(DebugFail,"RefObject::deref() called with count=%d [%p]",i,this);
    return (i <= 1);
}

void RefObject::zeroRefs()
{
    destroyed();
    delete this;
}

bool RefObject::zeroRefsTest()
{
    return true;
}

bool RefObject::resurrect()
{
    s_refmutex.lock();
    bool ret = (0 == m_refcount);
    if (ret)
	m_refcount = 1;
    s_refmutex.unlock();
    return ret;
}

void RefObject::destroyed()
{
}

Mutex& RefObject::refMutex()
{
    return s_refmutex;
}

void RefPointerBase::assign(RefObject* oldptr, RefObject* newptr, void* pointer)
{
    if (oldptr == newptr)
	return;
    // Always reference the new object before dereferencing the old one
    //  and also don't keep pointers to objects that fail referencing
    m_pointer = (newptr && newptr->ref()) ? pointer : 0;
    if (oldptr)
	oldptr->deref();
}


void SysUsage::init()
{
    if (!s_startTime)
	s_startTime = Time::now();
}

u_int64_t SysUsage::startTime()
{
    init();
    return s_startTime;
}

u_int64_t SysUsage::usecRunTime(Type type)
{
    switch (type) {
	case WallTime:
	    return Time::now() - startTime();
	case UserTime:
	    {
#ifdef _WINDOWS
		FILETIME dummy,ft;
		if (GetProcessTimes(GetCurrentProcess(),&dummy,&dummy,&dummy,&ft)) {
		    u_int64_t t = ft.dwLowDateTime | (((u_int64_t)ft.dwHighDateTime) << 32);
		    return t / 10;
		}
#else
		struct rusage usage;
		// FIXME: this is broken, may not sum all threads
		if (!::getrusage(RUSAGE_SELF,&usage))
		    return Time::fromTimeval(usage.ru_utime);
#endif
	    }
	    break;
	case KernelTime:
	    {
#ifdef _WINDOWS
		FILETIME dummy,ft;
		if (GetProcessTimes(GetCurrentProcess(),&dummy,&dummy,&ft,&dummy)) {
		    u_int64_t t = ft.dwLowDateTime | (((u_int64_t)ft.dwHighDateTime) << 32);
		    return t / 10;
		}
#else
		struct rusage usage;
		// FIXME: this is broken, may not sum all threads
		if (!::getrusage(RUSAGE_SELF,&usage))
		    return Time::fromTimeval(usage.ru_stime);
#endif
	    }
	    break;
    }
    return 0;
}

u_int64_t SysUsage::msecRunTime(Type type)
{
    return usecRunTime(type) / 1000;
}

u_int32_t SysUsage::secRunTime(Type type)
{
    return (u_int32_t)(usecRunTime(type) / 1000000);
}

double SysUsage::runTime(Type type)
{
#ifdef _WINDOWS
    // VC++ 6 does not implement conversion from UINT64 to double!
    return 0.000001 * (int64_t)usecRunTime(type);
#else
    return 0.000001 * usecRunTime(type);
#endif
}

};

/* vi: set ts=8 sw=4 sts=4 noet: */

#ifndef base_h
#define base_h

/////////////////////////////////////////////////
// Environment defines
#if defined (_WIN32)
  #define GLib_WIN
  #define GLib_WIN32
#elif defined (__WIN64)
#define GLib_WIN
  #define GLib_WIN64
#elif defined(__linux__)
  #define GLib_UNIX
  #define GLib_LINUX __linux__
#elif defined(__sun__)
  #define GLib_UNIX
  #define GLib_SOLARIS __sun__
  // !bn: defined in ctype_iso.h but apears as a parameter in GLib
  #undef _C
#elif defined(__FreeBSD__)
  #define GLib_UNIX
  #define GLib_BSD __FreeBSD__
#elif defined(__CYGWIN__)
  #define GLib_UNIX
  #define GLib_CYGWIN
#elif (defined(__APPLE__) && defined(__MACH__))
  #define GLib_UNIX
  #define GLib_MACOSX
#endif

#define _CMPWARN
// compiler
#if defined (_MSC_VER)
  #define GLib_MSC _MSC_VER
#endif
#if defined (__BCPLUSPLUS__)
  #define GLib_BCB __BCPLUSPLUS__
#endif
#if defined (__GNUC__)
  #define GLib_GCC __GNUC__
// !bn: to bo not samo dokler ne ugotovim kje so primerjave problematicne
  #ifdef FLTWARN
    #undef _CMPWARN
    #define _CMPWARN __attribute__ ((deprecated))
  #endif
#endif

// includes
#if defined (GLib_WIN32)
  #define WIN32_LEAN_AND_MEAN
  #include <windows.h>
  #if !defined(_WINSOCKAPI_) && !defined(_WINSOCK2API_)
    #include <winsock2.h>
  #endif
  #include <oleauto.h>
  #include <shellapi.h>
#endif

#if defined(GLib_UNIX)
  #define __STDC_LIMIT_MACROS
  #include <stdint.h>
  #include <stdarg.h>
  #include <errno.h>
  #include <fcntl.h>
  #include <dirent.h>
  #include <unistd.h>
  #include <signal.h>
  #include <sys/poll.h>
  #include <sys/socket.h>
  #include <sys/stat.h>
  #include <sys/time.h>
  #include <sys/types.h>
  #include <sys/wait.h>
  #include <sys/resource.h>
  #include <netdb.h>
  #include <arpa/inet.h>
  #include <netinet/in.h>
#endif

// word size
#if __WORDSIZE == 32 || defined(GLib_WIN32) || defined(__CYGWIN32__)
  #define GLib_32Bit
#elif __WORDSIZE == 64 || defined(GLib_WIN64) || defined(__CYGWIN64__)
  #define GLib_64Bit
#else
  #error "Undefined word size"
#endif

#if defined(GLib_UNIX)
  extern int _daylight;
  #ifndef _environ
    #if defined(GLib_MACOSX)
     #include <crt_externs.h>
     #define _environ (*_NSGetEnviron())
    #elif !defined(GLib_CYGWIN)
     #define _environ __environ
    #else
     #define _environ environ
    #endif
  #endif
  #ifndef __stdcall
    #define __stdcall
  #endif
#endif

#if defined(GLib_LINUX)
  #include <sys/epoll.h>
#endif

#if defined(GLib_SOLARIS)
  #include <ieeefp.h>
#endif

#if defined (__GLIBC__)
  #define GLib_GLIBC __GLIBC__
#endif

#if defined (__USE_XOPEN2K)
  #define GLib_POSIX_1j "1003.1j"
#endif

#include <ctype.h>
#include <float.h>
#include <limits.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <typeinfo>
#include <stdexcept>

#ifdef GLib_CYGWIN
  #define timezone _timezone
#endif

#include "base/bd.h"
#include "base/fl.h"
#include "base/dt.h"
#include "base/ut.h"
#include "base/ds.h"
#include "base/bits.h"
#include "base/hash.h"
#include "base/strut.h"
#include "base/xml.h"

#include "base/xmath.h"
#include "base/xmlser.h"

#include "base/unicode.h"
#include "base/unicodestring.h"
#include "base/tm.h"
#include "base/shash.h"
#include "base/os.h"
#include "base/console.h"

#include "base/app.h"
#include "base/env.h"
#include "base/wch.h"
#include "base/xdt.h"
#include "base/xfl.h"

#include "base/blobbs.h"
#include "base/cache.h"
#include "base/fds.h"
#include "base/lx.h"
#include "base/macro.h"
#include "base/pp.h"
#include "base/url.h"
#include "base/gix.h"

#include "base/exp.h"
#include "base/http.h"
#include "base/html.h"
#include "base/md5.h"
#include "base/ss.h"
#include "base/linalg.h"
//#include "base/prolog.h"
#include "base/json.h"
#include "base/zipfl.h"

void BaseTralala();

#endif

#include "stdafx.h"

#include "base.h"

void BaseTralala(){
  printf("Active defines:\n");
  #ifdef GLib_WIN
  printf("  GLib_WIN\n");
  #endif
  #ifdef GLib_WIN32
  printf("  GLib_WIN32\n");
  #endif
  #ifdef GLib_WIN64
  printf("  GLib_WIN64\n");
  #endif
  #ifdef GLib_UNIX
  printf("  GLib_UNIX\n");
  #endif
  #ifdef GLib_LINUX
  printf("  GLib_LINUX\n");
  #endif
  #ifdef GLib_SOLARIS
  printf("  GLib_SOLARIS\n");
  #endif
  #ifdef GLib_MSC
  printf("  GLib_MSC\n");
  #endif
  #ifdef GLib_CYGWIN
  printf("  GLib_CYGWIN\n");
  #endif
  #ifdef GLib_BCB
  printf("  GLib_BCB\n");
  #endif
  #ifdef GLib_GCC
  printf("  GLib_GCC\n");
  #endif
  #ifdef GLib_MACOSX
  printf("  GLib_MACOSX\n");
  #endif
  #ifdef GLib_64Bit
  printf("  GLib_64Bit\n");
  #endif
  #ifdef GLib_32Bit
  printf("  GLib_32Bit\n");
  #endif
  #ifdef GLib_GLIBC
  printf("  GLib_GLIBC\n");
  #endif
  #ifdef GLib_POSIX_1j
  printf("  GLib_POSIX_1j\n");
  #endif
}

#if defined(GLib_UNIX)
int _daylight = 0;
#endif

#include "base/bd.cpp"
#include "base/fl.cpp"
#include "base/dt.cpp"
#include "base/ut.cpp"
#include "base/hash.cpp"
#include "base/strut.cpp"

#include "base/unicode.cpp"
#include "base/unicodestring.cpp"
#include "base/tm.cpp"
#include "base/os.cpp"
#include "base/console.cpp"

#include "base/app.cpp"
#include "base/bits.cpp"
#include "base/env.cpp"
#include "base/wch.cpp"
#include "base/xdt.cpp"
#include "base/xfl.cpp"
#include "base/xmath.cpp"

#include "base/blobbs.cpp"
#include "base/lx.cpp"
#include "base/macro.cpp"
#include "base/pp.cpp"
#include "base/url.cpp"
#include "base/gix.cpp"

#include "base/exp.cpp"
#include "base/http.cpp"
#include "base/html.cpp"
#include "base/md5.cpp"
#include "base/ss.cpp"
#include "base/xml.cpp"
#include "base/linalg.cpp"
#include "base/json.cpp"
//#include "base/prolog.cpp"

#include "base/zipfl.cpp"
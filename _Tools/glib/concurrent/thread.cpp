#include "../stdafx.h"
#include "../base.h"
#include "thread.h"

#if defined(GLib_WIN)

#include "arch/win/thread_win.cpp"

#elif defined(GLib_UNIX)

#include "arch/posix/thread_posix.cpp"

#else

// no support for threads in non-Win and non-Unix systems

#endif

TInterruptibleThread& TInterruptibleThread::operator=(const TInterruptibleThread& Other) {
	ThreadHandle = Other.ThreadHandle;
	ThreadId = Other.ThreadId;
	SleeperBlocker = Other.SleeperBlocker;
	return *this;
}
	
void TInterruptibleThread::WaitForInterrupt(const int Msecs) {
	SleeperBlocker.Block(Msecs);
}

void TInterruptibleThread::Interrupt() {
	SleeperBlocker.Release();
}

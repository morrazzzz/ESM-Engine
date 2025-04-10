#pragma once

#include "net_utils.h"

#ifdef XR_NETSERVER_EXPORTS
	#define XRNETSERVER_API __declspec(dllexport)
#else
	#define XRNETSERVER_API __declspec(dllimport)
	#pragma comment(lib,	"xrNetServer"	)
#endif

IC u32 TimeGlobal	(CTimer* timer)	{ return timer->GetElapsed_ms();	}
IC u32 TimerAsync	(CTimer* timer) { return TimeGlobal	(timer);		}

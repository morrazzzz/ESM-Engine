// Engine.h: interface for the CEngine class.
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include "engineAPI.h"
#include "eventAPI.h"
#include "..\xrCpu_Pipe\xrCPU_Pipe.h"
#include "xrSheduler.h"

class ENGINE_API CEngine
{
	HMODULE				hPSGP;
public:
	// DLL api stuff
	CEngineAPI			External;
	CEventAPI			Event;
	CSheduler			Sheduler;

	void				Initialize	();
	void				Destroy		();
	
	CEngine();
	~CEngine();
};

ENGINE_API extern xrDispatchTable	PSGP;
ENGINE_API extern CEngine			Engine;

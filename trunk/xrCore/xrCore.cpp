// xrCore.cpp : Defines the entry point for the DLL application.
//
#include "stdafx.h"
#pragma hdrstop

#include <mmsystem.h>
#include <objbase.h>
#include "xrCore.h"
 
#pragma comment(lib,"winmm.lib")

#ifdef DEBUG
#	include	<malloc.h>
#endif // DEBUG

XRCORE_API		xrCore	Core;
XRCORE_API		u32		build_id;
XRCORE_API		LPCSTR	build_date;

namespace CPU
{
	extern	void			Detect	();
};

static u32	init_counter	= 0;

void xrCore::_initialize	(LPCSTR _ApplicationName, LogCallback cb, BOOL init_fs, LPCSTR fs_fname)
{
	strcpy_s					(ApplicationName,_ApplicationName);
	if (!init_counter) {
		// Init COM so we can use CoCreateInstance
//		HRESULT co_res = 
			CoInitializeEx (nullptr, COINIT_MULTITHREADED);

		strcpy_s			(Params,sizeof(Params),GetCommandLine());
		_strlwr_s			(Params,sizeof(Params));

		string_path		fn,dr,di;

		// application path
        GetModuleFileName(GetModuleHandle(MODULE_NAME),fn,sizeof(fn));
        _splitpath		(fn,dr,di,0,0);
        strconcat		(sizeof(ApplicationPath),ApplicationPath,dr,di);

		// working path
        if( strstr(Params,"-wf") )
        {
            string_path				c_name;
            sscanf					(strstr(Core.Params,"-wf ")+4,"%[^ ] ",c_name);
            SetCurrentDirectory     (c_name);

        }
		GetCurrentDirectory(sizeof(WorkingPath),WorkingPath);

		// User/Comp Name
		DWORD	sz_user		= sizeof(UserName);
		GetUserName			(UserName,&sz_user);

		DWORD	sz_comp		= sizeof(CompName);
		GetComputerName		(CompName,&sz_comp);

		// Mathematics & PSI detection
		CPU::Detect			();
		
		Memory._initialize	(strstr(Params,"-mem_debug") ? TRUE : FALSE);

		DUMP_PHASE;

		InitLog				();
		_initialize_cpu		();

		rtc_initialize		();

		xr_FS				= xr_new<CLocatorAPI>	();

		xr_EFS				= xr_new<EFS_Utils>		();
	}
	if (init_fs)
	{
		u32 flags			= 0;
		if (strstr(Params,"-build"))	 
			flags |= CLocatorAPI::flBuildCopy;
		if (strstr(Params,"-ebuild")) 
			flags |= CLocatorAPI::flBuildCopy|CLocatorAPI::flEBuildCopy;

#ifdef DEBUG
		if (strstr(Params,"-cache"))  flags |= CLocatorAPI::flCacheFiles;
		else flags &= ~CLocatorAPI::flCacheFiles;
#endif // DEBUG

		flags |= CLocatorAPI::flScanAppRoot;

		if (strstr(Params,"-file_activity"))	 
			flags |= CLocatorAPI::flDumpFileActivity;

		FS._initialize		(flags,0,fs_fname);
		     Msg("'%s' build %d, %s\n","xrCore",build_id, build_date);
			 Msg("Engine Discord: https://discord.gg/D4CK5Vu6t3");
		EFS._initialize		();

#ifdef DEBUG
		Msg					("CRT heap 0x%08x",_get_heap_handle());
		Msg					("Process heap 0x%08x",GetProcessHeap());
#endif // DEBUG

	}
	SetLogCB				(cb);
	init_counter++;
}

#include "compression_ppmd_stream.h"
extern compression::ppmd::stream	*trained_model;
void xrCore::_destroy		()
{
	--init_counter;
	if (!init_counter){
		FS._destroy			();
		EFS._destroy		();
		xr_delete			(xr_FS);
		xr_delete			(xr_EFS);

#ifndef	_EDITOR
		if (trained_model) {
			void			*buffer = trained_model->buffer();
			xr_free			(buffer);
			xr_delete		(trained_model);
		}
#endif

		Memory._destroy		();
		CoUninitialize();
	}
}

const char* xrCore::GetEngineVersion() {
		static string256 buff;
		std::snprintf(buff, sizeof(buff), "xrCore build %d, %s", build_id, build_date);
		return buff;
}

xr_string ANSIToUTF8(const xr_string& string)
{
	wchar_t* wcs{};
    int Lenght_ = MultiByteToWideChar(1251, 0, string.c_str(), (int)string.size(), wcs, 0);
	wcs = new wchar_t[Lenght_ + 1];
	MultiByteToWideChar(1251, 0, string.c_str(), (int)string.size(), wcs, Lenght_);
	wcs[Lenght_] = L'\0';

	char* u8s = nullptr;
	Lenght_ = WideCharToMultiByte(CP_UTF8, 0, wcs, (int)std::wcslen(wcs), u8s, 0, nullptr, nullptr);
	u8s = new char[Lenght_ + 1];
	WideCharToMultiByte(CP_UTF8, 0, wcs, (int)std::wcslen(wcs), u8s, Lenght_, nullptr, nullptr);
	u8s[Lenght_] = '\0';

	xr_string result(u8s);
	delete[] wcs;
	delete[] u8s;

	return result;
}

#ifndef XRCORE_STATIC
	BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD ul_reason_for_call, LPVOID lpvReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		{
			_clear87		();

#ifdef _M_IX86
			_control87		( _PC_53,   MCW_PC );
#endif
			_control87		( _RC_CHOP, MCW_RC );
			_control87		( _RC_NEAR, MCW_RC );
			_control87		( _MCW_EM,  MCW_EM );
		}
//.		LogFile.reserve		(256);
		break;
	case DLL_THREAD_ATTACH:
		timeBeginPeriod	(1);
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
#ifdef USE_MEMORY_MONITOR
		memory_monitor::flush_each_time	(true);
#endif // USE_MEMORY_MONITOR
		break;
	}
    return TRUE;
}
#endif // XRCORE_STATIC


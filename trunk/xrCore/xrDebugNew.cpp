#include "stdafx.h"
#pragma hdrstop

#include "os_clipboard.h"
#include "xrdebug.h"

#ifndef _M_X64
#include "dxerr9.h"
#endif

#pragma warning(push)
#pragma warning(disable:4995)
#include <malloc.h>
#include <direct.h>
#pragma warning(pop)

extern bool shared_str_initialized;

#ifdef __BORLANDC__
    #	include "d3d9.h"
    #	include "d3dx9.h"
    #	include "D3DX_Wrapper.h"
    #	pragma comment(lib,"EToolsB.lib")
    #	define DEBUG_INVOKE	DebugBreak()
        static BOOL			bException	= TRUE;
    #   define USE_BUG_TRAP
#else
    #   define USE_BUG_TRAP
    #	define DEBUG_INVOKE	__debugbreak()
        static BOOL			bException	= FALSE;
#endif

#ifndef _M_AMD64
#	ifndef __BORLANDC__
#		pragma comment(lib,"dxerr9.lib")
#	endif
#endif

#include <new.h>
#include <dbghelp.h>
#include <signal.h>
#include <Shlwapi.h>

#ifdef USE_BUG_TRAP
#	include "../3rd party/BugTrap/BugTrap/BugTrap.h"						// for BugTrap functionality
    #ifndef __BORLANDC__
        #	pragma comment(lib,"BugTrap.lib")		// Link to ANSI DLL
    #else
        #	pragma comment(lib,"BugTrapB.lib")		// Link to ANSI DLL
    #endif
#endif // USE_BUG_TRAP

#pragma comment(lib, "shlwapi")
#pragma comment(lib, "dbghelp")

#if 0//def DEBUG
#	define USE_OWN_ERROR_MESSAGE_WINDOW
#else // DEBUG
#	define USE_OWN_MINI_DUMP
#endif // DEBUG

XRCORE_API	xrDebug		Debug;

static bool	error_after_dialog = false;
bool g_SymEngineInitialized = false;
const u32 MaxFramesCountDefault = 512;

#ifdef _WIN64
#define MACHINE_TYPE IMAGE_FILE_MACHINE_AMD64
#else	
#define MACHINE_TYPE IMAGE_FILE_MACHINE_I386
#endif

bool GetNextStackFrameString(LPSTACKFRAME stackFrame, PCONTEXT threadCtx, xr_string& frameStr)
{
	BOOL result = StackWalk(
		MACHINE_TYPE,
		GetCurrentProcess(),
		GetCurrentThread(),
		stackFrame,
		threadCtx,
		nullptr,
		SymFunctionTableAccess,
		SymGetModuleBase,
		nullptr
	);

	if (!result || !stackFrame->AddrPC.Offset)
		return false;

	frameStr.clear();
	HINSTANCE hModule = reinterpret_cast<HINSTANCE>(SymGetModuleBase(GetCurrentProcess(), stackFrame->AddrPC.Offset));

	string512 formatBuff;
	if (hModule && GetModuleFileName(hModule, formatBuff, _countof(formatBuff)))
		frameStr.append(PathFindFileName(formatBuff));

	sprintf_s(formatBuff, _countof(formatBuff), " at %p", reinterpret_cast<void*>(stackFrame->AddrPC.Offset));
	frameStr.append(formatBuff);

	BYTE arrSymBuffer[512];
	ZeroMemory(arrSymBuffer, sizeof(arrSymBuffer));

	PIMAGEHLP_SYMBOL functionInfo = reinterpret_cast<PIMAGEHLP_SYMBOL>(arrSymBuffer);
	functionInfo->SizeOfStruct = sizeof(*functionInfo);
	functionInfo->MaxNameLength = sizeof(arrSymBuffer) - sizeof(*functionInfo) + 1;
	DWORD_PTR dwFunctionOffset;

	result = SymGetSymFromAddr(
		GetCurrentProcess(),
		stackFrame->AddrPC.Offset,
		&dwFunctionOffset,
		functionInfo);

	if (result)
	{
		if (dwFunctionOffset)
			sprintf_s(formatBuff, _countof(formatBuff), " %s() + %Iu byte(s)", functionInfo->Name, dwFunctionOffset);
		else
			sprintf_s(formatBuff, _countof(formatBuff), " %s()", functionInfo->Name);

		frameStr.append(formatBuff);
	}

	DWORD dwLineOffset;
	IMAGEHLP_LINE sourceInfo = { 0 };
	sourceInfo.SizeOfStruct = sizeof(sourceInfo);

	result = SymGetLineFromAddr(
		GetCurrentProcess(),
		stackFrame->AddrPC.Offset,
		&dwLineOffset,
		&sourceInfo);

	if (result)
	{
		if (dwLineOffset)
		{
			sprintf_s(
				formatBuff,
				_countof(formatBuff),
				" in %s line %u + %u byte(s)",
				sourceInfo.FileName,
				sourceInfo.LineNumber,
				dwLineOffset);
		}
		else
			sprintf_s(formatBuff, _countof(formatBuff), " in %s line %u", sourceInfo.FileName, sourceInfo.LineNumber);

		frameStr.append(formatBuff);
	}

	return true;
}

bool InitializeSymbolEngine()
{
	if (g_SymEngineInitialized)
		return true;

	DWORD dwOptions = SymGetOptions();
	SymSetOptions(dwOptions | SYMOPT_DEFERRED_LOADS | SYMOPT_LOAD_LINES | SYMOPT_UNDNAME);

	if (SymInitialize(GetCurrentProcess(), nullptr, TRUE))
		g_SymEngineInitialized = true;

	return g_SymEngineInitialized;
}

void DeinitializeSymbolEngine()
{
	if (!g_SymEngineInitialized)
		return;

	SymCleanup(GetCurrentProcess());
	g_SymEngineInitialized = false;
}

xr_vector<xr_string> BuildStackTrace(PCONTEXT threadCtx, u32 maxFramesCount = MaxFramesCountDefault)
{
	static xrCriticalSection DbgHelpCS;
	DbgHelpCS.Enter();

	SStringVec traceResult;
	STACKFRAME stackFrame = { 0 };
	xr_string frameStr;

	if (!InitializeSymbolEngine())
	{
		Msg("! BuildStackTrace InitializeSymbolEngine failed with error: %d", GetLastError());
		DbgHelpCS.Leave();
		return traceResult;
	}

	traceResult.reserve(maxFramesCount);

	stackFrame.AddrPC.Mode = AddrModeFlat;
	stackFrame.AddrStack.Mode = AddrModeFlat;
	stackFrame.AddrFrame.Mode = AddrModeFlat;

#ifdef _WIN64
	stackFrame.AddrPC.Offset = threadCtx->Rip;
	stackFrame.AddrStack.Offset = threadCtx->Rsp;
	stackFrame.AddrFrame.Offset = threadCtx->Rbp;
#else	
	stackFrame.AddrPC.Offset = threadCtx->Eip;
	stackFrame.AddrStack.Offset = threadCtx->Esp;
	stackFrame.AddrFrame.Offset = threadCtx->Ebp;
#endif	

	while (GetNextStackFrameString(&stackFrame, threadCtx, frameStr) && traceResult.size() <= maxFramesCount)
		traceResult.push_back(frameStr);

	DeinitializeSymbolEngine();
	DbgHelpCS.Leave();
	return traceResult;
}

SStringVec BuildStackTrace(u32 maxFramesCount = MaxFramesCountDefault)
{
	CONTEXT currentThreadCtx = { 0 };
	RtlCaptureContext(&currentThreadCtx);
	currentThreadCtx.ContextFlags = CONTEXT_FULL;
	return BuildStackTrace(&currentThreadCtx, maxFramesCount);
}

void LogStackTrace(LPCSTR header)
{
	SStringVec stackTrace = BuildStackTrace();

	Msg				("%s",header);

	for (const auto& frame : stackTrace)
		Msg("%s", frame.c_str());
}

void gather_info		(const char *expression, const char *description, const char *argument0, const char *argument1, const char *file, int line, const char *function, LPSTR assertion_info)
{
	LPSTR				buffer = assertion_info;
	LPCSTR				endline = "\n";
	LPCSTR				prefix = "[error]";
	bool				extended_description = (description && !argument0 && strchr(description,'\n'));
	for (int i=0; i<2; ++i) {
		if (!i)
			buffer		+= sprintf(buffer,"%sFATAL ERROR%s%s",endline,endline,endline);
		buffer			+= sprintf(buffer,"%sExpression    : %s%s",prefix,expression,endline);
		buffer			+= sprintf(buffer,"%sFunction      : %s%s",prefix,function,endline);
		buffer			+= sprintf(buffer,"%sFile          : %s%s",prefix,file,endline);
		buffer			+= sprintf(buffer,"%sLine          : %d%s",prefix,line,endline);
		
		if (extended_description) {
			buffer		+= sprintf(buffer,"%s%s%s",endline,description,endline);
			if (argument0) {
				if (argument1) {
					buffer	+= sprintf(buffer,"%s%s",argument0,endline);
					buffer	+= sprintf(buffer,"%s%s",argument1,endline);
				}
				else
					buffer	+= sprintf(buffer,"%s%s",argument0,endline);
			}
		}
		else {
			buffer		+= sprintf(buffer,"%sDescription   : %s%s",prefix,description,endline);
			if (argument0) {
				if (argument1) {
					buffer	+= sprintf(buffer,"%sArgument 0    : %s%s",prefix,argument0,endline);
					buffer	+= sprintf(buffer,"%sArgument 1    : %s%s",prefix,argument1,endline);
				}
				else
					buffer	+= sprintf(buffer,"%sArguments     : %s%s",prefix,argument0,endline);
			}
		}

		buffer			+= sprintf(buffer,"%s",endline);
		if (!i) {
			if (shared_str_initialized) {
				Msg		("%s",assertion_info);
				FlushLog();
			}
			buffer		= assertion_info;
			endline		= "\r\n";
			prefix		= "";
		}
	}
	OutputDebugString(buffer);
#ifdef USE_MEMORY_MONITOR
	memory_monitor::flush_each_time	(true);
	memory_monitor::flush_each_time	(false);
#endif // USE_MEMORY_MONITOR

	if (!IsDebuggerPresent() && !strstr(GetCommandLine(),"-no_call_stack_assert")) {
		if (shared_str_initialized)
			Msg			("stack trace:\n");

		xr_vector<xr_string> stackTrace = BuildStackTrace();
		for (size_t i = 2; i < stackTrace.size(); i++)
		{
			if (shared_str_initialized)
				Msg("%s", stackTrace[i].c_str());
		}

#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
			buffer		+= sprintf(buffer,"%s%s",g_stackTrace[i],endline);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW
		}
	
		if (shared_str_initialized)
			FlushLog	();

		os_clipboard::copy_to_clipboard	  (assertion_info);
}

void xrDebug::do_exit	(const std::string &message)
{
	FlushLog			();
	MessageBox			(NULL,message.c_str(),"Error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
	TerminateProcess	(GetCurrentProcess(),1);
}

void xrDebug::backend	(const char *expression, const char *description, const char *argument0, const char *argument1, const char *file, int line, const char *function, bool &ignore_always)
{
	static xrCriticalSection CS
#ifdef PROFILE_CRITICAL_SECTIONS
	(MUTEX_PROFILE_ID(xrDebug::backend))
#endif // PROFILE_CRITICAL_SECTIONS
	;

	CS.Enter			();

	error_after_dialog	= true;

	string4096			assertion_info;
	gather_info			(expression, description, argument0, argument1, file, line, function, assertion_info);

#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
	LPCSTR				endline = "\r\n";
	LPSTR				buffer = assertion_info + xr_strlen(assertion_info);
	buffer				+= sprintf(buffer,"%sPress CANCEL to abort execution%s",endline,endline);
	buffer				+= sprintf(buffer,"Press TRY AGAIN to continue execution%s",endline);
	buffer				+= sprintf(buffer,"Press CONTINUE to continue execution and ignore all the errors of this type%s%s",endline,endline);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW

	if (handler)
		handler			();

	if (get_on_dialog())
		get_on_dialog()	(true);

#ifdef XRCORE_STATIC
	MessageBox			(NULL,assertion_info,"X-Ray error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
#else
#	ifdef USE_OWN_ERROR_MESSAGE_WINDOW
		int					result = 
			MessageBox(
				GetTopWindow(NULL),
				assertion_info,
				"Fatal Error",
				MB_CANCELTRYCONTINUE|MB_ICONERROR|MB_SYSTEMMODAL
			);

		switch (result) {
			case IDCANCEL : {
				DEBUG_INVOKE;
				break;
			}
			case IDTRYAGAIN : {
				error_after_dialog	= false;
				break;
			}
			case IDCONTINUE : {
				error_after_dialog	= false;
				ignore_always	= true;
				break;
			}
			default : NODEFAULT;
		}
#	else // USE_OWN_ERROR_MESSAGE_WINDOW
#		ifdef USE_BUG_TRAP
			BT_SetUserMessage	(assertion_info);
#		endif // USE_BUG_TRAP
		DEBUG_INVOKE;
#	endif // USE_OWN_ERROR_MESSAGE_WINDOW
#endif

	if (get_on_dialog())
		get_on_dialog()	(false);

	CS.Leave			();
}

LPCSTR xrDebug::error2string	(long code)
{
	LPCSTR				result	= 0;
	static	string1024	desc_storage;

#ifdef _M_AMD64
#else
	result				= DXGetErrorDescription9	(code);
#endif
	if (0==result) 
	{
		FormatMessage	(FORMAT_MESSAGE_FROM_SYSTEM,0,code,0,desc_storage,sizeof(desc_storage)-1,0);
		result			= desc_storage;
	}
	return		result	;
}

void xrDebug::error		(long hr, const char* expr, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(error2string(hr),expr,0,0,file,line,function,ignore_always);
}

void xrDebug::error		(long hr, const char* expr, const char* e2, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(error2string(hr),expr,e2,0,file,line,function,ignore_always);
}

void xrDebug::fail		(const char *e1, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		("assertion failed",e1,0,0,file,line,function,ignore_always);
}

void xrDebug::fail		(const char *e1, const std::string &e2, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(e1,e2.c_str(),0,0,file,line,function,ignore_always);
}

void xrDebug::fail		(const char *e1, const char *e2, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(e1,e2,0,0,file,line,function,ignore_always);
}

void xrDebug::fail		(const char *e1, const char *e2, const char *e3, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(e1,e2,e3,0,file,line,function,ignore_always);
}

void xrDebug::fail		(const char *e1, const char *e2, const char *e3, const char *e4, const char *file, int line, const char *function, bool &ignore_always)
{
	backend		(e1,e2,e3,e4,file,line,function,ignore_always);
}

void __cdecl xrDebug::fatal(const char *file, int line, const char *function, const char* F,...)
{
	string1024	buffer;

	va_list		p;
	va_start	(p,F);
	vsprintf	(buffer,F,p);
	va_end		(p);

	bool		ignore_always = true;

	backend		("fatal error","<no expression>",buffer,0,file,line,function,ignore_always);
}

int out_of_memory_handler	(size_t size)
{
	Memory.mem_compact		();
#ifndef _EDITOR
	u32						crt_heap		= mem_usage_impl((HANDLE)_get_heap_handle(),0,0);
#else // _EDITOR
	u32						crt_heap		= 0;
#endif // _EDITOR
	u32						process_heap	= mem_usage_impl(GetProcessHeap(),0,0);
	int						eco_strings		= (int)g_pStringContainer->stat_economy			();
	int						eco_smem		= (int)g_pSharedMemoryContainer->stat_economy	();
	Msg						("* [x-ray]: crt heap[%d K], process heap[%d K]",crt_heap/1024,process_heap/1024);
	Msg						("* [x-ray]: economy: strings[%d K], smem[%d K]",eco_strings/1024,eco_smem);
	Debug.fatal				(DEBUG_INFO,"Out of memory. Memory request: %d K",size/1024);
	return					1;
}

extern LPCSTR log_name();

void CALLBACK PreErrorHandler	(INT_PTR)
{
#ifdef USE_BUG_TRAP
	if (!xr_FS || !FS.m_Flags.test(CLocatorAPI::flReady))
		return;

	string_path				log_folder;

	__try {
		FS.update_path		(log_folder,"$logs$","");
		if ((log_folder[0] != '\\') && (log_folder[1] != ':')) {
			string256		current_folder;
			_getcwd			(current_folder,sizeof(current_folder));
			
			string256		relative_path;
			strcpy			(relative_path,log_folder);
			strconcat		(sizeof(log_folder),log_folder,current_folder,"\\",relative_path);
		}
	}
	__except(EXCEPTION_EXECUTE_HANDLER) {
		strcpy				(log_folder,"logs");
	}

	BT_AddLogFile			(log_name());
#endif // USE_BUG_TRAP
}

#ifdef USE_BUG_TRAP
void SetupExceptionHandler()
{
	BT_InstallSehFilter		();

	BT_SetActivityType	(BTA_SAVEREPORT);

	BT_SetDialogMessage				(
		BTDM_INTRO2,
		"\
This is XRay Engine crash reporting client. \
To help the development process, \
please Submit Bug or save report and email it manually (button More...).\
\r\nMany thanks in advance and sorry for the inconvenience."
	);

	BT_SetPreErrHandler		(PreErrorHandler,0);
	BT_SetAppName			("XRay Engine");
	BT_SetReportFormat		(BTRF_TEXT);
	BT_SetFlags				(BTF_DETAILEDMODE | /**BTF_EDIETMAIL | /**/BTF_ATTACHREPORT /**| BTF_LISTPROCESSES /**| BTF_SHOWADVANCEDUI /**| BTF_SCREENCAPTURE/**/);
	BT_SetDumpType			(
		MiniDumpWithDataSegs |
//		MiniDumpWithFullMemory |
//		MiniDumpWithHandleData |
//		MiniDumpFilterMemory |
//		MiniDumpScanMemory |
//		MiniDumpWithUnloadedModules |
#ifndef _EDITOR
		MiniDumpWithIndirectlyReferencedMemory |
#endif // _EDITOR
//		MiniDumpFilterModulePaths |
//		MiniDumpWithProcessThreadData |
//		MiniDumpWithPrivateReadWriteMemory |
//		MiniDumpWithoutOptionalData |
//		MiniDumpWithFullMemoryInfo |
//		MiniDumpWithThreadInfo |
//		MiniDumpWithCodeSegs |
		0
	);
	BT_SetSupportEMail		("crash-report@stalker-game.com");
//	BT_SetSupportServer		("localhost", 9999);
//	BT_SetSupportURL		("www.gsc-game.com");
}
#endif // USE_BUG_TRAP

#if 1
extern void BuildStackTrace(struct _EXCEPTION_POINTERS *pExceptionInfo);
typedef LONG WINAPI UnhandledExceptionFilterType(struct _EXCEPTION_POINTERS *pExceptionInfo);
typedef LONG ( __stdcall *PFNCHFILTFN ) ( EXCEPTION_POINTERS * pExPtrs ) ;
extern "C" BOOL __stdcall SetCrashHandlerFilter ( PFNCHFILTFN pFn );

static UnhandledExceptionFilterType	*previous_filter = 0;

#ifdef USE_OWN_MINI_DUMP
typedef BOOL (WINAPI *MINIDUMPWRITEDUMP)(HANDLE hProcess, DWORD dwPid, HANDLE hFile, MINIDUMP_TYPE DumpType,
										 CONST PMINIDUMP_EXCEPTION_INFORMATION ExceptionParam,
										 CONST PMINIDUMP_USER_STREAM_INFORMATION UserStreamParam,
										 CONST PMINIDUMP_CALLBACK_INFORMATION CallbackParam
										 );

void FlushLog				(LPCSTR file_name);

void save_mini_dump			(_EXCEPTION_POINTERS *pExceptionInfo)
{
	// firstly see if dbghelp.dll is around and has the function we need
	// look next to the EXE first, as the one in System32 might be old 
	// (e.g. Windows 2000)
	HMODULE hDll	= NULL;
	string_path		szDbgHelpPath;

	if (GetModuleFileName( NULL, szDbgHelpPath, _MAX_PATH ))
	{
		char *pSlash = strchr( szDbgHelpPath, '\\' );
		if (pSlash)
		{
			strcpy	(pSlash+1, "DBGHELP.DLL" );
			hDll = ::LoadLibrary( szDbgHelpPath );
		}
	}

	if (hDll==NULL)
	{
		// load any version we can
		hDll = ::LoadLibrary( "DBGHELP.DLL" );
	}

	LPCTSTR szResult = NULL;

	if (hDll)
	{
		MINIDUMPWRITEDUMP pDump = (MINIDUMPWRITEDUMP)::GetProcAddress( hDll, "MiniDumpWriteDump" );
		if (pDump)
		{
			string_path	szDumpPath;
			string_path	szScratch;
			string64	t_stemp;

			timestamp	(t_stemp);
			strcpy		( szDumpPath, Core.ApplicationName);
			strcat		( szDumpPath, "_"					);
			strcat		( szDumpPath, Core.UserName			);
			strcat		( szDumpPath, "_"					);
			strcat		( szDumpPath, t_stemp				);
			strcat		( szDumpPath, ".mdmp"				);

			__try {
				if (FS.path_exist("$logs$"))
					FS.update_path	(szDumpPath,"$logs$",szDumpPath);
			}
            __except( EXCEPTION_EXECUTE_HANDLER ) {
				string_path	temp;
				strcpy		(temp,szDumpPath);
				strcpy		(szDumpPath,"logs/");
				strcat		(szDumpPath,temp);
            }

			string_path	log_file_name;
			strconcat	(sizeof(log_file_name), log_file_name, szDumpPath, ".log");
			FlushLog	(log_file_name);

			// create the file
			HANDLE hFile = ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			if (INVALID_HANDLE_VALUE==hFile)	
			{
				// try to place into current directory
				MoveMemory	(szDumpPath,szDumpPath+5,strlen(szDumpPath));
				hFile		= ::CreateFile( szDumpPath, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );
			}
			if (hFile!=INVALID_HANDLE_VALUE)
			{
				_MINIDUMP_EXCEPTION_INFORMATION ExInfo;

				ExInfo.ThreadId				= ::GetCurrentThreadId();
				ExInfo.ExceptionPointers	= pExceptionInfo;
				ExInfo.ClientPointers		= NULL;

				// write the dump
				MINIDUMP_TYPE	dump_flags	= MINIDUMP_TYPE(MiniDumpNormal | MiniDumpFilterMemory | MiniDumpScanMemory );

				BOOL bOK = pDump( GetCurrentProcess(), GetCurrentProcessId(), hFile, dump_flags, &ExInfo, NULL, NULL );
				if (bOK)
				{
					sprintf( szScratch, "Saved dump file to '%s'", szDumpPath );
					szResult = szScratch;
//					retval = EXCEPTION_EXECUTE_HANDLER;
				}
				else
				{
					sprintf( szScratch, "Failed to save dump file to '%s' (error %d)", szDumpPath, GetLastError() );
					szResult = szScratch;
				}
				::CloseHandle(hFile);
			}
			else
			{
				sprintf( szScratch, "Failed to create dump file '%s' (error %d)", szDumpPath, GetLastError() );
				szResult = szScratch;
			}
		}
		else
		{
			szResult = "DBGHELP.DLL too old";
		}
	}
	else
	{
		szResult = "DBGHELP.DLL not found";
	}
}
#endif // USE_OWN_MINI_DUMP

void format_message(LPSTR buffer, const u32& buffer_size)
{
	char*	message{};
	DWORD		error_code = GetLastError();

	if (!error_code) {
		*buffer	= 0;
		return;
	}

    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        error_code,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&message,
        0,
		NULL
	);

	sprintf		(buffer,"[error][%8d]    : %s",error_code,message);
    LocalFree	(message);
}

LONG WINAPI UnhandledFilter	(_EXCEPTION_POINTERS *pExceptionInfo)
{
	string256				error_message;
	format_message			(error_message,sizeof(error_message));

	if (!error_after_dialog && !strstr(GetCommandLine(),"-no_call_stack_assert")) {
		CONTEXT				save = *pExceptionInfo->ContextRecord;
		xr_vector<xr_string> stackTrace = BuildStackTrace(pExceptionInfo->ContextRecord, 1024);
		*pExceptionInfo->ContextRecord = save;

		if (shared_str_initialized)
			Msg				("ExceptionCode is [%x]\nstack trace:\n", pExceptionInfo->ExceptionRecord->ExceptionCode);

		os_clipboard::copy_to_clipboard	("stack trace:\r\n\r\n");

		string4096			buffer;
		for (xr_string& str : stackTrace) 
		{
			if (shared_str_initialized)
				Msg("%s", str.c_str());
			sprintf(buffer, "%s\r\n", str.c_str());
			os_clipboard::update_clipboard(buffer);
		}

		if (*error_message) {
			if (shared_str_initialized)
				Msg			("\n%s",error_message);

			xr_strcat			(error_message,sizeof(error_message),"\r\n");
			os_clipboard::update_clipboard(error_message);
		}
	}

	if (shared_str_initialized)
		FlushLog			();

#ifndef USE_OWN_ERROR_MESSAGE_WINDOW
#	ifdef USE_OWN_MINI_DUMP
		save_mini_dump		(pExceptionInfo);
#	endif // USE_OWN_MINI_DUMP
#else // USE_OWN_ERROR_MESSAGE_WINDOW
	if (!error_after_dialog) {
		if (Debug.get_on_dialog())
			Debug.get_on_dialog()	(true);

		MessageBox			(NULL,"Fatal error occured\n\nPress OK to abort program execution","Fatal error",MB_OK|MB_ICONERROR|MB_SYSTEMMODAL);
	}
#endif // USE_OWN_ERROR_MESSAGE_WINDOW

	if (!previous_filter) {
#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
		if (Debug.get_on_dialog())
			Debug.get_on_dialog()	(false);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW

		return				(EXCEPTION_CONTINUE_SEARCH) ;
	}

	previous_filter			(pExceptionInfo);

#ifdef USE_OWN_ERROR_MESSAGE_WINDOW
	if (Debug.get_on_dialog())
		Debug.get_on_dialog()		(false);
#endif // USE_OWN_ERROR_MESSAGE_WINDOW

	return					(EXCEPTION_CONTINUE_SEARCH) ;
}
#endif

//////////////////////////////////////////////////////////////////////
#ifdef M_BORLAND
	namespace std{
		extern new_handler _RTLENTRY _EXPFUNC set_new_handler( new_handler new_p );
	};

	static void __cdecl def_new_handler() 
    {
		FATAL		("Out of memory.");
    }

    void	xrDebug::_initialize		(const bool &dedicated)
    {
		handler							= 0;
		m_on_dialog						= 0;
        std::set_new_handler			(def_new_handler);	// exception-handler for 'out of memory' condition
//		::SetUnhandledExceptionFilter	(UnhandledFilter);	// exception handler to all "unhandled" exceptions
    }
#else
    typedef int		(__cdecl * _PNH)( size_t );
    _CRTIMP int		__cdecl _set_new_mode( int );
    //_CRTIMP _PNH	__cdecl _set_new_handler( _PNH );

#ifndef USE_BUG_TRAP
	void _terminate		()
	{
		if (strstr(GetCommandLine(),"-silent_error_mode"))
			exit				(-1);

		string4096				assertion_info;
		
		gather_info				(
			"<no expression>",
			"Unexpected application termination",
			0,
			0,
	#ifdef _ANONYMOUS_BUILD
			"",
			0,
	#else
			__FILE__,
			__LINE__,
	#endif
	#ifndef _EDITOR
			__FUNCTION__,
	#else // _EDITOR
			"",
	#endif // _EDITOR
			assertion_info
		);
		
		LPCSTR					endline = "\r\n";
		LPSTR					buffer = assertion_info + xr_strlen(assertion_info);
		buffer					+= sprintf(buffer,"Press OK to abort execution%s",endline);

		MessageBox				(
			GetTopWindow(NULL),
			assertion_info,
			"Fatal Error",
			MB_OK|MB_ICONERROR|MB_SYSTEMMODAL
		);
		
		exit					(-1);
	//	FATAL					("Unexpected application termination");
	}
#endif // USE_BUG_TRAP

	void debug_on_thread_spawn			()
	{
#ifdef USE_BUG_TRAP
		BT_SetTerminate					();
#else // USE_BUG_TRAP
		std::set_terminate				(_terminate);
#endif // USE_BUG_TRAP
	}

	static void handler_base				(LPCSTR reason_string)
	{
		bool							ignore_always = false;
		Debug.backend					(
			"error handler is invoked!",
			reason_string,
			0,
			0,
			DEBUG_INFO,
			ignore_always
		);
	}

	static void invalid_parameter_handler	(
			const wchar_t *expression,
			const wchar_t *function,
			const wchar_t *file,
			unsigned int line,
			uintptr_t reserved
		)
	{
		bool							ignore_always = false;

		string4096						expression_;
		string4096						function_;
		string4096						file_;
		size_t							converted_chars = 0;
//		errno_t							err = 
		if (expression)
			wcstombs_s	(
				&converted_chars, 
				expression_,
				sizeof(expression_),
				expression,
				(wcslen(expression) + 1)*2*sizeof(char)
			);
		else
			strcpy_s					(expression_,"");

		if (function)
			wcstombs_s	(
				&converted_chars, 
				function_,
				sizeof(function_),
				function,
				(wcslen(function) + 1)*2*sizeof(char)
			);
		else
			strcpy_s					(function_,__FUNCTION__);

		if (file)
			wcstombs_s	(
				&converted_chars, 
				file_,
				sizeof(file_),
				file,
				(wcslen(file) + 1)*2*sizeof(char)
			);
		else {
			line						= __LINE__;
			strcpy_s					(file_,__FILE__);
		}

		Debug.backend					(
			"error handler is invoked!",
			expression_,
			0,
			0,
			file_,
			line,
			function_,
			ignore_always
		);
	}

	static void std_out_of_memory_handler	()
	{
		handler_base					("std: out of memory");
	}

	static void pure_call_handler			()
	{
		handler_base					("pure virtual function call");
	}

#ifdef CS_USE_EXCEPTIONS
	static void unexpected_handler			()
	{
		handler_base					("unexpected program termination");
	}
#endif // CS_USE_EXCEPTIONS

	static void abort_handler				(int signal)
	{
		handler_base					("application is aborting");
	}

	static void floating_point_handler		(int signal)
	{
		handler_base					("floating point error");
	}

	static void illegal_instruction_handler	(int signal)
	{
		handler_base					("illegal instruction");
	}

//	static void storage_access_handler		(int signal)
//	{
//		handler_base					("illegal storage access");
//	}

	static void termination_handler			(int signal)
	{
		handler_base					("termination with exit code 3");
	}

    void	xrDebug::_initialize		(const bool &dedicated)
    {
		debug_on_thread_spawn			();

		_set_abort_behavior				(0,_WRITE_ABORT_MSG | _CALL_REPORTFAULT);
		signal							(SIGABRT,		abort_handler);
		signal							(SIGABRT_COMPAT,abort_handler);
		signal							(SIGFPE,		floating_point_handler);
		signal							(SIGILL,		illegal_instruction_handler);
		signal							(SIGINT,		0);
//		signal							(SIGSEGV,		storage_access_handler);
		signal							(SIGTERM,		termination_handler);

		_set_invalid_parameter_handler	(&invalid_parameter_handler);

		_set_new_mode					(1);
		_set_new_handler				(&out_of_memory_handler);
		std::set_new_handler			(&std_out_of_memory_handler);

		_set_purecall_handler			(&pure_call_handler);

#if 0// should be if we use exceptions
		std::set_unexpected				(_terminate);
#endif

#ifdef USE_BUG_TRAP
		SetupExceptionHandler();
#endif // USE_BUG_TRAP
		previous_filter					= ::SetUnhandledExceptionFilter(UnhandledFilter);	// exception handler to all "unhandled" exceptions

#if 0
		struct foo {static void	recurs	(const u32 &count)
		{
			if (!count)
				return;

			_alloca			(4096);
			recurs			(count - 1);
		}};
		foo::recurs			(u32(-1));
		std::terminate		();
#endif // 0
	}
#endif
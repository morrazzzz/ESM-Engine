#include "stdafx.h"
#include <process.h>

// mmsystem.h
#define MMNOSOUND
#define MMNOMIDI
#define MMNOAUX
#define MMNOMIXER
#define MMNOJOY
#include <mmsystem.h>

// Initialized on startup
XRCORE_API	Fmatrix			Fidentity;
XRCORE_API	Dmatrix			Didentity;
XRCORE_API	CRandom			Random;

namespace	FPU
{
	XRCORE_API void 	m24(void) {
#ifdef _M_IX86
		_control87(_PC_24, MCW_PC);
#endif
		_control87(_RC_CHOP, MCW_RC);
	}
	XRCORE_API void 	m24r(void) {
#ifdef _M_IX86
		_control87(_PC_24, MCW_PC);
#endif
		_control87(_RC_NEAR, MCW_RC);
	}
	XRCORE_API void 	m53(void) {
#ifdef _M_IX86
		_control87(_PC_53, MCW_PC);
#endif
		_control87(_RC_CHOP, MCW_RC);
	}
	XRCORE_API void 	m53r(void) {
#ifdef _M_IX86
		_control87(_PC_53, MCW_PC);
#endif
		_control87(_RC_NEAR, MCW_RC);
	}
	XRCORE_API void 	m64(void) {
#ifdef _M_IX86
		_control87(_PC_64, MCW_PC);
#endif
		_control87(_RC_CHOP, MCW_RC);
	}
	XRCORE_API void 	m64r(void) {
#ifdef _M_IX86
		_control87(_PC_64, MCW_PC);
#endif
		_control87(_RC_NEAR, MCW_RC);
	}

	void initialize()
	{
		clear87();
		m24r();
	}
};

namespace CPU 
{
	XRCORE_API u64				clk_per_second	;
	XRCORE_API u64				clk_per_milisec	;
	XRCORE_API u64				clk_per_microsec;
	XRCORE_API u64				clk_overhead	;
	XRCORE_API float			clk_to_seconds	;
	XRCORE_API float			clk_to_milisec	;
	XRCORE_API float			clk_to_microsec	;
	XRCORE_API u64				qpc_freq		= 0	;
	XRCORE_API u64				qpc_overhead	= 0	;
	XRCORE_API u32				qpc_counter		= 0	;

	XRCORE_API u64				QPC	()			{
		u64		_dest	;
		QueryPerformanceCounter			((PLARGE_INTEGER)&_dest);
		qpc_counter	++	;
		return	_dest	;
	}

	void Detect	()
	{
		cpuID.InitInfoCPU();

		// Timers & frequency
		u64			start,end;
		u32			dwStart,dwTest;

		SetPriorityClass		(GetCurrentProcess(),REALTIME_PRIORITY_CLASS);

		// Detect Freq
		dwTest	= timeGetTime();
		do { dwStart = timeGetTime(); } while (dwTest==dwStart);
		start	= GetCLK();
		while (timeGetTime()-dwStart<1000) ;
		end		= GetCLK();
		clk_per_second = end-start;

		// Detect RDTSC Overhead
		clk_overhead	= 0;
		u64 dummy		= 0;
		for (int i=0; i<256; i++)	{
			start			=	GetCLK();
			clk_overhead	+=	GetCLK()-start-dummy;
		}
		clk_overhead		/=	256;

		// Detect QPC Overhead
		QueryPerformanceFrequency	((PLARGE_INTEGER)&qpc_freq)	;
		qpc_overhead	= 0;
		for (u32 i=0; i<256; i++)	{
			start			=	QPC();
			qpc_overhead	+=	QPC()-start-dummy;
		}
		qpc_overhead		/=	256;

		SetPriorityClass	(GetCurrentProcess(),NORMAL_PRIORITY_CLASS);

		clk_per_second	-=	clk_overhead;
		clk_per_milisec	=	clk_per_second/1000;
		clk_per_microsec	=	clk_per_milisec/1000;
#ifdef _M_IX86
		_control87	( _PC_64,   MCW_PC );
#endif
//		_control87	( _RC_CHOP, MCW_RC );
		double a,b;
		a = 1;		b = double(clk_per_second);
		clk_to_seconds = float(double(a/b));
		a = 1000;	b = double(clk_per_second);
		clk_to_milisec = float(double(a/b));
		a = 1000000;b = double(clk_per_second);
		clk_to_microsec = float(double(a/b));
	}
};

//------------------------------------------------------------------------------------
void _initialize_cpu	(void) 
{
	cpuID.MessageInfoCPU();

	::Random.seed(u32(CPU::GetCLK() % (1i64 << 32i64)));

	Fidentity.identity		();	// Identity matrix
	Didentity.identity		();	// Identity matrix
	pvInitializeStatics		();	// Lookup table for compressed normals
	FPU::initialize			();
	_initialize_cpu_thread	();
}

// per-thread initialization
#include <xmmintrin.h>
#define _MM_DENORMALS_ZERO_MASK 0x0040
#define _MM_DENORMALS_ZERO_ON 0x0040
#define _MM_FLUSH_ZERO_MASK 0x8000
#define _MM_FLUSH_ZERO_ON 0x8000
#define _MM_SET_FLUSH_ZERO_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_FLUSH_ZERO_MASK) | (mode))
#define _MM_SET_DENORMALS_ZERO_MODE(mode) _mm_setcsr((_mm_getcsr() & ~_MM_DENORMALS_ZERO_MASK) | (mode))
static	BOOL	_denormals_are_zero_supported	= TRUE;
void debug_on_thread_spawn	();

void _initialize_cpu_thread	()
{
	debug_on_thread_spawn	();

	FPU::m24r();

	if (cpuID.SupportSSE())	
	{
		//_mm_setcsr ( _mm_getcsr() | (_MM_FLUSH_ZERO_ON+_MM_DENORMALS_ZERO_ON) );
		_MM_SET_FLUSH_ZERO_MODE			(_MM_FLUSH_ZERO_ON);
		if (_denormals_are_zero_supported)	{
			__try	{
				_MM_SET_DENORMALS_ZERO_MODE	(_MM_DENORMALS_ZERO_ON);
			} __except(EXCEPTION_EXECUTE_HANDLER) {
				_denormals_are_zero_supported	= FALSE;
			}
		}
	}
}

// threading API 
#pragma pack(push,8)
struct THREAD_NAME	{
	DWORD	dwType;
	LPCSTR	szName;
	DWORD	dwThreadID;
	DWORD	dwFlags;
};
void	thread_name	(const char* name)
{
	THREAD_NAME		tn;
	tn.dwType		= 0x1000;
	tn.szName		= name;
	tn.dwThreadID	= DWORD(-1);
	tn.dwFlags		= 0;
	__try
	{
		RaiseException(0x406D1388,0,sizeof(tn)/sizeof(DWORD),(ULONG_PTR*)&tn);
	}
	__except(EXCEPTION_CONTINUE_EXECUTION)
	{
	}
}
#pragma pack(pop)

struct	THREAD_STARTUP
{
	thread_t*	entry	;
	char*		name	;
	void*		args	;
};
void	__cdecl			thread_entry	(void*	_params )	{
	// initialize
	THREAD_STARTUP*		startup	= (THREAD_STARTUP*)_params	;
	thread_name			(startup->name);
	thread_t*			entry	= startup->entry;
	void*				arglist	= startup->args;
	xr_delete			(startup);
	_initialize_cpu_thread		();

	// call
	entry				(arglist);
}

void	thread_spawn	(thread_t*	entry, const char*	name, unsigned	stack, void* arglist )
{
	THREAD_STARTUP*		startup	= xr_new<THREAD_STARTUP>	();
	startup->entry		= entry;
	startup->name		= (char*)name;
	startup->args		= arglist;
	_beginthread		(thread_entry,stack,startup);
}

void spline1	( float t, Fvector *p, Fvector *ret )
{
	float     t2  = t * t;
	float     t3  = t2 * t;
	float     m[4];

	ret->x=0.0f;
	ret->y=0.0f;
	ret->z=0.0f;
	m[0] = ( 0.5f * ( (-1.0f * t3) + ( 2.0f * t2) + (-1.0f * t) ) );
	m[1] = ( 0.5f * ( ( 3.0f * t3) + (-5.0f * t2) + ( 0.0f * t) + 2.0f ) );
	m[2] = ( 0.5f * ( (-3.0f * t3) + ( 4.0f * t2) + ( 1.0f * t) ) );
	m[3] = ( 0.5f * ( ( 1.0f * t3) + (-1.0f * t2) + ( 0.0f * t) ) );

	for( int i=0; i<4; i++ )
	{
		ret->x += p[i].x * m[i];
		ret->y += p[i].y * m[i];
		ret->z += p[i].z * m[i];
	}
}

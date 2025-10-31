#include "stdafx.h"
#include	"xrsharedmem.h"
#include	<malloc.h>

xrMemory	Memory;
BOOL		mem_initialized	= FALSE;
bool		shared_str_initialized	= false;

// Processor specific implementations
extern		pso_MemCopy		xrMemCopy_MMX;
extern		pso_MemCopy		xrMemCopy_x86;
extern		pso_MemFill		xrMemFill_x86;

xrMemory::xrMemory()
{
	mem_copy	= xrMemCopy_x86;
	mem_fill	= xrMemFill_x86;
}

void	xrMemory::_initialize(BOOL bDebug)
{
	stat_calls = 0;
	stat_counter = 0;

	mem_copy = xrMemCopy_x86;
	mem_fill = xrMemFill_x86;

	if (!strstr(Core.Params, "-no_pure_alloc")) {
		// initialize POOLs
		u32	element = mem_pools_ebase;
		u32 sector = mem_pools_ebase * 1024;
		for (u32 pid = 0; pid < mem_pools_count; pid++)
		{
			mem_pools[pid]._initialize(element, sector, 0x1);
			element += mem_pools_ebase;
		}
	}

	mem_initialized = TRUE;

	//	DUMP_PHASE;
	g_pStringContainer = xr_new<str_container>();
	shared_str_initialized = true;
	//	DUMP_PHASE;
	g_pSharedMemoryContainer = xr_new<smem_container>();
	//	DUMP_PHASE;
}

void	xrMemory::_destroy()
{
	xr_delete					(g_pSharedMemoryContainer);
	xr_delete					(g_pStringContainer);

	mem_initialized				= FALSE;
}

void	xrMemory::mem_compact	()
{
	RegFlushKey						( HKEY_CLASSES_ROOT );
	RegFlushKey						( HKEY_CURRENT_USER );
	_heapmin						( );
	HeapCompact						(GetProcessHeap(),0);
	if (g_pStringContainer)			g_pStringContainer->clean		();
	if (g_pSharedMemoryContainer)	g_pSharedMemoryContainer->clean	();
	if (strstr(Core.Params,"-swap_on_compact"))
		SetProcessWorkingSetSize	(GetCurrentProcess(),size_t(-1),size_t(-1));
}

// xr_strdup
char*			xr_strdup		(const char* string)
{	
	VERIFY	(string);
	u32		len			= u32(xr_strlen(string))+1	;
	char *	memory		= (char*)	Memory.mem_alloc( len
#ifdef DEBUG_MEMORY_NAME
		, "strdup"
#endif // DEBUG_MEMORY_NAME
	);
	CopyMemory		(memory,string,len);
	return	memory;
}

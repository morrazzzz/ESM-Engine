#include "stdafx.h"
#include "xrMemory_align.h"

#ifndef	__BORLANDC__

MEMPOOL		mem_pools			[mem_pools_count];

// MSVC
ICF	u8*		acc_header			(void* P)	{	u8*		_P		= (u8*)P;	return	_P-1;	}
ICF	u32		get_header			(void* P)	{	return	(u32)*acc_header(P);				}
ICF	u32	get_pool(size_t size)
{
	u32		pid = u32(size / mem_pools_ebase);
	if (pid >= mem_pools_count)	return mem_generic;
	else						return pid;
}

bool g_use_pure_alloc = false;

void* xrMemory::mem_alloc(size_t size
#	ifdef DEBUG_MEMORY_NAME
	, const char* _name
#	endif // DEBUG_MEMORY_NAME
)
{
	stat_calls++;

	static bool checkPureAlloc = true;
	if (checkPureAlloc)
	{
		g_use_pure_alloc = !strstr(Core.Params, "-no_pure_alloc");
		checkPureAlloc = false;
	}

	if (g_use_pure_alloc) {
		void* result = malloc(size);
#ifdef USE_MEMORY_MONITOR
		memory_monitor::monitor_alloc(result, size, _name);
#endif // USE_MEMORY_MONITOR

		ASAN_UNPOISON_MEMORY_REGION(result, sizeof result);

		return							(result);
	}

	void* _ptr = 0;

	//
	if (!mem_initialized /*|| debug_mode*/)
	{
		// generic
		//	Igor: Reserve 1 byte for xrMemory header
		void* _real = xr_aligned_offset_malloc(1 + size, 16, 0x1);
		//void*	_real			=	xr_aligned_offset_malloc	(size + _footer, 16, 0x1);
		_ptr = (void*)(((u8*)_real) + 1);
		*acc_header(_ptr) = mem_generic;
	}
	else {
		//	accelerated
		//	Igor: Reserve 1 byte for xrMemory header
		u32	pool = get_pool(1 + size);
		//u32	pool				=	get_pool	(size+_footer);

//		ASAN_UNPOISON_MEMORY_REGION(_ptr, sizeof _ptr);

		if (mem_generic == pool)
		{
			// generic
			//	Igor: Reserve 1 byte for xrMemory header
			void* _real = xr_aligned_offset_malloc(1 + size, 16, 0x1);
			//void*	_real		=	xr_aligned_offset_malloc	(size + _footer,16,0x1);
			_ptr = (void*)(((u8*)_real) + 1);

			ASAN_UNPOISON_MEMORY_REGION(_ptr, sizeof _ptr);

			*acc_header(_ptr) = mem_generic;
		}
		else {
			// pooled
			//	Igor: Reserve 1 byte for xrMemory header
			//	Already reserved when getting pool id
			void* _real = mem_pools[pool].create();
			_ptr = (void*)(((u8*)_real) + 1);
			*acc_header(_ptr) = (u8)pool;
		}
	}

#ifdef USE_MEMORY_MONITOR
	memory_monitor::monitor_alloc(_ptr, size, _name);
#endif // USE_MEMORY_MONITOR

	return	_ptr;
}

void	xrMemory::mem_free(void* P)
{
	stat_calls++;
#ifdef USE_MEMORY_MONITOR
	memory_monitor::monitor_free(P);
#endif // USE_MEMORY_MONITOR

	if (g_use_pure_alloc) {
		free(P);
		return;
	}

	u32	pool = get_header(P);
	void* _real = (void*)(((u8*)P) - 1);
	if (mem_generic == pool)
	{
		// generic
		xr_aligned_free(_real);
	}
	else {
		// pooled
		VERIFY2(pool < mem_pools_count, "Memory corruption");
		mem_pools[pool].destroy(_real);
	}

	ASAN_POISON_MEMORY_REGION(_real, sizeof _real);
}

extern BOOL	g_bDbgFillMemory	;

void* xrMemory::mem_realloc(void* P, size_t size
#ifdef DEBUG_MEMORY_NAME
	, const char* _name
#endif // DEBUG_MEMORY_NAME
)
{
	stat_calls++;
	if (g_use_pure_alloc) {
		void* result = realloc(P, size);
#	ifdef USE_MEMORY_MONITOR
		memory_monitor::monitor_free(P);
		memory_monitor::monitor_alloc(result, size, _name);
#	endif // USE_MEMORY_MONITOR
		return result;
	}

	if (0 == P) {
		return mem_alloc(size
#	ifdef DEBUG_MEMORY_NAME
			, _name
#	endif // DEBUG_MEMORY_NAME
		);
	}

	u32		p_current = get_header(P);
	//	Igor: Reserve 1 byte for xrMemory header
	u32		p_new = get_pool(1 + size);
	u32		p_mode;

	if (mem_generic == p_current) {
		if (p_new < p_current)		p_mode = 2;
		else						p_mode = 0;
	}
	else 							p_mode = 1;

	void* _real = (void*)(((u8*)P) - 1);
	void* _ptr = NULL;
	if (0 == p_mode)
	{
		//	Igor: Reserve 1 byte for xrMemory header
		void* _real2 = xr_aligned_offset_realloc(_real, 1 + size, 16, 0x1);
		//void*	_real2			=	xr_aligned_offset_realloc	(_real,size+_footer,16,0x1);
		_ptr = (void*)(((u8*)_real2) + 1);
		*acc_header(_ptr) = mem_generic;
#ifdef USE_MEMORY_MONITOR
		memory_monitor::monitor_free(P);
		memory_monitor::monitor_alloc(_ptr, size, _name);
#endif // USE_MEMORY_MONITOR
	}
	else if (1 == p_mode) {
		// pooled realloc
		R_ASSERT2(p_current < mem_pools_count, "Memory corruption");
		u32		s_current = mem_pools[p_current].get_element();
		u32		s_dest = (u32)size;
		void* p_old = P;

		void* p_new_mem_alloc_old = mem_alloc(size
#ifdef DEBUG_MEMORY_NAME
			, _name
#endif // DEBUG_MEMORY_NAME
		);
		//	Igor: Reserve 1 byte for xrMemory header
		//	Don't bother in this case?
		mem_copy(p_new_mem_alloc_old, p_old, _min(s_current - 1, s_dest));
		//mem_copy				(p_new,p_old,_min(s_current,s_dest));
		mem_free(p_old);
		_ptr = p_new_mem_alloc_old;
	}
	else if (2 == p_mode) {
		// relocate into another mmgr(pooled) from real
		void* p_old = P;
		void* p_new_mem_alloc = mem_alloc(size
#	ifdef DEBUG_MEMORY_NAME
			, _name
#	endif // DEBUG_MEMORY_NAME
		);
		mem_copy(p_new_mem_alloc, p_old, (u32)size);
		mem_free(p_old);
		_ptr = p_new_mem_alloc;
	}

	ASAN_UNPOISON_MEMORY_REGION(_ptr, sizeof _ptr);

	return	_ptr;
}

#endif // __BORLANDC__
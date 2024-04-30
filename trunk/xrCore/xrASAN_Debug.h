#pragma once

//#define __ASAN_ENABLE_OPERATION_MEMORY__

#if defined(DEBUG) && defined(__ASAN_ENABLE_OPERATION_MEMORY__)
#define __ASAN_ENABLE__
#endif

#ifdef __ASAN_ENABLE__
#include <sanitizer/asan_interface.h>
#define ASAN_POISON_MEMORY_REGION(pAddr, size) __asan_poison_memory_region((pAddr), (size))
#define ASAN_UNPOISON_MEMORY_REGION(pAddr, size) __asan_unpoison_memory_region((pAddr), (size))
#else
#define ASAN_POISON_MEMORY_REGION(pAddr, size)
#define ASAN_UNPOISON_MEMORY_REGION(pAddr, size)
#endif
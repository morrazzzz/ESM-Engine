#include "stdafx.h"
#pragma hdrstop

#pragma warning(disable:4995)
void	__stdcall xrMemCopy_x86					(LPVOID dest, const void* src, u32 n)
{
	memcpy		(dest,src,n);
}

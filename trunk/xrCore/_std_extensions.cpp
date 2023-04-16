#include "stdafx.h"
#pragma hdrstop

#include <codecvt>
#include <locale>
#include <time.h>

#ifdef BREAK_AT_STRCMP
int								xr_strcmp				( const char* S1, const char* S2 )
{
#ifdef DEBUG_MEMORY_MANAGER
	Memory.stat_strcmp	++;
#endif // DEBUG_MEMORY_MANAGER
	int res				= (int)strcmp(S1,S2);
	return				res;
}
#endif

char*							timestamp				(string64& dest)
{
	string64	temp;

	/* Set time zone from TZ environment variable. If TZ is not set,
	* the operating system is queried to obtain the default value 
	* for the variable. 
	*/
	_tzset		();
	u32			it;

	// date
	_strdate	( temp );
	for (it=0; it<xr_strlen(temp); it++)
		if ('/'==temp[it]) temp[it]='-';
	strconcat	(sizeof(dest), dest, temp, "_" );

	// time
	_strtime	( temp );
	for (it=0; it<xr_strlen(temp); it++)
		if (':'==temp[it]) temp[it]='-';
	strcat		( dest, temp);
	return dest;
}

std::string StringToUTF8(const char* in)
{
	const size_t len = strlen(in);
	static const std::locale locale{ "" };
	using wcvt = std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t>;
	std::wstring wstr(len, L'\0');
	std::use_facet<std::ctype<wchar_t>>(locale).widen(in, in + len, wstr.data());
	return wcvt{}.to_bytes(wstr.data(), wstr.data() + wstr.size());
}
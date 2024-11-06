#pragma once

IC xr_string __cdecl make_string(LPCSTR string, ...)
{
	va_list	args;
	va_start(args, string);

	char temp[4096];
	vsprintf(temp, string, args);

	va_end(args);

	return temp;
}

#ifdef DEBUG
XRCORE_API xr_string get_string(bool v);
XRCORE_API xr_string get_string(const Fvector& v);
XRCORE_API xr_string get_string(const Fmatrix& dop);
XRCORE_API xr_string get_string(const Fbox& box);

XRCORE_API xr_string dump_string(LPCSTR name, const Fvector& v);
XRCORE_API xr_string dump_string(LPCSTR name, const Fmatrix& form);
XRCORE_API void dump(LPCSTR name, const Fmatrix& form);
XRCORE_API void dump(LPCSTR name, const Fvector& v);
#endif
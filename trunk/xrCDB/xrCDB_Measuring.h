#pragma once

#include "xrCDB.h"

class CGameFont;

class XRCDB_API CDBMeasuring
{
public:
#ifdef DEBUG	
	CStatTimer cdb_clRAY;
	CStatTimer cdb_clBOX;
	CStatTimer cdb_clFRUSTUM;
#endif

	CDBMeasuring() = default;
	~CDBMeasuring() = default;
};

extern XRCDB_API CDBMeasuring StatsCDB;

#ifdef DEBUG
#define CDB_RAY_TIMER(a) StatsCDB.cdb_clRAY.a
#define CDB_BOX_TIMER(a) StatsCDB.cdb_clBOX.a
#define CDB_FRUSTUM_TIMER(a) StatsCDB.cdb_clFRUSTUM.a
#else
#define CDB_RAY_TIMER(a)
#define CDB_BOX_TIMER(a)
#define CDB_FRUSTUM_TIMER(a)
#endif
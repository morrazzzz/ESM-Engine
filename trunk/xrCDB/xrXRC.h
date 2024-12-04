#pragma once

#include "xrCDB.h"
#include "xrCDB_Measuring.h"

class XRCDB_API xrXRC
{
	CDB::COLLIDER CL;
public:
	IC void	ray_options(u8 option)
	{
		CL.ray_options(option);
	}

	IC void ray_query(const CDB::MODEL* m_def, const Fvector& r_start, const Fvector& r_dir, float r_range = 10000.f)
	{
		CDB_RAY_TIMER(Begin());

		CL.ray_query(m_def, r_start, r_dir, r_range);

		CDB_RAY_TIMER(End());
	}

	IC void	box_options(u8 option)
	{
		CL.box_options(option);
	}

	IC void	box_query(const CDB::MODEL* m_def, const Fvector& b_center, const Fvector& b_dim)
	{
		CDB_BOX_TIMER(Begin());

		CL.box_query(m_def, b_center, b_dim);

		CDB_BOX_TIMER(End());
	}

	IC void frustum_options(u8 option)
	{
		CL.frustum_options(option);
	}

	IC void frustum_query(const CDB::MODEL* m_def, const CFrustum& F)
	{
		CDB_FRUSTUM_TIMER(Begin());

		CL.frustum_query(m_def, F);

		CDB_FRUSTUM_TIMER(End());
	}

	CDB::RESULT& operator[](size_t index) { return CL.r_index(index); }

	IC CDB::RESULT* r_begin() { return CL.r_begin(); };
	IC CDB::RESULT* r_end() { return CL.r_end(); };
	IC void	r_free() { CL.r_free(); }
	IC size_t r_count() { return CL.r_count(); };
	IC void	r_clear() { CL.r_clear(); };
	IC void	r_clear_compact() { CL.r_clear_compact(); };

	xrXRC() = default;
	~xrXRC() = default;
};
XRCDB_API extern xrXRC XRC;

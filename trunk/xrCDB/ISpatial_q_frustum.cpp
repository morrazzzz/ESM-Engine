#include "stdafx.h"
#include "ISpatial.h"
#include "frustum.h"

extern Fvector c_spatial_offset[8];

class walker
{
public:
	u32	mask;
	CFrustum* F;

	walker(u32 _mask, const CFrustum* _F)
	{
		mask = _mask;
		F = const_cast<CFrustum*>(_F);
	}

	void walk(xr_vector<ISpatial*>& spatial_result, ISpatial_NODE* N,  Fvector& n_C, float n_R, u32 fmask)
	{
		// box
		float	n_vR = 2 * n_R;
		Fbox BB;		
		BB.set(n_C.x - n_vR, n_C.y - n_vR, n_C.z - n_vR, n_C.x + n_vR, n_C.y + n_vR, n_C.z + n_vR);
		if (fcvNone == F->testAABB(BB.data(), fmask))	
			return;

		// test items
		for (u32 i = 0; i < N->items.size(); i++)
		{
			ISpatial* S = N->items[i];
			if (!(S->spatial.type & mask))	
				continue;

			Fvector& sC = S->spatial.sphere.P;
			float			sR = S->spatial.sphere.R;
			u32				tmask = fmask;
			if (fcvNone == F->testSphere(sC, sR, tmask))	
				continue;

			spatial_result.push_back(S);
		}

		// recurse
		float c_R = n_R / 2;
		for (u32 octant = 0; octant < 8; octant++)
		{
			if (!N->children[octant])	
				continue;
			Fvector	c_C;			
			c_C.mad(n_C, c_spatial_offset[octant], c_R);
			walk(spatial_result, N->children[octant], c_C, c_R, fmask);
		}
	}
};

void ISpatial_DB::q_frustum(xr_vector<ISpatial*>& R, u32 _o, u32 _mask, const CFrustum& _frustum)
{
	R.clear_not_free();

	walker W(_mask, &_frustum);
	W.walk(R, m_root, m_center, m_bounds, _frustum.getMask());
}

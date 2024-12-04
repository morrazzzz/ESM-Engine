#include "stdafx.h"
#include "ISpatial.h"

extern Fvector	c_spatial_offset[8];

class walker
{
public:
	u32 mask;
	Fvector	center;
	Fvector	size;
	Fbox box;

	walker(u32 _mask, const Fvector& _center, const Fvector& _size)
	{
		mask = _mask;
		center = _center;
		size = _size;
		box.setb(center, size);
	}

	void walk(xr_vector<ISpatial*>& result_spatial, ISpatial_NODE* N, Fvector& n_C, float n_R, bool first)
	{
		// box
		float n_vR = 2 * n_R;
		Fbox BB;	
		BB.set(n_C.x - n_vR, n_C.y - n_vR, n_C.z - n_vR, n_C.x + n_vR, n_C.y + n_vR, n_C.z + n_vR);
		if (!BB.intersect(box))			
			return;

		// test items
		for (u32 i = 0; i < N->items.size(); i++)
		{
			ISpatial* S = N->items[i];
			if (!(S->spatial.type & mask))	
				continue;

			Fvector& sC = S->spatial.sphere.P;
			float			sR = S->spatial.sphere.R;
			Fbox			sB;		sB.set(sC.x - sR, sC.y - sR, sC.z - sR, sC.x + sR, sC.y + sR, sC.z + sR);
			if (!sB.intersect(box))	
				continue;

			result_spatial.push_back(S);
			if (first)			
				return;
		}

		// recurse
		float	c_R = n_R / 2;
		for (u32 octant = 0; octant < 8; octant++)
		{
			if (0 == N->children[octant])	continue;
			Fvector		c_C;			c_C.mad(n_C, c_spatial_offset[octant], c_R);
			walk(result_spatial, N->children[octant], c_C, c_R, first);
			if (first && !result_spatial.empty())
				return;
		}
	}
};

void ISpatial_DB::q_box(xr_vector<ISpatial*>& R, u32 _o, u32 _mask, const Fvector& _center, const Fvector& _size)
{
	R.clear_not_free();

	walker W(_mask, _center, _size);
	W.walk(R, m_root, m_center, m_bounds, _o & O_ONLYFIRST);
}

void ISpatial_DB::q_sphere(xr_vector<ISpatial*>& R, u32 _o, u32 _mask, const Fvector& _center, const float _radius)
{
	Fvector	_size = { _radius,_radius,_radius };
	q_box(R, _o, _mask, _center, _size);
}

#include "stdafx.h"
#include "xrCDB.h"
#include "frustum.h"

using namespace CDB;
using namespace Opcode;

template <bool bClass3, bool bFirst>
class frustum_collider
{
public:
	COLLIDER*		dest;
	TRI*			tris;
	Fvector*		verts;
	
	const CFrustum*	F;
	
	IC void			_init		(COLLIDER* CL, Fvector* V, TRI* T, const CFrustum* _F)
	{
		dest		= CL;
		tris		= T;
		verts		= V;
		F			= _F;
	}
	IC EFC_Visible	_box		(Fvector& C, Fvector& E, u32& mask)
	{
		Fvector		mM[2];
		mM[0].sub	(C,E);
		mM[1].add	(C,E);
		return F->testAABB		(&mM[0].x,mask);
	}
	void			_prim		(DWORD prim)
	{
		RESULT result;
		if (bClass3)	{
			sPoly		src,dst;
			src.resize	(3);
			src[0]		= verts[ tris[prim].verts[0] ];
			src[1]		= verts[ tris[prim].verts[1] ];
			src[2]		= verts[ tris[prim].verts[2] ];
			if (F->ClipPoly(src,dst))
			{
				result.id		= prim;
				result.verts[0]	= verts[ tris[prim].verts[0] ];
				result.verts[1]	= verts[ tris[prim].verts[1] ];
				result.verts[2]	= verts[ tris[prim].verts[2] ];
				result.dummy		= tris[prim].dummy;
			}
		} else {
			result.id		= prim;
			result.verts[0]	= verts[ tris[prim].verts[0] ];
			result.verts[1]	= verts[ tris[prim].verts[1] ];
			result.verts[2]	= verts[ tris[prim].verts[2] ];
			result.dummy		= tris[prim].dummy;
		}

		dest->r_add(result);
	}
	
	void			_stab		(const AABBNoLeafNode* node, u32 mask)
	{
		// Actual frustum/aabb test
		EFC_Visible	result		= _box((Fvector&)node->mAABB.mCenter,(Fvector&)node->mAABB.mExtents,mask);
		if (fcvNone == result)	return;
		
		// 1st chield
		if (node->HasLeaf())	_prim	(node->GetPrimitive());
		else					_stab	(node->GetPos(),mask);
		
		// Early exit for "only first"
		if (bFirst && dest->r_count())												return;
		
		// 2nd chield
		if (node->HasLeaf2())	_prim	(node->GetPrimitive2());
		else					_stab	(node->GetNeg(),mask);
	}
};

void COLLIDER::frustum_query(const MODEL *m_def, const CFrustum& F)
{
	// Get nodes
	const AABBNoLeafTree*	T	= (const AABBNoLeafTree*)m_def->tree->GetTree();
	const AABBNoLeafNode*	N	= T->GetNodes();
	const DWORD				mask= F.getMask();
	r_clear					();
	
	// Binary dispatcher
	if (frustum_mode&OPT_FULL_TEST) 
	{
		if (frustum_mode&OPT_ONLYFIRST)
		{
			frustum_collider<true,true> BC;
			BC._init	(this,m_def->verts,m_def->tris,&F);
			BC._stab	(N,mask);
		} else {
			frustum_collider<true,false> BC;
			BC._init	(this,m_def->verts,m_def->tris,&F);
			BC._stab	(N,mask);
		}
	} else {
		if (frustum_mode&OPT_ONLYFIRST)
		{
			frustum_collider<false,true> BC;
			BC._init	(this,m_def->verts,m_def->tris,&F);
			BC._stab	(N,mask);
		} else {
			frustum_collider<false,false> BC;
			BC._init	(this,m_def->verts,m_def->tris,&F);
			BC._stab	(N,mask);
		}
	}
}

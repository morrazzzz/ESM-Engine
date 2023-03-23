#include "stdafx.h"

#include "../xrRender/du_cone.h"

void CRenderTarget::accum_spot_geom_create	()
{
	u32	dwUsage				= D3DUSAGE_WRITEONLY;

	// vertices
	{
		u32		vCount		= DU_CONE_NUMVERTEX;
		u32		vSize		= 3*4;
		R_CHK	(HW.pDevice->CreateVertexBuffer(
			vCount*vSize,
			dwUsage,
			0,
			D3DPOOL_MANAGED,
			&g_accum_spot_vb,
			0));
		BYTE*	pData				= 0;
		R_CHK						(g_accum_spot_vb->Lock(0,0,(void**)&pData,0));
		CopyMemory				(pData,du_cone_vertices,vCount*vSize);
		g_accum_spot_vb->Unlock	();
	}

	// Indices
	{
		u32		iCount		= DU_CONE_NUMFACES*3;

		BYTE*	pData		= 0;
		R_CHK				(HW.pDevice->CreateIndexBuffer(iCount*2,dwUsage,D3DFMT_INDEX16,D3DPOOL_MANAGED,&g_accum_spot_ib,0));
		R_CHK				(g_accum_spot_ib->Lock(0,0,(void**)&pData,0));
		CopyMemory		(pData,du_cone_faces,iCount*2);
		g_accum_spot_ib->Unlock	();
	}
}

void CRenderTarget::accum_spot_geom_destroy()
{
#ifdef DEBUG
	_SHOW_REF	("g_accum_spot_ib",g_accum_spot_ib);
#endif // DEBUG
	_RELEASE	(g_accum_spot_ib);
#ifdef DEBUG
	_SHOW_REF	("g_accum_spot_vb",g_accum_spot_vb);
#endif // DEBUG
	_RELEASE	(g_accum_spot_vb);
}

#include "stdafx.h"
#include "xr_area.h"
#include "../xr_3da/xr_object.h"
#include "../xr_3da/xrLevel.h"
#include "../xr_3da/xr_collide_form.h"

using namespace CObjectSpaceThreadSafe;
using namespace	collide;

//----------------------------------------------------------------------
// Class	: CObjectSpace
// Purpose	: stores space slots
//----------------------------------------------------------------------
CObjectSpace::CObjectSpace	( )
#ifdef PROFILE_CRITICAL_SECTIONS
	:Lock(MUTEX_PROFILE_ID(CObjectSpace::Lock))
#endif // PROFILE_CRITICAL_SECTIONS
{
	m_BoundingVolume.invalidate	();
}
//----------------------------------------------------------------------
CObjectSpace::~CObjectSpace	( )
{
//	Sound->set_geometry_occ		(nullptr);
//	Sound->set_handler			(nullptr);
}

//----------------------------------------------------------------------
int CObjectSpace::GetNearest		( xr_vector<ISpatial*>& q_spatial, xr_vector<CObject*>&	q_nearest, const Fvector &point, float range, CObject* ignore_object )
{
	// Query objects
	q_nearest.clear_not_free		( );
	Fsphere				Q;	Q.set	(point,range);
	Fvector				B;	B.set	(range,range,range);
	g_SpatialSpace->q_box(r_spatial,0,STYPE_COLLIDEABLE,point,B);

	// Iterate
	for (u32 i = 0; i < r_spatial.size(); i++)
	{
		CObject* O = r_spatial[i]->dcast_CObject();
		if (0 == O)				
			continue;
		if (O == ignore_object)
			continue;
		Fsphere mS = { O->spatial.sphere.P, O->spatial.sphere.R };
		if (Q.intersect(mS))	q_nearest.push_back(O);
	}

	return q_nearest.size();
}

//----------------------------------------------------------------------
IC int	CObjectSpace::GetNearest(xr_vector<CObject*>& q_nearest, const Fvector& point, float range, CObject* ignore_object)
{
	return							(
		GetNearest(
			r_spatial,
			q_nearest,
			point,
			range,
			ignore_object
		)
		);
}

//----------------------------------------------------------------------
IC int   CObjectSpace::GetNearest( xr_vector<CObject*>&	q_nearest, ICollisionForm* obj, float range)
{
	CObject*	O		= obj->Owner	();
	return				GetNearest( q_nearest, O->spatial.sphere.P, range + O->spatial.sphere.R, O );
}

//----------------------------------------------------------------------


void CObjectSpace::Load(CDB::build_callback build_callback)
{
	Load("$level$", "level.cform", build_callback);
}
void	CObjectSpace::Load(LPCSTR path, LPCSTR fname, CDB::build_callback build_callback)
{
#ifdef USE_ARENA_ALLOCATOR
	Msg("CObjectSpace::Load, g_collision_allocator.get_allocated_size() - %d", int(g_collision_allocator.get_allocated_size() / 1024.0 / 1024));
#endif // #ifdef USE_ARENA_ALLOCATOR
	IReader* F = FS.r_open(path, fname);
	R_ASSERT(F);
	Load(F, build_callback);
}
void	CObjectSpace::Load(IReader* F, CDB::build_callback build_callback)


{
	hdrCFORM					H;
	F->r(&H, sizeof(hdrCFORM));
	Fvector* verts = (Fvector*)F->pointer();
	CDB::TRI* tris = (CDB::TRI*)(verts + H.vertcount);
	Create(verts, tris, H, build_callback);
	FS.r_close(F);
}

void			CObjectSpace::Create				(  Fvector*	verts, CDB::TRI* tris, const hdrCFORM &H, CDB::build_callback build_callback  )
{
	R_ASSERT							(CFORM_CURRENT_VERSION==H.version);
	Static.build						( verts, H.vertcount, tris, H.facecount, build_callback );
	m_BoundingVolume.set				(H.aabb);
	g_SpatialSpace->initialize			(m_BoundingVolume);
	g_SpatialSpacePhysic->initialize	(m_BoundingVolume);
	//Sound->set_geometry_occ				( &Static );
	//Sound->set_handler					( _sound_event );
}

//----------------------------------------------------------------------
#ifdef DEBUG
void CObjectSpace::dbgRender()
{
	m_pRender->dbgRender();
}
#endif

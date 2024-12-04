#pragma once

#include "xr_collide_defs.h"

// refs
class ISpatial;
class ICollisionForm;
class CObject;

#include "../Include/xrRender/FactoryPtr.h"
#include "../Include/xrRender/ObjectSpaceRender.h"
#include "xrXRC.h"

#include "xrcdb.h"

namespace CObjectSpaceThreadSafe
{
	IC thread_local xrXRC xrc; // MT: dangerous
	IC thread_local collide::rq_results r_temp; // MT: dangerous
	IC thread_local xr_vector<ISpatial*> r_spatial; // MT: dangerous
}

//-----------------------------------------------------------------------------------------------------------
//Space Area
//-----------------------------------------------------------------------------------------------------------
struct hdrCFORM;
class XRCDB_API CObjectSpace
{
private:
	// Debug
	CDB::MODEL Static;
	Fbox m_BoundingVolume;
public:

#ifdef DEBUG
	FactoryPtr<IObjectSpaceRender> m_pRender;
#endif

private:
	bool _RayTest(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::ray_cache* cache, CObject* ignore_object);
	bool _RayPick(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::rq_result& R, CObject* ignore_object);
	bool _RayQuery2(collide::rq_results& dest, const collide::ray_defs& rq, collide::rq_callback* cb, LPVOID user_data, collide::test_callback* tb, CObject* ignore_object);
public:
	CObjectSpace();
	~CObjectSpace();

	void Load(CDB::build_callback build_callback);
	void Load(LPCSTR path, LPCSTR fname, CDB::build_callback build_callback);
	void Load(IReader* R, CDB::build_callback build_callback);
	void Create(Fvector* verts, CDB::TRI* tris, const hdrCFORM& H, CDB::build_callback build_callback);

	// Occluded/No
	bool RayTest(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::ray_cache* cache, CObject* ignore_object);

	// Game raypick (nearest) - returns object and addititional params
	bool  RayPick(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::rq_result& R, CObject* ignore_object);

	// General collision query
    bool RayQuery(collide::rq_results& dest, const collide::ray_defs& rq, collide::rq_callback* cb, LPVOID user_data, collide::test_callback* tb, CObject* ignore_object);
	bool RayQuery(collide::rq_results& dest, ICollisionForm* target, const collide::ray_defs& rq);

	int GetNearest(xr_vector<CObject*>& q_nearest, ICollisionForm* obj, float range);
	int GetNearest(xr_vector<CObject*>& q_nearest, const Fvector& point, float range, CObject* ignore_object);
	int	GetNearest(xr_vector<ISpatial*>& q_spatial, xr_vector<CObject*>& q_nearest, const Fvector& point, float range, CObject* ignore_object);

	CDB::TRI* GetStaticTris() { return Static.get_tris(); }
	Fvector* GetStaticVerts() { return Static.get_verts(); }
	CDB::MODEL* GetStaticModel() { return &Static; }

	const Fbox& GetBoundingVolume() { return m_BoundingVolume; }

	// Debugging
#ifdef DEBUG
	void								dbgRender();
#endif
};

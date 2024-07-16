#include "stdafx.h"
#include "xr_area.h"
#include "../xr_3da/xr_collide_form.h"
#include "../xr_3da/xr_object.h"
#include "../xr_3da/cl_intersect.h"
#ifdef	DEBUG
XRCDB_API bool cdb_bDebug = false;
bool bDebug()
{
	return cdb_bDebug;
}
#endif
using namespace	collide;

//--------------------------------------------------------------------------------
// RayTest - Occluded/No
//--------------------------------------------------------------------------------
bool CObjectSpace::RayTest(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::ray_cache* cache, CObject* ignore_object)
{
	Lock.Enter();
	bool _ret = _RayTest(start, dir, range, tgt, cache, ignore_object);
	r_spatial.clear();
	Lock.Leave();
	return _ret;
}
bool CObjectSpace::_RayTest(const Fvector& start, const Fvector& dir, float range, collide::rq_target tgt, collide::ray_cache* cache, CObject* ignore_object)
{
	VERIFY(_abs(dir.magnitude() - 1) < EPS);
	r_temp.r_clear();

	xrc.ray_options(CDB::OPT_ONLYFIRST);
	collide::ray_defs	Q(start, dir, range, CDB::OPT_ONLYFIRST, tgt);

	// dynamic test
	if (tgt & rqtDyn) {
		u32			d_flags = STYPE_COLLIDEABLE | ((tgt & rqtObstacle) ? STYPE_OBSTACLE : 0) | ((tgt & rqtShape) ? STYPE_SHAPE : 0);
		// traverse object database
		g_SpatialSpace->q_ray(r_spatial, 0, d_flags, start, dir, range);
		// Determine visibility for dynamic part of scene
		for (u32 o_it = 0; o_it < r_spatial.size(); o_it++)
		{
			ISpatial* spatial = r_spatial[o_it];
			CObject* collidable = spatial->dcast_CObject();
			if (collidable && (collidable != ignore_object)) {
				ECollisionFormType tp = collidable->collidable.model->Type();
				if ((tgt & (rqtObject | rqtObstacle)) && (tp == cftObject) && collidable->collidable.model->_RayQuery(Q, r_temp))	return TRUE;
				if ((tgt & rqtShape) && (tp == cftShape) && collidable->collidable.model->_RayQuery(Q, r_temp))		return TRUE;
			}
		}
	}
	// static test
	if (tgt & rqtStatic) {
		// If we get here - test static model
		if (cache)
		{
			// 0. similar query???
			if (cache->similar(start, dir, range)) {
				return cache->result;
			}

			// 1. Check cached polygon
			float _u, _v, _range;
			if (CDB::TestRayTri(start, dir, cache->verts, _u, _v, _range, false))
			{
				if (_range > 0 && _range < range) return TRUE;
			}

			// 2. Polygon doesn't pick - real database query
			xrc.ray_query(&Static, start, dir, range);
			if (0 == xrc.r_count()) {
				cache->set(start, dir, range, FALSE);
				return FALSE;
			}
			else {
				// cache polygon
				cache->set(start, dir, range, TRUE);
				CDB::RESULT* R = xrc.r_begin();
				CDB::TRI& T = Static.get_tris()[R->id];
				Fvector* V = Static.get_verts();
				cache->verts[0].set(V[T.verts[0]]);
				cache->verts[1].set(V[T.verts[1]]);
				cache->verts[2].set(V[T.verts[2]]);
				return TRUE;
			}
		}
		else {
			xrc.ray_query(&Static, start, dir, range);
			return static_cast<bool>(xrc.r_count());
		}
	}
	return FALSE;
}

//--------------------------------------------------------------------------------
// RayPick
//--------------------------------------------------------------------------------
bool CObjectSpace::RayPick(const Fvector& start, const Fvector& dir, float range, rq_target tgt, rq_result& R, CObject* ignore_object)
{
	Lock.Enter();
	bool _res = _RayPick(start, dir, range, tgt, R, ignore_object);
	r_spatial.clear();
	Lock.Leave();
	return _res;
}
bool CObjectSpace::_RayPick(const Fvector& start, const Fvector& dir, float range, rq_target tgt, rq_result& R, CObject* ignore_object)
{
	r_temp.r_clear();
	R.O = 0; R.range = range; R.element = -1;
	// static test
	if (tgt & rqtStatic) {
		xrc.ray_options(CDB::OPT_ONLYNEAREST | CDB::OPT_CULL);
		xrc.ray_query(&Static, start, dir, range);
		if (xrc.r_count())  R.set_if_less(xrc.r_begin());
	}
	// dynamic test
	if (tgt & rqtDyn) {
		collide::ray_defs Q(start, dir, R.range, CDB::OPT_ONLYNEAREST | CDB::OPT_CULL, tgt);
		// traverse object database
		u32			d_flags = STYPE_COLLIDEABLE | ((tgt & rqtObstacle) ? STYPE_OBSTACLE : 0) | ((tgt & rqtShape) ? STYPE_SHAPE : 0);
		g_SpatialSpace->q_ray(r_spatial, 0, d_flags, start, dir, range);
		// Determine visibility for dynamic part of scene
		for (u32 o_it = 0; o_it < r_spatial.size(); o_it++) {
			ISpatial* spatial = r_spatial[o_it];
			CObject* collidable = spatial->dcast_CObject();
			if (0 == collidable)				continue;
			if (collidable == ignore_object)	continue;
			ECollisionFormType tp = collidable->collidable.model->Type();
			if (((tgt & (rqtObject | rqtObstacle)) && (tp == cftObject)) || ((tgt & rqtShape) && (tp == cftShape))) {
				u32		C = color_xrgb(64, 64, 64);
				Q.range = R.range;
				if (collidable->collidable.model->_RayQuery(Q, r_temp)) {
					C = color_xrgb(128, 128, 196);
					R.set_if_less(r_temp.r_begin());
				}
#ifdef DEBUG
				if (bDebug()) {
					Fsphere	S;		S.P = spatial->spatial.sphere.P; S.R = spatial->spatial.sphere.R;
					m_pRender->dbgAddSphere(S, C);
				}
#endif
			}
		}
	}
	return R.element >= 0;
}

//--------------------------------------------------------------------------------
// RayQuery
//--------------------------------------------------------------------------------
bool CObjectSpace::RayQuery(collide::rq_results& dest, const collide::ray_defs& R, collide::rq_callback* CB, LPVOID user_data, collide::test_callback* tb, CObject* ignore_object)
{
	Lock.Enter();
	bool res = _RayQuery2(dest, R, CB, user_data, tb, ignore_object);
	r_spatial.clear_not_free();
	Lock.Leave();
	return res;
}
bool CObjectSpace::_RayQuery2(collide::rq_results& r_dest, const collide::ray_defs& R, collide::rq_callback* CB, LPVOID user_data, collide::test_callback* tb, CObject* ignore_object)
{
	// initialize query
	r_dest.r_clear();
	r_temp.r_clear();

	rq_target	s_mask = rqtStatic;
	rq_target	d_mask = rq_target(((R.tgt & rqtObject) ? rqtObject : rqtNone) |
		((R.tgt & rqtObstacle) ? rqtObstacle : rqtNone) |
		((R.tgt & rqtShape) ? rqtShape : rqtNone));
	u32			d_flags = STYPE_COLLIDEABLE | ((R.tgt & rqtObstacle) ? STYPE_OBSTACLE : 0) | ((R.tgt & rqtShape) ? STYPE_SHAPE : 0);

	// Test static
	if (R.tgt & s_mask) {
		xrc.ray_options(R.flags);
		xrc.ray_query(&Static, R.start, R.dir, R.range);
		if (xrc.r_count()) {
			CDB::RESULT* _I = xrc.r_begin();
			CDB::RESULT* _E = xrc.r_end();
			for (; _I != _E; _I++)
				r_temp.append_result(rq_result().set(0, _I->range, _I->id));
		}
	}
	// Test dynamic
	if (R.tgt & d_mask) {
		// Traverse object database
		g_SpatialSpace->q_ray(r_spatial, 0, d_flags, R.start, R.dir, R.range);
		for (u32 o_it = 0; o_it < r_spatial.size(); o_it++) {
			CObject* collidable = r_spatial[o_it]->dcast_CObject();
			if (0 == collidable)				continue;
			if (collidable == ignore_object)	continue;
			ICollisionForm* cform = collidable->collidable.model;
			ECollisionFormType tp = collidable->collidable.model->Type();
			if (((R.tgt & (rqtObject | rqtObstacle)) && (tp == cftObject)) || ((R.tgt & rqtShape) && (tp == cftShape))) {
				if (tb && !tb(R, collidable, user_data))continue;
				cform->_RayQuery(R, r_temp);
			}
		}
	}
	if (r_temp.r_count()) {
		r_temp.r_sort();
		collide::rq_result* _I = r_temp.r_begin();
		collide::rq_result* _E = r_temp.r_end();
		for (; _I != _E; _I++) {
			r_dest.append_result(*_I);
			if (!(CB ? CB(*_I, user_data) : true))						
				return r_dest.r_count();
			if (R.flags & (CDB::OPT_ONLYNEAREST | CDB::OPT_ONLYFIRST))	
				return r_dest.r_count();
		}
	}
	return r_dest.r_count();
}

bool CObjectSpace::RayQuery(collide::rq_results& r_dest, ICollisionForm* target, const collide::ray_defs& R)
{
	VERIFY(target);
	r_dest.r_clear();
	return target->_RayQuery(R, r_dest);
}

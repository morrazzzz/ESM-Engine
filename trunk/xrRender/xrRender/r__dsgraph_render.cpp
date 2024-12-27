#include "stdafx.h"

#include "../../xr_3da/render.h"
#include "../../xr_3da/irenderable.h"
#include "../../xr_3da/igame_persistent.h"
#include "../../xr_3da/environment.h"
#include "../../xr_3da/CustomHUD.h"

using namespace		R_dsgraph;

extern float		r_ssaDISCARD;
extern float		r_ssaDONTSORT;
extern float		r_ssaHZBvsTEX;
extern float		r_ssaGLOD_start,	r_ssaGLOD_end;

ICF float calcLOD	(float ssa/*fDistSq*/)
{
	return			_sqrt(clampr((ssa - r_ssaGLOD_end)/(r_ssaGLOD_start-r_ssaGLOD_end),0.f,1.f));
}

void __fastcall mapNormal_Render(mapNormalItems& N)
{
	// *** DIRECT ***
	for (u32 i = 0; i < N.size(); i++) 
	{
		_NormalItem& Ni = N[i];
		float LOD = calcLOD(Ni.ssa);
#ifdef USE_DX11
		RCache.LOD.set_LOD(LOD);
#endif
		Ni.pVisual->Render(LOD);
	}
}


// Matrix
void __fastcall mapMatrix_Render	(mapMatrixItems& N)
{
	// *** DIRECT ***
	for (u32 i = 0; i < N.size(); i++) 
	{
		_MatrixItem&	Ni				= N[i];
		RCache.set_xform_world			(Ni.Matrix);
		Device.Statistic->RenderDUMP.Begin();
		RImplementation.apply_object	(Ni.pObject);
		RImplementation.apply_lmaterial	();
		Device.Statistic->RenderDUMP.End();
		
		float LOD = calcLOD(Ni.ssa);
#ifdef USE_DX11
		RCache.LOD.set_LOD(LOD);
#endif
		Ni.pVisual->Render(LOD);
	}
	N.clear	();
}

// ALPHA
void __fastcall sorted_L1		(mapSorted_Node *N)
{
	VERIFY (N);
	dxRender_Visual *V				= N->val.pVisual;
	VERIFY (V && V->shader._get());
	RCache.set_Element				(N->val.se);
	RCache.set_xform_world			(N->val.Matrix);
	RImplementation.apply_object	(N->val.pObject);
	RImplementation.apply_lmaterial	();
	V->Render						(calcLOD(N->key));
}

void R_dsgraph_structure::r_dsgraph_render_graph	(u32	_priority, bool _clear)
{

	//PIX_EVENT(r_dsgraph_render_graph);

	// **************************************************** NORMAL
	// Perform sorting based on ScreenSpaceArea
	// Sorting by SSA and changes minimizations
	{
		RCache.set_xform_world(Fidentity);

		// Render several passes
		for (u32 iPass = 0; iPass < SHADER_PASSES_MAX; ++iPass)
		{
			mapNormalVS& vs_normal = mapNormalPasses[_priority][iPass];
			for (u32 vs_id = 0; vs_id < vs_normal.size(); vs_id++)
			{
				mapNormalVS::TNode& Nvs = vs_normal[vs_id];
				RCache.set_VS(Nvs.key);


#if defined(USE_DX10) || defined(USE_DX11)
				//	GS setup
				mapNormalGS& gs = Nvs.val;
				gs.ssa = 0;

				for (u32 gs_id = 0; gs_id < gs.size(); gs_id++)
				{
					mapNormalGS::TNode& Ngs = gs[gs_id];
					RCache.set_GS(Ngs.key);

					mapNormalPS& ps = Ngs.val;
					ps.ssa = 0;
#else	//	USE_DX10
				mapNormalPS& ps = Nvs.val;
				ps.ssa = 0;
#endif	//	USE_DX10

				for (u32 ps_id = 0; ps_id < ps.size(); ps_id++)
				{
					mapNormalPS::TNode& Nps = ps[ps_id];
					RCache.set_PS(Nps.key);
#ifdef USE_DX11
					mapNormalCS& cs = Nps.val.mapCS;		cs.ssa = 0;
					RCache.set_HS(Nps.val.hs);
					RCache.set_DS(Nps.val.ds);
#else
					mapNormalCS& cs = Nps.val;
					cs.ssa = 0;
#endif
					for (u32 cs_id = 0; cs_id < cs.size(); cs_id++)
					{
						mapNormalCS::TNode& Ncs = cs[cs_id];
						RCache.set_Constants(Ncs.key);

						mapNormalStates& states = Ncs.val;
						states.ssa = 0;

						for (u32 state_id = 0; state_id < states.size(); state_id++)
						{
							mapNormalStates::TNode& Nstate = states[state_id];
							RCache.set_States(Nstate.key);

							mapNormalTextures& tex = Nstate.val;
							tex.ssa = 0;
							for (u32 tex_id = 0; tex_id < tex.size(); tex_id++)
							{
								mapNormalTextures::TNode& Ntex = tex[tex_id];
								RCache.set_Textures(Ntex.key);
								RImplementation.apply_lmaterial();

								mapNormalItems& items = Ntex.val;
								items.ssa = 0;
								mapNormal_Render(items);
								if (_clear)				items.clear();
							}
							if (_clear) tex.clear();
						};
						if (_clear) states.clear();
					}
					if (_clear) cs.clear();

				}
				if (_clear) ps.clear();
#if defined(USE_DX10) || defined(USE_DX11)
				}
			if (_clear) gs.clear();
#endif	//	USE_DX10
			}
		if (_clear) vs_normal.clear();

		// **************************************************** MATRIX
		// Perform sorting based on ScreenSpaceArea
		// Sorting by SSA and changes minimizations
		// Render several passes
		mapMatrixVS& vs_matrix = mapMatrixPasses[_priority][iPass];
		for (u32 vs_id = 0; vs_id < vs_matrix.size(); vs_id++) {
			mapMatrixVS::TNode& Nvs_matrix = vs_matrix[vs_id];
			RCache.set_VS(Nvs_matrix.key);

#if defined(USE_DX10) || defined(USE_DX11)
			mapMatrixGS& gs_matrix = Nvs_matrix.val;
			gs_matrix.ssa = 0;

			for (u32 gs_id = 0; gs_id < gs_matrix.size(); gs_id++)
			{
				mapMatrixGS::TNode& Ngs_matrix = gs_matrix[gs_id];
				RCache.set_GS(Ngs_matrix.key);

				mapMatrixPS& ps_matrix = Ngs_matrix.val;
#else	//	USE_DX10
			mapMatrixPS& ps_matrix = Nvs_matrix.val;
#endif	//	USE_DX10
			ps_matrix.ssa = 0;

			for (u32 ps_id = 0; ps_id < ps_matrix.size(); ps_id++)
			{
				mapMatrixPS::TNode& Nps_matrix = ps_matrix[ps_id];
				RCache.set_PS(Nps_matrix.key);

#ifdef USE_DX11
				mapMatrixCS& cs_matrix = Nps_matrix.val.mapCS;
				cs_matrix.ssa = 0;
				RCache.set_HS(Nps_matrix.val.hs);
				RCache.set_DS(Nps_matrix.val.ds);
#else
				mapMatrixCS& cs_matrix = Nps_matrix.val;
				cs_matrix.ssa = 0;
#endif
				for (u32 cs_id = 0; cs_id < cs_matrix.size(); cs_id++)
				{
					mapMatrixCS::TNode& Ncs_matrix = cs_matrix[cs_id];
					RCache.set_Constants(Ncs_matrix.key);

					mapMatrixStates& states_matrix = Ncs_matrix.val;
					states_matrix.ssa = 0;

					for (u32 state_id = 0; state_id < states_matrix.size(); state_id++)
					{
						mapMatrixStates::TNode& Nstate_matrix = states_matrix[state_id];
						RCache.set_States(Nstate_matrix.key);

						mapMatrixTextures& tex_matrix = Nstate_matrix.val;
						tex_matrix.ssa = 0;

						for (u32 tex_id = 0; tex_id < tex_matrix.size(); tex_id++)
						{
							mapMatrixTextures::TNode& Ntex_matrix = tex_matrix[tex_id];
							RCache.set_Textures(Ntex_matrix.key);
							RImplementation.apply_lmaterial();

							mapMatrixItems& items_matrix = Ntex_matrix.val;
							items_matrix.ssa = 0;

							mapMatrix_Render(items_matrix);
						}
						if (_clear) tex_matrix.clear();
					}
					if (_clear) states_matrix.clear();
				}
				if (_clear) cs_matrix.clear();
			}
			if (_clear) ps_matrix.clear();
#if defined(USE_DX10) || defined(USE_DX11)
			}
		if (_clear) gs_matrix.clear();
#endif	//	USE_DX10
		}
	if (_clear) vs_matrix.clear();
		}
	}
}

//////////////////////////////////////////////////////////////////////////
// HUD render
void R_dsgraph_structure::r_dsgraph_render_hud	()
{
	extern ENGINE_API float		psHUD_FOV;

	//PIX_EVENT(r_dsgraph_render_hud);
	
								   

	// Change projection
	Fmatrix Pold				= Device.mProject;
	Fmatrix FTold				= Device.mFullTransform;
	Device.mProject.build_projection(
		deg2rad(psHUD_FOV*Device.fFOV /* *Device.fASPECT*/ ), 
		Device.fASPECT, VIEWPORT_NEAR, 
		g_pGamePersistent->Environment().CurrentEnv->far_plane);

	Device.mFullTransform.mul	(Device.mProject, Device.mView);
	RCache.set_xform_project	(Device.mProject);

	// Rendering
	rmNear						();
	mapHUD.traverseLR			(sorted_L1);
	mapHUD.clear				();
	
#if	RENDER==R_R1
	if (g_hud && g_hud->RenderActiveItemUIQuery())
		r_dsgraph_render_hud_ui						();				// hud ui
#endif
	/*
	if(g_hud && g_hud->RenderActiveItemUIQuery())
	{
#if	RENDER!=R_R1
		// Targets, use accumulator for temporary storage
		const ref_rt	rt_null;
		//	Reset all rt.
		//RCache.set_RT(0,	0);
		RCache.set_RT(0,	1);
		RCache.set_RT(0,	2);
		//if (RImplementation.o.albedo_wo)	RCache.set_RT(RImplementation.Target->rt_Accumulator->pRT,	0);
		//else								RCache.set_RT(RImplementation.Target->rt_Color->pRT,	0);
		if (RImplementation.o.albedo_wo)	RImplementation.Target->u_setrt		(RImplementation.Target->rt_Accumulator,	rt_null,	rt_null,	HW.pBaseZB);
		else								RImplementation.Target->u_setrt		(RImplementation.Target->rt_Color,			rt_null,	rt_null,	HW.pBaseZB);
		//	View port is reset in DX9 when you change rt
		rmNear						();
#endif
		g_hud->RenderActiveItemUI	();

#if	RENDER!=R_R1
		//RCache.set_RT(0,	0);
		// Targets, use accumulator for temporary storage
		if (RImplementation.o.albedo_wo)	RImplementation.Target->u_setrt		(RImplementation.Target->rt_Position,	RImplementation.Target->rt_Normal,	RImplementation.Target->rt_Accumulator,	HW.pBaseZB);
		else								RImplementation.Target->u_setrt		(RImplementation.Target->rt_Position,	RImplementation.Target->rt_Normal,	RImplementation.Target->rt_Color,		HW.pBaseZB);
#endif
	}
	*/

	rmNormal					();

	// Restore projection
	Device.mProject				= Pold;
	Device.mFullTransform		= FTold;
	RCache.set_xform_project	(Device.mProject);
}

void R_dsgraph_structure::r_dsgraph_render_hud_ui()
{
	VERIFY(g_hud && g_hud->RenderActiveItemUIQuery());

	extern ENGINE_API float		psHUD_FOV;

	// Change projection
	Fmatrix Pold				= Device.mProject;
	Fmatrix FTold				= Device.mFullTransform;
	Device.mProject.build_projection(
		deg2rad(psHUD_FOV*Device.fFOV /* *Device.fASPECT*/ ), 
		Device.fASPECT, VIEWPORT_NEAR, 
		g_pGamePersistent->Environment().CurrentEnv->far_plane);

	Device.mFullTransform.mul	(Device.mProject, Device.mView);
	RCache.set_xform_project	(Device.mProject);

#if	RENDER!=R_R1
	// Targets, use accumulator for temporary storage
	const ref_rt	rt_null;
	RCache.set_RT(0,	1);
	RCache.set_RT(0,	2);
#if	(RENDER==R_R3) || (RENDER==R_R4)
	if( !RImplementation.o.dx10_msaa )
	{
		if (RImplementation.o.albedo_wo)	RImplementation.Target->u_setrt		(RImplementation.Target->rt_Accumulator,	rt_null,	rt_null,	HW.pBaseZB);
		else								RImplementation.Target->u_setrt		(RImplementation.Target->rt_Color,			rt_null,	rt_null,	HW.pBaseZB);
	}
	else
	{
		if (RImplementation.o.albedo_wo)	RImplementation.Target->u_setrt		(RImplementation.Target->rt_Accumulator,	rt_null,	rt_null,	RImplementation.Target->rt_MSAADepth->pZRT);
		else								RImplementation.Target->u_setrt		(RImplementation.Target->rt_Color,			rt_null,	rt_null,	RImplementation.Target->rt_MSAADepth->pZRT);
	}
#else // (RENDER==R_R3) || (RENDER==R_R4)
	if (RImplementation.o.albedo_wo)	RImplementation.Target->u_setrt		(RImplementation.Target->rt_Accumulator,	rt_null,	rt_null,	HW.pBaseZB);
	else								RImplementation.Target->u_setrt		(RImplementation.Target->rt_Color,			rt_null,	rt_null,	HW.pBaseZB);
#endif // (RENDER==R_R3) || (RENDER==R_R4)
#endif // RENDER!=R_R1

	rmNear						();
	g_hud->RenderActiveItemUI	();
	rmNormal					();

	// Restore projection
	Device.mProject				= Pold;
	Device.mFullTransform		= FTold;
	RCache.set_xform_project	(Device.mProject);
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void	R_dsgraph_structure::r_dsgraph_render_sorted	()
{
	// Sorted (back to front)
	mapSorted.traverseRL	(sorted_L1);
	mapSorted.clear			();
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void	R_dsgraph_structure::r_dsgraph_render_emissive	()
{
#if	RENDER!=R_R1
	// Sorted (back to front)
	mapEmissive.traverseLR	(sorted_L1);
	mapEmissive.clear		();

	//	HACK: Calculate this only once

	extern ENGINE_API float		psHUD_FOV;

	// Change projection
	Fmatrix Pold				= Device.mProject;
	Fmatrix FTold				= Device.mFullTransform;
	Device.mProject.build_projection(
		deg2rad(psHUD_FOV*Device.fFOV /* *Device.fASPECT*/ ), 
		Device.fASPECT, VIEWPORT_NEAR, 
		g_pGamePersistent->Environment().CurrentEnv->far_plane);

	Device.mFullTransform.mul	(Device.mProject, Device.mView);
	RCache.set_xform_project	(Device.mProject);

	// Rendering
	rmNear						();
	// Sorted (back to front)
	mapHUDEmissive.traverseLR	(sorted_L1);
	mapHUDEmissive.clear		();

	rmNormal					();

	// Restore projection
	Device.mProject				= Pold;
	Device.mFullTransform		= FTold;
	RCache.set_xform_project	(Device.mProject);
#endif
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void	R_dsgraph_structure::r_dsgraph_render_wmarks	()
{
#if	RENDER!=R_R1
	// Sorted (back to front)
	mapWmark.traverseLR	(sorted_L1);
	mapWmark.clear		();
#endif
}

//////////////////////////////////////////////////////////////////////////
// strict-sorted render
void	R_dsgraph_structure::r_dsgraph_render_distort	()
{
	// Sorted (back to front)
	mapDistort.traverseRL	(sorted_L1);
	mapDistort.clear		();
}

//////////////////////////////////////////////////////////////////////////
// sub-space rendering - shortcut to render with frustum extracted from matrix
void	R_dsgraph_structure::r_dsgraph_render_subspace	(IRender_Sector* _sector, Fmatrix& mCombined, Fvector& _cop, BOOL _dynamic, BOOL _precise_portals)
{
	CFrustum	temp;
	temp.CreateFromMatrix			(mCombined,	FRUSTUM_P_ALL &(~FRUSTUM_P_NEAR));
	r_dsgraph_render_subspace		(_sector,&temp,mCombined,_cop,_dynamic,_precise_portals);
}

// sub-space rendering - main procedure
void	R_dsgraph_structure::r_dsgraph_render_subspace	(IRender_Sector* _sector, CFrustum* _frustum, Fmatrix& mCombined, Fvector& _cop, BOOL _dynamic, BOOL _precise_portals)
{
	VERIFY							(_sector);
	RImplementation.marker			++;			// !!! critical here

	// Save and build new frustum, disable HOM
	CFrustum	ViewSave			= ViewBase;
	ViewBase						= *_frustum;
	View							= &ViewBase;

	if (_precise_portals && RImplementation.rmPortals)		{
		// Check if camera is too near to some portal - if so force DualRender
		Fvector box_radius;		box_radius.set	(EPS_L*20,EPS_L*20,EPS_L*20);
		RImplementation.Sectors_xrc.box_options	(CDB::OPT_FULL_TEST);
		RImplementation.Sectors_xrc.box_query	(RImplementation.rmPortals,_cop,box_radius);
		for (int K=0; K<RImplementation.Sectors_xrc.r_count(); K++)
		{
			CPortal*	pPortal		= (CPortal*) RImplementation.Portals[RImplementation.rmPortals->get_tris()[RImplementation.Sectors_xrc.r_begin()[K].id].dummy];
			pPortal->bDualRender	= TRUE;
		}
	}

	// Traverse sector/portal structure
	PortalTraverser.traverse		( _sector, ViewBase, _cop, mCombined, 0 );

	// Determine visibility for static geometry hierrarhy
	for (u32 s_it=0; s_it<PortalTraverser.r_sectors.size(); s_it++)
	{
		CSector*	sector		= (CSector*)PortalTraverser.r_sectors[s_it];
		dxRender_Visual*	root	= sector->root();
		for (u32 v_it=0; v_it<sector->r_frustums.size(); v_it++)	{
			set_Frustum			(&(sector->r_frustums[v_it]));
			add_Geometry		(root);
		}
	}

	if (_dynamic)
	{
#if RENDER != R_R1
		PIX_EVENT(FRUSTUM_DYNAMIC_SCENE);
#endif
		set_Object						(0);

		// Traverse object database
		g_SpatialSpace->q_frustum
			(
			lstRenderables,
			ISpatial_DB::O_ORDERED,
			STYPE_RENDERABLE,
			ViewBase
			);

		// Determine visibility for dynamic part of scene
		for (u32 o_it=0; o_it<lstRenderables.size(); o_it++)
		{
			ISpatial*	spatial		= lstRenderables[o_it];
			CSector*	sector		= (CSector*)spatial->spatial.sector;
			if	(0==sector)										continue;	// disassociated from S/P structure
			if	(PortalTraverser.i_marker != sector->r_marker)	continue;	// inactive (untouched) sector
			for (u32 v_it=0; v_it<sector->r_frustums.size(); v_it++)
			{
				set_Frustum			(&(sector->r_frustums[v_it]));
				if (!View->testSphere_dirty(spatial->spatial.sphere.P,spatial->spatial.sphere.R))	continue;

				// renderable
				IRenderable*	renderable		= spatial->dcast_Renderable	();
				if (0==renderable)				continue;					// unknown, but renderable object (r1_glow???)

				renderable->renderable_Render	();
			}
		}
	}

	// Restore
	ViewBase						= ViewSave;
	View							= 0;
}

#include "fhierrarhyvisual.h"
#include "SkeletonCustom.h"
#include "../../xr_3da/fmesh.h"
#include "flod.h"

void	R_dsgraph_structure::r_dsgraph_render_R1_box	(IRender_Sector* _S, Fbox& BB, int sh)
{
	CSector*	S			= (CSector*)_S;
	lstVisuals.clear		();
	lstVisuals.push_back	(S->root());
	
	for (u32 test=0; test<lstVisuals.size(); test++)
	{
		dxRender_Visual*	V		= 	lstVisuals[test];
		
		// Visual is 100% visible - simply add it
		xr_vector<dxRender_Visual*>::iterator I,E;	// it may be usefull for 'hierrarhy' visuals
		
		switch (V->Type) {
		case MT_HIERRARHY:
			{
				// Add all children
				FHierrarhyVisual* pV = (FHierrarhyVisual*)V;
				I = pV->children.begin	();
				E = pV->children.end		();
				for (; I!=E; I++)		{
					dxRender_Visual* T			= *I;
					if (BB.intersect(T->vis.box))	lstVisuals.push_back(T);
				}
			}
			break;
		case MT_SKELETON_ANIM:
		case MT_SKELETON_RIGID:
			{
				// Add all children	(s)
				CKinematics * pV		= (CKinematics*)V;
				pV->CalculateBones		(TRUE);
				I = pV->children.begin	();
				E = pV->children.end		();
				for (; I!=E; I++)		{
					dxRender_Visual* T				= *I;
					if (BB.intersect(T->vis.box))	lstVisuals.push_back(T);
				}
			}
			break;
		case MT_LOD:
			{
				FLOD		* pV		=	(FLOD*) V;
				I = pV->children.begin		();
				E = pV->children.end		();
				for (; I!=E; I++)		{
					dxRender_Visual* T				= *I;
					if (BB.intersect(T->vis.box))	lstVisuals.push_back(T);
				}
			}
			break;
		default:
			{
				// Renderable visual
				ShaderElement* E	= V->shader->E[sh]._get();
				if (E && !(E->flags.bDistort))
				{
					for (u32 pass=0; pass<E->passes.size(); pass++)
					{
						RCache.set_Element			(E,pass);
						V->Render					(-1.f);
					}
				}
			}
			break;
		}
	}
}


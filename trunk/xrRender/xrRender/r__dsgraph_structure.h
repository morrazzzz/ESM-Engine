#pragma once

#include "..\..\xr_3da\render.h"
#include "../../xrCDB/ispatial.h"
#include "r__dsgraph_types.h"
#include "r__sector.h"

//////////////////////////////////////////////////////////////////////////
// feedback	for receiving visuals										//
//////////////////////////////////////////////////////////////////////////
class	R_feedback
{
public:
	virtual		void	rfeedback_static	(dxRender_Visual*	V)		= 0;
};

//////////////////////////////////////////////////////////////////////////
// common part of interface implementation for all D3D renderers		//
//////////////////////////////////////////////////////////////////////////
class	R_dsgraph_structure										: public IRender_interface, public pureFrame
{
public:
	BOOL														val_bInvisible;
	R_feedback*													val_feedback;		// feedback for geometry being rendered
	u32															val_feedback_breakp;// breakpoint
	u32															phase;
	u32															marker;
	bool														pmask		[2]		;
	bool														pmask_wmark			;
public:
	// Dynamic scene graph
	//R_dsgraph::mapNormal_T										mapNormal	[2]		;	// 2==(priority/2)
	R_dsgraph::mapNormalPasses_T								mapNormalPasses	[2]	;	// 2==(priority/2)
	//R_dsgraph::mapMatrix_T										mapMatrix	[2]		;
	R_dsgraph::mapMatrixPasses_T								mapMatrixPasses	[2]	;
	R_dsgraph::mapSorted_T										mapSorted;
	R_dsgraph::mapHUD_T											mapHUD;
	R_dsgraph::mapLOD_T											mapLOD;
	R_dsgraph::mapSorted_T										mapDistort;

#if RENDER!=R_R1
	R_dsgraph::mapSorted_T										mapWmark;			// sorted
	R_dsgraph::mapSorted_T										mapEmissive;
	R_dsgraph::mapSorted_T										mapHUDEmissive;
#endif

	xr_vector<R_dsgraph::_LodItem,render_alloc<R_dsgraph::_LodItem> >	lstLODs		;
	xr_vector<int,render_alloc<int> >									lstLODgroups;
	xr_vector<ISpatial* /**,render_alloc<ISpatial*>/**/>				lstRenderables;
	xr_vector<ISpatial* /**,render_alloc<ISpatial*>/**/>				lstSpatial	;

	u32															counter_S	;
	BOOL														b_loaded	;
public:
	virtual		void					set_Invisible			(BOOL 		V	)				{ val_bInvisible= V;				}
				void					set_Feedback			(R_feedback*V, u32	id)			{ val_feedback_breakp = id; val_feedback = V;		}
				void					get_Counters			(u32&	s,	u32& d)				{ s=counter_S;			}
				void					clear_Counters			()								{ counter_S=0; 			}
public:
	R_dsgraph_structure	()
	{
		val_bInvisible		= FALSE	;
		val_feedback		= 0;
		val_feedback_breakp	= 0;
		marker				= 0;
		r_pmask				(true,true);
		b_loaded			= FALSE	;
	};

	void		r_dsgraph_destroy()
	{
		lstLODs.clear			();
		lstLODgroups.clear		();
		lstRenderables.clear	();
		lstSpatial.clear		();

		//mapNormal[0].destroy	();
		//mapNormal[1].destroy	();
		//mapMatrix[0].destroy	();
		//mapMatrix[1].destroy	();
		for (int i=0; i<SHADER_PASSES_MAX; ++i)
		{
			mapNormalPasses[0][i].destroy	();
			mapNormalPasses[1][i].destroy	();
			mapMatrixPasses[0][i].destroy	();
			mapMatrixPasses[1][i].destroy	();
		}
		mapSorted.destroy		();
		mapHUD.destroy			();
		mapLOD.destroy			();
		mapDistort.destroy		();

#if RENDER!=R_R1
		mapWmark.destroy		();
		mapEmissive.destroy		();
#endif
	}

	void		r_pmask											(bool _1, bool _2, bool _wm=false)				{ pmask[0]=_1; pmask[1]=_2;	pmask_wmark = _wm; }

	void		r_dsgraph_insert_dynamic						(IRenderable* pRenderable, dxRender_Visual *pVisual, Fmatrix& xform, bool hud);
	void		r_dsgraph_insert_static							(dxRender_Visual	*pVisual);

	void		r_dsgraph_render_graph							(u32	_priority,	bool _clear=true);
	void		r_dsgraph_render_hud							();
	void		r_dsgraph_render_hud_ui							();
	void		r_dsgraph_render_lods							(bool	_setup_zb,	bool _clear);
	void		r_dsgraph_render_sorted							();
	void		r_dsgraph_render_emissive						();
	void		r_dsgraph_render_wmarks							();
	void		r_dsgraph_render_distort						();
	void		r_dsgraph_render_subspace						(IRender_Sector* _sector, CFrustum* _frustum, Fmatrix& mCombined, Fvector& _cop, BOOL _dynamic, BOOL _precise_portals=FALSE	);
	void		r_dsgraph_render_subspace						(IRender_Sector* _sector, Fmatrix& mCombined, Fvector& _cop, BOOL _dynamic, BOOL _precise_portals=FALSE	);
	void		r_dsgraph_render_R1_box							(dxRender_Visual* _sector, Fbox& _bb, int _element);
};

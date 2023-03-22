#pragma once

#include "vis_common.h"
#include "Include/xrRender/RenderVisual.h"

#define VLOAD_NOVERTICES		(1<<0)
#define VLOAD_NOINDICES			(1<<1)
#define VLOAD_FORCESOFTWARE		(1<<2)

struct	ENGINE_API				IRender_Mesh	
{
	// format
	ref_geom					rm_geom;

	// verts
	IDirect3DVertexBuffer9*		p_rm_Vertices;
	u32							vBase;
	u32							vCount;

	// indices
	IDirect3DIndexBuffer9*		p_rm_Indices;
	u32							iBase;
	u32							iCount;
	u32							dwPrimitives;

	IRender_Mesh				()				{ p_rm_Vertices=0; p_rm_Indices=0;						}
	virtual ~IRender_Mesh		();
private:
	IRender_Mesh				(const IRender_Mesh& other);
	void	operator=			( const IRender_Mesh& other);
};

// The class itself
class	ENGINE_API dxRender_Visual: public IRenderVisual
{
public:
#ifdef _EDITOR
    ogf_desc					desc		;
#endif
#ifdef DEBUG
	shared_str					dbg_name	;
#endif
public:
	// Common data for rendering
	u32							Type		;				// visual's type
	vis_data					vis			;				// visibility-data
	ref_shader					shader		;				// pipe state, shared

	virtual void				Render						(float LOD)		{};		// LOD - Level Of Detail  [0..1], Ignored
	virtual void				Load						(const char* N, IReader *data, u32 dwFlags);
	virtual void				Release						();						// Shared memory release
	virtual void				Copy						(dxRender_Visual* from);
	virtual void				Spawn						()				{};
	virtual void				Depart						()				{};

	virtual vis_data& getVisData() { return vis; }
	virtual u32	getType() { return Type; }

	dxRender_Visual();
	virtual ~dxRender_Visual();
};


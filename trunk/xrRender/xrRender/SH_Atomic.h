#pragma once
#include "xr_resource.h"
#include "tss_def.h"

#pragma pack(push,4)


//////////////////////////////////////////////////////////////////////////
// Atomic resources
//////////////////////////////////////////////////////////////////////////
struct		SVS				: public xr_resource_named							{
	IDirect3DVertexShader9*				vs;
	R_constant_table					constants;
	~SVS			();
};
typedef	resptr_core<SVS,resptr_base<SVS> >													ref_vs;

//////////////////////////////////////////////////////////////////////////
struct		SPS				: public xr_resource_named							{
	IDirect3DPixelShader9*				ps;
	R_constant_table					constants;
	~SPS			();
};
typedef	resptr_core<SPS,resptr_base<SPS> >													ref_ps;

//////////////////////////////////////////////////////////////////////////
struct		SState			: public xr_resource_flagged						{
	IDirect3DStateBlock9*				state;
	SimulatorStates						state_code;
	~SState			();
};
typedef	resptr_core<SState,resptr_base<SState> >											ref_state;

//////////////////////////////////////////////////////////////////////////
struct		SDeclaration	: public xr_resource_flagged						{
	IDirect3DVertexDeclaration9*		dcl;
	xr_vector<D3DVERTEXELEMENT9>		dcl_code;
	~SDeclaration	();
};
typedef	resptr_core<SDeclaration,resptr_base<SDeclaration> >								ref_declaration;

#pragma pack(pop)

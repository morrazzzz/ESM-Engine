// FProgressive.h: interface for the FProgressive class.
//
//////////////////////////////////////////////////////////////////////

#ifndef FProgressiveH
#define FProgressiveH
#pragma once

#include "FVisual.h"
struct	FSlideWindowItem;

class	FProgressive	: public Fvisual
{
protected:
	FSlideWindowItem	nSWI		;
	FSlideWindowItem*	xSWI		;
	u32					last_lod	;
public:
    					FProgressive();
	virtual 			~FProgressive();
	void RenderModelVisual(R_dsgraph::_MatrixItemS* matrixItem, float LOD, ShaderElement* shaderElement = nullptr,
		u32 pass = 0, ID3DBlob * signature = nullptr /*ugly hack!!! delete this!!!!*/) override;	// LOD - Level Of Detail  [0.0f - min, 1.0f - max], -1 = Ignored
	virtual void 		Load		(const char* N, IReader *data,u32 dwFlags);
	virtual void 		Copy		(dxRender_Visual *pFrom);
	virtual void 		Release		();
private:
	FProgressive				(const FProgressive& other);
	void	operator=			( const FProgressive& other);
};

#endif

#ifndef	dx103DFluidGrid_included
#define	dx103DFluidGrid_included
#pragma once

struct VS_INPUT_FLUIDSIM_STRUCT;

class dx103DFluidGrid
{
public:
	dx103DFluidGrid();
	~dx103DFluidGrid();

	void	Initialize( int gridWidth, int gridHeight, int gridDepth);

	void	DrawSlices();
	
	SGeometry* getGeomSlices() const { return &*m_GeomSlices; }
private:
	void	CreateVertexBuffers();

	void	InitSlice( int z, VS_INPUT_FLUIDSIM_STRUCT** vertices, int& index );

private:

	Ivector3	m_vDim;

	ref_geom		m_GeomSlices;

	ID3DBuffer*	m_pSlicesBuffer;

	int			m_iNumVerticesSlices;
};

#endif	//	dx103DFluidGrid_included
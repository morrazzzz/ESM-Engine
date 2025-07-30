#include "stdafx.h"
#include "dx103DFluidGrid.h"

#include "../dx10BufferUtils.h"

struct VS_INPUT_FLUIDSIM_STRUCT
{   
	D3DXVECTOR3 Pos; // Clip space position for slice vertices
	D3DXVECTOR3 Tex; // Cell coordinates in 0-"texture dimension" range
};

#define VERTICES_PER_SLICE 6

dx103DFluidGrid::dx103DFluidGrid()
{

}

dx103DFluidGrid::~dx103DFluidGrid()
{
	//	TODO: implement init/deinit functionality and guards
}

void dx103DFluidGrid::Initialize( int gridWidth, int gridHeight, int gridDepth)
{
	m_vDim[0] = gridWidth;
	m_vDim[1] = gridHeight;
	m_vDim[2] = gridDepth;

	CreateVertexBuffers();
}

void dx103DFluidGrid::CreateVertexBuffers()
{
	// Create layout
	//D3Dxx_INPUT_ELEMENT_DESC layoutDesc[] = 
	//{
	//	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0, 0, D3Dxx_INPUT_PER_VERTEX_DATA, 0 },
	//	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT,       0,12, D3Dxx_INPUT_PER_VERTEX_DATA, 0 }, 
	//};

	static D3DVERTEXELEMENT9 layoutDesc[] = 
	{
 		{ 0, 0,		D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_POSITION,		0 },
		{ 0, 12,	D3DDECLTYPE_FLOAT3,		D3DDECLMETHOD_DEFAULT, 	D3DDECLUSAGE_TEXCOORD,		0 },
		D3DDECL_END()
	};

	u32 vSize = D3DXGetDeclVertexSize(layoutDesc,0);

	//UINT numElements = sizeof(layoutDesc)/sizeof(layoutDesc[0]);
	//CreateLayout( layoutDesc, numElements, technique, &layout);

	int	index = 0;
	VS_INPUT_FLUIDSIM_STRUCT *slices = NULL;

	m_iNumVerticesSlices = VERTICES_PER_SLICE * (m_vDim[2] - 2);
	slices = xr_alloc<VS_INPUT_FLUIDSIM_STRUCT>(m_iNumVerticesSlices);

	VERIFY(m_iNumVerticesSlices);

	// Vertex buffer for "m_vDim[2]" quads to draw all the slices to a 3D texture
	// (a Geometry Shader is used to send each quad to the appropriate slice)
	index = 0;
	for( int z = 1; z < m_vDim[2]-1; z++ )
		InitSlice( z, &slices, index );
	VERIFY(index==m_iNumVerticesSlices);
	//V_RETURN(CreateVertexBuffer(sizeof(VS_INPUT_FLUIDSIM_STRUCT)*numVerticesSlices,
	//	D3Dxx_BIND_VERTEX_BUFFER, &slicesBuffer, slices , numVerticesSlices));
	CHK_DX(dx10BufferUtils::CreateVertexBuffer(&m_pSlicesBuffer, slices, vSize*m_iNumVerticesSlices));
	m_GeomSlices.create(layoutDesc, m_pSlicesBuffer, 0);

//cleanup:
	xr_free(slices);
	slices = NULL;
}

void dx103DFluidGrid::InitSlice( int z, VS_INPUT_FLUIDSIM_STRUCT** vertices, int& index )
{
	VS_INPUT_FLUIDSIM_STRUCT tempVertex1;
	VS_INPUT_FLUIDSIM_STRUCT tempVertex2;
	VS_INPUT_FLUIDSIM_STRUCT tempVertex3;
	VS_INPUT_FLUIDSIM_STRUCT tempVertex4;

	int w = m_vDim[0];
	int h = m_vDim[1];

	tempVertex1.Pos = D3DXVECTOR3( 1*2.0f/w - 1.0f      , -1*2.0f/h + 1.0f      , 0.0f      );
	tempVertex1.Tex = D3DXVECTOR3( 1.0f                 ,  1.0f                 , float(z)  );

	tempVertex2.Pos = D3DXVECTOR3( (w-1.0f)*2.0f/w-1.0f , -1*2.0f/h + 1.0f      , 0.0f      );
	tempVertex2.Tex = D3DXVECTOR3( (w-1.0f)             ,   1.0f                , float(z)  );

	tempVertex3.Pos = D3DXVECTOR3( (w-1.0f)*2.0f/w-1.0f , -(h-1)*2.0f/h+1.0f    , 0.0f      );
	tempVertex3.Tex = D3DXVECTOR3( (w-1.0f)             , (h-1.0f)              , float(z)  );

	tempVertex4.Pos = D3DXVECTOR3( 1*2.0f/w - 1.0f      , -(h-1.0f)*2.0f/h+1.0f , 0.0f      );
	tempVertex4.Tex = D3DXVECTOR3( 1.0f                 , (h-1.0f)              , float(z)  );


	(*vertices)[index++] = tempVertex1;
	(*vertices)[index++] = tempVertex2;
	(*vertices)[index++] = tempVertex3;
	(*vertices)[index++] = tempVertex1;
	(*vertices)[index++] = tempVertex3;
	(*vertices)[index++] = tempVertex4;

}

void dx103DFluidGrid::DrawSlices( void )
{
	//UINT stride[1] = { sizeof(VS_INPUT_FLUIDSIM_STRUCT) };
	//UINT offset[1] = { 0 };
	//DrawPrimitive( D3Dxx_PRIMITIVE_TOPOLOGY_TRIANGLELIST, layout, &slicesBuffer,
	//	stride, offset, 0, numVerticesSlices );

	RCache.set_Geometry(m_GeomSlices);
	RCache.Render( D3DPT_TRIANGLELIST, 0, m_iNumVerticesSlices/3);
}

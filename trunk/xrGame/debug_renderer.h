////////////////////////////////////////////////////////////////////////////
//	Module 		: debug_renderer.h
//	Created 	: 19.06.2006
//  Modified 	: 19.06.2006
//	Author		: Dmitriy Iassenev
//	Description : debug renderer
////////////////////////////////////////////////////////////////////////////

#pragma once

#include "../Include/xrRender/DebugRender.h"
class CDebugRenderer {
private:
	enum {line_vertex_limit	= 32767};

private:
//	xr_vector<FVF::L>	m_line_vertices;
	xr_vector<u16>		m_line_indices;

private:
	void add_lines(Fvector const* vertices, u32 const& vertex_count, u16 const* pairs, u32 const& pair_count, u32 const& color);

public:
					CDebugRenderer	();
	IC		void	render			();

public:
	IC		void	draw_line		(const Fmatrix &matrix, const Fvector &vertex0, const Fvector &vertex1, const u32 &color);
	IC		void	draw_aabb		(const Fvector &center, const float &half_radius_x, const float &half_radius_y, const float &half_radius_z, const u32 &color);
	        void draw_obb(const Fmatrix& matrix, const u32& color);
	        void	draw_obb		(const Fmatrix &matrix, const Fvector &half_size, const u32 &color);
			void	draw_ellipse	(const Fmatrix &matrix, const u32 &color);
};

#include "debug_renderer_inline.h"

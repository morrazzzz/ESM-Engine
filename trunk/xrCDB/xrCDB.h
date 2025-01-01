#ifndef XRCDB_H
#define XRCDB_H

#pragma once
// The following ifdef block is the standard way of creating macros which make exporting
// from a DLL simpler. All files within this DLL are compiled with the XRCDB_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see
// XRCDB_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.
#ifdef XRCDB_EXPORTS
#define XRCDB_API __declspec(dllexport)
#else
#define XRCDB_API __declspec(dllimport)
#endif
#ifdef M_VISUAL
#define ALIGN(a) alignas(a)
#else
#define ALIGN(a)
#endif

// forward declarations
class CFrustum;
namespace Opcode {
	class OPCODE_Model;
	class AABBNoLeafNode;
};

#pragma pack(push,8)
namespace CDB
{
#ifdef _M_X64
#pragma pack(push, 1)
	// Triangle for x86
	class XRCDB_API TRI_DEPRECATED //*** 16 bytes total (was 32 :)
	{
	public:
		u32 verts[3]; // 3*4 = 12b
		union
		{
			u32 dummy; // 4b
			struct
			{
				u32 material : 14;
				u32 suppress_shadows : 1;
				u32 suppress_wm : 1;
				u32 sector : 16;
			};
		};

	public:
		IC u32 IDvert(u32 ID) { return verts[ID]; }
	};
#pragma pack (pop)
#endif
	// Triangle
	class XRCDB_API TRI						//*** 16 bytes total (was 32 :)
	{
	public:
		u32				verts	[3];		// 3*4 = 12b
		union	{
			u32 dummy; // 4b
			struct
			{
				size_t material : 14;
				size_t suppress_shadows : 1;
				size_t suppress_wm : 1;
				size_t sector : 16;
			};
		};
#ifdef _M_X64
		TRI(TRI_DEPRECATED& oldTri)
		{
			verts[0] = oldTri.verts[0];
			verts[1] = oldTri.verts[1];
			verts[2] = oldTri.verts[2];
			dummy = oldTri.dummy;
		}

		TRI()
		{
			verts[0] = 0;
			verts[1] = 0;
			verts[2] = 0;
			dummy = 0;
		}

		TRI& operator= (const TRI_DEPRECATED& oldTri)
		{
			verts[0] = oldTri.verts[0];
			verts[1] = oldTri.verts[1];
			verts[2] = oldTri.verts[2];
			dummy = oldTri.dummy;
			return *this;
		}
#endif

	public:
		IC u32			IDvert	(u32 ID)		{ return verts[ID];	}
	};

	// Build callback
	typedef		void __stdcall	build_callback	(Fvector* V, int Vcnt, TRI* T, int Tcnt, void* params);

	// Model definition
	class XRCDB_API MODEL
	{
		friend class COLLIDER;
		enum
		{
			S_READY				= 0,
			S_INIT				= 1,
			S_BUILD				= 2,
			S_forcedword		= u32(-1)
		};
	private:
		Opcode::OPCODE_Model*	tree;
		u32						status;		// 0=ready, 1=init, 2=building

		// tris
		TRI*					tris;
		int						tris_count;
		Fvector*				verts;
		int						verts_count;
	public:
		MODEL();
		~MODEL();

		IC Fvector*				get_verts		()			{ return verts;		}
		IC int					get_verts_count	()	const	{ return verts_count;}
		IC TRI*					get_tris		()			{ return tris;		}
		IC int					get_tris_count	()	const	{ return tris_count;}

		void build_internal(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc = nullptr, void* bcp = nullptr, const bool rebuildTrisRequired = true);
		void build(Fvector* V, int Vcnt, TRI* T, int Tcnt, build_callback* bc = nullptr, void* bcp = nullptr, const bool rebuildTrisRequired = true);
		u32						memory			();
	};

	// Collider result
	struct XRCDB_API RESULT
	{
		Fvector			verts	[3];
		union	{
			u32 dummy; // 4b
			struct
			{
				size_t material : 14;
				size_t suppress_shadows : 1;
				size_t suppress_wm : 1;
				size_t sector : 16;
			};
		};
		int				id;
		float			range;
		float			u,v;
	};

	// Collider Options
	enum ColliderOptions : u8 {
		OPT_CULL		= (1<<0),
		OPT_ONLYFIRST	= (1<<1),
		OPT_ONLYNEAREST	= (1<<2),
		OPT_FULL_TEST   = (1<<3)		// for box & frustum queries - enable class III test(s)
	};

	// Collider itself
	class XRCDB_API COLLIDER
	{
		// Ray data and methods
		u8 ray_mode;
		u8 box_mode;
		u8 frustum_mode;

		// Result management
		xr_vector<RESULT>	rd;
	public:
		COLLIDER		();
		~COLLIDER		();

		ICF void		ray_options		(u8 f)	{	ray_mode = f;		}
		void			ray_query		(const MODEL *m_def, const Fvector& r_start,  const Fvector& r_dir, float r_range = 10000.f);

		ICF void		box_options		(u8 f)	{	box_mode = f;		}
		void			box_query		(const MODEL *m_def, const Fvector& b_center, const Fvector& b_dim);

		ICF void		frustum_options	(u8 f)	{	frustum_mode = f;	}
		void			frustum_query	(const MODEL *m_def, const CFrustum& F);

		ICF RESULT& r_index(size_t index) { return rd[index]; }
		ICF RESULT*		r_begin			()	{	return &*rd.begin();		};
		ICF RESULT*		r_end			()	{	return &*rd.end();			};
		void r_add(const RESULT& result);
		void r_free();
		ICF size_t			r_count			()	{	return rd.size();			};
		ICF void		r_clear			()	{	rd.clear_not_free();		};
		ICF void		r_clear_compact	()	{	rd.clear_and_free();		};
	};

	//
	class XRCDB_API Collector
	{
		xr_vector<Fvector>	verts;
		xr_vector<TRI>		faces;

		u32				VPack				( const Fvector& V, float eps);
	public:
		void			add_face			( const Fvector& v0, const Fvector& v1, const Fvector& v2, u16 material, u16 sector	);
		void			add_face_packed		( const Fvector& v0, const Fvector& v1, const Fvector& v2, u16 material, u16 sector, float eps = EPS );
		void			add_face_packed_D	( const Fvector& v0, const Fvector& v1, const Fvector& v2, size_t dummy, float eps = EPS );
		void			calc_adjacency		( xr_vector<u32>& dest		);

		Fvector*		getV			()	{ return &*verts.begin();		}
		size_t			getVS			() 	{ return verts.size();			}
		TRI*			getT			()	{ return &*faces.begin();		}
		size_t			getTS			()	{ return faces.size();			}
		void			clear			()	{ verts.clear(); faces.clear();	}
	};
};

#pragma pack(pop)
#endif

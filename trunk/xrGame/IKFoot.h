#pragma once

struct local_vector
{
	Fvector v;
	u16		bone;
	local_vector(): bone( u16(-1) ), v( Fvector().set( 0, 0, 0 ) ){ }
};

class	IKinematics;
struct  SCalculateData;
struct  SIKCollideData;
class	CGameObject;


class	CIKFoot
{
public:
						CIKFoot						( );
	void				Create						( IKinematics	*K, LPCSTR section, u16 bones[4] );
IC	void				set_ref_bone				( u16 ref_bone ) { m_ref_bone =  ref_bone; }
	void				set_ref_bone				( );
	u16					get_ref_bone				( const Fmatrix & foot_transform, const Fmatrix &toe_transform ) const;
public:
IC	Fvector				&ToePosition				( Fvector &toe_position ) const;
IC	Fvector				&HeelPosition				( Fvector &heel_position ) const;
IC	u16					ref_bone					( ) const { return m_ref_bone; }
	Fmatrix				&ref_bone_to_foot			( Fmatrix &foot, const Fmatrix &ref_bone ) const;
	Fmatrix				&ref_bone_to_foot			( Fmatrix &ref_bone ) const;
private:
public:
IC	Fvector				&FootNormal					( Fvector &foot_normal ) const;
private:
IC	Fvector				&get_local_vector			( Fvector &v, const local_vector &lv )const;
IC	Fvector&			get_local_vector			( u16 bone, Fvector &v, const local_vector &lv )const;
	Fmatrix				&ref_bone_to_foot_transform	( Fmatrix& m ) const;
	Fmatrix				&foot_to_ref_bone_transform	( Fmatrix& m ) const;
	Fmatrix				&foot_to_ref_bone			( Fmatrix &ref_bone, const Fmatrix &foot ) const;
	Fmatrix				&foot_to_ref_bone			( Fmatrix &foot ) const;
private:

IC	bool				make_shift					( Fmatrix &xm, const Fvector &cl_point, bool collide, const Fplane &p, const Fvector &pick_dir  )const;

private:
	void				set_toe						( u16 bones[4] );
public:
IC  IKinematics	*		Kinematics					( )const			{ return m_K; }

private:
	IKinematics			*m_K;

	local_vector		m_toe_position;
	local_vector		m_heel_position;

	local_vector		m_foot_normal;
	local_vector		m_foot_direction;

	Fmatrix				m_bind_b2_to_b3;
	float				m_foot_width;
	u16					m_ref_bone;
	u16					m_foot_bone_id;
	u16					m_toe_bone_id;
};

#include	"IKFoot_inl.h"
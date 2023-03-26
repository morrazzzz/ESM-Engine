#include "stdafx.h"

#include "ikfoot.h"

//#include "ik_collide_data.h"
#include "GameObject.h"


//#include "ode_include.h"
#include "../include/xrrender/Kinematics.h"
#include "../xr_3da/bone.h"
#include "../xr_3da/ennumerateVertices.h"
#include <boost/noncopyable.hpp>


#ifdef DEBUG
#include "PHDebug.h"
#endif


CIKFoot::CIKFoot( ):
	m_bind_b2_to_b3		( Fidentity ),
	m_ref_bone			( u16( -1 ) ),
	m_foot_bone_id		( BI_NONE ),
	m_toe_bone_id		( BI_NONE ),
	m_K					( 0 )
{

}


void CIKFoot::Create		(  IKinematics	*K, LPCSTR section, u16 bones[4] )
{
	VERIFY(K);
	m_K = K;
	
	///defaults
	m_ref_bone		= 2;
	if( m_ref_bone	== 2 )
	{
		m_foot_normal.v			.set( 1, 0, 0 );//2
		m_foot_normal.bone		= 2;
		m_foot_direction.v		.set( 0, 0, 1 );//2
		m_foot_direction.bone	= 2;
	} else
	{
		m_foot_normal.v		.set( 0, 0, -1 );//3
		m_foot_normal.bone		= 3;
		m_foot_direction.v	.set( 1, 0, 0 );//3
		m_foot_direction.bone	= 3;
	}

	//	m_foot_normal.v			.set( 1, 0, 0 );//2
	//	m_foot_normal.bone		= 2;

	//load settings	
	if( section )
	{
		if( !!K->LL_UserData()->r_bool( section, "align_toe" ))
			m_ref_bone = 3;
		m_foot_normal.bone		= m_ref_bone;
		m_foot_direction.bone	= m_ref_bone;

		m_foot_normal.v		= Kinematics()->LL_UserData()->r_fvector3( section, "foot_normal" );
		m_foot_direction.v	= Kinematics()->LL_UserData()->r_fvector3( section, "foot_direction" );
	}
	set_toe( bones );
}

struct envc :
private boost::noncopyable,
public SEnumVerticesCallback
{
	Fvector &pos;
	Fvector start_pos;
	const Fmatrix &i_bind_transform;
	const Fvector &ax;
	envc( const Fmatrix &_i_bind_transform, const Fvector &_ax,  Fvector &_pos ): 
	SEnumVerticesCallback(), i_bind_transform( _i_bind_transform ), ax( _ax ), pos( _pos ) { start_pos.set( 0, 0, 0 ); }
	void operator () (const Fvector& p)
	{
		Fvector lpos;
		i_bind_transform.transform_tiny(lpos, p );
		//Fvector diff;diff.sub( lpos, pos );
		if( Fvector().sub( lpos, start_pos ).dotproduct( ax ) > Fvector().sub( pos, start_pos ).dotproduct( ax ) )
						pos.set( lpos );
	}
};
void CIKFoot::set_toe(  u16 bones[4] )
{

	VERIFY( Kinematics() );

	m_foot_bone_id = bones[2];
	m_toe_bone_id =	bones[3];

	xr_vector<Fmatrix> binds;
	Kinematics()->LL_GetBindTransform( binds );
	
	const Fmatrix bind_ref	= binds[ bones[m_ref_bone] ];
	const Fmatrix ibind_ref = Fmatrix().invert( bind_ref );

	const Fmatrix bind2		= binds[ bones[2] ] ;
	const Fmatrix ibind2	= Fmatrix().invert( bind2 );

	//const Fmatrix ref_to_b2	= Fmatrix().mul_43( ibind2, bind_ref );
	const Fmatrix b2to_ref	= Fmatrix().mul_43(  ibind_ref, bind2 );

	const Fmatrix bind3		= binds[ bones[3] ] ;
	const Fmatrix ibind3	= Fmatrix().invert( bind3 );

	m_bind_b2_to_b3.mul_43( ibind2, bind3 );
	///////////////////////////////////////////////////////
	Fvector ax ,foot_normal, foot_dir; 
	get_local_vector( 2, foot_normal, m_foot_normal );
	get_local_vector( 2, foot_dir, m_foot_direction );

	//ref_to_b2.transform_tiny( foot_normal, m_foot_normal.v );
	//ref_to_b2.transform_tiny( foot_dir, m_foot_direction.v );

	ax.add( foot_normal, foot_dir );
	ax.normalize();
	///////////////////////////////////////////////////////
	Fvector pos; pos.set( 0, 0, 0 );
	Fmatrix ibind = ibind3;
	envc pred( ibind, ax, pos );
	/////////////////////////////////////////////////////////
	Kinematics()->EnumBoneVertices( pred, bones[3] );
	bind3.transform_tiny( pos );
	ibind2.transform_tiny( pos );
	m_toe_position.v.set( pos );
	/////////////////////////////////////////////////////////
	ibind.set( ibind2 );
	ax.set( foot_normal );
	Kinematics()->EnumBoneVertices( pred, bones[2] );
	m_toe_position.v.x = _max( pos.x, m_toe_position.v.x );
	/////////////////////////////////////////////////////////
	ax.sub( foot_normal, foot_dir );
	ax.normalize();
	pred.start_pos.set(0,0,0);pos.set( 0, 0, 0 );
	Kinematics()->EnumBoneVertices( pred, bones[2] );
	m_heel_position.v = pred.pos	;
	m_heel_position.v.add( Fvector().mul( foot_dir,
		Fvector().sub( m_toe_position.v, pos ).dotproduct( foot_dir ) * 0.2f	) 
	);
	m_heel_position.bone = 2;
	///////////////////////////////////////////////////////////
	/////////////////////////////////////////////////////////
	bind2.transform_tiny( m_toe_position.v );
	ibind_ref.transform_tiny( m_toe_position.v );
	m_toe_position.bone = ref_bone();
	/////////////////////////////////////////////////////////

	get_local_vector( foot_normal, m_foot_normal );
	m_foot_width = ( Fvector().sub( m_toe_position.v, b2to_ref.c ) ).dotproduct(  foot_normal );


}

Fmatrix&	CIKFoot::foot_to_ref_bone_transform	( Fmatrix& m )  const
{
	if(m_ref_bone == 2)
	{
		m.set( Fidentity );
		return m;
	}
	m.mul_43( Fmatrix().invert( Kinematics()->LL_GetTransform( m_foot_bone_id ) ), Kinematics()->LL_GetTransform( m_toe_bone_id ) );
	return m;
}

Fmatrix&	CIKFoot::ref_bone_to_foot_transform	( Fmatrix& m )  const
{
	return m.invert(Fmatrix().set( foot_to_ref_bone_transform ( m ) ));
}

Fmatrix&	CIKFoot::foot_to_ref_bone	( Fmatrix &ref_bone, const Fmatrix &foot ) const
{
	if( m_ref_bone == 2 )
	{
		ref_bone = foot;
		return ref_bone;
	}
	Fmatrix m;
	foot_to_ref_bone_transform	( m );
	ref_bone.mul_43( foot, m );
	return ref_bone;
}

Fmatrix&	CIKFoot::foot_to_ref_bone	( Fmatrix &m )  const
{
	return foot_to_ref_bone( m, Fmatrix( ).set( m ) );
}

Fmatrix&	CIKFoot::ref_bone_to_foot	( Fmatrix &foot, const Fmatrix &ref_bone )  const
{
	if(m_ref_bone == 2)
	{
		foot = ref_bone;
		return foot;
	}
	Fmatrix m;
	ref_bone_to_foot_transform( m );
/*
	Fmatrix b2 = Kinematics()->LL_GetTransform( m_bones[2] );
	Fmatrix b3 = Kinematics()->LL_GetTransform( m_bones[3] );
	//m.mul_43( Fmatrix().invert( Kinematics()->LL_GetTransform(m_bones[2] ) ),Kinematics()->LL_GetTransform( m_bones[3] ) );

	Fmatrix ib3; ib3.invert( b3 );
	Fmatrix ib2; ib2.invert( b2 );
	m.mul_43( ib3, b2  );
	m.mul_43( ib2, b3  );
	m.invert();
*/
	foot.mul_43( ref_bone, m );
	return foot;
}

Fmatrix&	CIKFoot::ref_bone_to_foot	( Fmatrix &m )  const
{
	return ref_bone_to_foot( m, Fmatrix().set( m ) );
}
int ik_allign_free_foot = 0;

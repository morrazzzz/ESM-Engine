#include "stdafx.h"
#include "alife_space.h"
#include "hit.h"
#include "PHDestroyable.h"
#include "CharacterPhysicsSupport.h"
#include "PHMovementControl.h"
#include "CustomMonster.h"



#include "../Include/xrRender/Kinematics.h"
#include "../Include/xrRender/KinematicsAnimated.h"


#include "../xrPhysics/PhysicsShell.h"
#include "../xrPhysics/iActivationShape.h"
//#include "../xrphysics/Extendedgeom.h"
#include "../xrPhysics/geometry.h"
//#include "../xrphysics/phdynamicdata.h"
#include "../xrPhysics/IPHCapture.h"
//#include "../xrphysics/ICollideValidator.h"
#include "../xrPhysics/IPHWorld.h"

//#include "Physics.h"


#include "IKLimbsController.h"
#include "Actor.h"
#include "ai/stalker/ai_stalker.h"
#include "imotion_position.h"
#include "imotion_velocity.h"
#include "animation_movement_controller.h"
#include "xrServer_Object_Base.h"
#include "CustomZone.h"
#include "level.h"
#include "physics_shell_animated.h"

//const float default_hinge_friction = 5.f;//gray_wolf comment
#ifdef DEBUG
#	include "PHDebug.h"
#endif // DEBUG

#include "../xr_3da/device.h"

#ifdef PRIQUEL
#	define USE_SMART_HITS
#	define USE_IK
#endif // PRIQUEL

//void  NodynamicsCollide( bool& do_colide, bool bo1, dContact& c, SGameMtl * /*material_1*/, SGameMtl * /*material_2*/ )
//{
//	dBodyID body1=dGeomGetBody( c.geom.g1 );
//	dBodyID body2=dGeomGetBody( c.geom.g2 );
//	if( !body1 || !body2 || ( dGeomUserDataHasCallback( c.geom.g1,NodynamicsCollide )&& dGeomUserDataHasCallback( c.geom.g2, NodynamicsCollide ) ) )
//		return;
//	do_colide = false; 
//}



IC bool is_imotion(interactive_motion *im)
{
	return im && im->is_enabled();
}

CCharacterPhysicsSupport::~CCharacterPhysicsSupport()
{
	if(m_flags.test(fl_skeleton_in_shell))
	{
		if(m_physics_skeleton)
			m_physics_skeleton->Deactivate();
		xr_delete(m_physics_skeleton);//!b_skeleton_in_shell
	}
	xr_delete(m_PhysicMovementControl);
	VERIFY(!m_interactive_motion);
}

CCharacterPhysicsSupport::CCharacterPhysicsSupport(EType atype,CEntityAlive* aentity) 
:	m_pPhysicsShell(aentity->PPhysicsShell()),
	m_EntityAlife(*aentity),
	mXFORM(aentity->XFORM()),
	m_ph_sound_player(aentity),
	m_interactive_motion(0),
	m_physics_shell_animated(NULL),
	m_physics_shell_animated_time_destroy(u32(-1)),
	m_hit_valide_time(u32(-1))
{
	m_PhysicMovementControl=xr_new<CPHMovementControl>(aentity);
	m_flags.assign(0);
	m_eType=atype;
	m_eState=esAlive;
	//b_death_anim_on					= false;
	m_flags.set(fl_death_anim_on,FALSE);
	m_pPhysicsShell					=	NULL;
	//m_saved_impulse					= 0.f;
	m_physics_skeleton				=	NULL;
	//b_skeleton_in_shell				= false;
	m_flags.set(fl_skeleton_in_shell,FALSE);
	m_ik_controller					=	NULL;
	m_BonceDamageFactor				=1.f;
	m_collision_hit_callback		=	NULL;
	switch(atype)
	{
	case etActor:
		m_PhysicMovementControl->AllocateCharacterObject(CPHMovementControl::actor);
		m_PhysicMovementControl->SetRestrictionType(rtActor);
		break;
	case etStalker:
		m_PhysicMovementControl->AllocateCharacterObject(CPHMovementControl::ai);
		m_PhysicMovementControl->SetRestrictionType(rtStalker);
		m_PhysicMovementControl->SetActorMovable(false);
		break;
	case etBitting:
		m_PhysicMovementControl->AllocateCharacterObject(CPHMovementControl::ai);

		m_PhysicMovementControl->SetRestrictionType(rtMonsterMedium);
		//m_PhysicMovementControl->SetActorMovable(false);
	}
};

void CCharacterPhysicsSupport::SetRemoved()
{
	m_eState=esRemoved;
	if(m_flags.test(fl_skeleton_in_shell))//b_skeleton_in_shell
	{
		if(m_pPhysicsShell->isEnabled())
		{
			m_EntityAlife.processing_deactivate();
		}
		if(m_pPhysicsShell)m_pPhysicsShell->Deactivate();
		xr_delete(m_pPhysicsShell);
	}
	else
	{
		if(m_physics_skeleton)m_physics_skeleton->Deactivate();
		xr_delete(m_physics_skeleton);
		m_EntityAlife.processing_deactivate();
	}
	
}

void CCharacterPhysicsSupport::in_Load(LPCSTR section)
{

	m_character_shell_control.Load(section);
	m_flags.set(fl_specific_bonce_demager,TRUE);
	if(pSettings->line_exist(section,"bonce_damage_factor"))
	{
		
		m_BonceDamageFactor=pSettings->r_float(section,"bonce_damage_factor_for_objects");
	}else
	{
		m_BonceDamageFactor=pSettings->r_float("collision_damage","bonce_damage_factor_for_objects");
	}
	CPHDestroyable::Load(section);
}

void CCharacterPhysicsSupport::in_NetSpawn(CSE_Abstract* e)
{
	m_sv_hit = SHit();
	if(m_EntityAlife.use_simplified_visual	())
	{
		m_flags.set(fl_death_anim_on,TRUE);
		IKinematics*	ka=smart_cast<IKinematics*>(m_EntityAlife.Visual());
		VERIFY(ka);
		ka->CalculateBones_Invalidate();
		ka->CalculateBones( TRUE );
		CollisionCorrectObjPos(m_EntityAlife.Position());
		m_pPhysicsShell		= P_build_Shell(&m_EntityAlife,false);
		ka->CalculateBones_Invalidate();
		ka->CalculateBones( TRUE );
		return;
	}

	CPHDestroyable::Init();//this zerows colbacks !!;
	IRenderVisual *pVisual = m_EntityAlife.Visual();
	IKinematicsAnimated*ka= smart_cast<IKinematicsAnimated*>( pVisual );
	IKinematics*pK= smart_cast<IKinematics*>( pVisual );
	VERIFY( &e->spawn_ini() );
	m_death_anims.setup( ka, *e->s_name , pSettings );
	if(!m_EntityAlife.g_Alive())
	{
		
		if(m_eType==etStalker)
			ka->PlayCycle("waunded_1_idle_0");
		else
			ka->PlayCycle("death_init");

	}else if( !m_EntityAlife.animation_movement_controlled( ) )
	{
	
		ka->PlayCycle( "death_init" );///непонятно зачем это вообще надо запускать
									  ///этот хак нужен, потому что некоторым монстрам 
									  ///анимация после спона, может быть вообще не назначена
	}
	IKinematics* K = ka->dcast_PKinematics();
	K->CalculateBones_Invalidate();
	K->CalculateBones( TRUE );
	
	CPHSkeleton::Spawn(e);
	movement()->EnableCharacter();
	movement()->SetPosition(m_EntityAlife.Position());
	movement()->SetVelocity	(0,0,0);
	if(m_eType!=etActor)
	{
		m_flags.set(fl_specific_bonce_demager,TRUE);
		m_BonceDamageFactor=1.f;
	}
	if(Type() == etStalker)
	{
		m_hit_animations.SetupHitMotions(*smart_cast<IKinematicsAnimated*>(m_EntityAlife.Visual()));
	}
	anim_mov_state.init();

	anim_mov_state.active = m_EntityAlife.animation_movement_controlled();
}


void CCharacterPhysicsSupport::CreateCharacter( )
{
	//if( m_eType == etBitting )
		//return;
	if( m_PhysicMovementControl->CharacterExist( ) )return;
	CollisionCorrectObjPos( m_EntityAlife.Position( ), true );
	m_PhysicMovementControl->CreateCharacter( );
	m_PhysicMovementControl->SetPhysicsRefObject( &m_EntityAlife );
	m_PhysicMovementControl->SetPosition	( m_EntityAlife.Position( ) );
}
void CCharacterPhysicsSupport::SpawnInitPhysics(CSE_Abstract* e)
{
	//if(!m_physics_skeleton)CreateSkeleton(m_physics_skeleton);

	if(m_EntityAlife.g_Alive())
	{
#ifdef DEBUG
		if(ph_dbg_draw_mask1.test(ph_m1_DbgTrackObject)&&stricmp(PH_DBG_ObjectTrack(),*m_EntityAlife.cName())==0)
		{
			Msg("CCharacterPhysicsSupport::SpawnInitPhysics obj %s before collision correction %f,%f,%f",PH_DBG_ObjectTrack(),m_EntityAlife.Position().x,m_EntityAlife.Position().y,m_EntityAlife.Position().z);
		}
#endif
#ifdef	USE_IK
		if( etStalker == m_eType || etActor == m_eType )
				CreateIKController( );
#endif
		if( !m_EntityAlife.animation_movement_controlled( ) )
			CreateCharacter( );
#ifdef DEBUG  
		if( ph_dbg_draw_mask1.test( ph_m1_DbgTrackObject ) && stricmp( PH_DBG_ObjectTrack( ), *m_EntityAlife.cName()) == 0 )
		{
			Msg( "CCharacterPhysicsSupport::SpawnInitPhysics obj %s after collision correction %f,%f,%f", PH_DBG_ObjectTrack(),m_EntityAlife.Position( ).x, m_EntityAlife.Position().y, m_EntityAlife.Position().z );
		}
#endif
		//m_PhysicMovementControl.SetMaterial( )
	}
	else
	{
		ActivateShell( NULL );
	}
}

void CCharacterPhysicsSupport::destroy_imotion()
{
	destroy(m_interactive_motion);
}

void CCharacterPhysicsSupport::in_NetDestroy( )
{
	destroy( m_interactive_motion );
	m_PhysicMovementControl->DestroyCharacter( );

	if( m_physics_skeleton )
	{
		m_physics_skeleton->Deactivate( );
		xr_delete( m_physics_skeleton );
	}
	if(m_pPhysicsShell)
	{
		m_pPhysicsShell->Deactivate();
		xr_delete(m_pPhysicsShell);
	}

	m_flags.set( fl_skeleton_in_shell, FALSE );
	CPHSkeleton::RespawnInit( );
	CPHDestroyable::RespawnInit( );
	m_eState = esAlive;
	DestroyIKController( );
}

void	CCharacterPhysicsSupport::in_NetSave( NET_Packet& P )
{
	
	CPHSkeleton::SaveNetState( P );
}

void CCharacterPhysicsSupport::in_Init( )
{
	
	//b_death_anim_on					= false;
	//m_pPhysicsShell					= NULL;
	//m_saved_impulse					= 0.f;
}

void CCharacterPhysicsSupport::in_shedule_Update( u32 DT )
{
	//CPHSkeleton::Update(DT);
	if( !m_EntityAlife.use_simplified_visual	( ) )
		CPHDestroyable::SheduleUpdate( DT );
	else	if( m_pPhysicsShell&&m_pPhysicsShell->isFullActive( ) && !m_pPhysicsShell->isEnabled( ) )
	{
		m_EntityAlife.deactivate_physics_shell( );
	}
	movement( )->in_shedule_Update( DT );

#if	0
	if( anim_mov_state.active )
	{
		DBG_OpenCashedDraw( );
		DBG_DrawMatrix( mXFORM, 0.5f );
		DBG_ClosedCashedDraw( 5000 );
	}
#endif

}

#ifdef DEBUG
	string64 sdbg_stalker_death_anim = "none";
	LPSTR dbg_stalker_death_anim = sdbg_stalker_death_anim;
#endif
BOOL  b_death_anim_velocity = TRUE;
const float cmp_angle = M_PI/10.f;
const float cmp_ldisp = 0.1f;
IC bool cmp(const Fmatrix &f0, const Fmatrix &f1 )
{
	Fmatrix if0;if0.invert(f0);
	Fmatrix cm;cm.mul_43(if0,f1);
	
	Fvector ax;float ang;
	Fquaternion q;
	q.set(cm);
	q.get_axis_angle(ax,ang);

	return ang < cmp_angle && cm.c.square_magnitude() < cmp_ldisp* cmp_ldisp ;
}

bool is_similar(const Fmatrix &m0,const Fmatrix &m1,float param)
{
	Fmatrix tmp1;tmp1.invert(m0);
	Fmatrix tmp2;tmp2.mul(tmp1,m1);
	Fvector ax;float ang;
	Fquaternion q;
	q.set(tmp2);
	q.get_axis_angle(ax,ang);
	return _abs(ang)<M_PI/2.f;
	/*
	return  fsimilar(tmp2._11,1.f,param)&&
			fsimilar(tmp2._22,1.f,param)&&
			fsimilar(tmp2._33,1.f,param)&&
			fsimilar(tmp2._41,0.f,param)&&
			fsimilar(tmp2._42,0.f,param)&&
			fsimilar(tmp2._43,0.f,param);
			*/
	/*
			fsimilar(tmp2._12,0.f,param)&&
			fsimilar(tmp2._13,0.f,param)&&
			fsimilar(tmp2._21,0.f,param)&&
			fsimilar(tmp2._23,0.f,param)&&
			fsimilar(tmp2._31,0.f,param)&&
			fsimilar(tmp2._32,0.f,param)&&
			fsimilar(tmp2._41,0.f,param)&&
			fsimilar(tmp2._42,0.f,param)&&
			fsimilar(tmp2._43,0.f,param);
			*/
}

void CCharacterPhysicsSupport::KillHit( SHit &H )
{
#ifdef	DEBUG
	if (death_anim_debug)
		Msg("death anim: kill hit  ");
#endif
	VERIFY(m_EntityAlife.Visual());
	VERIFY(m_EntityAlife.Visual()->dcast_PKinematics());

	//IKinematicsAnimated * KA = m_EntityAlife.Visual( )->dcast_PKinematicsAnimated	();
	//VERIFY( KA );
	//KA->SetUpdateTracksCalback( &tracks_disable_update );

	m_character_shell_control.TestForWounded( m_EntityAlife.XFORM( ), m_EntityAlife.Visual( )->dcast_PKinematics( ) );
	Fmatrix prev_pose;prev_pose.set(mXFORM);

	Fvector start;start.set( m_EntityAlife.Position( ) );
	Fvector velocity;
	Fvector death_position;

	CreateShell( H.who, death_position, velocity);
	//ActivateShell( H.who );

//	if(Type() == etStalker && xr_strcmp(dbg_stalker_death_anim, "none") != 0)
	float hit_angle = 0;
	MotionID m = m_death_anims.motion(m_EntityAlife, H, hit_angle);

	//TODO: If I transfer the smart cover`s from the CoP. Then you need to return to the usual appearance. It's not necessary yet. Let it be only for wounded
	CAI_Stalker* const	holder = m_EntityAlife.cast_stalker();
	if (holder && (holder->wounded() /* || holder->movement().current_params().cover())*/)) 
		m = MotionID();

	if( m.valid( ) )//&& cmp( prev_pose, mXFORM ) 
	{
		destroy( m_interactive_motion );
		if( false && b_death_anim_velocity )
			m_interactive_motion = xr_new<imotion_velocity>( );
		else
			m_interactive_motion = xr_new<imotion_position>( );
		m_interactive_motion->setup( m ,m_pPhysicsShell, hit_angle );
	} else 
		DestroyIKController( );
	//KA->SetUpdateTracksCalback( 0 );

	if(is_imotion(m_interactive_motion))
				m_interactive_motion->play( );

	


	m_character_shell_control.set_fatal_impulse(H);

	if( !is_imotion( m_interactive_motion ) )
	{
		
#ifdef	DEBUG
//		if( death_anim_debug )
//		{
//			Msg( "death anim: kill hit use free ragdoll " );
//			Msg( "death anim: fatal impulse: %f, ", H.impulse );
//		}
#endif
		
		EndActivateFreeShell( H.who, start, death_position, velocity);
		m_flags.set( fl_block_hit, TRUE );
	}
}
constexpr u32 hit_valide_time = 1000;
void CCharacterPhysicsSupport::in_Hit( SHit &H, bool is_killing )
{
	m_sv_hit = H;
	m_hit_valide_time = Device.dwTimeGlobal + hit_valide_time;
	if( m_EntityAlife.use_simplified_visual	( ) || esRemoved == m_eState )
		return;
	if(m_flags.test(fl_block_hit))
	{
		VERIFY(!m_EntityAlife.g_Alive());
		if(Device.dwTimeGlobal-m_EntityAlife.GetLevelDeathTime()>=2000)
			m_flags.set(fl_block_hit,FALSE);
		else return;
	}

	//is_killing=is_killing||(m_eState==esAlive&&!m_EntityAlife.g_Alive());
	if( m_EntityAlife.g_Alive( ) && is_killing && H.type( ) == ALife::eHitTypeExplosion && H.damage( ) > 70.f )
		CPHDestroyable::Destroy();

    if( ( !m_EntityAlife.g_Alive() || is_killing ) )
		m_character_shell_control.set_kill_hit( H );

	if(!m_pPhysicsShell&&is_killing)
		KillHit( H );

	if(!(m_pPhysicsShell&&m_pPhysicsShell->isActive()))
	{
		if(!is_killing&&m_EntityAlife.g_Alive())
			m_PhysicMovementControl->ApplyHit( H.direction( ), H.phys_impulse( ), H.type( ) );

#ifdef USE_SMART_HITS
		if(Type()==etStalker)
		{
				m_hit_animations.PlayHitMotion(dir,p_in_object_space,element,m_EntityAlife);
		}
#endif // USE_SMART_HITS

	}else 
		m_pPhysicsShell->applyHit( H.bone_space_position( ), H.direction( ), H.phys_impulse( ), H.bone(), H.type( ) );
}


IC		void	CCharacterPhysicsSupport::						UpdateDeathAnims				()
{
	VERIFY(m_pPhysicsShell->isFullActive());

	if(!m_flags.test(fl_death_anim_on) && !is_imotion(m_interactive_motion))//!m_flags.test(fl_use_death_motion)//!b_death_anim_on&&m_pPhysicsShell->isFullActive()
	{
		smart_cast<IKinematicsAnimated*>(m_EntityAlife.Visual())->PlayCycle("death_init");
		m_flags.set(fl_death_anim_on,TRUE);
	}
}


void CCharacterPhysicsSupport::in_UpdateCL( )
{
	if( m_eState==esRemoved )
	{
		return;
	}
/*
#ifdef DEBUG
	if (dbg_draw_character_bones)
		dbg_draw_geoms(m_weapon_geoms);

	if (dbg_draw_character_bones)
		DBG_DrawBones(m_EntityAlife);

	if (dbg_draw_character_binds)
		DBG_DrawBind(m_EntityAlife);

	if (dbg_draw_character_physics_pones)
		DBG_PhysBones(m_EntityAlife);

	if (dbg_draw_character_physics && m_pPhysicsShell)
		m_pPhysicsShell->dbg_draw_geometry(0.2f, D3DCOLOR_ARGB(100, 255, 0, 0));
#endif
*/
	update_animation_collision();
	m_character_shell_control.CalculateTimeDelta( );
	if( m_pPhysicsShell )
	{
		VERIFY( m_pPhysicsShell->isFullActive( ) );
		m_pPhysicsShell->SetRagDoll( );//Теперь шела относиться к классу объектов cbClassRagDoll
		
		if( !is_imotion(m_interactive_motion ) )//!m_flags.test(fl_use_death_motion)
			m_pPhysicsShell->InterpolateGlobalTransform( &mXFORM );
		else
			m_interactive_motion->update( );
	
		UpdateDeathAnims();

		m_character_shell_control.UpdateFrictionAndJointResistanse( m_pPhysicsShell );
	} 
	//else if ( !m_EntityAlife.g_Alive( ) && !m_EntityAlife.use_simplified_visual( ) )
	//{
	//	ActivateShell( NULL );
	//	m_PhysicMovementControl->DestroyCharacter( );
	//} 
	else if( ik_controller( ) )
		ik_controller( )->Update();


#ifdef DEBUG
	if(Type()==etStalker && ph_dbg_draw_mask1.test(phDbgHitAnims))
	{
		Fmatrix m;
		m_hit_animations.GetBaseMatrix(m,m_EntityAlife);
		DBG_DrawMatrix(m,1.5f);
/*
		IKinematicsAnimated 	*K = smart_cast<IKinematicsAnimated *>(m_EntityAlife.Visual());
		u16 hb = K->LL_BoneID("bip01_head");
		u16 pb = K->LL_GetBoneRoot();
		u16 nb = K->LL_BoneID("bip01_neck");
		u16 eb = K->LL_BoneID("eye_right");
		Fmatrix &mh  = K->LL_GetTransform(hb);
		Fmatrix &mp  = K->LL_GetTransform(pb);
		Fmatrix &me	 = K->LL_GetTransform(eb);
		Fmatrix &mn	 = K->LL_GetTransform(nb);
		float d = DET(mh);
		if(Fvector().sub(mh.c,mp.c).magnitude() < 0.3f||d<0.7 )//|| Fvector().sub(me.c,mn.c) < 0.5
		{
			
			K->CalculateBones_Invalidate();
			K->CalculateBones();
			;
		}
*/
	}
#endif
}

void CCharacterPhysicsSupport::CreateSkeleton(CPhysicsShell* &pShell)
{

	R_ASSERT2(!pShell,"pShell already initialized!!");
	if (!m_EntityAlife.Visual())
		return;
#ifdef DEBUG
	CTimer t;t.Start();
#endif	
	pShell		= P_create_Shell();

	IKinematics* k = smart_cast<IKinematics*>(m_EntityAlife.Visual());

	phys_shell_verify_object_model(m_EntityAlife);
	pShell->preBuild_FromKinematics(k);


	pShell->mXFORM.set(mXFORM);

	pShell->SmoothElementsInertia(0.3f);
	pShell->set_PhysicsRefObject(&m_EntityAlife);
	SAllDDOParams disable_params;
	disable_params.Load(smart_cast<IKinematics*>(m_EntityAlife.Visual())->LL_UserData());
	pShell->set_DisableParams(disable_params);

	pShell->Build();
#ifdef DEBUG	
	Msg("shell for %s[%d] created in %f ms",*m_EntityAlife.cName(),m_EntityAlife.ID(),t.GetElapsed_sec()*1000.f);
#endif
}

bool CCharacterPhysicsSupport::DoCharacterShellCollide()
{
	if(m_eType==etStalker)
	{
		CAI_Stalker*	OBJ=smart_cast<CAI_Stalker*>(&m_EntityAlife);
		VERIFY			(OBJ);
		return			!OBJ->wounded();
	}
	return true;
}

bool CCharacterPhysicsSupport::CollisionCorrectObjPos(const Fvector& start_from,bool	character_create/*=false*/)
{
	//Fvector shift;shift.sub( start_from, m_EntityAlife.Position() );
	Fvector shift; shift.set(0, 0, 0);
	Fbox box;
	if (character_create)
		box.set(movement()->Box());
	else
	{
		if (m_pPhysicsShell)
		{
			VERIFY(m_pPhysicsShell->isFullActive());
			Fvector sz, c;
			get_box(m_pPhysicsShell, mXFORM, sz, c);
			box.setb(Fvector().sub(c, m_EntityAlife.Position()), Fvector(sz).mul(0.5f));
			m_pPhysicsShell->DisableCollision();
		}
		else
			box.set(m_EntityAlife.BoundingBox());
	}

	Fvector vbox; Fvector activation_pos;
	box.get_CD(activation_pos, vbox);
	shift.add(activation_pos);
	vbox.mul(2.f);
	activation_pos.add(shift, m_EntityAlife.Position());
	bool not_collide_characters = !DoCharacterShellCollide() && !character_create;
	bool set_rotation = !character_create;

	Fvector activation_res = Fvector().set(0, 0, 0);
	////////////////

	bool ret = ActivateShapeCharacterPhysicsSupport(activation_res, vbox, activation_pos, mXFORM, not_collide_characters, set_rotation, &m_EntityAlife);
	//////////////////


	if (m_pPhysicsShell)
		m_pPhysicsShell->EnableCollision();
	return ret;
}

void CCharacterPhysicsSupport::set_movement_position( const Fvector &pos )
{
	VERIFY( movement() );

	CollisionCorrectObjPos( pos, true );
	
	movement()->SetPosition( m_EntityAlife.Position() );
}
void CCharacterPhysicsSupport::ForceTransform(const Fmatrix& m)
{
	if (!m_EntityAlife.g_Alive())
		return;
	VERIFY(_valid(m));
	m_EntityAlife.XFORM().set(m);
	if (movement()->CharacterExist())
		movement()->EnableCharacter();
	set_movement_position(m.c);
	movement()->SetVelocity(0, 0, 0);

}
static const u32 physics_shell_animated_destroy_delay = 3000;
void	CCharacterPhysicsSupport::destroy_animation_collision()
{
	xr_delete(m_physics_shell_animated);
	m_physics_shell_animated_time_destroy = u32(-1);
}
void CCharacterPhysicsSupport::create_animation_collision()
{
	m_physics_shell_animated_time_destroy = Device.dwTimeGlobal + physics_shell_animated_destroy_delay;
	if (m_physics_shell_animated)
		return;
	m_physics_shell_animated = xr_new<physics_shell_animated>(&m_EntityAlife, true);
}

void CCharacterPhysicsSupport::update_animation_collision()
{
	if (animation_collision())
	{
		animation_collision()->update(mXFORM);
		//animation_collision( )->shell()->set_LinearVel( movement()->GetVelocity() );
		if (Device.dwTimeGlobal > m_physics_shell_animated_time_destroy)
			destroy_animation_collision();
	}
}

void CCharacterPhysicsSupport::ActivateShell(CObject* who)
{
	R_ASSERT( _valid(m_EntityAlife.Position( )) );
	Fvector start;start.set( m_EntityAlife.Position( ) );
	Fvector velocity;
	Fvector death_position;
	CreateShell( who, death_position, velocity );
	EndActivateFreeShell( who, start ,death_position, velocity );
	VERIFY( m_pPhysicsShell );
	m_pPhysicsShell->Enable();
	m_pPhysicsShell->set_LinearVel( Fvector().set(0,-1,0) );
}

void	CCharacterPhysicsSupport::	CreateShell						( CObject* who, Fvector& dp, Fvector & velocity  )
{
	//xr_delete( m_collision_activating_delay );
	//xr_delete( m_interactive_animation );
	destroy_animation_collision( );
	//DestroyIKController( );
	IKinematics* K=smart_cast<IKinematics*>( m_EntityAlife.Visual( ) );
	//animation movement controller issues
	bool	anim_mov_ctrl =m_EntityAlife.animation_movement_controlled( );
	CBoneInstance	&BR = K->LL_GetBoneInstance( K->LL_GetBoneRoot( ) );
	Fmatrix start_xform; start_xform.identity( );
	CBlend *anim_mov_blend = 0;
	//float	blend_time = 0;
	if( anim_mov_ctrl )
	{
		m_EntityAlife.animation_movement( )->ObjStartXform( start_xform );
		anim_mov_blend = m_EntityAlife.animation_movement( )->ControlBlend( );
		/*
		VERIFY( anim_mov_blend->blend != CBlend::eFREE_SLOT );
		anim_mov_blend->timeCurrent -= 2 * Device.fTimeDelta * anim_mov_blend->speed;
		blend_time = anim_mov_blend->timeCurrent;
		anim_mov_blend->playing = true;

		K->CalculateBones_Invalidate( );
		K->CalculateBones	();
		anim_mov_blend->playing = false;
		*/
		m_EntityAlife.destroy_anim_mov_ctrl( );
		//BR.Callback_overwrite = TRUE;
		BR.set_callback_overwrite(true);
	}
	//
	u16 anim_root = K->LL_GetBoneRoot();
	u16 physics_root = anim_root;

	if (m_eType != etBitting)
	{
		physics_root = K->LL_BoneID("bip01_pelvis");
		K->LL_SetBoneRoot(physics_root);

	}
//
	if( !m_physics_skeleton ) 
		CreateSkeleton( m_physics_skeleton );

	if( m_eType == etActor )
	{
		CActor* A=smart_cast<CActor*>( &m_EntityAlife );
		R_ASSERT2( A, "not an actor has actor type" );
		if( A->Holder( ) ) return;
		if( m_eState==esRemoved )return;
	}
	
//////////////////////this needs to evaluate object box//////////////////////////////////////////////////////
	if( m_eType != etBitting )
		K->LL_SetBoneRoot( anim_root );

	for( u16 I = K->LL_BoneCount( )-1; I!=u16(-1); --I )
				K->LL_GetBoneInstance( I ).reset_callback( );

	if (anim_mov_ctrl)	//we do not whant to move by long animation in root 
		//			BR.Callback_overwrite = TRUE;
		BR.set_callback_overwrite(true);

	K->CalculateBones_Invalidate();
	K->CalculateBones	(TRUE);
////////////////////////////////////////////////////////////////////////////
	if( m_pPhysicsShell ) 
		return;

	m_PhysicMovementControl->GetCharacterVelocity		( velocity );

	if( !m_PhysicMovementControl->CharacterExist( ) )
		dp.set( m_EntityAlife.Position( ) );
	else m_PhysicMovementControl->GetDeathPosition( dp );
	m_PhysicMovementControl->DestroyCharacter( );

	//shell create
	R_ASSERT2(m_physics_skeleton,"No skeleton created!!");
	m_pPhysicsShell=m_physics_skeleton;
	m_physics_skeleton=NULL;
	m_pPhysicsShell->set_Kinematics(K);
	m_pPhysicsShell->RunSimulation();
	m_pPhysicsShell->mXFORM.set(mXFORM);
	m_pPhysicsShell->SetCallbacks( );
	//

	if (anim_mov_ctrl) //we do not whant to move by long animation in root 
		//			BR.Callback_overwrite = TRUE; 
		BR.set_callback_overwrite(true);

	if(!DoCharacterShellCollide())
		m_pPhysicsShell->DisableCharacterCollision();

	if( m_eType != etBitting )
		K->LL_SetBoneRoot( anim_root );

	K->CalculateBones_Invalidate();
	K->CalculateBones	(TRUE);


	if (m_eType != etBitting)
		K->LL_SetBoneRoot(physics_root);
	//reset_root_bone_start_pose( *m_pPhysicsShell );

	m_flags.set(fl_death_anim_on,FALSE);
	m_eState=esDead;
	m_flags.set(fl_skeleton_in_shell,TRUE);
	
	if(IsGameTypeSingle())
	{
		m_pPhysicsShell->SetPrefereExactIntegration	();//use exact integration for ragdolls in single
		m_pPhysicsShell->SetRemoveCharacterCollLADisable();
	}
	else
	{
		m_pPhysicsShell->SetIgnoreDynamic();
	}
	m_pPhysicsShell->SetIgnoreSmall();
}

void	CCharacterPhysicsSupport::EndActivateFreeShell(CObject * who, const Fvector & inital_entity_position, const Fvector & dp, const Fvector & velocity)
{
	VERIFY ( m_pPhysicsShell );
	VERIFY( m_eState==esDead );
#ifdef	DEBUG
/*
if( dbg_draw_ragdoll_spawn )
{
	DBG_OpenCashedDraw();
	m_pPhysicsShell->dbg_draw_geometry( 0.2f, D3DCOLOR_XRGB( 255, 100, 0 ) );
	DBG_ClosedCashedDraw( 50000 );
}
*/
#endif

    CollisionCorrectObjPos( dp );
	m_pPhysicsShell->SetGlTransformDynamic(mXFORM);

#ifdef	DEBUG
/*
if( dbg_draw_ragdoll_spawn )
{
	DBG_OpenCashedDraw();
	m_pPhysicsShell->dbg_draw_geometry( 0.2f, D3DCOLOR_XRGB( 255, 0, 100 ) );
	DBG_ClosedCashedDraw( 50000 );
}
*/
#endif
	//fly back after correction
	FlyTo(Fvector().sub(inital_entity_position,m_EntityAlife.Position()));

#ifdef	DEBUG
	/*
if( dbg_draw_ragdoll_spawn )
{
	DBG_OpenCashedDraw();
	m_pPhysicsShell->dbg_draw_geometry( 0.2f, D3DCOLOR_XRGB( 100, 255, 100 ) );
	DBG_ClosedCashedDraw( 50000 );
}
*/
#endif
	Fvector v = velocity;
	m_character_shell_control.apply_start_velocity_factor( who, v );

#ifdef	DEBUG
//		if( death_anim_debug )
//		{
//			Msg( "death anim: ragdoll velocity picked from char controller =(%f,%f,%f), velocity applied to ragdoll =(%f,%f,%f)  ", velocity.x, velocity.y, velocity.z, v.x, v.y, v.z );
//		}
#endif
	m_pPhysicsShell->set_LinearVel( v );
	//actualize
	m_pPhysicsShell->GetGlobalTransformDynamic(&mXFORM);
	m_pPhysicsShell->mXFORM.set(mXFORM);



	//if( false &&  anim_mov_ctrl && anim_mov_blend && anim_mov_blend->blend != CBlend::eFREE_SLOT &&  anim_mov_blend->timeCurrent + Device.fTimeDelta*anim_mov_blend->speed < anim_mov_blend->timeTotal-SAMPLE_SPF-EPS)//.
	//{
	//	const Fmatrix sv_xform = mXFORM;
	//	mXFORM.set( start_xform );
	//	//anim_mov_blend->blendPower = 1;
	//	anim_mov_blend->timeCurrent  += Device.fTimeDelta * anim_mov_blend->speed;
	//	m_pPhysicsShell->AnimToVelocityState( Device.fTimeDelta, 2 * default_l_limit, 10.f * default_w_limit );
	//	mXFORM.set( sv_xform );
	//}
	IKinematics* K=smart_cast<IKinematics*>( m_EntityAlife.Visual( ) );
	//u16 root =K->LL_GetBoneRoot();
	//if( root!=0 )
	//{
	//	K->LL_GetTransform( 0 ).set( Fidentity );
	//	
	//	K->LL_SetBoneVisible( 0, FALSE, FALSE );
	//}
	
	K->CalculateBones_Invalidate();
	K->CalculateBones	(TRUE);
}
void CCharacterPhysicsSupport::in_ChangeVisual()
{
	
	IKinematicsAnimated* KA = smart_cast<IKinematicsAnimated*>(m_EntityAlife.Visual());
	if(m_ik_controller)
	{
		DestroyIKController();
		if( KA )
			CreateIKController();
	}
	//xr_delete(m_interactive_animation);
	destroy_animation_collision();
	destroy(m_interactive_motion);

	if( KA )
	{
		m_death_anims.setup( KA, m_EntityAlife.cNameSect().c_str() , pSettings );
		if( Type( ) != etBitting )
				m_hit_animations.SetupHitMotions( *KA );
	}

	if(!m_physics_skeleton&&!m_pPhysicsShell) return;

	if(m_pPhysicsShell)
	{
		VERIFY(m_eType!=etStalker);
		if(m_physics_skeleton)
		{
			m_EntityAlife.processing_deactivate()	;
			m_physics_skeleton->Deactivate()		;
			xr_delete(m_physics_skeleton)			;
		}
		CreateSkeleton(m_physics_skeleton);
		if(m_pPhysicsShell)m_pPhysicsShell->Deactivate();
		xr_delete(m_pPhysicsShell);
		ActivateShell(NULL);
	}
}

bool CCharacterPhysicsSupport::CanRemoveObject()
{
	if(m_eType==etActor)
	{
		return false;
	}
	else
	{
		return !m_EntityAlife.IsPlaying();
	}
}

void CCharacterPhysicsSupport::PHGetLinearVell(Fvector &velocity)
{
	if(m_pPhysicsShell&&m_pPhysicsShell->isActive())
	{
		m_pPhysicsShell->get_LinearVel(velocity);
	}
	else
		movement()->GetCharacterVelocity(velocity);
		
}

void CCharacterPhysicsSupport::CreateIKController()
{

	VERIFY(!m_ik_controller);
	m_ik_controller=xr_new<CIKLimbsController>();
	m_ik_controller->Create(&m_EntityAlife);
	
}
void CCharacterPhysicsSupport::DestroyIKController()
{
	if(!m_ik_controller)return;
	m_ik_controller->Destroy(&m_EntityAlife);
	xr_delete(m_ik_controller);
}

void		 CCharacterPhysicsSupport::in_NetRelcase(CObject* O)																													
{
	m_PhysicMovementControl->NetRelcase( O );

	if( m_sv_hit.is_valide() && m_sv_hit.initiator() == O )
		m_sv_hit = SHit();
}
 
void CCharacterPhysicsSupport::set_collision_hit_callback( ICollisionHitCallback* cc )
{
	
	xr_delete( m_collision_hit_callback );
	m_collision_hit_callback = cc;

}
ICollisionHitCallback * CCharacterPhysicsSupport::get_collision_hit_callback()
{
	return m_collision_hit_callback;
}



void						CCharacterPhysicsSupport::FlyTo(const	Fvector &disp)
{
		VERIFY(m_pPhysicsShell);
		float ammount=disp.magnitude();
		if(fis_zero(ammount,EPS_L))	return;
		physics_world()->Freeze();
		bool g=m_pPhysicsShell->get_ApplyByGravity();
		m_pPhysicsShell->set_ApplyByGravity(false);
		m_pPhysicsShell->add_ObjectContactCallback(StaticEnvironmentCB);
		void*	cd=m_pPhysicsShell->get_CallbackData();
		m_pPhysicsShell->set_CallbackData(m_pPhysicsShell->PIsland());
		m_pPhysicsShell->UnFreeze();
		Fvector vel;vel.set(disp);
		const	u16	steps_num=10;
		const	float	fsteps_num=steps_num;
		vel.mul(1.f/fsteps_num/fixed_step);


		for(u16	i=0;steps_num>i;++i)
		{
			m_pPhysicsShell->set_LinearVel(vel);
			physics_world()->Step();
		}
		//u16 step_num=disp.magnitude()/fixed_step;
		m_pPhysicsShell->set_ApplyByGravity(g);
		m_pPhysicsShell->set_CallbackData(cd);
		m_pPhysicsShell->remove_ObjectContactCallback(StaticEnvironmentCB);
		physics_world()->UnFreeze();
}

void CCharacterPhysicsSupport::on_create_anim_mov_ctrl	()
{
	VERIFY( !anim_mov_state.active );
	//anim_mov_state.character_exist = m_PhysicMovementControl->CharacterExist(); 
	//if(anim_mov_state.character_exist)
		//m_PhysicMovementControl->DestroyCharacter();
	m_PhysicMovementControl->SetNonInteractive(true);
	anim_mov_state.active = true;
}

void CCharacterPhysicsSupport::on_destroy_anim_mov_ctrl	()
{
	VERIFY( anim_mov_state.active );
	//if( anim_mov_state.character_exist )
						//CreateCharacter();
	m_PhysicMovementControl->SetNonInteractive(false);
	anim_mov_state.active = false;
}

bool CCharacterPhysicsSupport::interactive_motion()
{
	return is_imotion(m_interactive_motion);
}

void		CCharacterPhysicsSupport::in_Die()
{
	if (m_hit_valide_time < Device.dwTimeGlobal || !m_sv_hit.is_valide())
	{
		if (m_EntityAlife.use_simplified_visual())
			return;
		ActivateShell(NULL);
		m_PhysicMovementControl->DestroyCharacter();
		return;
	}
	in_Hit(m_sv_hit, true);
}
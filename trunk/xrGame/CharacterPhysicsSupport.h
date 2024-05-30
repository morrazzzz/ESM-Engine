
#ifndef CHARACTER_PHYSICS_SUPPORT
#define CHARACTER_PHYSICS_SUPPORT

#include "alife_space.h"
#include "PHSkeleton.h"
#include "Entity_Alive.h"
#include "PHSoundPlayer.h"
#include "Phdestroyable.h"
#include "character_hit_animations.h"
#include "character_shell_control.h"

class CPhysicsShell;
class CPHMovementControl;
class CIKLimbsController;
class interactive_motion;
class physics_shell_animated;


class CCharacterPhysicsSupport :
public CPHSkeleton,
public CPHDestroyable
{
public:
enum EType
{
	etActor,
	etStalker,
	etBitting
};

enum EState
{
	esDead,
	esAlive,
	esRemoved
};

private:
	EType								m_eType;
	EState								m_eState;
	Flags8								m_flags;
	enum Fags 
	{
		fl_death_anim_on			=1<<0,
		fl_skeleton_in_shell		=1<<1,
		fl_specific_bonce_demager	=1<<2,
		fl_block_hit				=1<<3,
	};

	struct	animation_movement_state{ 
		bool		active;
		bool		character_exist;
		void		init( ){ active	=  false ; character_exist =  false ; }
		animation_movement_state( )		{ init( ); }
	}									anim_mov_state;

	CEntityAlive						&m_EntityAlife																																		;
	Fmatrix								&mXFORM																																				;
	CPhysicsShell						*&m_pPhysicsShell																																	;
	CPhysicsShell						*m_physics_skeleton																																	;
	CPHMovementControl					*m_PhysicMovementControl																															;
	CPHSoundPlayer						m_ph_sound_player																																	;
	CIKLimbsController					*m_ik_controller																																	;

	ICollisionHitCallback				*m_collision_hit_callback;
	character_hit_animation_controller	m_hit_animations;
	float								m_BonceDamageFactor;
	physics_shell_animated				*m_physics_shell_animated;

	interactive_motion					*m_interactive_motion;
	character_shell_control				m_character_shell_control;
	SHit								m_sv_hit;
	u32									m_hit_valide_time;
	u32									m_physics_shell_animated_time_destroy;
public:
EType Type()
	{
		return m_eType;
	}
EState STate()
	{
		return m_eState;
	}
void	SetState(EState astate)
	{
		m_eState=astate;
	}
IC	bool isDead()
	{
		return m_eState==esDead;
	}
IC	bool isAlive()
	{
		return !m_pPhysicsShell;
	}
protected:
virtual void							SpawnInitPhysics				(CSE_Abstract	*D)																									;
virtual CPhysicsShellHolder*			PPhysicsShellHolder				()	{return m_EntityAlife.PhysicsShellHolder();}	

virtual bool							CanRemoveObject					();
public:
IC		CPHMovementControl				*movement						()	{return m_PhysicMovementControl;}
IC	const	CPHMovementControl			*movement						( ) const{ return m_PhysicMovementControl; }
IC		CPHSoundPlayer					*ph_sound_player				()	{return &m_ph_sound_player;}
IC		CIKLimbsController				*ik_controller					()	{return	m_ik_controller;}
		bool							interactive_motion				( ) ;
		void							SetRemoved						();
		bool							IsRemoved						(){return m_eState==esRemoved;}
		bool							IsSpecificDamager				()																{return !!m_flags.test(fl_specific_bonce_demager)	;}
		float							BonceDamageFactor				(){return m_BonceDamageFactor;}
		void							set_movement_position			( const Fvector &pos );
		void							ForceTransform					( const Fmatrix &m);
//////////////////base hierarchi methods///////////////////////////////////////////////////
		void							CreateCharacter					();
		void 							in_UpdateCL()																																		;
		void 							in_shedule_Update				( u32 DT )																											;
		void 							in_NetSpawn						(CSE_Abstract* e)																									;
		void 							in_NetDestroy					()																													;
		void							destroy_imotion					( );
		void							in_NetRelcase					(CObject* O)																										;
		void 							in_Init							()																													;
		void 							in_Load							(LPCSTR section)																									;
		void 							in_Hit							( SHit &H, bool is_killing=false );
		void							in_NetSave						(NET_Packet& P)																										;

		void							in_ChangeVisual					();
		void							in_Die							();
		void							on_create_anim_mov_ctrl			();
		void							on_destroy_anim_mov_ctrl		();
		void							PHGetLinearVell					(Fvector& velocity);
		ICollisionHitCallback*			get_collision_hit_callback		( );
		void							set_collision_hit_callback		( ICollisionHitCallback* cc );
IC		physics_shell_animated			*animation_collision			( ){ return m_physics_shell_animated; }
IC		const physics_shell_animated	*animation_collision			( )const{ return m_physics_shell_animated; }
		void							create_animation_collision		( );
		void							destroy_animation_collision		( );
private:
	void							update_animation_collision();
public:
/////////////////////////////////////////////////////////////////
		CCharacterPhysicsSupport& operator = (CCharacterPhysicsSupport& /**asup/**/){R_ASSERT2(false,"Can not assign it");}
								CCharacterPhysicsSupport				(EType atype,CEntityAlive* aentity)																					;
virtual							~CCharacterPhysicsSupport				()																													;
private:
		void 							CreateSkeleton					(CPhysicsShell* &pShell)																							;
		void 							CreateSkeleton					();
		void 							ActivateShell					(CObject* who)																										;
		void							CreateShell						( CObject* who, Fvector& dp, Fvector & velocity  )																	;
		void							EndActivateFreeShell			( CObject* who, const Fvector& inital_entity_position, const Fvector& dp, const Fvector & velocity )				;
		void							KillHit							( SHit &H )																											;
static	void							DeathAnimCallback				(CBlend *B)																											;
		void							CreateIKController				()																													;
		void							DestroyIKController				()																													;
		bool							CollisionCorrectObjPos			( const Fvector& start_from, bool character_create=false );
		void							FlyTo							(const	Fvector &disp);
IC		void							UpdateDeathAnims				();
IC		bool							DoCharacterShellCollide			();
};
#endif  //CHARACTER_PHYSICS_SUPPORT
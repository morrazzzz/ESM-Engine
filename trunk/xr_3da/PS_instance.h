#ifndef _CPS_Instance_H_
#define _CPS_Instance_H_

#include "../xrCDB/ispatial.h"
#include "isheduled.h"
#include "irenderable.h"

class ENGINE_API CPS_Instance	:
	public ISpatial,
	public ISheduled,
	public IRenderable
{
	friend class			IGame_Persistent;

private:
	bool					m_destroy_on_game_load;
protected:
	int						m_iLifeTime			;
	BOOL					m_bAutoRemove		;
	BOOL					m_bDead				;

protected:
	virtual					~CPS_Instance		();
	virtual void			PSI_internal_delete	();

public:
							CPS_Instance		(bool destroy_on_game_load);

	IC		const bool		&destroy_on_game_load() const				{	return m_destroy_on_game_load;	}
	virtual void			PSI_destroy			();
	IC BOOL					PSI_alive			()						{	return m_iLifeTime>0;				}
	IC BOOL					PSI_IsAutomatic		()						{	return m_bAutoRemove;				}
	IC void					PSI_SetLifeTime		(float life_time)		{	m_iLifeTime=iFloor(life_time*1000);	}

	virtual void Play()	= 0;
	virtual void UpdateParticles() = 0;

	virtual	shared_str		shedule_Name		() const		{ return shared_str("particle_instance"); };
	bool shedule_Needed() override { return true; }
	float shedule_Scale() override;


	virtual void			shedule_Update		(u32 dt);
	virtual	IRenderable*	dcast_Renderable	()				{ return this;	}
private:
	const Fvector& PositionParticle();
};

#endif

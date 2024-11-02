#ifndef ParticlesObjectH
#define ParticlesObjectH

#include "../xr_3da/PS_instance.h"

extern const Fvector zero_vel;

class CParticlesObject : public CPS_Instance
{
	typedef CPS_Instance	inherited;

	u32					dwLastTime;
	void				Init(std::string_view p_name, IRender_Sector* S, bool bAutoRemove);
	void				UpdateSpatial();

protected:
	bool				m_bLooped;			//флаг, что система зациклена
	bool				m_bStopping;		//вызвана функция Stop()

	virtual				~CParticlesObject();

public:
	CParticlesObject(std::string_view p_name, bool bAutoRemove, bool destroy_on_game_load);

	void renderable_Render() override;
	void UpdateParticles() override;
	void PerformAllTheWork();

	void				SetXFORM(const Fmatrix& m);
	IC	Fmatrix& XFORM() { return renderable.xform; }
	void				UpdateParent(const Fmatrix& m, const Fvector& vel);

	void				play_at_pos(const Fvector& pos, BOOL xform = false);
	virtual void		Play(bool hudMode = false);
	void				Stop(BOOL bDefferedStop = true);

	bool				IsLooped() { return m_bLooped; }
	bool				IsAutoRemove();
	bool				IsPlaying();
	void				SetAutoRemove(bool auto_remove);

	const shared_str			Name();
public:
	static CParticlesObject* Create(std::string_view p_name, bool bAutoRemove = true, bool remove_on_game_load = true)
	{
		return xr_new<CParticlesObject>(p_name, bAutoRemove, remove_on_game_load);
	}
	static void					Destroy(CParticlesObject*& p)
	{
		if (p) {
			p->PSI_destroy();
			p = nullptr;
		}
	}
};

#endif /*ParticlesObjectH*/

#pragma once
#include "weaponmagazined.h"
#include "rocketlauncher.h"


class CWeaponFakeGrenade;


class CWeaponMagazinedWGrenade : public CWeaponMagazined,
								 public CRocketLauncher
{
	typedef CWeaponMagazined inherited;
public:
					CWeaponMagazinedWGrenade	(LPCSTR name="AK74",ESoundTypes eSoundType=SOUND_TYPE_WEAPON_SUBMACHINEGUN);
	virtual			~CWeaponMagazinedWGrenade	();

	virtual void	Load				(LPCSTR section);
	
	virtual BOOL	net_Spawn			(CSE_Abstract* DC);
	virtual void	net_Destroy			();
	virtual void	net_Export			(NET_Packet& P);
	
	virtual void	OnH_B_Independent	(bool just_before_destroy);

	virtual void	save				(NET_Packet &output_packet);
	virtual void	load				(IReader &input_packet);


	virtual bool	Attach(PIItem pIItem, bool b_send_event);
	virtual bool	Detach(const char* item_section_name, bool b_spawn_item);
	virtual bool	CanAttach(PIItem pIItem);
	virtual bool	CanDetach(const char* item_section_name);
	virtual void	InitAddons();
	virtual bool	UseScopeTexture();
	virtual	float	CurrentZoomFactor	();

	
	virtual void	OnStateSwitch	(u32 S);
	
	virtual void	switch2_Idle	();
	virtual void	switch2_Reload	();
	virtual void	state_Fire		(float dt);
	virtual void	OnShot			();
	virtual void	SwitchState		(u32 S);
	virtual void	OnEvent			(NET_Packet& P, u16 type);
	virtual void	ReloadMagazine	();

	virtual bool	Action			(s32 cmd, u32 flags);

	virtual void	UpdateSounds	();
	virtual void	StopHUDSounds	();

	//������������ � ����� �������������
	virtual bool	SwitchMode		();
	void			PerformSwitchGL	();
	void			OnAnimationEnd	(u32 state);

	virtual bool	IsNecessaryItem	    (const shared_str& item_sect);

	//����������� ������� ��� ������������ �������� HUD
	virtual void	PlayAnimShow();
	virtual void	PlayAnimHide();
	virtual void	PlayAnimReload();
	virtual void	PlayAnimIdle();
	virtual void	PlayAnimShoot();
	virtual void	PlayAnimModeSwitch();
	
	HUD_SOUND			sndShotG;
	HUD_SOUND			sndReloadG;
	HUD_SOUND			sndSwitch;


	//�������� � ������������ ��������������
	//(����� ������� ��������)
	MotionSVec			mhud_idle_g;
	MotionSVec			mhud_idle_g_aim;
	MotionSVec			mhud_reload_g;
	MotionSVec			mhud_shots_g;
	MotionSVec			mhud_switch_g, mhud_switch;
	MotionSVec			mhud_show_g;
	MotionSVec			mhud_hide_g;
	//(����� �������� �� �������������)
	MotionSVec			mhud_idle_w_gl;
	MotionSVec			mhud_idle_w_gl_aim;
	MotionSVec			mhud_reload_w_gl;
	MotionSVec			mhud_shots_w_gl;
	MotionSVec			mhud_show_w_gl;
	MotionSVec			mhud_hide_w_gl;


	//�������������� ��������� �������� 
	//��� �������������
	CWeaponAmmo*			m_pAmmo2;
	shared_str				m_ammoSect2;
	xr_vector<shared_str>	m_ammoTypes2;
	u32						m_ammoType2;
	shared_str				m_ammoName2;
	int						iMagazineSize2;
	xr_vector<CCartridge>	m_magazine2;
	bool					m_bGrenadeMode;

	CCartridge				m_DefaultCartridge2;

	virtual void UpdateGrenadeVisibility(bool visibility);

	//�������� �������� ��� ������� ������������� � HUD
	shared_str grenade_bone_name;
};
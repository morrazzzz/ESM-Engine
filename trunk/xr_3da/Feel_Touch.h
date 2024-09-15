#pragma once

#include "pure_relcase.h"

class ENGINE_API CObject;

namespace Feel
{
	class ENGINE_API Touch: private pure_relcase
	{
		friend class pure_relcase;

		std::mutex ObjectsFeelLocked;

	public:
		struct DenyTouch
		{
			CObject* O;
			DWORD		Expire;
		};
	private:
		xr_vector<DenyTouch> FeelTouchDisableCopy;
		xr_vector<CObject*> FeelTouchCopy;
	protected:
		xr_vector<DenyTouch> feel_touch_disable;
	public:
		xr_vector<CObject*>		feel_touch;

		Touch();
		virtual	~Touch() = default;

		void __stdcall			feel_touch_relcase(CObject* O);
		virtual BOOL			feel_touch_contact			(CObject* O);
		virtual void			feel_touch_update			(Fvector& P, float	R);
		virtual void			feel_touch_deny				(CObject* O, DWORD	T);
		virtual void			feel_touch_new				(CObject* O)			{	};
		virtual void			feel_touch_delete			(CObject* O)			{	};
	};
};

////////////////////////////////////////////////////////////////////////////
//	Module 		: restriction_space.h
//	Created 	: 30.08.2004
//  Modified 	: 30.08.2004
//	Author		: Dmitriy Iassenev
//	Description : Restriction space
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace RestrictionSpace {
	struct CTimeIntrusiveBase : public intrusive_base {
		u32			m_last_time_dec;

		IC			CTimeIntrusiveBase	() : m_last_time_dec(0)
		{
		}

		template <typename T>
		IC	void	_release		(T*object)
		{
			m_last_time_dec = Device.dwTimeGlobal;
		}
	};

	enum ERestrictorTypes : u32{
		eDefaultRestrictorTypeNone = static_cast<u8>(0),
		eDefaultRestrictorTypeOut  = static_cast<u8>(1),
		eDefaultRestrictorTypeIn   = static_cast<u8>(2),
		eRestrictorTypeNone		   = static_cast<u8>(3),
		eRestrictorTypeIn		   = static_cast<u8>(4),
		eRestrictorTypeOut		   = static_cast<u8>(5),
	};
};


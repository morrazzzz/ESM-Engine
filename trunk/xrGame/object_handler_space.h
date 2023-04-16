////////////////////////////////////////////////////////////////////////////
//	Module 		: object_handler_space.h
//	Created 	: 08.05.2004
//  Modified 	: 08.05.2004
//	Author		: Dmitriy Iassenev
//	Description : Object handler space
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace ObjectHandlerSpace {
	enum EWorldProperties : u32{
		eWorldPropertyItemID		= static_cast<u32>(0),
		eWorldPropertyHidden,
		eWorldPropertyStrapped,
		eWorldPropertyStrapped2Idle,
		eWorldPropertySwitch1,
		eWorldPropertySwitch2,
		eWorldPropertyAimed1,
		eWorldPropertyAimed2,
		eWorldPropertyAiming1,
		eWorldPropertyAiming2,
		eWorldPropertyEmpty1,
		eWorldPropertyEmpty2,
		eWorldPropertyFull1,
		eWorldPropertyFull2,
		eWorldPropertyReady1,
		eWorldPropertyReady2,
		eWorldPropertyFiring1,
		eWorldPropertyFiring2,
		eWorldPropertyAmmo1,
		eWorldPropertyAmmo2,
		eWorldPropertyIdle,
		eWorldPropertyIdleStrap,
		eWorldPropertyDropped,
		eWorldPropertyQueueWait1,
		eWorldPropertyQueueWait2,
		eWorldPropertyAimingReady1,
		eWorldPropertyAimingReady2,
		eWorldPropertyAimForceFull1,
		eWorldPropertyAimForceFull2,
		
		eWorldPropertyThrowStarted,
		eWorldPropertyThrowIdle,
		eWorldPropertyThrow,
		eWorldPropertyThreaten,

		eWorldPropertyPrepared,
		eWorldPropertyUsed,
		eWorldPropertyUseEnough,

		eWorldPropertyNoItems				= static_cast<u32>((u16(-1) << 16) | eWorldPropertyItemID),
		eWorldPropertyNoItemsIdle			= static_cast<u32>((u16(-1) << 16) | eWorldPropertyIdle),
		eWorldPropertyDummy					= static_cast<u32>(-1),
	};

	enum EWorldOperators {
		eWorldOperatorShow			= static_cast<u32>(0),
		eWorldOperatorHide,
		eWorldOperatorDrop,
		eWorldOperatorStrapping,
		eWorldOperatorStrapping2Idle,
		eWorldOperatorUnstrapping,
		eWorldOperatorUnstrapping2Idle,
		eWorldOperatorStrapped,
		eWorldOperatorIdle,
		eWorldOperatorAim1,
		eWorldOperatorAim2,
		eWorldOperatorAimForceFull1,
		eWorldOperatorAimForceFull2,
		eWorldOperatorReload1,
		eWorldOperatorReload2,
		eWorldOperatorForceReload1,
		eWorldOperatorForceReload2,
		eWorldOperatorFire1,
		eWorldOperatorFire2,
		eWorldOperatorSwitch1,
		eWorldOperatorSwitch2,
		eWorldOperatorQueueWait1,
		eWorldOperatorQueueWait2,
		eWorldOperatorAimingReady1,
		eWorldOperatorAimingReady2,
		eWorldOperatorGetAmmo1,
		eWorldOperatorGetAmmo2,

		eWorldOperatorThrowStart,
		eWorldOperatorThrowIdle,
		eWorldOperatorThrow,
		eWorldOperatorThreaten,
		eWorldOperatorAfterThreaten,

		eWorldOperatorPrepare,
		eWorldOperatorUse,

		eWorldOperatorNoItemsIdle	= static_cast<u32>((u16(-1) << 16) | eWorldOperatorIdle),
		eWorldOperatorDummy			= static_cast<u32>(-1),
	};
};
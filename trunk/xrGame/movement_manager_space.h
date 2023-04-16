////////////////////////////////////////////////////////////////////////////
//	Module 		: movement_manager_space.h
//	Created 	: 02.10.2001
//  Modified 	: 12.11.2003
//	Author		: Dmitriy Iassenev
//	Description : Movement manager space
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace MovementManager {
	enum EPathType : u32 {
		ePathTypeGamePath = static_cast<u32>(0),
		ePathTypeLevelPath,
		ePathTypePatrolPath,
		ePathTypeNoPath,
		ePathTypeDummy = static_cast<u32>(-1),
	};
};


////////////////////////////////////////////////////////////////////////////
//	Module 		: patrol_path_manager_space.h
//	Created 	: 03.12.2003
//  Modified 	: 03.12.2003
//	Author		: Dmitriy Iassenev
//	Description : Patrol path manager space
////////////////////////////////////////////////////////////////////////////

#pragma once

namespace PatrolPathManager {
	enum EPatrolStartType : u32 {
		ePatrolStartTypeFirst = static_cast<u32>(0),
		ePatrolStartTypeLast,
		ePatrolStartTypeNearest,
		ePatrolStartTypePoint,
		ePatrolStartTypeNext,
		ePatrolStartTypeDummy = static_cast<u32>(-1),
	};
	enum EPatrolRouteType : u32{
		ePatrolRouteTypeStop = static_cast<u32>(0),
		ePatrolRouteTypeContinue,
		ePatrolRouteTypeDummy = static_cast<u32>(-1),
	};
};


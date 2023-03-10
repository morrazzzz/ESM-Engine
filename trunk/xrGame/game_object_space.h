#pragma once

namespace GameObject {
	enum ECallbackType {
		eTradeStart = static_cast<u32>(0),
		eTradeStop,
		eTradeSellBuyItem,
		eTradePerformTradeOperation,

		eZoneEnter,
		eZoneExit,
		eExitLevelBorder,
		eEnterLevelBorder,
		eDeath,

		ePatrolPathInPoint,

		eInventoryPda,
		eInventoryInfo,
		eArticleInfo,
		eTaskStateChange,
		eMapLocationAdded,

		eUseObject,

		eHit,

		eSound,

		eActionTypeMovement,
		eActionTypeWatch,
		eActionTypeAnimation,
		eActionTypeSound,
		eActionTypeParticle,
		eActionTypeObject,

		eActorSleep,

		eHelicopterOnPoint,
		eHelicopterOnHit,

		eOnItemTake,
		eOnItemDrop,

		eScriptAnimation,
		
		eTraderGlobalAnimationRequest,
		eTraderHeadAnimationRequest,
		eTraderSoundEnd,

		eInvBoxItemTake,

		eSelectPdaContact,

		eDummy = static_cast<u32>(-1),
	};
};

enum EBoneCallbackType{
	bctDummy			= static_cast<u32>(0),	// 0 - required!!!
	bctPhysics,
	bctCustom,
	bctForceU32			= static_cast<u32>(-1),
};
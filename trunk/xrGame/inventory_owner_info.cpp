//////////////////////////////////////////////////////////////////////
// inventory_owner_info.h:	для работы с сюжетной информацией
//
//////////////////////////////////////////////////////////////////////

#include "pch_script.h"
#include "InventoryOwner.h"
#include "GameObject.h"
#include "xrMessages.h"
#include "ai_debug.h"
#include "alife_registry_container.h"
#include "script_game_object.h"
#include "level.h"
#include "infoportion.h"
#include "alife_registry_wrappers.h"
#include "script_callback_ex.h"
#include "game_object_space.h"

class CFindByIDPred
{
public:
	CFindByIDPred(shared_str element_to_find) {element = element_to_find;}
	bool operator () (const INFO_DATA& data) const {return data.info_id == element;}
private:
	shared_str element;
};


bool CInventoryOwner::OnReceiveInfo(shared_str info_id, CInfoPortion* infoPortion) const
{
	VERIFY( info_id.size() );
	//добавить запись в реестр
	KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();
	KNOWN_INFO_VECTOR_IT it = std::find_if(known_info.begin(), known_info.end(), CFindByIDPred(info_id));
	if( known_info.end() == it)
		known_info.push_back(INFO_DATA(info_id, Level().GetGameTime()));
	else
		return false;

#ifdef DEBUG
	if(psAI_Flags.test(aiInfoPortion))
		Msg("[%s] Received Info [%s]", Name(), *info_id);
#endif

	//Запустить скриптовый callback
	const CGameObject* pThisGameObject = smart_cast<const CGameObject*>(this);
	VERIFY(pThisGameObject);

//	SCRIPT_CALLBACK_EXECUTE_2(*m_pInfoCallback, pThisGameObject->lua_game_object(), info_index);
//	pThisGameObject->callback(GameObject::eInventoryInfo)(pThisGameObject->lua_game_object(), *info_id);
	

	CInfoPortion* infoPortionReceive = nullptr;
	if (!infoPortion)
	{
		CInfoPortion infoPortionLoad;
		infoPortionLoad.Load(info_id);
		infoPortionReceive = &infoPortionLoad;
	}
	else
		infoPortionReceive = infoPortion;

	//запустить скриптовые функции
	infoPortionReceive->RunScriptActions(pThisGameObject);

	//выкинуть те info portions которые стали неактуальными
	for (u32 i = 0; i < infoPortionReceive->DisableInfos().size(); i++)
		TransferInfo(infoPortionReceive->DisableInfos()[i], false);

	return true;
}
#ifdef DEBUG
void CInventoryOwner::DumpInfo() const
{
	KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();

	Msg("------------------------------------------");	
	Msg("Start KnownInfo dump for [%s]",Name());	
	KNOWN_INFO_VECTOR_IT it = known_info.begin();
	for(int i=0;it!=known_info.end();++it,++i){
		Msg("known info[%d]:%s",i,*(*it).info_id);	
	}
	Msg("------------------------------------------");	

}
#endif

void CInventoryOwner::OnDisableInfo(shared_str info_id) const
{
	VERIFY( info_id.size() );
	//удалить запись из реестра
	
#ifdef DEBUG
	if(psAI_Flags.test(aiInfoPortion))
		Msg("[%s] Disabled Info [%s]", Name(), *info_id);
#endif

	KNOWN_INFO_VECTOR& known_info = m_known_info_registry->registry().objects();

	KNOWN_INFO_VECTOR_IT it = std::find_if(known_info.begin(), known_info.end(), CFindByIDPred(info_id));
	if( known_info.end() == it)	return;
	known_info.erase(it);
}

void CInventoryOwner::TransferInfo(shared_str info_id, bool add_info) const
{
	VERIFY( info_id.size() );

	if(add_info)
		OnReceiveInfo(info_id, nullptr);
	else
		OnDisableInfo(info_id);
}

bool CInventoryOwner::HasInfo(shared_str info_id) const
{
	VERIFY( info_id.size() );
	const KNOWN_INFO_VECTOR* known_info = m_known_info_registry->registry().objects_ptr ();
	if(!known_info) return false;

	if(std::find_if(known_info->begin(), known_info->end(), CFindByIDPred(info_id)) == known_info->end())
		return false;

	return true;
}

bool CInventoryOwner::GetInfo	(shared_str info_id, INFO_DATA& info_data) const
{
	VERIFY( info_id.size() );
	const KNOWN_INFO_VECTOR* known_info = m_known_info_registry->registry().objects_ptr ();
	if(!known_info) return false;

	KNOWN_INFO_VECTOR::const_iterator it = std::find_if(known_info->begin(), known_info->end(), CFindByIDPred(info_id));
	if(known_info->end() == it)
		return false;

	info_data = *it;
	return true;
}
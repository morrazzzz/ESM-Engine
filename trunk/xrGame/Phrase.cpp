///////////////////////////////////////////////////////////////
// Phrase.cpp
// класс, описывающий фразу (элемент диалога)
///////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "phrase.h"

#include "ai_space.h"
#include "gameobject.h"
#include "script_game_object.h"
#include <ai_debug.h>

CPhrase::CPhrase()
{
	m_b_finalizer = false;
	m_ID				= "";
	m_iGoodwillLevel	= 0;
}
CPhrase::~CPhrase()
{
}

LPCSTR CPhrase::GetText()	const			
{
	return m_text.c_str();
}

bool	CPhrase::IsDummy()		const
{
	if( xr_strlen(GetText()) == 0 )
		return true;

	return false;
}

void CPhrase::AddTextsForPhrase(const char* text)
{
	phraseTexts.emplace_back(text);
}

void CPhrase::ReserveCountTextsForPhrase(int reserve)
{
	phraseTexts.reserve(static_cast<size_t>(reserve));
}

void CPhrase::RandomSetTextFromTexts()
{
	if (phraseTexts.empty())
		return;

	int randomIndex = Random.randI(phraseTexts.size());
	const char* newText = phraseTexts[randomIndex].c_str();

#ifdef DEBUG
	if (psAI_Flags.test(aiDialogs))
	{
		Msg("~#~ [%s]: Random text for phrase id: [%d]. Old text: [%s], new text: [%s], random integer: [%d], count texts: [%d]",
			__FUNCTION__, m_ID.c_str(), m_text.c_str(), newText, randomIndex, phraseTexts.size());
	}
#endif
	SetText(newText);
}
#pragma once
#include "UILabel.h"
#include "../script_export_space.h"
#include "UIOptionsItem.h"
#include "UICustomEdit.h"
class CUIFrameLineWnd;

class CUIEditBox : public CUIOptionsItem, public CUICustomEdit
{
public:
					CUIEditBox		();
	virtual			~CUIEditBox		();

	virtual void	Init(float x, float y, float width, float heigt);

	// CUIOptionsItem
	virtual void			SetCurrentOptValue	();// opt->current
	virtual void			SaveBackUpOptValue	();// current->backup
	virtual void			SaveOptValue		();// current->opt
	virtual void			UndoOptValue		();// backup->current
	virtual bool			IsChangedOptValue	() const;// backup!=current

	// CUIMultiTextureOwner
	virtual void	InitTexture				(LPCSTR texture);
	virtual void	InitTextureEx			(LPCSTR texture, LPCSTR  shader);
protected:
	CUIFrameLineWnd*	m_frameLine;
	shared_str			m_opt_backup_value;
	DECLARE_SCRIPT_REGISTER_FUNCTION
};

add_to_type_list(CUIEditBox)
#undef script_type_list
#define script_type_list save_type_list(CUIEditBox)

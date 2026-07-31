#include "stdafx.h"
#include "UIWeaponScope.h"
#include "UIStatic.h"
#include "UIXmlInit.h"

CUIWeaponScope::~CUIWeaponScope()
{
	delete WindowScope;
	delete ScopeTexture;
}

void CUIWeaponScope::InitScope(CUIXml* WpnScopeXml, const char* tex_name, bool TextureScope)
{
	if (TextureScope)
	{
		ScopeTexture = new CUIStaticItem();

		ScopeTexture->Init(tex_name, "hud\\default", 0, 0);
        return;
	}

	VERIFY(WpnScopeXml);
	WindowScope	 = new CUIWindow();
	CUIXmlInit::InitWindow(*WpnScopeXml, tex_name, 0, WindowScope);
}

void CUIWeaponScope::DrawScope()
{
	if (ScopeTexture)
	{
		ScopeTexture->SetPos(0, 0);
		ScopeTexture->SetSize(Fvector2().set(UI_BASE_WIDTH, UI_BASE_HEIGHT));
		ScopeTexture->Render();
		return;
	}

	WindowScope->Draw();
}
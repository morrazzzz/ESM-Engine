#pragma once

#include "UIWindow.h"

class CUIStaticItem;
class CUIXml;

class CUIWeaponScope/* : public CUIWindow*/
{
    CUIStaticItem* ScopeTexture = nullptr;
    CUIWindow* WindowScope = nullptr;
public:
    CUIWeaponScope() = default;
    ~CUIWeaponScope();

    void InitScope(CUIXml*, const char*, bool texture = false);
    void DrawScope();

    CUIWindow* GetWindowScope() { return WindowScope; }
};


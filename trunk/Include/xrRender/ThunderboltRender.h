#pragma once

class CEffect_Thunderbolt;

class IThunderboltRender
{
public:
	virtual ~IThunderboltRender() {;}
	virtual void Copy(IThunderboltRender &_in) = 0;

	virtual void Render(CEffect_Thunderbolt &owner) = 0;
};

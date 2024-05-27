#pragma once
#include "physicsexternalcommon.h"
class IPHStaticGeomShell
{
	protected:
		virtual								~IPHStaticGeomShell() =0{}
	//	virtual void						set_ObjectContactCallback	(ObjectContactCallbackFun* callback);											
};

class IPhysicsShellHolder;
class IClimableObject;
IPHStaticGeomShell	*P_BuildStaticGeomShell(IPhysicsShellHolder* obj,ObjectContactCallbackFun* object_contact_callback);
IPHStaticGeomShell	*P_BuildLeaderGeomShell( IClimableObject* obj, ObjectContactCallbackFun* callback, const Fobb &b );
void				DestroyStaticGeomShell( IPHStaticGeomShell* &p );

//CPHStaticGeomShell* P_BuildStaticGeomShell(CGameObject* obj,ObjectContactCallbackFun* object_contact_callback,Fobb &b);
//void				P_BuildStaticGeomShell(CPHStaticGeomShell* shell,CGameObject* obj,ObjectContactCallbackFun* object_contact_callback,Fobb &b);
#pragma	once


bool			valid_pos(const Fvector& P);
const Fbox& ph_boundaries();
#ifdef DEBUG
//class IPhysicsShellHolder;
//std::string dbg_valide_pos_string(const Fvector& pos, const Fbox& bounds, const IPhysicsShellHolder* obj, LPCSTR msg);
//std::string dbg_valide_pos_string(const Fvector& pos, const IPhysicsShellHolder* obj, LPCSTR msg);

class CPhysicsShellHolder;
std::string dbg_valide_pos_string(const Fvector& pos, const Fbox& bounds, const CPhysicsShellHolder* obj, LPCSTR msg);
std::string dbg_valide_pos_string(const Fvector& pos, const CPhysicsShellHolder* obj, LPCSTR msg);

#define	VERIFY_BOUNDARIES2(pos,bounds,obj,msg) VERIFY2(  valid_pos( pos, bounds ), dbg_valide_pos_string( pos, bounds, obj, msg ) )
#define	VERIFY_BOUNDARIES(pos,bounds,obj)	VERIFY_BOUNDARIES2(pos,bounds,obj,"	")

#else
#define	VERIFY_BOUNDARIES(pos,bounds,obj)
#define	VERIFY_BOUNDARIES2(pos,bounds,obj,msg)
#endif
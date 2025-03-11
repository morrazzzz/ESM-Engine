////////////////////////////////////////////////////////////////////////////
//	Module 		: id_generator.h
//	Created 	: 28.08.2003
//  Modified 	: 28.08.2003
//	Author		: Dmitriy Iassenev and Oles' Shyshkovtsov
//	Description : ID generation class template
////////////////////////////////////////////////////////////////////////////

#pragma once

#define NEW_GENERATOR_DEBUG

class CID_Generator {
private:
	xr_vector<u16> FreeIDs{};
#ifdef NEW_GENERATOR_DEBUG
	xr_vector<u16> IDsReg{};
#endif
	//xr_unordered_map<CSE_Abstract*, u16> ObjectsIDs;

	//bool FreeLastID{ true };
	//u32 PrevIDGen;
public:
	CID_Generator()
	{
		ClearIDS();
	};

	IC void ClearIDS()
	{
		for (u16 i = static_cast<u16>(-1); i > 1; i--)
			FreeIDs.emplace_back(static_cast<u16>(i - 1));
	}

	IC u16 tfGetID(u16 tValueID = static_cast<u16>(-1))
	{
		R_ASSERT2(!FreeIDs.empty(), "Not enough IDs");

		//TODO: Not actual???
		if (tValueID != static_cast<u16>(-1))
		{
			R_ASSERT2(false, "Fix this???");
			//ObjectsIDs.insert()
			u16 id = FreeIDs[FreeIDs.size() - tValueID];
			FreeIDs.erase(FreeIDs.end() - tValueID);
#ifdef NEW_GENERATOR_DEBUG
			R_ASSERT(std::find(IDsReg.begin(), IDsReg.end(), id) == IDsReg.end());
			IDsReg.emplace_back(id);
#endif
			return id;
		}

		//R_ASSERT2(FreeLastID, "Not enough IDs");
		//FreeLastID = false;

		u16 index = static_cast<u16>(FreeIDs.size() - Random.randI(1, 256));

		u16 id = 0;
		if (index >= FreeIDs.size())
		{
			id = FreeIDs.front();
			index = 0;
		}
		else
			id = FreeIDs[index];

#ifdef NEW_GENERATOR_DEBUG
		R_ASSERT(std::find(IDsReg.begin(), IDsReg.end(), id) == IDsReg.end());
		IDsReg.emplace_back(id);
#endif
		FreeIDs.erase(FreeIDs.begin() + index);

		return id;//static_cast<u16>(-1);
	}

	IC void vfFreeID(u16 valueID)
	{
#ifdef NEW_GENERATOR_DEBUG
		auto it = std::find(IDsReg.begin(), IDsReg.end(), valueID);
		R_ASSERT(it != IDsReg.end());
		IDsReg.erase(it);
#endif

		FreeIDs.emplace_back(valueID);
	}

#ifdef NEW_GENERATOR_DEBUG
	IC void VerifyRegisterID(u16 valueID)
	{
		auto it = std::find(IDsReg.begin(), IDsReg.end(), valueID);
		R_ASSERT(it != IDsReg.end());
		
		auto it2 = std::find(FreeIDs.begin(), FreeIDs.end(), valueID);
		R_ASSERT(it2 == FreeIDs.end());
	}
#endif
};

extern CID_Generator IDGeneratorManager;
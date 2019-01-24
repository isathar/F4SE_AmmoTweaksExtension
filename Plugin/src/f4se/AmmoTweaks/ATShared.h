#pragma once
#include "ATDefinitions.h"



// Papyrus VM:

struct StaticFunctionTag;
class VirtualMachine;


/********************************************
	Shared functions:
*/



namespace ATShared
{
	// returns a form's plugin's name
	const char* GetPluginNameFromFormID(UInt32 formID);

	// returns a formatted string containing (string=pluginName|UInt=formID without loadorder) 
	const char* GetIdentifierFromFormID(UInt32 formID, const char* strSplit = "|");

	// returns a formID from a formatted string
	UInt32 GetFormIDFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");

	// returns a form from a formatted string
	TESForm * GetFormFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");


	bool RegisterPapyrus(VirtualMachine * vm);

	TESObjectWEAP::InstanceData * GetInstanceData_WeapRef(TESForm* form);
	TESObjectWEAP::InstanceData * GetInstanceData_WeapForm(TESObjectWEAP* form);
	TESObjectWEAP::InstanceData * GetInstanceData_WeapEquipped(Actor* actor, UInt32 iSlotIndex);

	extern ATSharedData* SharedData;

}




namespace ATConfig
{
	void EditGameData();
	void SaveGameDataToINI();
}


namespace ATWeaponRef
{
	bool RegisterPapyrus(VirtualMachine * vm);
}

namespace ATNPCTweaks
{
	bool RegisterPapyrus(VirtualMachine * vm);
}

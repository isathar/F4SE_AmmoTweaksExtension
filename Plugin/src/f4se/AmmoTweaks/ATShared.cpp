#include "ATShared.h"



bool ATShared::RegisterPapyrus(VirtualMachine * vm)
{
	ATWeaponRef::RegisterPapyrus(vm);
	return true;
}


const char* ATShared::GetPluginNameFromFormID(UInt32 formID)
{
	char *modName = "none";
	if (formID > 0x0) {
		char varVal[9] = "00000000";
		sprintf_s(varVal, 9, "%08X", formID);
		std::string formStr = varVal;
		//formStr->append(varVal);

		std::string indexStr = formStr.substr(0, 2);
		//indexStr->append(formStr->substr(0, 2));

		unsigned int modIndexInt = 0;
		sscanf_s(indexStr.c_str(), "%02X", &modIndexInt);
		UInt8 modIndex = modIndexInt;
		ModInfo *tempMod = nullptr;

		if (modIndex != 0xFF) {
			(*g_dataHandler)->modList.loadedMods.GetNthItem(modIndex, tempMod);
			modName = tempMod->name;
		}
		else {
			if (modIndex == 0xFE) {
				std::string lightindexStr = formStr.substr(2, 2);
				unsigned int lightIndexInt = 0;
				sscanf_s(lightindexStr.c_str(), "%02X", &lightIndexInt);
				UInt8 lightIndex = lightIndexInt;

				if (lightIndex != 0xFF) {
					(*g_dataHandler)->modList.lightMods.GetNthItem(lightIndex, tempMod);
					modName = tempMod->name;
				}
			}
			else
				modName = "References";
		}
	}
	return modName;
}


const char* ATShared::GetIdentifierFromFormID(UInt32 formID, const char* strSplit)
{
	if (formID > 0x0) {
		std::string *finalFormString = new std::string(GetPluginNameFromFormID(formID));
		
		char varVal[9] = "00000000";
		sprintf_s(varVal, 9, "%08X", formID);
		std::string tempFormString = varVal;
		
		finalFormString->append(strSplit);
		finalFormString->append(tempFormString.substr(2));

		return finalFormString->c_str();
	}
	else
		return "none";
}


UInt32 ATShared::GetFormIDFromIdentifier(const std::string & formIdentifier, const char* strSplit)
{
	UInt32 formId = 0x0;
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of(strSplit);
		std::string *modName = new std::string();
		std::string *modForm = new std::string();
		modName->append(formIdentifier.substr(0, pos));
		modForm->append(formIdentifier.substr(pos + 1));
		sscanf_s(modForm->c_str(), "%X", &formId);

		if (formId != 0x0) {
			UInt8 modIndex = (*g_dataHandler)->GetLoadedModIndex(modName->c_str());
			if (modIndex != 0xFF) {
				formId |= ((UInt32)modIndex) << 24;
			}
			else {
				UInt16 lightModIndex = (*g_dataHandler)->GetLoadedLightModIndex(modName->c_str());
				if (lightModIndex != 0xFFFF) {
					formId |= 0xFE000000 | (UInt32(lightModIndex) << 12);
				}
			}
		}
	}
	//_MESSAGE("New FormID = %s - %08X", formIdentifier.c_str(), formId);
	return formId;

}


TESForm * ATShared::GetFormFromIdentifier(const std::string & formIdentifier, const char* strSplit)
{
	UInt32 formId = GetFormIDFromIdentifier(formIdentifier, strSplit);
	if (formId > 0x0)
		return LookupFormByID(formId);
	else
		return nullptr;
}


namespace ATShared
{
	tArray<UInt32> index_Weapons;
	tArray<UInt32> index_Calibers;

	tArray<ATCaliber> ATCalibers;
	tArray<ATWeapon> ATWeapons;

	tArray<ATCritEffectTable> ATCritEffectTables;
}

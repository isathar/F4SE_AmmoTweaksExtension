#include "ATShared.h"
#include <string>



// cached data
namespace ATGameData
{
	tArray<ATWeapon> ATWeapons;
	tArray<ATCritEffectTable> ATCritEffectTables;
	tArray<ATCritEffectTable> ATCritFailureTables;
	tArray<ATCraftingCategory> ATCraftingData;
}

namespace ATUtilities
{
	ATxoroshiro128p ATrng;
}


// ---------------- FormID/Identifier Utilities:

// returns a form's plugin name
const char * ATUtilities::GetPluginNameFromFormID(UInt32 formID)
{
	if (formID > 0x0) {
		char varVal[9] = "00000000";
		sprintf_s(varVal, 9, "%08X", formID);
		std::string formStr = varVal;
		std::string indexStr = formStr.substr(0, 2);

		unsigned int modIndexInt = 0;
		sscanf_s(indexStr.c_str(), "%02X", &modIndexInt);
		UInt8 modIndex = modIndexInt;
		ModInfo *tempMod = nullptr;

		if (modIndex != 0xFF) {
			(*g_dataHandler)->modList.loadedMods.GetNthItem(modIndex, tempMod);
			return tempMod->name;
		}
		else {
			if (modIndex == 0xFE) {
				std::string lightindexStr = formStr.substr(2, 2);
				unsigned int lightIndexInt = 0;
				sscanf_s(lightindexStr.c_str(), "%02X", &lightIndexInt);
				UInt8 lightIndex = lightIndexInt;

				if (lightIndex != 0xFF) {
					(*g_dataHandler)->modList.lightMods.GetNthItem(lightIndex, tempMod);
					return tempMod->name;
				}
			}
			else
				return "References";
		}
	}
	return "none";
}

// returns a formatted string containing (string=pluginName|UInt=formID without loadorder) 
const char * ATUtilities::GetIdentifierFromFormID(UInt32 formID)
{
	if (formID > 0x0) {
		std::string finalFormString = GetPluginNameFromFormID(formID);
		
		char varVal[9] = "00000000";
		sprintf_s(varVal, 9, "%08X", formID);
		std::string tempFormString = varVal;
		
		finalFormString.append("|");
		finalFormString.append(tempFormString.substr(2));

		return finalFormString.c_str();
	}
	else
		return "none";
}

// returns a formID from a formatted string
UInt32 ATUtilities::GetFormIDFromIdentifier(const std::string & formIdentifier)
{
	UInt32 formId = 0;
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of("|");
		std::string modName = formIdentifier.substr(0, pos);
		std::string modForm = formIdentifier.substr(pos + 1);
		sscanf_s(modForm.c_str(), "%X", &formId);

		if (formId != 0x0) {
			UInt8 modIndex = (*g_dataHandler)->GetLoadedModIndex(modName.c_str());
			if (modIndex != 0xFF) {
				formId |= ((UInt32)modIndex) << 24;
			}
			else {
				UInt16 lightModIndex = (*g_dataHandler)->GetLoadedLightModIndex(modName.c_str());
				if (lightModIndex != 0xFFFF) {
					formId |= 0xFE000000 | (UInt32(lightModIndex) << 12);
				}
				else {
					_MESSAGE("FormID %s not found!", formIdentifier.c_str());
					formId = 0;
				}
			}
		}
	}
	return formId;
}

// returns a form from a formatted string
TESForm * ATUtilities::GetFormFromIdentifier(const std::string & formIdentifier)
{
	UInt32 formId = GetFormIDFromIdentifier(formIdentifier);
	return (formId > 0x0) ? LookupFormByID(formId) : nullptr;
}

// reads an ATCritTableBase::CritEffect (Spell, max. roll value) from the passed identifier string
bool ATUtilities::GetCritEffectFromIdentifier(const std::string & formIdentifier, ATCritEffect & tempEffect)
{
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of(", ");
		std::string spellIdentifier = formIdentifier.substr(0, pos);
		std::string spellValueStr = formIdentifier.substr(pos + 2);
		
		tempEffect.critSpell = (SpellItem*)GetFormFromIdentifier(spellIdentifier.c_str());
		tempEffect.rollMax = std::stoi(spellValueStr);
		return true;
	}
	return false;
}


//********************* Weapon instanceData:

// returns the instanceData of a passed weapon reference
TESObjectWEAP::InstanceData * ATUtilities::GetWeapRefInstanceData(VMRefOrInventoryObj * curObj)
{
	if (curObj) {
		TESObjectREFR * tempRef = curObj->GetObjectReference();
		// only allow weapons with objectreferences
		if (tempRef) {
			if (tempRef->extraDataList) {
				BSExtraData * extraData = tempRef->extraDataList->GetByType(ExtraDataType::kExtraData_InstanceData);
				if (extraData) {
					ExtraInstanceData * objectModData = DYNAMIC_CAST(extraData, BSExtraData, ExtraInstanceData);
					if (objectModData) {
						TESObjectWEAP::InstanceData* weapInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(objectModData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
						if (weapInstanceData) {
							return weapInstanceData;
						}
					}
				}
			}
		}
	}
	return nullptr;
}

// returns an actor's equipped weapon form
TESObjectWEAP * ATUtilities::GetEquippedWeapon(Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor) {
		// Invalid slot id
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		ActorEquipData * equipData = ownerActor->equipData;
		if (!equipData)
			return nullptr;

		// Make sure there is an item in this slot
		TESForm *item = equipData->slots[iEquipSlot].item;
		if (!item)
			return nullptr;

		return (TESObjectWEAP*)item;
	}
	return nullptr;
}

// returns the instanceData of thisActor's weapon equipped at slot iSlot
TESObjectWEAP::InstanceData * ATUtilities::GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor) {
		// Invalid slot id
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		ActorEquipData * equipData = ownerActor->equipData;
		if (!equipData)
			return nullptr;

		// Make sure there is an item in this slot
		auto item = equipData->slots[iEquipSlot].item;
		if (!item)
			return nullptr;

		return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(equipData->slots[iEquipSlot].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	}
	return nullptr;
}

// returns the FormID of thisActor's weapon equipped at slot iSlot
UInt32 ATUtilities::GetEquippedItemFormID(Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor) {
		// Invalid slot id
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return 0x0;

		ActorEquipData * equipData = ownerActor->equipData;
		if (!equipData)
			return 0x0;

		// Make sure there is an item in this slot
		TESForm *item = equipData->slots[iEquipSlot].item;
		if (!item)
			return 0x0;

		return item->formID;
	}
	return 0x0;
}



// --------- data cache interactions:

// ---- ATWeapon:

int ATGameData::GetWeaponIndex(UInt32 formID)
{
	for (UInt32 i = 0; i < ATWeapons.count; i++) {
		if (ATWeapons[i].objectID == formID) {
			return i;
		}
	}
	return -1;
}

bool ATGameData::GetWeaponByID(UInt32 formID, ATWeapon & weaponData)
{
	for (UInt32 i = 0; i < ATWeapons.count; i++) {
		if (ATWeapons[i].objectID == formID) {
			weaponData = ATWeapons[i];
			return true;
		}
	}
	return false;
}

// returns the weapon's equipped caliber keyword
bool ATWeapon::GetInstanceCaliber(TESObjectWEAP::InstanceData * instanceData, ATCaliber & thisCaliber)
{
	if (instanceData) {
		if (instanceData->keywords) {
			BGSKeyword *caliberKW = nullptr;
			//_MESSAGE("ID: 0x%08x  Caliber count: - %i", objectID, calibers.count);
			for (UInt32 j = 0; j < instanceData->keywords->numKeywords; j++) {
				caliberKW = instanceData->keywords->keywords[j];
				if (caliberKW) {
					if (GetCaliberDataByID(caliberKW->formID, thisCaliber)) {
						return true;
					}
				}
			}
		}
	}
	return false;
}

int ATWeapon::GetCaliberDataIndex(UInt32 formID)
{
	for (UInt32 i = 0; i < calibers.count; i++) {
		if (calibers[i].objectID == formID) {
			return i;
		}
	}
	return -1;
}

bool ATWeapon::GetCaliberDataByID(UInt32 formID, ATCaliber & caliberData)
{
	for (UInt32 i = 0; i < calibers.count; i++) {
		if (calibers[i].objectID == formID) {
			caliberData = calibers[i];
			return true;
		}
	}
	return false;
}

BGSMod::Attachment::Mod * ATWeapon::GetDamagedMod(Actor * ownerActor, UInt32 iEquipSlot, int iModSlot)
{
	if (damagedModSlots.count > 0) {
		TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(ownerActor, iEquipSlot);
		if (instanceData) {
			ActorEquipData * equipData = ownerActor->equipData;
			if (equipData) {
				BGSObjectInstanceExtra::Data * data = equipData->slots[iEquipSlot].extraData->data;
				if (data) {
					if (iModSlot < 0) {
						iModSlot = ATUtilities::ATrng.RandomInt(0, damagedModSlots.count - 1);
					}
					_MESSAGE("Damaged Mod Slot: %i", iModSlot);

					if (damagedModSlots.count > iModSlot) {
						ATDamagedModSlot tempSlot = damagedModSlots[iModSlot];
						if (tempSlot.damagedMods.count > 0) {
							UInt32 dataSize = data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);

							bool bAllow = false;
							BGSMod::Attachment::Mod * objectMod = nullptr;

							for (UInt32 i = 0; i < tempSlot.damagedMods.count; i++) {
								if (bAllow) {
									_MESSAGE("picked damaged mod: %s", tempSlot.damagedMods[i]->fullName.name.c_str());
									return tempSlot.damagedMods[i];
								}
								else {
									for (UInt32 j = 0; j < dataSize; j++) {
										objectMod = (BGSMod::Attachment::Mod*)Runtime_DynamicCast(LookupFormByID(data->forms[j].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
										if (tempSlot.damagedMods[i] == objectMod) {
											bAllow = true;
											break;
										}
									}
								}
							}

							if (bAllow) {
								_MESSAGE("worst damaged mod already equipped for slot %i", iModSlot);
							}
							else {
								
								if (tempSlot.damagedMods.count > 0) {
									_MESSAGE("no damaged mod found at slot %i, attaching default mod...", iModSlot);
									return tempSlot.damagedMods[0];
								}
								else {
									_MESSAGE("no damaged mods found at slot %i", iModSlot);
								}
							}
						}
					}
					else {
						_MESSAGE("Not enough damaged mod slots... count: %i, slot: %i", damagedModSlots.count, iModSlot);
					}
				}
			}
		}
	}
	return nullptr;
}


// ---- ATCritEffectTables:

int ATGameData::GetCritTableIndex(UInt32 formID)
{
	for (UInt32 n = 0; n < ATCritEffectTables.count; n++) {
		ATCritEffectTable tempTable = ATCritEffectTables[n];
		if (tempTable.objectID == formID) {
			return (int)n;
		}
	}
	return -1;
}

bool ATGameData::GetCritTableByID(UInt32 formID, ATCritEffectTable & critTable)
{
	for (UInt32 i = 0; i < ATCritEffectTables.count; i++) {
		if (ATCritEffectTables[i].objectID == formID) {
			critTable = ATCritEffectTables[i];
			return true;
		}
	}
	return false;
}

SpellItem * ATCritEffectTable::GetCritSpell(UInt32 iRollMod, UInt32 iRaceID)
{
	int iRoll = ATUtilities::ATrng.RandomInt(0, 85 + iRollMod);

	if ((iRaceID > 0) && (critVariations.count > 0)) {
		for (UInt32 i = 0; i < critVariations.count; i++) {
			ATCritEffectTable::AltCritTable tempCritVar = critVariations[i];
			if (tempCritVar.critRaces.GetItemIndex(iRaceID) > -1) {
				for (UInt32 j = 0; j < tempCritVar.critEffects.count; j++) {
					if (iRoll < tempCritVar.critEffects[j].rollMax) {
						if (tempCritVar.critEffects[j].critSpell) {
							_MESSAGE("Critical Effect: %s, Race: 0x%08X", tempCritVar.critEffects[j].critSpell->name.name.c_str(), iRaceID);
						}
						return tempCritVar.critEffects[j].critSpell;
					}
				}
			}
		}
	}

	for (UInt32 i = 0; i < defaultEffects.count; i++) {
		if (iRoll < defaultEffects[i].rollMax) {
			if (defaultEffects[i].critSpell) {
				_MESSAGE("Critical Effect: %s", defaultEffects[i].critSpell->name.name.c_str());
			}
			return defaultEffects[i].critSpell;
		}
	}

	return nullptr;
}


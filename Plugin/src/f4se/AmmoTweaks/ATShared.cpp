#include "ATShared.h"
//#include <time.h>



//********************* weapon/caliber/crit tables cache

namespace ATShared
{
	// global variables + weapon/caliber data
	ATDataStore ATData;
}


//*********************************************** Shared Functions:

//********************* FormID/Identifier:

// returns a form's plugin's name
const char* ATShared::GetPluginNameFromFormID(UInt32 formID)
{
	char *modName = "none";
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

// returns a formatted string containing (string=pluginName|UInt=formID without loadorder) 
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

// returns a formID from a formatted string
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
	return formId;

}

// returns a form from a formatted string
TESForm * ATShared::GetFormFromIdentifier(const std::string & formIdentifier, const char* strSplit)
{
	UInt32 formId = GetFormIDFromIdentifier(formIdentifier, strSplit);
	if (formId > 0x0)
		return LookupFormByID(formId);
	else
		return nullptr;
}

// reads a ValueModifier (ActorValue, amount) from the passed identifier string
TBO_InstanceData::ValueModifier ATShared::GetAVModiferFromIdentifer(const std::string & formIdentifier)
{
	TBO_InstanceData::ValueModifier avMod;
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of(", ");
		std::string *avIdentifier = new std::string();
		std::string *avValueStr = new std::string();
		avIdentifier->append(formIdentifier.substr(0, pos));
		avValueStr->append(formIdentifier.substr(pos + 2));

		ActorValueInfo *newAV = (ActorValueInfo*)GetFormFromIdentifier(avIdentifier->c_str());
		if (newAV) {
			avMod.avInfo = newAV;
			UInt32 newVal = std::stoi(*avValueStr);
			avMod.unk08 = newVal;
		}
	}
	return avMod;
}

// reads a TBO_InstanceData::DamageTypes (DamageType, amount) from the passed identifier string
TBO_InstanceData::DamageTypes ATShared::GetDamageTypeFromIdentifier(const std::string & formIdentifier)
{
	TBO_InstanceData::DamageTypes tempDT;
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of(", ");
		std::string *avIdentifier = new std::string();
		std::string *avValueStr = new std::string();
		avIdentifier->append(formIdentifier.substr(0, pos));
		avValueStr->append(formIdentifier.substr(pos + 2));

		BGSDamageType *newDT = (BGSDamageType*)GetFormFromIdentifier(avIdentifier->c_str());
		if (newDT) {
			tempDT.damageType = newDT;
			UInt32 newVal = std::stoi(*avValueStr);
			tempDT.value = newVal;
		}
	}
	return tempDT;
}

// reads an ATCritTableBase::CritEffect (Spell, max. roll value) from the passed identifier string
ATCritEffect ATShared::GetCritEffectFromIdentifier(const std::string & formIdentifier)
{
	ATCritEffect critEffect;
	if (formIdentifier.c_str() != "none") {
		std::size_t pos = formIdentifier.find_first_of(", ");
		std::string *spellIdentifier = new std::string();
		std::string *spellValueStr = new std::string();
		spellIdentifier->append(formIdentifier.substr(0, pos));
		spellValueStr->append(formIdentifier.substr(pos + 2));

		SpellItem *newSpell = (SpellItem*)GetFormFromIdentifier(spellIdentifier->c_str());
		if (newSpell) {
			critEffect.critSpell = newSpell;
		}
		critEffect.rollMax = std::stoi(*spellValueStr);
	}
	return critEffect;
}


//********************* Weapon instanceData:

// returns the instanceData of a passed weapon reference
TESObjectWEAP::InstanceData *ATShared::GetWeapRefInstanceData(VMRefOrInventoryObj *curObj)
{
	if (curObj) {
		TESObjectREFR *tempRef = curObj->GetObjectReference();
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
TESObjectWEAP *ATShared::GetEquippedWeapon(Actor *thisActor, UInt32 iSlot)
{
	if (thisActor) {
		// Invalid slot id
		if (iSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		ActorEquipData * equipData = thisActor->equipData;
		if (!equipData)
			return nullptr;

		// Make sure there is an item in this slot
		TESForm *item = equipData->slots[iSlot].item;
		if (!item)
			return nullptr;

		return (TESObjectWEAP*)item;
	}
	return nullptr;
}

// returns the instanceData of thisActor's weapon equipped at slot iSlot
TESObjectWEAP::InstanceData *ATShared::GetEquippedInstanceData(Actor *thisActor, UInt32 iSlot)
{
	if (thisActor) {
		// Invalid slot id
		if (iSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		ActorEquipData * equipData = thisActor->equipData;
		if (!equipData)
			return nullptr;

		// Make sure there is an item in this slot
		auto item = equipData->slots[iSlot].item;
		if (!item)
			return nullptr;

		return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(equipData->slots[iSlot].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	}
	return nullptr;
}

// returns the FormID of thisActor's weapon equipped at slot iSlot
UInt32 ATShared::GetEquippedItemFormID(Actor *thisActor, UInt32 iSlot)
{
	if (thisActor) {
		// Invalid slot id
		if (iSlot >= ActorEquipData::kMaxSlots)
			return 0x0;

		ActorEquipData * equipData = thisActor->equipData;
		if (!equipData)
			return 0x0;

		// Make sure there is an item in this slot
		TESForm *item = equipData->slots[iSlot].item;
		if (!item)
			return 0x0;

		return item->formID;
	}
	return 0x0;
}

// returns the weapon's equipped caliber keyword
BGSKeyword* ATShared::GetInstanceCaliber(TESObjectWEAP::InstanceData *instanceData)
{
	if (instanceData) {
		if (instanceData->keywords) {
			for (UInt8 j = 0; j < instanceData->keywords->numKeywords; j++) {
				if (instanceData->keywords->keywords[j]) {
					UInt32 tempCaliberKW = instanceData->keywords->keywords[j]->formID;
					if (ATShared::ATData.GetCaliberIndex(tempCaliberKW) > -1) {
						return (BGSKeyword*)LookupFormByID(tempCaliberKW);
					}
				}
			}
		}
	}
	return nullptr;
}


//********************* Custom Classes:

// --------- ATCritEffectTable:

SpellItem *ATCritEffectTable::GetCritSpell(UInt32 iRollMod, UInt32 iTargetType)
{
	int iRoll = (rand() % 86) + iRollMod;

	if ((iTargetType == 1) && (critEffects_Human.count > 0)) {
		for (UInt32 i = 0; i < critEffects_Human.count; i++) {
			ATCritEffect newEffect = critEffects_Human[i];
			if (iRoll < newEffect.rollMax) {
				return newEffect.critSpell;
			}
		}
	}
	else if ((iTargetType == 2) && (critEffects_Robot.count > 0)) {
		for (UInt32 i = 0; i < critEffects_Robot.count; i++) {
			ATCritEffect newEffect = critEffects_Robot[i];
			if (iRoll < newEffect.rollMax) {
				return newEffect.critSpell;
			}
		}
	}
	else {
		
		for (UInt32 i = 0; i < critEffects.count; i++) {
			ATCritEffect newEffect = critEffects[i];
			if (iRoll < newEffect.rollMax) {
				return newEffect.critSpell;
			}
		}
	}
	return nullptr;
}


// --------- ATCritFailureTable:

SpellItem *ATCritEffectTable::GetCritFailureSpell(UInt32 iRollMod, UInt32 iLuck)
{
	int iRoll = ((rand() % 101) + iRollMod) - (7 * (iLuck - 7));
	
	for (UInt32 i = 0; i < critEffects.count; i++) {
		ATCritEffect newEffect = critEffects[i];
		if (iRoll < newEffect.rollMax) {
			return newEffect.critSpell;
		}
	}
	
	return nullptr;
}


// --------- ATWeapon:

BGSMod::Attachment::Mod *ATWeapon::GetDamagedMod(Actor *thisActor, UInt32 iEquipSlot, int iSlot)
{
	TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(thisActor, iEquipSlot);
	ActorEquipData * equipData = thisActor->equipData;
	if (equipData) {
		auto data = equipData->slots[iEquipSlot].extraData->data;
		
		if (instanceData) {
			if (damagedMods.count > iSlot) {
				if (iSlot < 0) {
					iSlot = rand() % damagedMods.count;
				}

				ModSlot tempSlot;
				if (damagedMods.GetNthItem(iSlot, tempSlot)) {
					if (tempSlot.swappableMods.count > 0) {
						int iIndex = 0;
						if (tempSlot.swappableMods.count > 1) {
							iIndex = rand() % tempSlot.swappableMods.count;
						}

						ATSwappableMod tempMod;
						if (tempSlot.swappableMods.GetNthItem(iIndex, tempMod)) {
							if (instanceData->keywords) {
								bool bAllow = false;

								if (tempMod.requiredKW) {
									for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
										if (instanceData->keywords->keywords[i] == tempMod.requiredKW) {
											bAllow = true;
											break;
										}
									}
								}
								else {
									bAllow = true;
								}

								// equipped mod check
								if (bAllow) {
									if (data) {
										for (UInt32 j = 0; j < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); j++) {
											BGSMod::Attachment::Mod * objectMod = (BGSMod::Attachment::Mod *)Runtime_DynamicCast(LookupFormByID(data->forms[j].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (tempMod.swapMod == objectMod) {
												bAllow = false;
												break;
											}
										}
									}

									if (bAllow) {
										return tempMod.swapMod;
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return nullptr;
}


// --------- ATDataStore:

int ATDataStore::GetWeaponIndex(UInt32 formID)
{
	for (UInt32 i = 0; i < Weapons.count; i++) {
		if (Weapons[i].objectID == formID) {
			return (int)i;
		}
	}
	return -1;
}
bool ATDataStore::GetWeaponByID(UInt32 formID, ATWeapon weaponData)
{
	for (UInt32 i = 0; i < Weapons.count; i++) {
		if (Weapons[i].objectID == formID) {
			weaponData = Weapons[i];
			return true;
		}
	}
	return false;
}


int ATDataStore::GetCaliberIndex(UInt32 formID)
{
	for (UInt32 i = 0; i < Calibers.count; i++) {
		if (Calibers[i].objectID == formID) {
			return (int)i;
		}
	}
	return -1;
}
bool ATDataStore::GetCaliberByID(UInt32 formID, ATCaliber caliberData)
{
	for (UInt32 i = 0; i < Calibers.count; i++) {
		if (Calibers[i].objectID == formID) {
			caliberData = Calibers[i];
			return true;
		}
	}
	return false;
}


int ATDataStore::GetCritTableIndex(UInt32 formID)
{
	for (UInt32 n = 0; n < CritEffectTables.count; n++) {
		ATCritEffectTable tempTable = CritEffectTables[n];
		if (tempTable.objectID == formID) {
			return (int)n;
		}
	}
	return -1;
}

int ATDataStore::GetCritFailTableIndex(UInt32 formID)
{
	for (UInt32 n = 0; n < CritFailureTables.count; n++) {
		ATCritEffectTable tempTable = CritFailureTables[n];
		if (tempTable.objectID == formID) {
			return (int)n;
		}
	}
	return -1;
}


//********************* Papyrus:

// registers native functions
void ATShared::RegisterPapyrus(VirtualMachine * vm)
{
	_MESSAGE("\nRegistering Papyrus functions....");
	ATGlobals::RegisterPapyrus(vm);
	ATWeaponRef::RegisterPapyrus(vm);
}

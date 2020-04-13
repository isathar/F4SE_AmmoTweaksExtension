#include "ATShared.h"



// ---- search predicates:

// weaponData = std::find_if(ATWeapons.begin(), ATWeapons.end(), FindPred_ATWeapon_ByID(weapID));
struct FindPred_ATWeapon_ByID
{
	UInt32 formID;
	FindPred_ATWeapon_ByID(UInt32 formID) : formID(formID) {}
	bool operator () (const ATWeapon& weap) const { return (formID == weap.iWeaponID); }
};

// caliberData = std::find_if(calibers.begin(), calibers.end(), FindPred_ATCaliberData_ByID(caliberID));
struct FindPred_ATCaliberData_ByID
{
	UInt32 formID;
	FindPred_ATCaliberData_ByID(UInt32 formID) : formID(formID) {}
	bool operator () (const ATCaliberData& caliber) const { return (formID == caliber.iCaliberKWID); }
};

// avNameData = std::find_if(AVNames.begin(), AVNames.end(), FindPred_ATAVName_ByID(avFormID));
struct FindPred_ATAVName_ByID
{
	UInt32 formID;
	FindPred_ATAVName_ByID(UInt32 formID) : formID(formID) {}
	bool operator () (const ActorValueShortName& avNameData) const { return (avNameData.avID == formID); }
};


// CritFailureEffect = std::find_if(critFailures.begin(), critFailures.end(), FindPred_CritFailureEffect_ByRoll(iCritFailRoll));
struct FindPred_CritFailureEffect_ByRoll
{
	int iRoll;
	FindPred_CritFailureEffect_ByRoll(UInt32 iRoll) : iRoll(iRoll) {}
	bool operator () (const ATWeapon::CritFailure & critFailure) const { return (iRoll <= critFailure.iRollMax); }
};


// -------------------------------- ATGameData:

// -------- Equipped items by EquipSlot:

// returns the Form of the item equipped at an actor's iEquipSlot
TESForm * ATGameData::GetEquippedFormBySlot(Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor && ownerActor->equipData) {
		// Invalid slot id
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		return ownerActor->equipData->slots[iEquipSlot].item;
	}
	return nullptr;
}

// gets the instanceData at the specified EquipSlot, if any
TBO_InstanceData * ATGameData::GetEquippedInstanceBySlot(Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor && ownerActor->equipData) {
		// Invalid slot id
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return nullptr;

		// Make sure there is an item in this slot
		auto item = ownerActor->equipData->slots[iEquipSlot].item;
		if (!item)
			return nullptr;

		return ownerActor->equipData->slots[iEquipSlot].instanceData;
	}
	return nullptr;
}



// returns true if the item equipped at the actor's iEquipSlot has the ObjectMod checkMod attached
bool ATGameData::EquippedItemAtSlotHasMod(BGSMod::Attachment::Mod * checkMod, Actor * ownerActor, UInt32 iEquipSlot)
{
	if (ownerActor && ownerActor->equipData && checkMod) {
		if (iEquipSlot >= ActorEquipData::kMaxSlots)
			return false;
		
		BGSObjectInstanceExtra * data = ownerActor->equipData->slots[iEquipSlot].extraData;
		if (data && data->data) {
			UInt32 dataSize = data->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
			if (dataSize > 0) {
				UInt32 checkModID = checkMod->formID;
				for (UInt32 j = 0; j < dataSize; j++) {
					if (data->data->forms[j].formId == checkModID) {
						return true;
					}
				}
			}
		}
	}
	return false;
}


// -------- Equipped Weapons by inventory/item type: iType=(0 - main weapon, TBD: 1 - left weapon, 2 - grenade/mine)

// returns an equipped weapon's Form
TESObjectWEAP * ATGameData::GetEquippedWeaponForm(Actor * ownerActor, UInt32 iItemType)
{
	if (!ownerActor || !ownerActor->inventoryList || (ownerActor->inventoryList->items.count < 1)) {
		return nullptr;
	}

	bool bFound = false;
	UInt16 iEquippedFlag = 0x0001;
	for (UInt32 i = 0; i < ownerActor->inventoryList->items.count; i++) {
		bFound = false;
		BGSInventoryItem inventoryItem;
		if (ownerActor->inventoryList->items.GetNthItem(i, inventoryItem)) {
			if (inventoryItem.form->formType == kFormType_WEAP) {
				if (ATGameData::GetWeaponIndex(inventoryItem.form->formID) != -1) {
					inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
					{
						if ((stack->flags & iEquippedFlag) == iEquippedFlag) {
							bFound = true;
							return false;
						}
						return true;
					});
					if (bFound) {
						return (TESObjectWEAP*)inventoryItem.form;
					}
				}
			}
		}
	}
	return nullptr;
}

// returns an equipped weapon's ExtraDataList
ExtraDataList * ATGameData::GetEquippedWeaponExtraData(Actor * ownerActor, UInt32 iItemType)
{
	if (!ownerActor || !ownerActor->inventoryList || (ownerActor->inventoryList->items.count < 1)) {
		return nullptr;
	}
	TESObjectWEAP::InstanceData* weapInstanceData = nullptr;
	ExtraDataList * extraData = nullptr;
	ExtraInstanceData * objectModData = nullptr;
	UInt16 iEquippedFlag = 0x0001;
	for (UInt32 i = 0; i < ownerActor->inventoryList->items.count; i++) {
		BGSInventoryItem inventoryItem;
		if (ownerActor->inventoryList->items.GetNthItem(i, inventoryItem)) {
			// only allow the full check on weapons managed by AT
			if (inventoryItem.form->formType == kFormType_WEAP) {
				if (GetWeaponIndex(inventoryItem.form->formID) != -1) {
					// search the item type's stack
					inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
					{
						if ((stack->flags & iEquippedFlag) == iEquippedFlag) {
							extraData = stack->extraData;
							return false;
						}
						return true;
					});
					// return if anything was found for this item type
					if (extraData) {
						return extraData;
					}
				}
			}
		}
	}
	return nullptr;
}


// returns an equipped weapon's InstanceData
TESObjectWEAP::InstanceData * ATGameData::GetEquippedWeaponInstance(Actor * ownerActor, UInt32 iItemType)
{
	if (!ownerActor) {
		return nullptr;
	}
	ExtraDataList * extraData = GetEquippedWeaponExtraData(ownerActor, iItemType);
	if (!extraData) {
		return nullptr;
	}
	BSExtraData * extraDataInst = extraData->GetByType(ExtraDataType::kExtraData_InstanceData);
	if (!extraDataInst) {
		return nullptr;
	}
	ExtraInstanceData * objectModData = DYNAMIC_CAST(extraDataInst, BSExtraData, ExtraInstanceData);
	if (objectModData) {
		TESObjectWEAP::InstanceData * weapInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(objectModData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		return weapInstanceData;
	}
	return nullptr;
}

// returns an equipped weapon's InstanceData from extraData
TESObjectWEAP::InstanceData * ATGameData::GetWeaponInstanceData(ExtraDataList * extraData)
{
	if (!extraData) {
		return nullptr;
	}
	BSExtraData * extraDataInst = extraData->GetByType(ExtraDataType::kExtraData_InstanceData);
	if (!extraDataInst) {
		return nullptr;
	}
	ExtraInstanceData * objectModData = DYNAMIC_CAST(extraDataInst, BSExtraData, ExtraInstanceData);
	if (objectModData) {
		TESObjectWEAP::InstanceData * weapInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(objectModData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
		return weapInstanceData;
	}
	return nullptr;
}

// returns an equipped weapon's InstanceData from extraData
TESObjectARMO::InstanceData * ATGameData::GetArmorInstanceData(ExtraDataList * extraData)
{
	if (!extraData) {
		return nullptr;
	}
	BSExtraData * extraDataInst = extraData->GetByType(ExtraDataType::kExtraData_InstanceData);
	if (!extraDataInst) {
		return nullptr;
	}
	ExtraInstanceData * objectModData = DYNAMIC_CAST(extraDataInst, BSExtraData, ExtraInstanceData);
	if (objectModData) {
		TESObjectARMO::InstanceData * weapInstanceData = (TESObjectARMO::InstanceData*)Runtime_DynamicCast(objectModData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectARMO__InstanceData);
		return weapInstanceData;
	}
	return nullptr;
}


// returns true if the item equipped at the actor's iEquipSlot has the ObjectMod checkMod attached
bool ATGameData::InstanceHasMod(ExtraDataList * extraData, BGSMod::Attachment::Mod * checkMod)
{
	if (!checkMod || !extraData) {
		return false;
	}
	BSExtraData * extraDataInst = extraData->GetByType(ExtraDataType::kExtraData_ObjectInstance);
	if (!extraDataInst) {
		return false;
	}
	BGSObjectInstanceExtra * objectModData = DYNAMIC_CAST(extraDataInst, BSExtraData, BGSObjectInstanceExtra);
	if (!objectModData || !objectModData->data) {
		return false;
	}
	UInt32 dataSize = objectModData->data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
	if (dataSize > 0) {
		for (UInt32 j = 0; j < dataSize; j++) {
			if (checkMod->formID == objectModData->data->forms[j].formId) {
				return true;
			}
		}
	}
	return false;
}


// gets the health of a reference's instance, optionally generating a random value if none is found
float ATGameData::GetInstanceHealth(ExtraDataList * extraData, bool bRandomIfNone)
{
	if (!extraData) {
		return -1.0;
	}
	bool bDoRandom = false;
	BSExtraData * extraDataHealth = extraData->GetByType(ExtraDataType::kExtraData_Health);
	if (!extraDataHealth) {
		bDoRandom = true;
	} else {
		ExtraHealth * healthData = DYNAMIC_CAST(extraDataHealth, BSExtraData, ExtraHealth);
		if (healthData) {
			return healthData->health;
		}
	}

	if (bRandomIfNone) {
		if (bDoRandom) {
			int iMinCND = (int)floor(ATGlobalVars::fMinStartingCND * 100.0);
			float fNewHealth = ((float)ATConfigData::rng.RandomInt(iMinCND, 100)) * 0.01;
			ExtraHealth * newHealthData = ExtraHealth::Create(fNewHealth);
			extraData->Add(newHealthData->type, (BSExtraData*)newHealthData);
			_MESSAGE("GetInstanceHealth:  none found, created new health: %.02f", newHealthData->health);
			return fNewHealth;
		}
	}
	return -1.0;
}

// sets the health of a reference's instance
bool ATGameData::SetInstanceHealth(ExtraDataList * extraData, float fNewCND)
{
	if (!extraData) {
		return false;
	}
	BSExtraData * extraDataHealth = extraData->GetByType(ExtraDataType::kExtraData_Health);
	ExtraHealth * healthData = nullptr;
	if (extraDataHealth) {
		healthData = DYNAMIC_CAST(extraDataHealth, BSExtraData, ExtraHealth);
	}
	if (healthData) {
		healthData->health = fNewCND;
	}
	else {
		healthData = ExtraHealth::Create(fNewCND);
		extraData->Add(healthData->type, (BSExtraData*)healthData);
		_MESSAGE("SetWeaponConditionRef:  created new health: %.02f", healthData->health);
	}
	return true;
}


// adds a new AVModd to a weapon or replaces an existing one
bool ATGameData::SetWeaponInstanceAVModifier(TESObjectWEAP::InstanceData * instanceData, ActorValueInfo * avToAdd, UInt32 iAmount)
{
	if (!instanceData || !avToAdd) {
		return false;
	}
	if (!instanceData->modifiers) {
		instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>;
	}
	// remove the existing AVMod
	if (instanceData->modifiers->count > 0) {
		TBO_InstanceData::ValueModifier tempavMod;
		for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
			if (instanceData->modifiers->GetNthItem(i, tempavMod)) {
				if (tempavMod.avInfo) {
					if (tempavMod.avInfo->formID == avToAdd->formID) {
						if (tempavMod.unk08 != iAmount) {
							instanceData->modifiers->Remove(i);
							break;
						}
						else {
							// no need to edit
							return true;
						}
					}
				}
			}
		}
	}
	// add a new AVMod if needed
	if (iAmount != 0) {
		TBO_InstanceData::ValueModifier newAVMod;
		newAVMod.avInfo = avToAdd;
		newAVMod.unk08 = iAmount;
		instanceData->modifiers->Push(newAVMod);
	}
	return true;
}


UInt32 ATGameData::GetWeaponInstanceAVModifier(TESObjectWEAP::InstanceData * instanceData, ActorValueInfo * avToGet)
{
	if (!instanceData || !avToGet) {
		return 0;
	}
	if (!instanceData->modifiers || (instanceData->modifiers->count < 1)) {
		return 0;
	}
	TBO_InstanceData::ValueModifier tempavMod;
	for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
		if (instanceData->modifiers->GetNthItem(i, tempavMod)) {
			if (tempavMod.avInfo) {
				if (tempavMod.avInfo->formID == avToGet->formID) {
					return tempavMod.unk08;
				}
			}
		}
	}
	return 0;
}



// -------- Equipped Armor:

// returns an actor's armor equipped at iEquipSlot
TESObjectARMO * ATGameData::GetEquippedArmorForm(Actor * ownerActor, UInt32 iEquipSlot)
{
	return (TESObjectARMO*)GetEquippedFormBySlot(ownerActor, iEquipSlot);;
}


// -------- Misc

// ---- Keyword Checks:

bool ATGameData::WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData * instanceData, BGSKeyword * checkKW)
{
	if (!checkKW || !instanceData || !instanceData->keywords) {
		return false;
	}
	auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&instanceData->keywords->keywordBase, 1);
	if (HasKeyword_Internal(&instanceData->keywords->keywordBase, checkKW, 0)) {
		return true;
	}
	return false;
}

bool ATGameData::ArmorInstanceHasKeyword(TESObjectARMO::InstanceData * instanceData, BGSKeyword * checkKW)
{
	if (!checkKW || !instanceData || !instanceData->keywords) {
		return false;
	}
	auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&instanceData->keywords->keywordBase, 1);
	if (HasKeyword_Internal(&instanceData->keywords->keywordBase, checkKW, 0)) {
		return true;
	}
	return false;
}

bool ATGameData::TargetHasKeyword(Actor * targetActor, BGSKeyword * checkKW)
{
	if (!targetActor || !checkKW) {
		return false;
	}
	auto HasKeyword_Internal = GetVirtualFunction<_IKeywordFormBase_HasKeyword>(&targetActor->keywordFormBase, 1);
	if (HasKeyword_Internal(&targetActor->keywordFormBase, checkKW, 0)) {
		return true;
	}
	return false;
}


// gets the number of items in ownerActor's inventory (by the item's formID)
int ATGameData::GetItemCount(Actor * ownerActor, UInt32 itemID)
{
	if (!ownerActor || (itemID == 0)) {
		return 0;
	}
	if (!ownerActor->inventoryList || (ownerActor->inventoryList->items.count < 1)) {
		return 0;
	}
	BGSInventoryItem inventoryItem;
	for (UInt32 i = 0; i < ownerActor->inventoryList->items.count; i++) {
		if (ownerActor->inventoryList->items.GetNthItem(i, inventoryItem)) {
			if (inventoryItem.form->formID == itemID) {
				return inventoryItem.stack->count;
			}
		}
	}
	return 0;
}

float ATGameData::CalcSkillRequirement(float fWeight)
{
	return min(max(2.0, (fWeight / ATGlobalVars::fSkillReq_MaxWeight) * ATGlobalVars::fSkillReq_MaxAmount),	ATGlobalVars::fSkillReq_MaxAmount);
}




// -------------------------------- ATWeapon:

bool ATWeapon::GetWeaponAnim(UInt32 iAnimID, WeaponAnim & newAnim)
{
	newAnim = WeaponAnim();
	if (!weaponAnims.empty()) {
		for (std::vector<WeaponAnim>::iterator animsIt = weaponAnims.begin(); animsIt != weaponAnims.end(); ++animsIt) {
			WeaponAnim curAnim = *animsIt;
			if (curAnim.iAnimID == iAnimID) {
				newAnim = curAnim;
				return true;
			}
		}
	}
	return false;
}

bool ATWeapon::GetCurMagItem(ExtraDataList * extraData, MagazineItem & magItem)
{
	if (!extraData || (magazines.size() < 1)) {
		return false;
	}
	for (UInt32 i = 0; i < magazines.size(); i++) {
		if (magazines[i].magMod) {
			if (ATGameData::InstanceHasMod(extraData, magazines[i].magMod)) {
				magItem = magazines[i];
				return true;
			}
		}
		else {
			magItem = magazines[i];
			return true;
		}
	}
	return false;
}

bool ATWeapon::GetCurWeapArmor(TESObjectWEAP::InstanceData * instanceData, ATWeapon::HolsteredArmor & newHolster)
{
	if (!instanceData || (holsters.size() < 1)) {
		return nullptr;
	}
	HolsteredArmor tempHolster;
	for (UInt8 j = 0; j < holsters.size(); j++) {
		tempHolster = holsters[j];
		int iNumReqKws = tempHolster.requiredKWs.size();
		if (iNumReqKws > 0) {
			for (UInt32 i = 0; i < tempHolster.requiredKWs.size(); i++) {
				if (ATGameData::WeaponInstanceHasKeyword(instanceData, tempHolster.requiredKWs[i])) {
					iNumReqKws -= 1;
				}
			}
		}
		if (iNumReqKws <= 0) {
			newHolster = tempHolster;
			return true;
		}
	}
	return nullptr;
}


// gets the current hud weapon icon based on the weapon's keywords
UInt32 ATWeapon::GetHUDIcon(TESObjectWEAP::InstanceData * instanceData)
{
	if (!instanceData || hudIcons.empty()) {
		return -1;
	}
	for (std::vector<HUDIconMod>::iterator iconIt = hudIcons.begin(); iconIt != hudIcons.end(); ++iconIt) {
		HUDIconMod curIconMod = *iconIt;
		if (!curIconMod.reqKW || ATGameData::WeaponInstanceHasKeyword(instanceData, curIconMod.reqKW)) {
			return curIconMod.iIconIndex;
		}
	}
	return -1;
}

// gets the current hud firemode icon based on the weapon's firemode mod
UInt32 ATWeapon::GetFiremodeIcon(ExtraDataList * weapExtraData)
{
	if (!weapExtraData || firemodes.empty()) {
		return -1;
	}
	for (std::vector<ATFiremode>::iterator fmIt = firemodes.begin(); fmIt != firemodes.end(); ++fmIt) {
		ATFiremode curFM = *fmIt;
		if (!curFM.firemodeMod || ATGameData::InstanceHasMod(weapExtraData, curFM.firemodeMod)) {
			return curFM.iHudIcon;
		}
	}
	return -1;
}


ATWeaponModStats ATWeapon::GetModStats(ExtraDataList * extraData, TESObjectWEAP::InstanceData * instanceData)
{
	ATWeaponModStats newWeaponStats = ATWeaponModStats();
	// -- Weapon Stats Modifiers --
	newWeaponStats.fCritMult = 100.0;
	newWeaponStats.fCritFailMult = 100.0;
	newWeaponStats.fWearMult = 100.0;

	// Firemode effects
	ATFiremode curFiremode;
	if (GetCurrentFiremode(extraData, curFiremode)) {
		newWeaponStats.fCritMult *= curFiremode.modEffects.fCritMult;
		newWeaponStats.fCritFailMult *= curFiremode.modEffects.fCritFailMult;
		newWeaponStats.fArmorPenetrationAdd += curFiremode.modEffects.fArmorPenetrationAdd;
		newWeaponStats.fWearMult *= curFiremode.modEffects.fWearMult;
		newWeaponStats.fMaxConditionMultAdd += curFiremode.modEffects.fMaxConditionMultAdd;
	}

	// Caliber/Ammo effects
	ATCaliber::AmmoType curAmmoType;
	if (GetCurrentAmmoType(instanceData, extraData, curAmmoType)) {
		newWeaponStats.fCritMult *= curAmmoType.modEffects.fCritMult;
		newWeaponStats.fCritFailMult *= curAmmoType.modEffects.fCritFailMult;
		newWeaponStats.fArmorPenetrationAdd += curAmmoType.modEffects.fArmorPenetrationAdd;
		newWeaponStats.fWearMult *= curAmmoType.modEffects.fWearMult;
		newWeaponStats.fMaxConditionMultAdd += curAmmoType.modEffects.fMaxConditionMultAdd;
	}

	// misc mod effects
	if (!modEffects.empty()) {
		for (std::vector<ATWeaponModStats>::iterator modEffectsIt = modEffects.begin(); modEffectsIt != modEffects.end(); ++modEffectsIt) {
			ATWeaponModStats addWeaponStats = *modEffectsIt;
			if (addWeaponStats.checkKW) {
				if (!ATGameData::WeaponInstanceHasKeyword(instanceData, addWeaponStats.checkKW)) {
					continue;
				}
			}
			newWeaponStats.fCritMult *= addWeaponStats.fCritMult;
			newWeaponStats.fCritFailMult *= addWeaponStats.fCritFailMult;
			newWeaponStats.fArmorPenetrationAdd += addWeaponStats.fArmorPenetrationAdd;
			newWeaponStats.fWearMult *= addWeaponStats.fWearMult;
			newWeaponStats.fMaxConditionMultAdd += addWeaponStats.fMaxConditionMultAdd;
		}
	}

	// final stats calculations
	newWeaponStats.fWearMult = fBaseWearDivisor > 0.0 ? newWeaponStats.fWearMult / fBaseWearDivisor : newWeaponStats.fWearMult;
	newWeaponStats.fCritMult = max(0.0, newWeaponStats.fCritMult);
	newWeaponStats.fCritFailMult = max(0.0, newWeaponStats.fCritFailMult);
	newWeaponStats.fArmorPenetrationAdd = min(100.0, newWeaponStats.fArmorPenetrationAdd);
	newWeaponStats.fMaxConditionMultAdd = max(1.0, newWeaponStats.fMaxConditionMultAdd);

	return newWeaponStats;
}

// returns the weapon's equipped caliber
bool ATWeapon::GetCurrentCaliber(TESObjectWEAP::InstanceData * instanceData, ATCaliber & curCaliber)
{
	if (!instanceData || (calibers.size() < 1)) {
		return false;
	}
	for (std::vector<ATCaliber>::iterator itCaliber = calibers.begin(); itCaliber != calibers.end(); ++itCaliber) {
		if (ATGameData::WeaponInstanceHasKeyword(instanceData, itCaliber->kwCaliberID)) {
			curCaliber = *itCaliber;
			return true;
		}
	}
	return false;
}

// returns a random weapon mod from damaged mod slots
BGSMod::Attachment::Mod * ATWeapon::GetDamagedMod(ExtraDataList * extraData, int iModSlot)
{
	if (!extraData || (damagedModSlots.size() < 1)) {
		_MESSAGE("    Error: No damaged mod slots\n");
		return nullptr;
	}
	// select a random mod slot if needed
	if (iModSlot < 0) {
		if (damagedModSlots.size() > 1) {
			iModSlot = ATConfigData::rng.RandomInt(0, damagedModSlots.size() - 1);
		}
		else {
			iModSlot = 0;
		}
	}
	if (iModSlot >= damagedModSlots.size()) {
		_MESSAGE("    Error: damaged mod slot index out of range (%i/%i)\n", iModSlot, damagedModSlots.size());
		return nullptr;
	}

	ATDamagedModSlot tempSlot = damagedModSlots.at(iModSlot);
	if (tempSlot.damagedMods.size() < 1) {
		_MESSAGE("    Error: No damaged mods in slot %i - %s\n", iModSlot, tempSlot.sSlotName.c_str());
		return nullptr;
	}
	BGSMod::Attachment::Mod * curMod = nullptr;
	if (tempSlot.damagedMods.size() == 1) {
		if (!ATGameData::InstanceHasMod(extraData, tempSlot.damagedMods[0])) {
			curMod = tempSlot.damagedMods[0];
			_MESSAGE("    Damaged Mod:  %i - %s: 0 - 0x%08X - %s\n", iModSlot, tempSlot.sSlotName.c_str(), curMod->formID, curMod->fullName.name.c_str());
			return curMod;
		}
	}
	else {
		for (UInt32 i = 0; i < tempSlot.damagedMods.size(); i++) {
			curMod = tempSlot.damagedMods[i];
			if (ATGameData::InstanceHasMod(extraData, tempSlot.damagedMods[i])) {
				if (i < tempSlot.damagedMods.size() - 2) {
					curMod = tempSlot.damagedMods[i + 1];
					if (curMod) {
						_MESSAGE("    Damaged Mod:  %i - %s: %i - 0x%08X - %s\n", iModSlot, tempSlot.sSlotName.c_str(), i, curMod->formID, curMod->fullName.name.c_str());
					}
					else {
						_MESSAGE("    Error: No Damaged Mod:  %i - %s: %i\n", iModSlot, tempSlot.sSlotName.c_str(), i);
					}
					return curMod;
				}
				else {
					_MESSAGE("    Worst damaged mod already equipped for slot %i - %s\n", iModSlot, tempSlot.sSlotName.c_str());
					return nullptr;
				}
			}
		}
	}
	_MESSAGE("    Error: No equipped damaged mod for Slot: %i - %s\n", iModSlot, tempSlot.sSlotName.c_str());
	return tempSlot.damagedMods[0];
}



bool ATWeapon::GetCurrentAmmoType(TESObjectWEAP::InstanceData * instanceData, ExtraDataList * extraData, ATCaliber::AmmoType & ammoMod)
{
	if (!instanceData || !extraData) {
		return false;
	}
	ATCaliber curCaliber;
	if (!GetCurrentCaliber(instanceData, curCaliber)) {
		return false;
	}
	if (curCaliber.ammoTypes.empty()) {
		return false;
	}
	ATCaliber::AmmoType tempAT;
	for (std::vector<ATCaliber::AmmoType>::iterator atIt = curCaliber.ammoTypes.begin(); atIt != curCaliber.ammoTypes.end(); ++atIt) {
		tempAT = *atIt;
		if (tempAT.ammoMod) {
			if (ATGameData::InstanceHasMod(extraData, tempAT.ammoMod)) {
				ammoMod = *atIt;
				return true;
			}
		}
	}
	// no ammo mod attached, return default
	ammoMod = *curCaliber.ammoTypes.begin();
	return true;
}


bool ATWeapon::GetNextAmmoType(Actor * ownerActor, ATCaliber::AmmoType & nextAmmoType)
{
	if (!ownerActor) {
		return false;
	}
	ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
	if (!extraData) {
		return false;
	}
	TESObjectWEAP::InstanceData * instanceData = ATGameData::GetEquippedWeaponInstance(ownerActor, 0);
	if (!instanceData) {
		return false;
	}
	ATCaliber curCaliber;
	if (!GetCurrentCaliber(instanceData, curCaliber)) {
		return false;
	}
	int iAmmoTypesCount = curCaliber.ammoTypes.size();
	if (iAmmoTypesCount < 1) {
		return false;
	}

	int iFound = -1;
	for (std::vector<ATCaliber::AmmoType>::iterator itAmmoType = curCaliber.ammoTypes.begin(); itAmmoType != curCaliber.ammoTypes.end(); ++itAmmoType) {
		if (ATGameData::InstanceHasMod(extraData, itAmmoType->ammoMod)) {
			iFound = (size_t)(itAmmoType - curCaliber.ammoTypes.begin());
			break;
		}
	}

	// -- check for ammo items
	if (iFound < 0) {
		return false;
	}

	iFound = iFound + 1;
	if (iFound >= iAmmoTypesCount) {
		iFound = 0;
	}
	if (iFound > 0) {
		for (std::vector<ATCaliber::AmmoType>::iterator itAmmoType = curCaliber.ammoTypes.begin() + iFound; itAmmoType != curCaliber.ammoTypes.end(); ++itAmmoType) {
			if (itAmmoType->ammoItem) {
				if (ATGameData::GetItemCount(ownerActor, itAmmoType->ammoItem->formID) != 0) {
					nextAmmoType = *itAmmoType;
					return true;
				}
			}
		}
	}
	else {
		iFound = iAmmoTypesCount;
	}
	// round 2
	for (std::vector<ATCaliber::AmmoType>::iterator itAmmoType = curCaliber.ammoTypes.begin(); itAmmoType != (curCaliber.ammoTypes.begin() + iFound); ++itAmmoType) {
		if (itAmmoType->ammoItem) {
			if (ATGameData::GetItemCount(ownerActor, itAmmoType->ammoItem->formID) != 0) {
				nextAmmoType = *itAmmoType;
				return true;
			}
		}
	}
	return false;
}


bool ATWeapon::UpdateProjectile(TESObjectWEAP::InstanceData * instanceData)
{
	if (!instanceData) {
		return false;
	}
	ATCaliber curCaliber;
	if (!GetCurrentCaliber(instanceData, curCaliber)) {
		return false;
	}
	if (curCaliber.projectiles.empty()) {
		return false;
	}
	int iFoundReqs = 0;
	for (std::vector<ATCaliber::ProjectileOverride>::iterator itProjOvrd = curCaliber.projectiles.begin(); itProjOvrd != curCaliber.projectiles.end(); ++itProjOvrd) {
		// check required keywords
		iFoundReqs = itProjOvrd->projectileKWs.size();
		if (iFoundReqs > 0) {
			for (std::vector<BGSKeyword*>::iterator itProjKW = itProjOvrd->projectileKWs.begin(); itProjKW != itProjOvrd->projectileKWs.end(); ++itProjKW) {
				if (*itProjKW) {
					if (ATGameData::WeaponInstanceHasKeyword(instanceData, *itProjKW)) {
						iFoundReqs -= 1;
					}
				}
			}
		}
		if (iFoundReqs <= 0) {
			if (instanceData->firingData) {
				if (itProjOvrd->projectile) {
					instanceData->firingData->projectileOverride = itProjOvrd->projectile;
				}
			}
			if (itProjOvrd->impactData) {
				instanceData->unk58 = itProjOvrd->impactData;
			}
			return true;
		}
	}
	return false;
}


// -------------------------------- ATCritFailureTables:

SpellItem * ATWeapon::GetCritFailureSpell(float fCritFailChance, UInt32 iRollMod)
{
	int iCritRoll = ATConfigData::rng.RandomInt(0, 10000);
	if (iCritRoll > (int)floor(fCritFailChance * 100.0)) {
		// crit failure roll failed
		return nullptr;
	}
	int iRoll = ATConfigData::rng.RandomInt(0, 85 - (int)iRollMod);
	_MESSAGE("\nCrit Fail Chance Roll Passed:  Chance = %.02f,  rolled %.02f", fCritFailChance, (float)iCritRoll / 100.0);
	_MESSAGE("  Crit Fail Table Roll:  %i, roll mod: %i", iRoll, iRollMod);
	std::vector<CritFailure>::iterator critEffectIt = std::find_if(criticalFailureTable.begin(), criticalFailureTable.end(), FindPred_CritFailureEffect_ByRoll(iRoll));
	if (critEffectIt != criticalFailureTable.end()) {
		if (critEffectIt->spellCritFailure) {
			_MESSAGE("    Critical Failure: %s\n", critEffectIt->spellCritFailure->name.name.c_str());
			return critEffectIt->spellCritFailure;
		}
	}
	return nullptr;
}


// -------------------------------- ATModSlot

bool ATWeapon::GetNextSwapMod(Actor * ownerActor, UInt32 iModSlot, ATModSlot::SwappableMod & swapMod)
{
	if (!ownerActor || (iModSlot >= modSlots.size())) {
		return false;
	}
	ATModSlot curSlot = modSlots[iModSlot];
	if (curSlot.swappableMods.empty()) {
		return false;
	}
	ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
	if (!extraData) {
		return false;
	}
	if (curSlot.swappableMods.empty()) {
		return false;
	}

	int iSwapModsCount = curSlot.swappableMods.size();
	int iFound = 0;
	for (UInt32 i = 0; i < iSwapModsCount; i++) {
		if (ATGameData::InstanceHasMod(extraData, curSlot.swappableMods[i].swapMod)) {
			iFound = i + 1;
			if (iFound >= iSwapModsCount) {
				iFound = 0;
			}
			break;
		}
	}

	// ---- check for required items
	std::vector<UInt32> checkArray;
	if (iFound != 0) {
		checkArray.push_back(iFound);
	}
	checkArray.push_back(0);

	bool bFoundModItem = false;
	bool bFoundModMiscItem = false;
	//for (UInt32 i = 0; i < checkArray.count; i++) {
	for (std::vector<UInt32>::iterator checkIt = checkArray.begin(); checkIt != checkArray.end(); ++checkIt) {
		UInt32 checkVal = *checkIt;
		UInt32 modsCount = checkVal;
		for (std::vector<ATModSlot::SwappableMod>::iterator modSlotsIt = curSlot.swappableMods.begin(); modSlotsIt != curSlot.swappableMods.end(); ++modSlotsIt) {
			if (modsCount >= iSwapModsCount) {
				modsCount += 1;
				continue;
			}
			modsCount += 1;

			ATModSlot::SwappableMod curSwapMod = *modSlotsIt;
			TESObjectMISC *modMiscItem = nullptr;
			bFoundModItem = false;
			bFoundModMiscItem = false;
			
			if (curSwapMod.bRequireModMisc) {
				auto pair = g_modAttachmentMap->Find(&curSwapMod.swapMod);
				if (pair) {
					modMiscItem = pair->miscObject;
				}
			}

			if (curSwapMod.requiredItem) {
				bFoundModItem = (ATGameData::GetItemCount(ownerActor, curSwapMod.requiredItem->formID) > 0);
			}
			else {
				bFoundModItem = true;
			}
			if (modMiscItem) {
				bFoundModMiscItem = (ATGameData::GetItemCount(ownerActor, modMiscItem->formID) > 0);
			}
			else {
				bFoundModMiscItem = true;
			}

			if (bFoundModItem && bFoundModMiscItem) {
				swapMod = curSwapMod;
				return true;
			}
		}
		iSwapModsCount = checkVal;
	}

	return false;
}

bool ATWeapon::GetCurrentSwapMod(ExtraDataList * extraData, UInt32 iModSlot, ATModSlot::SwappableMod & curSwapMod)
{
	if (!extraData || (iModSlot >= modSlots.size())) {
		return false;
	}
	ATModSlot curSlot = modSlots[iModSlot];
	if (curSlot.swappableMods.empty()) {
		return false;
	}
	for (std::vector<ATModSlot::SwappableMod>::iterator itSwapMod = curSlot.swappableMods.begin(); itSwapMod != curSlot.swappableMods.end(); ++itSwapMod) {
		if (itSwapMod->swapMod) {
			if (ATGameData::InstanceHasMod(extraData, itSwapMod->swapMod)) {
				curSwapMod = *itSwapMod;
				return true;
			}
		}
	}
	// no ammo mod attached, return default
	curSwapMod = *curSlot.swappableMods.begin();
	return true;
}


bool ATWeapon::GetCurrentFiremode(ExtraDataList * extraData, ATFiremode & curFiremode)
{
	if (!extraData) {
		return false;
	}
	if (firemodes.empty()) {
		return false;
	}
	for (std::vector<ATFiremode>::iterator itFiremode = firemodes.begin(); itFiremode != firemodes.end(); ++itFiremode) {
		ATFiremode checkFiremode = *itFiremode;
		if (checkFiremode.firemodeMod) {
			if (ATGameData::InstanceHasMod(extraData, checkFiremode.firemodeMod)) {
				curFiremode = checkFiremode;
				return true;
			}
		}
	}
	// no ammo mod attached, return default
	curFiremode = *firemodes.begin();
	return true;
}


// -------------------------------- ATDataStore:

int ATGameData::GetCaliberDataIndex(UInt32 formID)
{
	if ((formID != 0) && !ATConfigData::ATCaliberBase.empty()) {
		std::vector<ATCaliberData>::iterator caliberDataIt = std::find_if(ATConfigData::ATCaliberBase.begin(), ATConfigData::ATCaliberBase.end(), FindPred_ATCaliberData_ByID(formID));
		if (caliberDataIt != ATConfigData::ATCaliberBase.end()) {
			return (int)(size_t)(caliberDataIt - ATConfigData::ATCaliberBase.begin());
		}
	}
}
bool ATGameData::GetCaliberDataByID(UInt32 formID, ATCaliberData & caliberData)
{
	if (!ATConfigData::ATCaliberBase.empty()) {
		std::vector<ATCaliberData>::iterator caliberDataIt = std::find_if(ATConfigData::ATCaliberBase.begin(), ATConfigData::ATCaliberBase.end(), FindPred_ATCaliberData_ByID(formID));
		if (caliberDataIt != ATConfigData::ATCaliberBase.end()) {
			caliberData = *caliberDataIt;
			return true;
		}
	}
	return false;
}

int ATGameData::GetWeaponIndex(UInt32 formID)
{
	if (!ATConfigData::ATWeapons.empty()) {
		std::vector<ATWeapon>::iterator weaponDataIt = std::find_if(ATConfigData::ATWeapons.begin(), ATConfigData::ATWeapons.end(), FindPred_ATWeapon_ByID(formID));
		if (weaponDataIt != ATConfigData::ATWeapons.end()) {
			return (int)(size_t)(weaponDataIt - ATConfigData::ATWeapons.begin());
		}
	}
	return -1;
}
bool ATGameData::GetWeaponByID(UInt32 formID, ATWeapon & weaponData)
{
	if (!ATConfigData::ATWeapons.empty()) {
		std::vector<ATWeapon>::iterator weaponDataIt = std::find_if(ATConfigData::ATWeapons.begin(), ATConfigData::ATWeapons.end(), FindPred_ATWeapon_ByID(formID));
		if (weaponDataIt != ATConfigData::ATWeapons.end()) {
			weaponData = *weaponDataIt;
			return true;
		}
	}
	return false;
}

bool ATGameData::GetEquippedWeapon(Actor * ownerActor, ATWeapon & weaponData)
{
	if (!ATConfigData::ATWeapons.empty()) {
		TESObjectWEAP * tempWeap = GetEquippedWeaponForm(ownerActor, 0);
		if (tempWeap) {
			return GetWeaponByID(tempWeap->formID, weaponData);
		}
	}
	return false;
}



const char * ATGameData::GetAVShortName(UInt32 avFormID)
{
	if (avFormID != 0 && !ATConfigData::AVNames.empty()) {
		std::vector<ActorValueShortName>::iterator avNameData = std::find_if(ATConfigData::AVNames.begin(), ATConfigData::AVNames.end(), FindPred_ATAVName_ByID(avFormID));
		if (avNameData != ATConfigData::AVNames.end()) {
			return avNameData->avName.c_str();
		}
	}
	return "";
}


// -- statics:
ATxoroshiro128p ATConfigData::rng = ATxoroshiro128p();

ActorValueInfo * ATConfigData::AV_ArmorPenetration = nullptr;
ActorValueInfo * ATConfigData::AV_ArmorPenetrationThrown = nullptr;
ActorValueInfo * ATConfigData::AV_TargetArmorMult = nullptr;
ActorValueInfo * ATConfigData::AV_TargetArmorMultThrown = nullptr;

ActorValueInfo * ATConfigData::AV_WeaponAmmoCount = nullptr;
ActorValueInfo * ATConfigData::AV_WeaponAmmoCountSecondary = nullptr;

ActorValueInfo * ATConfigData::AV_WeaponCNDMax = nullptr;
ActorValueInfo * ATConfigData::AV_WeaponCNDWear = nullptr;
ActorValueInfo * ATConfigData::AV_WeaponCNDPct = nullptr;
ActorValueInfo * ATConfigData::AV_WeaponDamageLevel = nullptr;

ActorValueInfo * ATConfigData::AV_CritFailChance = nullptr;
ActorValueInfo * ATConfigData::AV_CritFailRollMod = nullptr;
ActorValueInfo * ATConfigData::AV_CritFailMult = nullptr;
ActorValueInfo * ATConfigData::AV_RecoilMod = nullptr;
ActorValueInfo * ATConfigData::AV_SkillReqDiff = nullptr;
ActorValueInfo * ATConfigData::AV_SkillModActive = nullptr;

ActorValueInfo * ATConfigData::AV_StatusPoisoned = nullptr;
ActorValueInfo * ATConfigData::AV_StatusBleeding = nullptr;
ActorValueInfo * ATConfigData::AV_StatusRadiated = nullptr;
ActorValueInfo * ATConfigData::AV_StatusBurning = nullptr;
ActorValueInfo * ATConfigData::AV_StatusChilled = nullptr;
ActorValueInfo * ATConfigData::AV_StatusIonized = nullptr;
ActorValueInfo * ATConfigData::AV_StatusArmorDmg = nullptr;
ActorValueInfo * ATConfigData::AV_StatusBlinded = nullptr;

ActorValueInfo * ATConfigData::AV_Luck = nullptr;

TESAmmo * ATConfigData::Ammo_Broken = nullptr;
TESAmmo	* ATConfigData::Ammo_Jammed = nullptr;
TESAmmo	* ATConfigData::Ammo_Overheated = nullptr;

BGSMod::Attachment::Mod * ATConfigData::Mod_NullMagazine = nullptr;


std::vector<ATWeapon> ATConfigData::ATWeapons;
std::vector<ATCaliberData> ATConfigData::ATCaliberBase;
std::vector<ATThrownWeapon> ATConfigData::ATThrownWeapons;
std::vector<ActorValueShortName> ATConfigData::AVNames;

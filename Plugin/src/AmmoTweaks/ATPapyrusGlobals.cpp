#include "ATPapyrusGlobals.h"

#define SCRIPTNAME "AT:Globals"


namespace ATGlobals
{
	// *********************** Equipped Weapon Info

	BSFixedString GetEquippedWeapName(StaticFunctionTag*, Actor * thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.hudName;
			}
		}
		return BSFixedString("");
	}

	UInt32 GetEquippedAttackDamage(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->baseDamage >= 0)
					return (UInt32)instanceData->baseDamage;
				else {

				}
			}
		}
		return 0;
	}

	float GetEquippedCritChargeMult(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->critChargeBonus;
			}
		}
		return 0.0;
	}

	float GetEquippedWeight(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}

	ActorValueInfo * GetEquippedReqSkill(StaticFunctionTag*, Actor * ownerActor, UInt32 iSlot = 41)
	{
		if (ownerActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(ownerActor, iSlot);
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.requiredSkill;
			}
		}
		return nullptr;
	}


	// -------- Caliber/AmmoType

	// returns the equipped weapon's caliber keyword
	BGSKeyword * GetEquippedCaliber(StaticFunctionTag*, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (ownerActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(ownerActor, iEquipSlot);
			if (instanceData) {
				TESObjectWEAP * curWeapon = ATUtilities::GetEquippedWeapon(ownerActor, iEquipSlot);
				if (curWeapon) {
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
						ATCaliber tempCaliber;
						if (tempWeap.GetInstanceCaliber(instanceData, tempCaliber)) {
							BGSKeyword * tempKW = ((int)tempCaliber.objectID > 0) ? (BGSKeyword*)LookupFormByID(tempCaliber.objectID) : nullptr;
							return tempKW;
						}
					}
				}
			}
		}
		return nullptr;
	}

	// returns the index of the equipped ammo type in the caliber's ammo list
	UInt32 GetEquippedAmmoIndex(StaticFunctionTag*, BGSKeyword * curCaliber, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (curCaliber && ownerActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(ownerActor, iEquipSlot);
			if (instanceData) {
				TESObjectWEAP * curWeapon = ATUtilities::GetEquippedWeapon(ownerActor, iEquipSlot);
				if (curWeapon) {
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
						ATCaliber tempCaliber;
						if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
							if (tempCaliber.ammoTypes.count > 0) {
								for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
									if (tempCaliber.ammoTypes[i].ammoItem) {
										if (instanceData->ammo == tempCaliber.ammoTypes[i].ammoItem) {
											return i;
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return -1;
	}

	// returns the index of the equipped ammo type in the caliber's ammo list
	BSFixedString GetEquippedAmmoName(StaticFunctionTag*, BGSKeyword * curCaliber, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (curCaliber && ownerActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(ownerActor, iEquipSlot);
			if (instanceData) {
				if (instanceData->ammo) {
					TESObjectWEAP * curWeapon = ATUtilities::GetEquippedWeapon(ownerActor, iEquipSlot);
					if (curWeapon) {
						ATWeapon tempWeap;
						if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
							ATCaliber tempCaliber;
							if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
								if (tempCaliber.ammoTypes.count > 0) {
									for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
										if (tempCaliber.ammoTypes[i].ammoItem) {
											if (instanceData->ammo == tempCaliber.ammoTypes[i].ammoItem) {
												return tempCaliber.ammoTypes[i].ammoItem->fullName.name;
											}
										}
									}
								}
							}
						}
					}
					return instanceData->ammo->fullName.name;
				}
			}
		}
		return "";
	}

	// returns the number of ammo subtypes for this caliber
	UInt32 GetNumAmmoTypesForCaliber(StaticFunctionTag*, TESObjectWEAP * curWeapon, BGSKeyword * curCaliber)
	{
		if (curWeapon && curCaliber) {
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
					return tempCaliber.ammoTypes.count;
				}
			}
		}
		return 0;
	}

	// updates input arrays with data needed by the ammo selection menu
	bool GetHUDDataForCaliber(StaticFunctionTag*, Actor * ownerActor, TESObjectWEAP * curWeapon, BGSKeyword * curCaliber, VMArray<BSFixedString> modNames, VMArray<BSFixedString> modDescriptions, VMArray<UInt32> modItemCounts, VMArray<bool> modAllowedList)
	{
		if (curWeapon && ownerActor && curCaliber) {
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
					if (tempCaliber.ammoTypes.count > 0) {
						for (UInt32 i = 0; i < tempCaliber.ammoTypes.count; i++) {
							ATCaliber::AmmoType tempAmmo = tempCaliber.ammoTypes[i];

							// ammo name
							if (tempAmmo.ammoItem) {
								BSFixedString modName = BSFixedString(tempAmmo.ammoItem->GetFullName());
								modNames.Set(&modName, i);
							}

							// ammo description
							BSString str;
							CALL_MEMBER_FN(&tempAmmo.ammoMod->description, Get)(&str, nullptr);
							BSFixedString modDescStr = BSFixedString(str.Get());
							modDescriptions.Set(&modDescStr, i);

							// ammo count
							UInt32 itemCount = 0;
							if (tempAmmo.ammoItem) {
								if (ownerActor->inventoryList) {
									for (UInt32 j = 0; j < ownerActor->inventoryList->items.count; j++) {
										BGSInventoryItem inventoryItem;
										ownerActor->inventoryList->items.GetNthItem(j, inventoryItem);
										if (inventoryItem.form->formID == tempAmmo.ammoItem->formID) {
											if (inventoryItem.stack)
												itemCount = inventoryItem.stack->count;
											else
												itemCount = 1;
											break;
										}
									}
								}
							}
							else {
								itemCount = 0;
							}
							modItemCounts.Set(&itemCount, i);

							// misc item check - could be used to require tools, etc. for some ammo types
							bool bAllowed = false;
							TESObjectMISC * modMiscItem = nullptr;
							auto pair = g_modAttachmentMap->Find(&tempAmmo.ammoMod);
							if (pair) {
								modMiscItem = pair->miscObject;
							}
							if (modMiscItem) {
								if (ownerActor->inventoryList) {
									for (UInt32 j = 0; j < ownerActor->inventoryList->items.count; j++) {
										BGSInventoryItem inventoryItem;
										ownerActor->inventoryList->items.GetNthItem(j, inventoryItem);
										if (inventoryItem.form == modMiscItem) {
											bAllowed = true;
											break;
										}
									}
								}
							}
							else {
								bAllowed = true;
							}
							modAllowedList.Set(&bAllowed, i);
						}
						return true;
					}
				}
			}
		}
		return false;
	}

	// returns a caliber's casing item
	TESObjectMISC * GetCurrentCasing(StaticFunctionTag*, TESObjectWEAP * curWeapon, BGSKeyword * curCaliber, UInt32 iAmmoIndex)
	{
		if (curWeapon && curCaliber) {
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
					if (iAmmoIndex < tempCaliber.ammoTypes.count) {
						ATCaliber::AmmoType tempAmmoType = tempCaliber.ammoTypes[iAmmoIndex];
						return tempAmmoType.casingItem;
					}
				}
			}
		}
		return nullptr;
	}

	// returns a caliber's ammo subtype forms
	VMArray<TESAmmo*> GetCaliberAmmoTypes(StaticFunctionTag*, TESObjectWEAP * curWeapon, BGSKeyword * curCaliber)
	{
		VMArray<TESAmmo*> result;
		if (curCaliber && curWeapon) {
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
					if (tempCaliber.ammoTypes.count > 0) {
						TESAmmo * tempAmmo = nullptr;
						for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
							result.Push(&tempCaliber.ammoTypes[i].ammoItem);
						}
					}
				}
			}
		}
		return result;
	}

	// returns the ammo type mod at the given index for a caliber
	BGSMod::Attachment::Mod * GetAmmoModAtIndex(StaticFunctionTag*, TESObjectWEAP * curWeapon, BGSKeyword * curCaliber, UInt32 iAmmoIndex)
	{
		if (curWeapon && curCaliber) {
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
					if (iAmmoIndex < tempCaliber.ammoTypes.count) {
						ATCaliber::AmmoType tempAmmoType = tempCaliber.ammoTypes[iAmmoIndex];
						return tempAmmoType.ammoMod;
					}
				}
			}
		}
		return nullptr;
	}


	// -------- Swappable Mods

	// returns the number of active mod slots for this weapon type
	UInt32 GetNumSwapModSlots(StaticFunctionTag*, TESObjectWEAP * curWeapon)
	{
		if (curWeapon) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.modSlots.count;
			}
		}
		return 0;
	}

	// returns the name of the mod slot at the given index for a weapon
	BSFixedString GetModSlotName(StaticFunctionTag*, TESObjectWEAP * curWeapon, UInt32 iModSlot)
	{
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ATModSlot tempSlot;
					if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
						return tempSlot.slotName;
					}
				}
			}
		}
		return BSFixedString("");
	}

	// returns the number ObjectMods in a modSlot
	UInt32 GetNumModsForSlot(StaticFunctionTag*, TESObjectWEAP * curWeapon, UInt32 iModSlot)
	{
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				if (tempWeap.modSlots.count > iModSlot) {
					ATWeapon::ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
					return tempSlot.swappableMods.count;
				}
			}
		}
		return 0;
	}

	// returns the index of the equipped mod for a slot
	UInt32 GetEquippedModIndex(StaticFunctionTag*, Actor * tempActor, UInt32 iEquipSlot = 41, UInt32 iModSlot = 0)
	{
		if (tempActor && ((int)iModSlot > -1)) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(tempActor, iEquipSlot);
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iEquipSlot);
			if ((weapID > 0) && instanceData) {
				ActorEquipData * equipData = tempActor->equipData;
				if (equipData) {
					auto data = equipData->slots[iEquipSlot].extraData->data;
					if (data) {
						ATWeapon tempWeap;
						if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
							if (iModSlot < tempWeap.modSlots.count) {
								ATWeapon::ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
								for (UInt32 j = 0; j < tempSlot.swappableMods.count; j++) {
									ATWeapon::ATSwappableMod tempMod = tempSlot.swappableMods[j];
									if (tempMod.swapMod) {
										for (UInt32 i = 0; i < data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form); i++) {
											BGSMod::Attachment::Mod * objectMod = (BGSMod::Attachment::Mod *)Runtime_DynamicCast(LookupFormByID(data->forms[i].formId), RTTI_TESForm, RTTI_BGSMod__Attachment__Mod);
											if (tempMod.swapMod == objectMod) {
												return j;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return -1;
	}

	// updates input arrays with data needed by the attachment selection menu
	bool GetHUDDataForModSlot(StaticFunctionTag*, Actor * ownerActor, UInt32 iEquipSlot, UInt32 iModSlot, VMArray<BSFixedString> modNames, VMArray<BSFixedString> modDescriptions, VMArray<UInt32> modItemCounts, VMArray<bool> modAllowedList)
	{
		if (ownerActor) {
			TESObjectWEAP *curWeapon = ATUtilities::GetEquippedWeapon(ownerActor, iEquipSlot);
			if (curWeapon && ((int)iModSlot > -1)) {
				UInt32 weapID = curWeapon->formID;
				ATWeapon tempWeap;
				if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
					if (tempWeap.modSlots.count > iModSlot) {
						ATWeapon::ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
						if (tempSlot.swappableMods.count > 0) {
							for (UInt32 i = 0; i < tempSlot.swappableMods.count; i++) {
								ATWeapon::ATSwappableMod tempMod = tempSlot.swappableMods[i];

								// attachment name
								BSFixedString modName = tempMod.swapMod->fullName.name;
								modNames.Set(&modName, i);

								// description
								BSString str;
								CALL_MEMBER_FN(&tempMod.swapMod->description, Get)(&str, nullptr);
								BSFixedString modDescStr = BSFixedString(str.Get());
								modDescriptions.Set(&modDescStr, i);

								// required item check
								UInt32 itemCount = 0;
								if (tempMod.modItem) {
									if (ownerActor->inventoryList) {
										for (UInt32 j = 0; j < ownerActor->inventoryList->items.count; j++) {
											BGSInventoryItem inventoryItem;
											ownerActor->inventoryList->items.GetNthItem(j, inventoryItem);
											if (inventoryItem.form == tempMod.modItem) {
												itemCount = 1;
											}
										}
									}
								}
								else {
									itemCount = -1;
								}
								modItemCounts.Set(&itemCount, i);

								bool bAllowed = true;

								// misc item check
								if (tempMod.bRequireModMisc) {
									bAllowed = false;
									TESObjectMISC *modMiscItem = nullptr;
									auto pair = g_modAttachmentMap->Find(&tempMod.swapMod);
									if (pair) {
										modMiscItem = pair->miscObject;
									}
									if (modMiscItem) {
										if (ownerActor->inventoryList) {
											for (UInt32 j = 0; j < ownerActor->inventoryList->items.count; j++) {
												BGSInventoryItem inventoryItem;
												ownerActor->inventoryList->items.GetNthItem(j, inventoryItem);
												if (inventoryItem.form == modMiscItem) {
													bAllowed = true;
													break;
												}
											}
										}
									}
									else {
										bAllowed = true;
									}
								}
								modAllowedList.Set(&bAllowed, i);
							}
							return true;
						}
					}
				}
			}
		}
		return false;
	}

	// returns the mod at the given slot:index for a weapon
	BGSMod::Attachment::Mod * GetModAtIndex(StaticFunctionTag*, TESObjectWEAP * curWeapon, UInt32 iModSlot, UInt32 iModIndex)
	{
		if (curWeapon && ((int)iModSlot > -1) && ((int)iModIndex > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
					if (iModIndex < tempSlot.swappableMods.count) {
						ATWeapon::ATSwappableMod tempMod = tempSlot.swappableMods[iModIndex];
						return tempMod.swapMod;
					}
				}
			}
		}
		return nullptr;
	}

	// returns a mod's required item forms
	VMArray<TESForm*> GetModSlotRequiredItems(StaticFunctionTag*, TESObjectWEAP * curWeapon, UInt32 iModSlot)
	{
		VMArray<TESForm*> result;
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
					if (tempSlot.swappableMods.count > 0) {
						for (UInt8 i = 0; i < tempSlot.swappableMods.count; i++) {
							ATWeapon::ATSwappableMod tempSwapMod = tempSlot.swappableMods[i];
							result.Push(&tempSwapMod.modItem);
						}
					}
				}
			}
		}
		return result;
	}


	// ******************* Ammo

	// returns the equipped ammo form - used for ammo swapping checks and jamming
	TESAmmo * GetEquippedAmmo(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->ammo;
			}
		}
		return nullptr;
	}

	// temporarily equips the specified ammo form - used for jamming
	bool SetEquippedAmmo(StaticFunctionTag*, TESAmmo *newAmmo, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->ammo != newAmmo) {
					instanceData->ammo = newAmmo;
					return true;
				}
			}
		}
		return false;
	}

	// ******************* Projectiles

	BGSProjectile * GetEquippedProjectile(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					return instanceData->firingData->projectileOverride;
				}
			}
		}
		return nullptr;
	}

	// used to set the 'misfire' projectile preceding a jam (and random misfires)
	bool SetEquippedProjectile(StaticFunctionTag*, BGSProjectile * newProjectile, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					if (instanceData->firingData->projectileOverride != newProjectile) {
						instanceData->firingData->projectileOverride = newProjectile;
						return true;
					}
				}
			}
		}
		return false;
	}





	// ********************** Critical Effects/Failures

	// performs a critical roll and returns the resulting spell
	SpellItem * GetCritEffect(StaticFunctionTag*, BGSKeyword * critTableKW, Actor * targetActor, UInt32 iRollMod = 0)
	{
		if (critTableKW) {
			int tempCritTable = ATGameData::GetCritTableIndex(critTableKW->formID);
			if (tempCritTable > -1) {
				if (targetActor->race) {
					return ATGameData::ATCritEffectTables[tempCritTable].GetCritSpell(iRollMod, targetActor->race->formID);
				}
			}
		}
		return nullptr;
	}


	// performs a critical failure roll and returns the resulting spell
	SpellItem * GetCritFailure(StaticFunctionTag*, TESObjectWEAP * curWeapon, UInt32 iRollMod = 0, UInt32 iLuck = 0, float fCritFailChance = 1.0)
	{
		if (fCritFailChance > 0.0) {
			if (curWeapon) {
				int iRoll = ATUtilities::ATrng.RandomInt();
				_MESSAGE("crit fail chance roll: %i", iRoll);
				if (iRoll < (int)round(fCritFailChance)) {
					UInt32 weapID = curWeapon->formID;
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
						int iRollTable = ATUtilities::ATrng.RandomInt(0, (85 + iRollMod)) - (7 * (iLuck - 7));
						_MESSAGE("crit fail table roll: %i", iRollTable);
						for (UInt32 i = 0; i < tempWeap.critFailures.count; i++) {
							if (iRollTable < tempWeap.critFailures[i].rollMax) {
								if (tempWeap.critFailures[i].critSpell) {
									_MESSAGE("Critical Failure: %s", tempWeap.critFailures[i].critSpell->name.name.c_str());
								}
								return tempWeap.critFailures[i].critSpell;
							}
						}
					}
				}
			}
		}
		return nullptr;
	}

	BGSMod::Attachment::Mod *GetRandomDamagedMod(StaticFunctionTag*, Actor * ownerActor, UInt32 iEquipSlot, UInt32 iModSlot)
	{
		if (ownerActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(ownerActor, iEquipSlot);
			if (weapID > 0) {
				ATWeapon tempWeap;
				if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
					return tempWeap.GetDamagedMod(ownerActor, iEquipSlot, (int)iModSlot);
				}
			}
		}
		return nullptr;
	}



	// ********************** Magazines

	// returns the droppable misc object for the equipped magazine mod
	TESObjectMISC * GetEquippedMagItem(StaticFunctionTag*, Actor * thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(thisActor, iSlot);
				if (instanceData) {
					if (instanceData->keywords) {
						for (UInt8 j = 0; j < tempWeap.magazines.count; j++) {
							ATWeapon::ATMagItem tempMag = tempWeap.magazines[j];
							if (tempMag.magKW > 0) {
								for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
									if (instanceData->keywords->keywords[i]) {
										if (instanceData->keywords->keywords[i]->formID == tempMag.magKW) {
											return tempMag.magItem;
										}
									}
								}
							}
							else {
								return tempMag.magItem;
							}
						}
					}

				}
			}
		}
		return nullptr;
	}


	// ********************** Holsters/Sheathed Weapons

	TESObjectARMO * GetHolsterArmor(StaticFunctionTag*, Actor * thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
				TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(thisActor, iSlot);

				if (instanceData) {
					if (instanceData->keywords) {
						for (UInt8 j = 0; j < tempWeap.holsters.count; j++) {
							ATWeapon::ATHolsterArmor tempHolster = tempWeap.holsters[j];
							if (tempHolster.requiredKW > 0) {
								for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
									if (instanceData->keywords->keywords[i]) {
										if (instanceData->keywords->keywords[i]->formID == tempHolster.requiredKW) {
											return tempHolster.armorWeapon;
										}
									}
								}
							}
							else {
								return tempHolster.armorWeapon;
							}
						}
					}

				}
			}
		}
		return nullptr;
	}


	// ************************** Weapon instance updates:

	// updates the equipped weapon's projectile based on caliber, ammo type and found attachment keywords
	bool UpdateEquippedProjectile(StaticFunctionTag*, BGSKeyword * curCaliber, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (ownerActor) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(ownerActor, iEquipSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					TESObjectWEAP *curWeapon = ATUtilities::GetEquippedWeapon(ownerActor, iEquipSlot);
					if (curWeapon) {
						ATWeapon tempWeap;
						if (ATGameData::GetWeaponByID(curWeapon->formID, tempWeap)) {
							ATCaliber tempCaliber;
							if (tempWeap.GetCaliberDataByID(curCaliber->formID, tempCaliber)) {
								bool bFoundAmmo = false;
								ATCaliber::AmmoType tempAmmoType;

								for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
									if (tempCaliber.ammoTypes.GetNthItem(i, tempAmmoType)) {
										if (instanceData->ammo == tempAmmoType.ammoItem) {
											bFoundAmmo = true;
											break;
										}
									}
								}

								if (bFoundAmmo) {
									for (UInt8 j = 0; j < tempAmmoType.projectiles.count; j++) {
										ATCaliber::ProjectileOverride tempProjOvr;
										tempAmmoType.projectiles.GetNthItem(j, tempProjOvr);
										if (tempProjOvr.projectileKW > 0) {
											if (instanceData->keywords) {
												for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
													if (instanceData->keywords->keywords[i]) {
														if (instanceData->keywords->keywords[i]->formID == tempProjOvr.projectileKW) {
															if (tempProjOvr.impactData) {
																instanceData->unk58 = tempProjOvr.impactData;
															}
															if (instanceData->firingData->projectileOverride != tempProjOvr.projectile) {
																instanceData->firingData->projectileOverride = tempProjOvr.projectile;
																return true;
															}
														}
													}
												}
											}
											else {
												if (tempProjOvr.impactData) {
													instanceData->unk58 = tempProjOvr.impactData;
												}
												if (instanceData->firingData->projectileOverride != tempProjOvr.projectile) {
													instanceData->firingData->projectileOverride = tempProjOvr.projectile;
													return true;
												}
											}
										}
										else {
											if (tempProjOvr.impactData) {
												instanceData->unk58 = tempProjOvr.impactData;
											}
											if (instanceData->firingData->projectileOverride != tempProjOvr.projectile) {
												instanceData->firingData->projectileOverride = tempProjOvr.projectile;
												return true;
											}
										}
									}
								}
							}
						}
					}
				}
			}
		}
		return false;
	}


	/** updates recoil based on the passed multiplier
		return values: -1=already edited + no update needed, 0=already edited + update needed, 1=edit successful
	**/
	UInt32 UpdateEquippedRecoil(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41, float fRecoilMult = 1.0)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				ATAimModel * tempAimModel = (ATAimModel*)instanceData->aimModel;
				if (instanceData->aimModel) {
					ActorValueInfo * skillModAV = (ActorValueInfo*)ATUtilities::GetFormFromIdentifier("AmmoTweaks.esm|001DF2");
					if (skillModAV) {
						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}
						if (instanceData->modifiers) {
							// don't edit if the weapon is already modified
							UInt32 checkVal = (int)(fRecoilMult * 100.0);
							for (UInt8 i = 0; i < instanceData->modifiers->count; i++) {
								TBO_InstanceData::ValueModifier tempAVMod;
								if (instanceData->modifiers->GetNthItem(i, tempAVMod)) {
									if (tempAVMod.avInfo == skillModAV) {
										if (tempAVMod.unk08 == checkVal)
											return 0;
										else
											return 1;
									}
								}
							}

							if (checkVal != 100) {
								// edit recoil
								ATAimModel * tempAimModel = (ATAimModel*)instanceData->aimModel;
								if (tempAimModel) {
									tempAimModel->Rec_HipMult *= fRecoilMult;
									tempAimModel->Rec_MinPerShot *= fRecoilMult;
									tempAimModel->Rec_MaxPerShot = max(tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot * fRecoilMult);

									// add the check AV
									if (checkVal != 0) {
										TBO_InstanceData::ValueModifier skillAVMod;
										skillAVMod.avInfo = skillModAV;
										skillAVMod.unk08 = checkVal;
										instanceData->modifiers->Push(skillAVMod);
										_MESSAGE("\nRecoil updated...\n    Min: %.4f, Max: %.4f, HipMult: %.4f", tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot, tempAimModel->Rec_HipMult);
									}
									return 1;
								}
							}
						}
					}
				}
			}
		}
		return 0;
	}


	// -------- animations:

	// returns the idle to play when starting an ammo swap
	TESForm * GetAmmoSwapIdle_1P(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if ((weapID > 0) && instanceData) {
				int weapIndex = ATGameData::GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
						ATCaliber tempCaliber;
						if (tempWeap.GetInstanceCaliber(instanceData, tempCaliber)) {
							return tempCaliber.swapIdle_1P;
						}
					}
				}
			}
		}
		return nullptr;
	}

	// returns the idle to play when starting an ammo swap
	TESForm * GetAmmoSwapIdle_3P(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);
			if ((weapID > 0) && instanceData) {
				int weapIndex = ATGameData::GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
						ATCaliber tempCaliber;
						if (tempWeap.GetInstanceCaliber(instanceData, tempCaliber)) {
							return tempCaliber.swapIdle_3P;
						}
					}
				}
			}
		}
		return nullptr;
	}

	// returns the animation event to listen for before swapping ammo
	BSFixedString GetAmmoSwapAnimEvent(StaticFunctionTag*, Actor * tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATUtilities::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(tempActor, iSlot);

			if ((weapID > 0) && instanceData) {
				int weapIndex = ATGameData::GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap;
					if (ATGameData::GetWeaponByID(weapID, tempWeap)) {
						ATCaliber tempCaliber;
						if (tempWeap.GetInstanceCaliber(instanceData, tempCaliber)) {
							return tempCaliber.swapAnimEventName;
						}
					}
				}
			}
		}
		return BSFixedString("");
	}



	// -------- debug/dev


	bool LogEquippedWeaponUnknowns(StaticFunctionTag*, Actor * thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			TESObjectWEAP::InstanceData * instanceData = ATUtilities::GetEquippedInstanceData(thisActor, iSlot);

			if (instanceData) {
				_MESSAGE(
					"ints:\n   18: %i\n   20: %i\n   50: %i\n  100: %i\n  114: %i\n  118: %i\n  11C: %i\n  134: %i",
					instanceData->unk18, instanceData->unk20, instanceData->unk50, instanceData->unk100, instanceData->unk114,
					instanceData->unk118, instanceData->unk11C, instanceData->unk134
				);

				if (instanceData->ammo) {
					_MESSAGE("\n   Ammo unk160: %016X", instanceData->ammo->unk160);
				}

				//_MESSAGE("  137: %i (Weapon Anim Type)", instanceData->unk137);
			}
		}
		return false;
	}


}

bool ATGlobals::RegisterPapyrus(VirtualMachine* vm)
{
	ATGlobals::RegisterFuncs(vm);
	return true;
}


void ATGlobals::RegisterFuncs(VirtualMachine* vm)
{
	// ---- Weapon Info
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BSFixedString, Actor*, UInt32>("GetEquippedWeapName", SCRIPTNAME, GetEquippedWeapName, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("GetEquippedAttackDamage", SCRIPTNAME, ATGlobals::GetEquippedAttackDamage, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, float, Actor*, UInt32>("GetEquippedCritChargeMult", SCRIPTNAME, ATGlobals::GetEquippedCritChargeMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, float, Actor*, UInt32>("GetEquippedWeight", SCRIPTNAME, ATGlobals::GetEquippedWeight, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, ActorValueInfo*, Actor*, UInt32>("GetEquippedReqSkill", SCRIPTNAME, ATGlobals::GetEquippedReqSkill, vm));

	// ---- Ammo Mods
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSKeyword*, Actor*, UInt32>("GetEquippedCaliber", SCRIPTNAME, GetEquippedCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, BGSKeyword*, Actor*, UInt32>("GetEquippedAmmoIndex", SCRIPTNAME, GetEquippedAmmoIndex, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, BSFixedString, BGSKeyword*, Actor*, UInt32>("GetEquippedAmmoName", SCRIPTNAME, GetEquippedAmmoName, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, TESObjectWEAP*, BGSKeyword*>("GetNumAmmoTypesForCaliber", SCRIPTNAME, GetNumAmmoTypesForCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction7 <StaticFunctionTag, bool, Actor*, TESObjectWEAP*, BGSKeyword*, VMArray<BSFixedString>, VMArray<BSFixedString>, VMArray<UInt32>, VMArray<bool>>("GetHUDDataForCaliber", SCRIPTNAME, GetHUDDataForCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, TESObjectMISC*, TESObjectWEAP*, BGSKeyword*, UInt32>("GetCurrentCasing", SCRIPTNAME, GetCurrentCasing, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, VMArray<TESAmmo*>, TESObjectWEAP*, BGSKeyword*>("GetCaliberAmmoTypes", SCRIPTNAME, GetCaliberAmmoTypes, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, BGSMod::Attachment::Mod*, TESObjectWEAP*, BGSKeyword*, UInt32>("GetAmmoModAtIndex", SCRIPTNAME, GetAmmoModAtIndex, vm));

	// ---- Swappable Mods
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, TESObjectWEAP*>("GetNumSwapModSlots", SCRIPTNAME, GetNumSwapModSlots, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BSFixedString, TESObjectWEAP*, UInt32>("GetModSlotName", SCRIPTNAME, GetModSlotName, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, TESObjectWEAP*, UInt32>("GetNumModsForSlot", SCRIPTNAME, GetNumModsForSlot, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, Actor*, UInt32, UInt32>("GetEquippedModIndex", SCRIPTNAME, GetEquippedModIndex, vm));
	vm->RegisterFunction(
		new NativeFunction7 <StaticFunctionTag, bool, Actor*, UInt32, UInt32, VMArray<BSFixedString>, VMArray<BSFixedString>, VMArray<UInt32>, VMArray<bool>>("GetHUDDataForModSlot", SCRIPTNAME, GetHUDDataForModSlot, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, BGSMod::Attachment::Mod*, TESObjectWEAP*, UInt32, UInt32>("GetModAtIndex", SCRIPTNAME, GetModAtIndex, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, VMArray<TESForm*>, TESObjectWEAP*, UInt32>("GetModSlotRequiredItems", SCRIPTNAME, GetModSlotRequiredItems, vm));

	// ---- Ammo
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESAmmo*, Actor*, UInt32>("GetEquippedAmmo", SCRIPTNAME, GetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, TESAmmo*, Actor*, UInt32>("SetEquippedAmmo", SCRIPTNAME, SetEquippedAmmo, vm));

	// ---- Projectiles
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, BGSProjectile*, Actor*, UInt32>("SetEquippedProjectile", SCRIPTNAME, ATGlobals::SetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSProjectile*, Actor*, UInt32>("GetEquippedProjectile", SCRIPTNAME, ATGlobals::GetEquippedProjectile, vm));

	// ---- Crit Effect Tables
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, SpellItem*, BGSKeyword*, Actor*, UInt32>("GetCritEffect", SCRIPTNAME, ATGlobals::GetCritEffect, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, SpellItem*, TESObjectWEAP*, UInt32, UInt32, float>("GetCritFailure", SCRIPTNAME, ATGlobals::GetCritFailure, vm));
	
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, BGSMod::Attachment::Mod*, Actor*, UInt32, UInt32>("GetRandomDamagedMod", SCRIPTNAME, ATGlobals::GetRandomDamagedMod, vm));

	// ---- Magazine Items
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectMISC*, Actor*, UInt32>("GetEquippedMagItem", SCRIPTNAME, ATGlobals::GetEquippedMagItem, vm));

	// ---- Holster/Sheathed Weapon Armors
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectARMO*, Actor*, UInt32>("GetHolsterArmor", SCRIPTNAME, ATGlobals::GetHolsterArmor, vm));


	// ---- Weapon Updates
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, BGSKeyword*, Actor*, UInt32>("UpdateEquippedProjectile", SCRIPTNAME, UpdateEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, Actor*, UInt32, float>("UpdateEquippedRecoil", SCRIPTNAME, UpdateEquippedRecoil, vm));

	// ---- Animations
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESForm*, Actor*, UInt32>("GetAmmoSwapIdle_1P", SCRIPTNAME, GetAmmoSwapIdle_1P, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESForm*, Actor*, UInt32>("GetAmmoSwapIdle_3P", SCRIPTNAME, GetAmmoSwapIdle_3P, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BSFixedString, Actor*, UInt32>("GetAmmoSwapAnimEvent", SCRIPTNAME, GetAmmoSwapAnimEvent, vm));


	// ---- Debug/Dev stuff
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("LogEquippedWeaponUnknowns", SCRIPTNAME, ATGlobals::LogEquippedWeaponUnknowns, vm));


	_MESSAGE("Registered native functions for %s", SCRIPTNAME);
}


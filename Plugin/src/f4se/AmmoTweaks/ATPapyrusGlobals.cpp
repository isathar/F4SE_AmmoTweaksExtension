#include "ATPapyrusGlobals.h"

#define SCRIPTNAME "AT:Globals"



namespace ATGlobals
{
	// *********************** Equipped Weapon Info

	BSFixedString GetEquippedWeapName(StaticFunctionTag*, Actor *thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.hudName;
			}
		}
		return BSFixedString("");
	}

	UInt32 GetEquippedAttackDamage(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->baseDamage >= 0)
					return (UInt32)instanceData->baseDamage;
				else {

				}
			}
		}
		return 0;
	}

	float GetEquippedCritChargeMult(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->critChargeBonus;
			}
		}
		return 0.0;
	}

	float GetEquippedWeight(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}

	ActorValueInfo* GetEquippedReqSkill(StaticFunctionTag*, Actor *ownerActor, UInt32 iSlot = 41)
	{
		if (ownerActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(ownerActor, iSlot);
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.requiredSkill;
			}
		}
		return nullptr;
	}


	// -------- Caliber/AmmoType

	// returns the equipped weapon's caliber keyword
	BGSKeyword* GetEquippedCaliber(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return ATShared::GetInstanceCaliber(instanceData);
			}
		}
		return nullptr;
	}

	// returns the index of the equipped ammo type in the caliber's ammo list
	UInt32 GetEquippedAmmoIndex(StaticFunctionTag*, BGSKeyword *curCaliber, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (curCaliber && tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->ammo) {
					ATCaliber tempCaliber;
					if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
						if (tempCaliber.ammoTypes.count > 0) {
							TESAmmo *tempAmmo = nullptr;
							for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
								tempAmmo = (TESAmmo*)tempCaliber.ammoTypes[i].modItem;
								if (instanceData->ammo == tempAmmo) {
									return i;
								}
							}
						}
					}
				}
			}
		}
		return -1;
	}

	// returns the equipped ammo mod's name (for the HUD widget)
	BSFixedString GetEquippedAmmoName(StaticFunctionTag*, BGSKeyword *curCaliber, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (curCaliber && tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);

			if (instanceData) {
				if (instanceData->ammo) {
					ATCaliber tempCaliber;
					if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
						if (tempCaliber.ammoTypes.count > 0) {
							TESAmmo *tempAmmo = nullptr;
							for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
								ATCaliber::AmmoType tempAmmoType = tempCaliber.ammoTypes[i];
								tempAmmo = (TESAmmo*)tempAmmoType.modItem;
								if (instanceData->ammo == tempAmmo) {
									return tempAmmoType.swapMod->fullName.name;
								}
							}
							return instanceData->ammo->fullName.name;
						}
					}
				}
			}
		}
		return BSFixedString("");
	}

	// returns the number of ammo subtypes for this caliber
	UInt32 GetNumAmmoTypesForCaliber(StaticFunctionTag*, BGSKeyword *curCaliber)
	{
		if (curCaliber) {
			ATCaliber tempCaliber;
			if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
				return tempCaliber.ammoTypes.count;
			}
		}
		return 0;
	}

	// updates input arrays with data needed by the ammo selection menu
	bool GetHUDDataForCaliber(StaticFunctionTag*, Actor *ownerActor, BGSKeyword *curCaliber, VMArray<BSFixedString> modNames, VMArray<BSFixedString> modDescriptions, VMArray<UInt32> modItemCounts, VMArray<bool> modAllowedList)
	{
		if (ownerActor && curCaliber) {
			ATCaliber tempCaliber;
			if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
				if (tempCaliber.ammoTypes.count > 0) {
					for (UInt32 i = 0; i < tempCaliber.ammoTypes.count; i++) {
						ATCaliber::AmmoType tempAmmo = tempCaliber.ammoTypes[i];

						// ammo name
						if (tempAmmo.modItem) {
							BSFixedString modName = BSFixedString(tempAmmo.modItem->GetFullName());
							modNames.Set(&modName, i);
						}

						// ammo description
						BSString str;
						CALL_MEMBER_FN(&tempAmmo.swapMod->description, Get)(&str, nullptr);
						BSFixedString modDescStr = BSFixedString(str.Get());
						modDescriptions.Set(&modDescStr, i);

						// ammo count
						UInt32 itemCount = 0;
						if (tempAmmo.modItem) {
							if (ownerActor->inventoryList) {
								for (UInt32 j = 0; j < ownerActor->inventoryList->items.count; j++) {
									BGSInventoryItem inventoryItem;
									ownerActor->inventoryList->items.GetNthItem(j, inventoryItem);
									if (inventoryItem.form == tempAmmo.modItem) {
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
						TESObjectMISC *modMiscItem = nullptr;
						auto pair = g_modAttachmentMap->Find(&tempAmmo.swapMod);
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
		return false;
	}

	// returns a caliber's casing item
	TESObjectMISC *GetCurrentCasing(StaticFunctionTag*, BGSKeyword *curCaliber, UInt32 iAmmoIndex)
	{
		if (curCaliber) {
			ATCaliber tempCaliber;
			if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
				if (iAmmoIndex < tempCaliber.ammoTypes.count) {
					ATCaliber::AmmoType tempAmmoType = tempCaliber.ammoTypes[iAmmoIndex];
					return tempAmmoType.casingItem;
				}
			}
		}
		return nullptr;
	}

	// returns a caliber's ammo subtype forms
	VMArray<TESAmmo*> GetCaliberAmmoTypes(StaticFunctionTag*, BGSKeyword *curCaliber)
	{
		VMArray<TESAmmo*> result;
		if (curCaliber) {
			ATCaliber tempCaliber;
			if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
				if (tempCaliber.ammoTypes.count > 0) {
					TESAmmo *tempAmmo = nullptr;
					for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
						tempAmmo = (TESAmmo*)tempCaliber.ammoTypes[i].modItem;
						result.Push(&tempAmmo);
					}
				}
			}
		}
		return result;
	}

	// returns the ammo type mod at the given index for a caliber
	BGSMod::Attachment::Mod *GetAmmoModAtIndex(StaticFunctionTag*, BGSKeyword *curCaliber, UInt32 iAmmoIndex)
	{
		if (curCaliber) {
			ATCaliber tempCaliber;
			if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
				if (iAmmoIndex < tempCaliber.ammoTypes.count) {
					ATCaliber::AmmoType tempAmmoType = tempCaliber.ammoTypes[iAmmoIndex];
					return tempAmmoType.swapMod;
				}
			}
		}
		return nullptr;
	}


	// -------- Swappable Mods

	// returns the number of active mod slots for this weapon type
	UInt32 GetNumSwapModSlots(StaticFunctionTag*, TESObjectWEAP *curWeapon)
	{
		if (curWeapon) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				return tempWeap.modSlots.count;
			}
		}
		return 0;
	}

	// returns the name of the mod slot at the given index for a weapon
	BSFixedString GetModSlotName(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot)
	{
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ModSlot tempSlot;
					if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
						return tempSlot.slotName;
					}
				}
			}
		}
		return BSFixedString("");
	}

	// returns the number ObjectMods in a modSlot
	UInt32 GetNumModsForSlot(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot)
	{
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				if (tempWeap.modSlots.count > iModSlot) {
					ATWeapon::ModSlot tempSlot = tempWeap.modSlots[iModSlot];
					return tempSlot.swappableMods.count;
				}
			}
		}
		return 0;
	}

	// returns the index of the equipped mod for a slot
	UInt32 GetEquippedModIndex(StaticFunctionTag*, Actor *tempActor, UInt32 iEquipSlot = 41, UInt32 iModSlot = 0)
	{
		if (tempActor && ((int)iModSlot > -1)) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(tempActor, iEquipSlot);
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iEquipSlot);
			if ((weapID > 0) && instanceData) {
				ActorEquipData * equipData = tempActor->equipData;
				if (equipData) {
					auto data = equipData->slots[iEquipSlot].extraData->data;
					if (data) {
						ATWeapon tempWeap;
						if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
							if (iModSlot < tempWeap.modSlots.count) {
								ATWeapon::ModSlot tempSlot = tempWeap.modSlots[iModSlot];
								for (UInt32 j = 0; j < tempSlot.swappableMods.count; j++) {
									ATSwappableMod tempMod = tempSlot.swappableMods[j];
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
	bool GetHUDDataForModSlot(StaticFunctionTag*, Actor *ownerActor, UInt32 iEquipSlot, UInt32 iModSlot, VMArray<BSFixedString> modNames, VMArray<BSFixedString> modDescriptions, VMArray<UInt32> modItemCounts, VMArray<bool> modAllowedList)
	{
		if (ownerActor) {
			TESObjectWEAP *curWeapon = ATShared::GetEquippedWeapon(ownerActor, iEquipSlot);
			if (curWeapon && ((int)iModSlot > -1)) {
				UInt32 weapID = curWeapon->formID;
				ATWeapon tempWeap;
				if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
					if (tempWeap.modSlots.count > iModSlot) {
						ATWeapon::ModSlot tempSlot = tempWeap.modSlots[iModSlot];
						if (tempSlot.swappableMods.count > 0) {
							for (UInt32 i = 0; i < tempSlot.swappableMods.count; i++) {
								ATSwappableMod tempMod = tempSlot.swappableMods[i];

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
	BGSMod::Attachment::Mod *GetModAtIndex(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot, UInt32 iModIndex)
	{
		if (curWeapon && ((int)iModSlot > -1) && ((int)iModIndex > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ModSlot tempSlot = tempWeap.modSlots[iModSlot];
					if (iModIndex < tempSlot.swappableMods.count) {
						ATSwappableMod tempMod = tempSlot.swappableMods[iModIndex];
						return tempMod.swapMod;
					}
				}
			}
		}
		return nullptr;
	}

	// returns a mod's required item forms
	VMArray<TESForm*> GetModSlotRequiredItems(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot)
	{
		VMArray<TESForm*> result;
		if (curWeapon && ((int)iModSlot > -1)) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				if (iModSlot < tempWeap.modSlots.count) {
					ATWeapon::ModSlot tempSlot = tempWeap.modSlots[iModSlot];
					if (tempSlot.swappableMods.count > 0) {
						for (UInt8 i = 0; i < tempSlot.swappableMods.count; i++) {
							ATSwappableMod tempSwapMod = tempSlot.swappableMods[i];
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
	TESAmmo* GetEquippedAmmo(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
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
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
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

	BGSProjectile* GetEquippedProjectile(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					return instanceData->firingData->projectileOverride;
				}
			}
		}
		return nullptr;
	}

	// used to set the 'misfire' projectile preceding a jam (and random misfires)
	bool SetEquippedProjectile(StaticFunctionTag*, BGSProjectile *newProjectile, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
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
	SpellItem *GetCritEffect(StaticFunctionTag*, BGSKeyword *critTableKW, UInt32 iRollMod = 0)
	{
		if (critTableKW) {
			UInt32 tempKW = critTableKW->formID;
			if (tempKW > 0) {
				int tempCritTable = ATShared::ATData.GetCritTableIndex(tempKW);
				if (tempCritTable > -1) {
					return ATShared::ATData.CritEffectTables[tempCritTable].GetCritSpell(iRollMod);
				}
			}
		}
		return ATShared::ATData.CritEffectTables[0].GetCritSpell(iRollMod);
	}


	// performs a critical failure roll and returns the resulting spell
	SpellItem *GetCritFailure(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iRollMod = 0, UInt32 iLuck = 0)
	{
		if (curWeapon) {
			UInt32 weapID = curWeapon->formID;
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				if (tempWeap.critFailureType) {
					int critFailIndex = ATShared::ATData.GetCritFailTableIndex(tempWeap.critFailureType->formID);
					if (critFailIndex > -1) {
						ATCritEffectTable tempTable = ATShared::ATData.CritFailureTables[critFailIndex];
						return tempTable.GetCritFailureSpell(iRollMod, iLuck);
					}
				}
			}
		}
		return nullptr;
	}


	// ********************** Magazines

	// returns the droppable misc object for the equipped magazine mod
	TESObjectMISC *GetEquippedMagItem(StaticFunctionTag*, Actor *thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(thisActor, iSlot);

				if (instanceData) {
					if (instanceData->keywords) {
						for (UInt8 j = 0; j < tempWeap.magazines.count; j++) {
							ATWeapon::DroppableMag tempMag = tempWeap.magazines[j];
							if (tempMag.magKW) {
								for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
									if (instanceData->keywords->keywords[i] == tempMag.magKW) {
										return tempMag.magItem;
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

	TESObjectARMO *GetHolsterArmor(StaticFunctionTag*, Actor *thisActor, UInt32 iSlot = 41, bool weapDrawn = true)
	{
		if (thisActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(thisActor, iSlot);
			ATWeapon tempWeap;
			if (ATShared::ATData.GetWeaponByID(weapID, tempWeap)) {
				TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(thisActor, iSlot);

				if (instanceData) {
					if (instanceData->keywords) {
						for (UInt8 j = 0; j < tempWeap.holsters.count; j++) {
							ATWeapon::HolsterArmor tempHolster = tempWeap.holsters[j];
							if (tempHolster.requiredKW) {
								for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
									if (instanceData->keywords->keywords[i] == tempHolster.requiredKW) {
										if (weapDrawn)
											return tempHolster.armorHolster;
										else
											return tempHolster.armorWeapon;
									}
								}
							}
							else {
								if (weapDrawn)
									return tempHolster.armorHolster;
								else
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
	bool UpdateEquippedProjectile(StaticFunctionTag*, BGSKeyword *curCaliber, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					ATCaliber tempCaliber;
					if (ATShared::ATData.GetCaliberByID(curCaliber->formID, tempCaliber)) {
						bool bFoundAmmo = false;
						ATCaliber::AmmoType tempAmmoType;
						TESAmmo *tempAmmo = nullptr;

						for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
							if (tempCaliber.ammoTypes.GetNthItem(i, tempAmmoType)) {
								tempAmmo = (TESAmmo*)tempAmmoType.modItem;
								if (instanceData->ammo == tempAmmo) {
									bFoundAmmo = true;
									break;
								}
							}
						}
						if (bFoundAmmo) {
							for (UInt8 j = 0; j < tempAmmoType.projectiles.count; j++) {
								ATCaliber::ProjectileOverride tempProjOvr;
								tempAmmoType.projectiles.GetNthItem(j, tempProjOvr);
								if (tempProjOvr.projectileKW) {
									if (instanceData->keywords) {
										for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
											if (instanceData->keywords->keywords[i] == tempProjOvr.projectileKW) {
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
		return false;
	}

	/** updates recoil based on the passed multiplier
		return values: -1=already edited + no update needed, 0=already edited + update needed, 1=edit successful
	**/
	UInt32 UpdateEquippedRecoil(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41, float fRecoilMult = 1.0)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				ATAimModel *tempAimModel = (ATAimModel*)instanceData->aimModel;
				if (tempAimModel) {
					ActorValueInfo *skillModAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001DF2");
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
		return 0;
	}

	// -------- animations:

	// returns the idle to play when starting an ammo swap
	TESForm *GetAmmoSwapIdle_1P(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);

			if ((weapID > 0) && instanceData) {
				int weapIndex = ATShared::ATData.GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap = ATShared::ATData.Weapons[weapIndex];
					return tempWeap.ammoSwapIdle_1P;
				}
			}
		}
		return nullptr;
	}

	// returns the idle to play when starting an ammo swap
	TESForm *GetAmmoSwapIdle_3P(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);

			if ((weapID > 0) && instanceData) {
				int weapIndex = ATShared::ATData.GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap = ATShared::ATData.Weapons[weapIndex];
					return tempWeap.ammoSwapIdle_3P;
				}
			}
		}
		return nullptr;
	}

	// returns the animation event to listen for before swapping ammo
	BSFixedString GetAmmoSwapAnimEvent(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = ATShared::GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(tempActor, iSlot);

			if ((weapID > 0) && instanceData) {
				int weapIndex = ATShared::ATData.GetWeaponIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap = ATShared::ATData.Weapons[weapIndex];
					return tempWeap.ammoSwapAnimEvent;
				}
			}
		}
		return BSFixedString("");
	}



	// -------- debug/dev


	bool LogEquippedWeaponUnknowns(StaticFunctionTag*, Actor *thisActor, UInt32 iSlot = 41)
	{
		if (thisActor) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetEquippedInstanceData(thisActor, iSlot);

			if (instanceData) {
				_MESSAGE(
					"ints:\n   18: %i\n   20: %i\n   50: %i\n  100: %i\n  114: %i\n  118: %i\n  11C: %i\n  134: %i", 
					instanceData->unk18, instanceData->unk20, instanceData->unk50, instanceData->unk100, instanceData->unk114,
					instanceData->unk118, instanceData->unk11C, instanceData->unk134
				);
				
				//_MESSAGE("  137: %i (Weapon Anim Type)", instanceData->unk137);
			}
		}
		return false;
	}

	bool AddObjectModMiscItem(StaticFunctionTag*, BGSMod::Attachment::Mod *thisMod, TESObjectMISC *miscItem)
	{
		if (thisMod && miscItem) {
			ObjectModMiscPair *pair = g_modAttachmentMap->Find(&thisMod);
			if (pair) {
				pair->miscObject = miscItem;
				return true;
			}
			else {
				pair = new ObjectModMiscPair();
				if (pair) {
					pair->key = thisMod;
					pair->miscObject = miscItem;
					return g_modAttachmentMap->Add(pair);
				}
			}
		}
		return false;
	}

	bool AddSpellToRace(StaticFunctionTag*, SpellItem *newSpell, TESRace *thisRace)
	{

		return false;
	}


}


void ATGlobals::RegisterPapyrus(VirtualMachine* vm)
{
	ATGlobals::RegisterFuncs(vm);
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
		new NativeFunction1 <StaticFunctionTag, UInt32, BGSKeyword*>("GetNumAmmoTypesForCaliber", SCRIPTNAME, GetNumAmmoTypesForCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction6 <StaticFunctionTag, bool, Actor*, BGSKeyword*, VMArray<BSFixedString>, VMArray<BSFixedString>, VMArray<UInt32>, VMArray<bool>>("GetHUDDataForCaliber", SCRIPTNAME, GetHUDDataForCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectMISC*, BGSKeyword*, UInt32>("GetCurrentCasing", SCRIPTNAME, GetCurrentCasing, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESAmmo*>, BGSKeyword*>("GetCaliberAmmoTypes", SCRIPTNAME, GetCaliberAmmoTypes, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSMod::Attachment::Mod*, BGSKeyword*, UInt32>("GetAmmoModAtIndex", SCRIPTNAME, GetAmmoModAtIndex, vm));

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
		new NativeFunction2 <StaticFunctionTag, SpellItem*, BGSKeyword*, UInt32>("GetCritEffect", SCRIPTNAME, ATGlobals::GetCritEffect, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, SpellItem*, TESObjectWEAP*, UInt32, UInt32>("GetCritFailure", SCRIPTNAME, ATGlobals::GetCritFailure, vm));
	
	// ---- Magazine Items
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectMISC*, Actor*, UInt32>("GetEquippedMagItem", SCRIPTNAME, ATGlobals::GetEquippedMagItem, vm));

	// ---- Holster/Sheathed Weapon Armors
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, TESObjectARMO*, Actor*, UInt32, bool>("GetHolsterArmor", SCRIPTNAME, ATGlobals::GetHolsterArmor, vm));


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


	_MESSAGE("    %s", SCRIPTNAME);
}


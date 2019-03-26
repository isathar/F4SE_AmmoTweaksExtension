#include "ATWeaponRef.h"
#include <time.h>

#define SCRIPTNAME_E "AT:Globals"
#define SCRIPTNAME_R "AT:Items:WeaponReferenceBase"



BGSMod::Attachment::Mod *ATWeapon::GetDamagedMod(TESObjectWEAP::InstanceData* instanceData, int iSlot)
{
	if (instanceData) {
		if (damagedMods.count > iSlot) {
			if (iSlot < 0) {
				srand(time(NULL));
				iSlot = rand() % damagedMods.count;
			}

			ATModSlot tempSlot;
			if (damagedMods.GetNthItem(iSlot, tempSlot)) {
				if (tempSlot.swappableMods.count > 0) {
					int iIndex = 0;
					if (tempSlot.swappableMods.count > 1) {
						srand(time(NULL));
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

							if (bAllow) {
								for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
									if (tempMod.excludeKW) {
										if (instanceData->keywords->keywords[i] == tempMod.requiredKW) {
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
	return nullptr;
}

namespace ATWeaponRef
{
	
	TESObjectWEAP::InstanceData *GetWeapRefInstanceData(VMRefOrInventoryObj *curObj, bool bReset = false)
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

	TESObjectWEAP::InstanceData *GetEquippedInstanceData(Actor *thisActor, UInt32 iSlot = 41)
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

	UInt32 GetEquippedItemFormID(Actor *thisActor, UInt32 iSlot = 41)
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

	const char *GetWeapName(VMRefOrInventoryObj *curObj)
	{
		if (curObj) {
			TESObjectREFR *tempRef = curObj->GetObjectReference();
			if (tempRef) {
				if (tempRef->extraDataList) {
					BSExtraData * extraData = tempRef->extraDataList->GetByType(ExtraDataType::kExtraData_TextDisplayData);
					if (extraData) {
						ExtraTextDisplayData * nameData = DYNAMIC_CAST(extraData, BSExtraData, ExtraTextDisplayData);
						if (nameData) {
							return *CALL_MEMBER_FN(nameData, GetReferenceName)(tempRef);
						}
					}
				}
			}
		}
		return "none";
	}


	// log weapon stats:

	// gun
	void LogWeaponStats_Gun(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectREFR *tempRef = thisObj->GetObjectReference();
			UInt32 tempWeapID = 0x0;
			if (tempRef) {
				tempWeapID = tempRef->formID;
			}
			_MESSAGE("\n Weapon: 0x%08x - %s", tempWeapID, GetWeapName(thisObj));
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);

			if (instanceData) {
				// Ammo
				if (instanceData->ammo)
					_MESSAGE("    Ammo                : %s", instanceData->ammo->GetFullName());
				else
					_MESSAGE("    Ammo                : default");

				_MESSAGE("    Damage              : %i", instanceData->baseDamage);

				//		DamageTypes:
				const char *damageTypeName = "";
				if (instanceData->damageTypes) {
					if (instanceData->damageTypes->count > 0) {
						_MESSAGE("    DamageTypes:");
						TBO_InstanceData::DamageTypes tempValMod;
						BGSDamageType * damageType = nullptr;
						for (UInt32 i = 0; i < (instanceData->damageTypes->count); i++) {
							if (instanceData->damageTypes->GetNthItem(i, tempValMod)) {
								if (tempValMod.value > 0) {
									damageTypeName = ATShared::GetIdentifierFromFormID(tempValMod.damageType->formID);
									_MESSAGE("        %s %i", damageTypeName, tempValMod.value);
								}
							}
						}
					}
				}

				_MESSAGE("    Effective Range     : %.2f (%.2f m, %.2f`)",
					instanceData->maxRange, instanceData->maxRange * 0.01428, instanceData->maxRange / 64.0 * 3.0
				);
				_MESSAGE("    Out of Range Mult.  : %.2f", instanceData->outOfRangeMultiplier);

				_MESSAGE("    Crit Dmg Mult       : %.2f", instanceData->critDamageMult);
				_MESSAGE("    Crit Charge/Chance Mult: %.2f", instanceData->critChargeBonus);

				if (instanceData->firingData) {
					// projectile override
					if (instanceData->firingData->projectileOverride)
						_MESSAGE("    Projectile Override : %s", ATShared::GetIdentifierFromFormID(instanceData->firingData->projectileOverride->formID));
					else
						_MESSAGE("    Projectile Override : none");

					// projectiles per shot
					if (instanceData->firingData->numProjectiles > 0x200)
						_MESSAGE("    Num Projectiles     : %i (+0x200)", instanceData->firingData->numProjectiles - 0x200);
					else if (instanceData->firingData->numProjectiles > 0x100)
						_MESSAGE("    Num Projectiles     : %i (+0x100)", instanceData->firingData->numProjectiles - 0x100);
					else
						_MESSAGE("    Num Projectiles     : %i", instanceData->firingData->numProjectiles);
				}

				_MESSAGE("    Sound Level         : %i", instanceData->unk114);
				_MESSAGE("    Hit Effect          : %i", instanceData->unk118);

				if (instanceData->flags) {
					if ((instanceData->flags & 0x0008000) == 0x0008000)
						_MESSAGE("    Auto Flag           : true");
					else
						_MESSAGE("    Auto Flag           : false");
				}
				else {
					_MESSAGE("    Auto Flag           : false");
				}

				if (instanceData->keywords) {
					bool bHasAutoKW = false;
					BGSKeyword *autoKW = (BGSKeyword*)LookupFormByID(0x4A0A2);
					for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
						if (instanceData->keywords->keywords[i] == autoKW) {
							bHasAutoKW = true;
							i = instanceData->keywords->numKeywords;
						}
					}
					if (bHasAutoKW)
						_MESSAGE("    Auto KW             : true");
					else
						_MESSAGE("    Auto KW             : false");
				}
				else {
					_MESSAGE("    Auto KW             : false");
				}

				if (instanceData->unk58)
					_MESSAGE("    ImpactDataSet       : %s", ATShared::GetIdentifierFromFormID(((TESForm*)instanceData->unk58)->formID));
				else
					_MESSAGE("    ImpactDataSet	      : none");

				if (instanceData->aimModel) {
					ATAimModel *tempAT = (ATAimModel*)instanceData->aimModel;
					if (tempAT) {
						if (tempAT->formID == 0x0)
							_MESSAGE("    AimModel            : *edited*");
						else
							_MESSAGE("    AimModel            : %s", ATShared::GetIdentifierFromFormID(tempAT->formID));

						_MESSAGE("        Recoil per Shot : %.2f - %.2f", tempAT->Rec_MinPerShot, tempAT->Rec_MaxPerShot);
						_MESSAGE("        Recoil max. Arc : %.2f", tempAT->Rec_ArcMaxDegrees);
						_MESSAGE("        Cone of Fire    : %.2f - %.2f", tempAT->CoF_MinAngle, tempAT->CoF_MaxAngle);
					}
				}
				else
					_MESSAGE("    AimModel            : none");

				if (instanceData->zoomData) {
					if (((TESForm*)instanceData->zoomData)->formID > 0)
						_MESSAGE("    ZoomData            : %s", ATShared::GetIdentifierFromFormID(((TESForm*)instanceData->zoomData)->formID));
					else
						_MESSAGE("    ZoomData            : *edited*");
				}
				else
					_MESSAGE("    ZoomData            : none");

				// ActorValue modifiers
				if (instanceData->modifiers) {
					if (instanceData->modifiers->count > 0) {
						TBO_InstanceData::ValueModifier tempValMod;
						_MESSAGE("    ActorValue Modifiers -");
						for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
							if (instanceData->modifiers->GetNthItem(i, tempValMod))
								_MESSAGE("        %s -    %i", tempValMod.avInfo->avName, tempValMod.unk08);
						}
					}
				}

				if (instanceData->enchantments) {
					_MESSAGE("    Enchantments -");
					for (UInt32 i = 0; i < instanceData->enchantments->count; i++) {
						_MESSAGE("        %s -    %s", instanceData->enchantments->entries[i]->GetFullName(), ATShared::GetIdentifierFromFormID(instanceData->enchantments->entries[i]->formID));
					}
				}

				_MESSAGE("    Animation Times -");
				_MESSAGE("        attack:  %.4f s", instanceData->unkC0); //animation attack seconds
				if (instanceData->firingData) {
					_MESSAGE("        fire  :  %.4f s", instanceData->firingData->unk00); //animation fire seconds
					_MESSAGE("        reload:  %.4f s", instanceData->firingData->unk18); //animation reload seconds
				}
			}
			else
				_MESSAGE("no instanceData");
		}
	}

	// melee
	void LogWeaponStats_Melee(VMRefOrInventoryObj *thisObj)
	{

	}

	// thrown
	void LogWeaponStats_Thrown(VMRefOrInventoryObj *thisObj)
	{

	}

	
	//*******************************************************************************
	//						Papyrus functions:

	// Weapon Update

	void LogWeaponStats(VMRefOrInventoryObj *thisObj, UInt32 weaponType)
	{
		if (thisObj) {
			if (weaponType == 0) {
				// gun
				LogWeaponStats_Gun(thisObj);
			}
			else if (weaponType == 1) {
				// melee
				LogWeaponStats_Melee(thisObj);
			}
			else if (weaponType == 2) {
				// thrown
				LogWeaponStats_Thrown(thisObj);
			}
		}
	}


	// ******************* Ammo Management *******************
	
	// returns the equipped weapon's caliber keyword
	BGSKeyword* GetEquippedCaliber(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);

			if (instanceData) {
				if (instanceData->keywords) {
					for (UInt8 i = 0; i < ATShared::index_Calibers.count; i++) {
						UInt32 tempCaliberKW = 0x0;
						ATShared::index_Calibers.GetNthItem(i, tempCaliberKW);
						if (tempCaliberKW > 0) {
							for (UInt8 j = 0; j < instanceData->keywords->numKeywords; j++) {
								if (instanceData->keywords->keywords[j]) {
									if (instanceData->keywords->keywords[j]->formID == tempCaliberKW) {
										return (BGSKeyword*)LookupFormByID(tempCaliberKW);
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

	// returns a caliber's ammo subtype forms
	VMArray<TESAmmo*> GetCaliberAmmoTypes(StaticFunctionTag*, BGSKeyword *curCaliber)
	{
		VMArray<TESAmmo*> result;
		if (curCaliber) {
			int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
			if (caliberIndex >= 0) {
				ATCaliber tempCaliber;
				ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber);
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

	// returns the index of the equipped ammo type in the caliber's ammo list
	UInt32 GetEquippedAmmoIndex(StaticFunctionTag*, BGSKeyword *curCaliber, Actor *tempActor, UInt32 iSlot = 41)
	{
		VMArray<TESAmmo*> result;
		if (curCaliber && tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);

			if (instanceData) {
				if (instanceData->ammo) {
					int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
					if (caliberIndex >= 0) {
						ATCaliber tempCaliber;
						ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber);
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


	// returns the equipped ammo form - used for ammo swapping checks and jamming
	TESAmmo* GetEquippedAmmo(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
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
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->ammo != newAmmo) {
					instanceData->ammo = newAmmo;
					return true;
				}
			}
		}
		return false;
	}

	// returns an array of a caliber's ammo type mods - used for HUD text
	VMArray<BGSMod::Attachment::Mod*> GetCaliberAmmoMods(StaticFunctionTag*, BGSKeyword *curCaliber)
	{
		VMArray<BGSMod::Attachment::Mod*> result;
		if (curCaliber) {
			int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
			if (caliberIndex >= 0) {
				ATCaliber tempCaliber;
				ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber);
				if (tempCaliber.ammoTypes.count > 0) {
					for (UInt8 i = 0; i < tempCaliber.ammoTypes.count; i++) {
						result.Push(&tempCaliber.ammoTypes[i].swapMod);
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
			int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
			if (caliberIndex >= 0 && (int)iAmmoIndex >= 0) {
				ATCaliber tempCaliber;
				if (ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber)) {
					ATCaliber::ATAmmoType tempAmmoType;
					if (tempCaliber.ammoTypes.GetNthItem(iAmmoIndex, tempAmmoType)) {
						return tempAmmoType.swapMod;
					}
				}
			}
		}
		return nullptr;
	}

	// returns a caliber's casing item
	TESObjectMISC *GetCurrentCasing(StaticFunctionTag*, BGSKeyword *curCaliber, UInt32 iAmmoIndex)
	{
		if (curCaliber) {
			int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
			if (caliberIndex >= 0 && (int)iAmmoIndex >= 0) {
				ATCaliber tempCaliber;
				if (ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber)) {
					ATCaliber::ATAmmoType tempAmmoType;
					if (tempCaliber.ammoTypes.GetNthItem(iAmmoIndex, tempAmmoType)) {
						return tempAmmoType.casingItem;
					}
				}
			}
		}
		return nullptr;
	}

	
	// **************** Swappable Mods

	// returns the list of ObjectMods for a slot index
	VMArray<BGSMod::Attachment::Mod*> GetModsForSlot(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot)
	{
		VMArray<BGSMod::Attachment::Mod*> result;
		if (curWeapon) {
			UInt32 weapID = curWeapon->formID;
			int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
			if (weapIndex >= 0) {
				ATWeapon tempWeap;
				if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
					if (iModSlot < tempWeap.modSlots.count) {
						ATWeapon::ATModSlot tempSlot;
						if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
							for (UInt8 i = 0; i < tempSlot.swappableMods.count; i++) {
								ATSwappableMod tempSwapMod;
								if (tempSlot.swappableMods.GetNthItem(i, tempSwapMod)) {
									if (tempSwapMod.swapMod) {
										result.Push(&tempSwapMod.swapMod);
									}
								}
							}
						}
					}
				}
			}
		}
		return result;
	}

	// returns the muzzle attachment at the given index for a weapon
	BGSMod::Attachment::Mod *GetModAtIndex(StaticFunctionTag*, TESObjectWEAP *curWeapon, UInt32 iModSlot, UInt32 iModIndex)
	{
		if (curWeapon) {
			UInt32 weapID = curWeapon->formID;
			int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
			if (weapIndex >= 0) {
				ATWeapon tempWeap;
				if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
					if (iModSlot < tempWeap.modSlots.count) {
						ATWeapon::ATModSlot tempSlot;
						if (tempWeap.modSlots.GetNthItem(1, tempSlot)) {
							ATSwappableMod tempMod;
							if (tempSlot.swappableMods.GetNthItem(iModIndex, tempMod))
								return tempMod.swapMod;
						}
					}
				}
			}
		}
		return nullptr;
	}


	UInt32 GetEquippedModIndex(StaticFunctionTag*, Actor *tempActor, UInt32 iEquipSlot = 41, UInt32 iModSlot = 0)
	{
		if (tempActor) {
			UInt32 weapID = GetEquippedItemFormID(tempActor, iEquipSlot);
			if (weapID > 0) {
				TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iEquipSlot);

				if (instanceData) {
					if (instanceData->keywords) {
						int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
						if (weapIndex >= 0) {
							ATWeapon tempWeap;
							if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
								if (iModSlot < tempWeap.modSlots.count) {
									ATWeapon::ATModSlot tempSlot;
									if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
										for (UInt32 i = 0; i < tempSlot.swappableMods.count; i++) {
											ATSwappableMod tempSwapMod;
											if (tempSlot.swappableMods.GetNthItem(i, tempSwapMod)) {
												if (tempSwapMod.excludeKW) {
													for (UInt8 j = 0; j < instanceData->keywords->numKeywords; j++) {
														if (instanceData->keywords->keywords[j] == tempSwapMod.excludeKW) {
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
					}
				}
			}
		}
		return (UInt32)-1;
	}


	BGSKeyword *GetSwapModRequiredKeyword(StaticFunctionTag*, Actor *tempActor, UInt32 iEquipSlot = 41, UInt32 iModSlot = 0, UInt32 iModIndex = 0)
	{
		if (tempActor) {
			UInt32 weapID = GetEquippedItemFormID(tempActor, iEquipSlot);
			if (weapID > 0) {
				int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
				if (weapIndex >= 0) {
					ATWeapon tempWeap;
					if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
						ATWeapon::ATModSlot tempSlot;
						if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
							ATSwappableMod tempSwapMod;
							if (tempSlot.swappableMods.GetNthItem(iModIndex, tempSwapMod)) {
								return tempSwapMod.requiredKW;
							}
						}
					}
				}
			}
		}
		return nullptr;
	}

	BGSKeyword *GetSwapModExcludeKeyword(StaticFunctionTag*, Actor *tempActor, UInt32 iEquipSlot = 41, UInt32 iModSlot = 0, UInt32 iModIndex = 0)
	{
		if (tempActor) {
			UInt32 weapID = GetEquippedItemFormID(tempActor, iEquipSlot);
			if (weapID > 0) {
				int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
				if (weapIndex >= 0) {
					ATWeapon tempWeap;
					if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
						ATWeapon::ATModSlot tempSlot;
						if (tempWeap.modSlots.GetNthItem(iModSlot, tempSlot)) {
							ATSwappableMod tempSwapMod;
							if (tempSlot.swappableMods.GetNthItem(iModIndex, tempSwapMod)) {
								return tempSwapMod.excludeKW;
							}
						}
					}
				}
			}
		}
		return nullptr;
	}



	// ******************* Misc *******************

	// updates the equipped weapon's projectile based on caliber, ammo type and found attachment keywords
	bool UpdateEquippedProjectile(StaticFunctionTag*, BGSKeyword *curCaliber, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					int caliberIndex = ATShared::index_Calibers.GetItemIndex(curCaliber->formID);
					if (caliberIndex >= 0) {
						ATCaliber tempCaliber;
						ATShared::ATCalibers.GetNthItem(caliberIndex, tempCaliber);
						bool bFoundAmmo = false;
						ATCaliber::ATAmmoType tempAmmoType;
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
								ATCaliber::ATProjectileOverride tempProjOvr;
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

	// used to set the 'misfire' projectile preceding a jam (and random misfires)
	bool SetEquippedProjectile(StaticFunctionTag*, BGSProjectile *newProjectile, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
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

	// used to set the 'misfire' projectile preceding a jam (and random misfires)
	BGSProjectile* GetEquippedProjectile(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				if (instanceData->firingData) {
					return instanceData->firingData->projectileOverride;
				}
			}
		}
		return nullptr;
	}


	TESAmmo* GetRefAmmo(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->ammo;
			}
		}
		return nullptr;
	}


	float GetRefMaxRange(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->maxRange;
			}
		}
		return 0.0;
	}

	// used to calculate DT for damage threshold framework
	UInt32 GetRefAttackDamage(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (instanceData->baseDamage >= 0)
					return (UInt32)instanceData->baseDamage;
				else {

				}
			}
		}
		return 0;
	}
	UInt32 GetEquippedAttackDamage(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
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
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->critChargeBonus;
			}
		}
		return 0.0;
	}

	// used to calculate a weapon's skill requirement
	float GetRefWeight(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}
	float GetEquippedWeight(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}


	// returns the required skill/attribute AV
	ActorValueInfo* GetRefReqSkill(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->skill;
			}
		}
		return nullptr;
	}
	ActorValueInfo* GetEquippedReqSkill(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
			if (instanceData) {
				return instanceData->skill;
			}
		}
		return nullptr;
	}

	// returns the droppable misc object for the equipped magazine mod
	TESObjectMISC *GetEquippedMagItem(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41)
	{
		if (tempActor) {
			UInt32 weapID = GetEquippedItemFormID(tempActor, iSlot);
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);

			if ((weapID > 0) && instanceData) {
				int weapIndex = ATShared::index_Weapons.GetItemIndex(weapID);
				if (weapIndex > -1) {
					ATWeapon tempWeap;
					if (ATShared::ATWeapons.GetNthItem(weapIndex, tempWeap)) {
						if (instanceData->keywords) {
							for (UInt8 j = 0; j < tempWeap.magazines.count; j++) {
								ATWeapon::ATMagItem tempMag;
								if (tempWeap.magazines.GetNthItem(j, tempMag)) {
									for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
										if (instanceData->keywords->keywords[i] == tempMag.magKW) {
											return tempMag.magItem;
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


	/** updates recoil based on the passed multiplier
		return values: -1=already edited + no update needed, 0=already edited + update needed, 1=edit successful
	**/
	UInt32 UpdateEquippedRecoil(StaticFunctionTag*, Actor *tempActor, UInt32 iSlot = 41, float fRecoilMult = 1.0)
	{
		if (tempActor) {
			TESObjectWEAP::InstanceData *instanceData = GetEquippedInstanceData(tempActor, iSlot);
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
											return -1;
									}
								}
							}

							if (fRecoilMult != 1.0) {
								// edit recoil
								tempAimModel->Rec_HipMult *= fRecoilMult;
								tempAimModel->Rec_MinPerShot *= fRecoilMult;
								tempAimModel->Rec_MaxPerShot = max(tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot * fRecoilMult);

								// add the check AV
								TBO_InstanceData::ValueModifier skillAVMod;
								skillAVMod.avInfo = skillModAV;
								if (fRecoilMult != 0.0)
									skillAVMod.unk08 = (int)(fRecoilMult * 100.0);
								instanceData->modifiers->Push(skillAVMod);

								_MESSAGE("\nRecoil updated...\n    Min: %.4f, Max: %.4f, HipMult: %.4f", tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot, tempAimModel->Rec_HipMult);
								return 1;
							}
						}
					}
				}
			}
			return 0;
		}
		return 0;
	}

}


bool ATWeaponRef::RegisterPapyrus(VirtualMachine* vm)
{
	ATWeaponRef::RegisterFuncs(vm);
	_MESSAGE("Registered ATWeaponRef native functions.\n");
	return true;
}


void ATWeaponRef::RegisterFuncs(VirtualMachine* vm)
{
	// debug

	vm->RegisterFunction(
		new NativeFunction1 <VMRefOrInventoryObj, void, UInt32>("LogWeaponStats", SCRIPTNAME_R, ATWeaponRef::LogWeaponStats, vm));
	
	// Ammo Management

	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSKeyword*, Actor*, UInt32>("GetEquippedCaliber", SCRIPTNAME_E, ATWeaponRef::GetEquippedCaliber, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<TESAmmo*>, BGSKeyword*>("GetCaliberAmmoTypes", SCRIPTNAME_E, ATWeaponRef::GetCaliberAmmoTypes, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESAmmo*, Actor*, UInt32>("GetEquippedAmmo", SCRIPTNAME_E, ATWeaponRef::GetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, TESAmmo*, Actor*, UInt32>("SetEquippedAmmo", SCRIPTNAME_E, ATWeaponRef::SetEquippedAmmo, vm));

	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, BGSKeyword*, Actor*, UInt32>("GetEquippedAmmoIndex", SCRIPTNAME_E, ATWeaponRef::GetEquippedAmmoIndex, vm));
	
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, VMArray<BGSMod::Attachment::Mod*>, BGSKeyword*>("GetCaliberAmmoMods", SCRIPTNAME_E, ATWeaponRef::GetCaliberAmmoMods, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSMod::Attachment::Mod*, BGSKeyword*, UInt32>("GetAmmoModAtIndex", SCRIPTNAME_E, ATWeaponRef::GetAmmoModAtIndex, vm));

	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectMISC*, BGSKeyword*, UInt32>("GetCurrentCasing", SCRIPTNAME_E, ATWeaponRef::GetCurrentCasing, vm));

	// Swappable Mods
	
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, VMArray<BGSMod::Attachment::Mod*>, TESObjectWEAP*, UInt32>("GetModsForSlot", SCRIPTNAME_E, ATWeaponRef::GetModsForSlot, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, BGSMod::Attachment::Mod*, TESObjectWEAP*, UInt32, UInt32>("GetModAtIndex", SCRIPTNAME_E, ATWeaponRef::GetModAtIndex, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, Actor*, UInt32, UInt32>("GetEquippedModIndex", SCRIPTNAME_E, ATWeaponRef::GetEquippedModIndex, vm));
	
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, BGSKeyword*, Actor*, UInt32, UInt32, UInt32>("GetSwapModExcludeKeyword", SCRIPTNAME_E, ATWeaponRef::GetSwapModExcludeKeyword, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, BGSKeyword*, Actor*, UInt32, UInt32, UInt32>("GetSwapModRequiredKeyword", SCRIPTNAME_E, ATWeaponRef::GetSwapModRequiredKeyword, vm));

	// Weapon Reference

	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, TESAmmo*>("GetRefAmmo", SCRIPTNAME_R, ATWeaponRef::GetRefAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, float>("GetRefMaxRange", SCRIPTNAME_R, ATWeaponRef::GetRefMaxRange, vm));

	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, UInt32>("GetRefAttackDamage", SCRIPTNAME_R, ATWeaponRef::GetRefAttackDamage, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, float>("GetRefWeight", SCRIPTNAME_R, ATWeaponRef::GetRefWeight, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, ActorValueInfo*>("GetRefReqSkill", SCRIPTNAME_R, ATWeaponRef::GetRefReqSkill, vm));
	
	// Misc:

	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, BGSKeyword*, Actor*, UInt32>("UpdateEquippedProjectile", SCRIPTNAME_E, ATWeaponRef::UpdateEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, BGSProjectile*, Actor*, UInt32>("SetEquippedProjectile", SCRIPTNAME_E, ATWeaponRef::SetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSProjectile*, Actor*, UInt32>("GetEquippedProjectile", SCRIPTNAME_E, ATWeaponRef::GetEquippedProjectile, vm));
		
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("GetEquippedAttackDamage", SCRIPTNAME_E, ATWeaponRef::GetEquippedAttackDamage, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, float, Actor*, UInt32>("GetEquippedCritChargeMult", SCRIPTNAME_E, ATWeaponRef::GetEquippedCritChargeMult, vm));
	
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, float, Actor*, UInt32>("GetEquippedWeight", SCRIPTNAME_E, ATWeaponRef::GetEquippedWeight, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, ActorValueInfo*, Actor*, UInt32>("GetEquippedReqSkill", SCRIPTNAME_E, ATWeaponRef::GetEquippedReqSkill, vm));
	
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, TESObjectMISC*, Actor*, UInt32>("GetEquippedMagItem", SCRIPTNAME_E, ATWeaponRef::GetEquippedMagItem, vm));
	
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, UInt32, Actor*, UInt32, float>("UpdateEquippedRecoil", SCRIPTNAME_E, ATWeaponRef::UpdateEquippedRecoil, vm));
	
}

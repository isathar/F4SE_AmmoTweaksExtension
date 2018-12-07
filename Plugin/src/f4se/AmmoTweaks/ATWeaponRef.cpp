#include "f4se/GameExtraData.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include <time.h>

#include "ATWeaponRef.h"


#define SCRIPTNAME "AmmoTweaks:Weapon:WeaponRefBase"



UInt32 ATCaliber::ProcessInstanceData(TESObjectWEAP::InstanceData *instanceData, int ammoTypeIndex)
{
	if (instanceData) {
		// add a random ammo type if the weapon was just created
		if (ammoTypeIndex < 0) {
			// time-based RNG seed
			srand(time(NULL) * ATShared::SharedData->numProcessedWeaps);
			ammoTypeIndex = std::rand() % (ammoTypes.count);
		}
		

		ATAmmoType newAmmoType;
		if (ammoTypes.GetNthItem(ammoTypeIndex, newAmmoType)) {
			bool hasSuppressor = false;
			bool hasMuzBrake = false;
			bool hasLegExpl = false;
			bool hasLegTwoShot = false;

			// ammo
			instanceData->ammo = newAmmoType.ammoForm;
			// damage
			instanceData->baseDamage = (UInt16)max(0x0, min(((int)((float)(int)instanceData->baseDamage * newAmmoType.fDamageMult)), 0xFF));
			// range
			instanceData->maxRange = max(instanceData->minRange, (instanceData->maxRange) * newAmmoType.fRangeMult);
			instanceData->outOfRangeMultiplier = instanceData->outOfRangeMultiplier * newAmmoType.fOoRMult;

			// check for special case mods
			if (instanceData->keywords) {
				for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
					if (instanceData->keywords->keywords[i] == ATShared::SharedData->suppressorKW)
						hasSuppressor = true;
					else if (instanceData->keywords->keywords[i] == ATShared::SharedData->muzBrakeKW)
						hasMuzBrake = true;
					else if (instanceData->keywords->keywords[i] == ATShared::SharedData->compensatorKW)
						hasMuzBrake = true;
					else if (instanceData->keywords->keywords[i] == ATShared::SharedData->legendaryExplKW)
						hasLegExpl = true;
					else if (instanceData->keywords->keywords[i] == ATShared::SharedData->legendaryTwoShotKW)
						hasLegTwoShot = true;
				}
			}

			// sound level
			if (hasLegExpl) {
				instanceData->unk114 = newAmmoType.iSoundLevelExp;
			}
			else if (hasSuppressor) {
				instanceData->unk114 = newAmmoType.iSoundLevelSup;
				if (newAmmoType.iSoundLevelSup != 2) {
					if (instanceData->keywords) {
						for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
							if (instanceData->keywords->keywords[i] == ATShared::SharedData->soundKW_Silenced) {
								instanceData->keywords->keywords[i] = ATShared::SharedData->soundKW_SilencedDisable;
								i = instanceData->keywords->numKeywords;
							}
						}
					}
				}
			}
			else {
				instanceData->unk114 = newAmmoType.iSoundLevel;
			}


			// hiteffect
			if (hasLegExpl) {
				instanceData->unk118 = (UInt32)3;
			}
			else {
				instanceData->unk118 = newAmmoType.iHitEffect;
			}

			// critical dmg/chance mults
			if (hasSuppressor) {
				instanceData->critDamageMult = instanceData->critDamageMult * (newAmmoType.fCritDmgMult + 0.25);
				instanceData->critChargeBonus = instanceData->critChargeBonus * (newAmmoType.fCritChanceMult + 0.25);
			}
			else {
				instanceData->critDamageMult = instanceData->critDamageMult * newAmmoType.fCritDmgMult;
				instanceData->critChargeBonus = instanceData->critChargeBonus * newAmmoType.fCritChanceMult;
			}

			// impactDataSet
			if (hasLegExpl && newAmmoType.projImpactDataExp) {
				instanceData->unk58 = newAmmoType.projImpactDataExp;
			}
			else if (newAmmoType.projImpactData) {
				instanceData->unk58 = newAmmoType.projImpactData;
			}

			if (instanceData->firingData) {
				// set projectile
				if (hasLegExpl) {
					//_MESSAGE("    Explosive");
					instanceData->firingData->projectileOverride = projectiles[newAmmoType.projectileExp];
				}
				else if (hasSuppressor) {
					//_MESSAGE("    Suppressor");
					instanceData->firingData->projectileOverride = projectiles[newAmmoType.projectileSup];
				}
				else if (hasMuzBrake) {
					//_MESSAGE("    MuzBrake");
					instanceData->firingData->projectileOverride = projectiles[newAmmoType.projectileBrk];
				}
				else {
					//_MESSAGE("    Standard");
					instanceData->firingData->projectileOverride = projectiles[newAmmoType.projectileStd];
				}
				
				// projectile count:
				UInt32 iProjOffset = 0x0;
				UInt32 iProjectileCount = instanceData->firingData->numProjectiles;

				// handle the extra data in numProjectiles
				if (iProjectileCount > 0x100)
					iProjOffset = (UInt32)0x100;
				else if (iProjectileCount > 0x200)
					iProjOffset = (UInt32)0x200;
				
				UInt32 iProjMult = 1;
				if (ATShared::SharedData->bTwoShotLegendaryTweak) {
					// multiply count by 2 if the weapon has the two-shot legendary effect
					if (hasLegTwoShot)
						iProjMult = 2;
				}
				instanceData->firingData->numProjectiles = iProjOffset + (UInt32)max(0x01, min(newAmmoType.iNumProjectiles * iProjMult, 0xFF));
			}

			// aim model:
			if (instanceData->aimModel) {
				ATAimModel *aimModel = (ATAimModel*)instanceData->aimModel;
				// recoil
				aimModel->Rec_MinPerShot = aimModel->Rec_MinPerShot * newAmmoType.fRecoilMult;
				aimModel->Rec_MaxPerShot = max(aimModel->Rec_MinPerShot, aimModel->Rec_MaxPerShot * newAmmoType.fRecoilMult);
				aimModel->Rec_ArcMaxDegrees = aimModel->Rec_ArcMaxDegrees * newAmmoType.fRecoilMult;
				// cone of fire
				aimModel->CoF_MinAngle = aimModel->CoF_MinAngle * newAmmoType.fCoFMult;
				aimModel->CoF_MaxAngle = max(aimModel->CoF_MinAngle, aimModel->CoF_MaxAngle * newAmmoType.fCoFMult);

				// toggle recoil spring force
				if (ATShared::SharedData->bDisableRecoilSpringForce) {
					aimModel->Rec_DimSpringForce = 0.0;
				}
			}

			// increase weapon weight by ammo weight * ammoCapacity
			if (newAmmoType.ammoForm) {
				float fWeightAdd = newAmmoType.ammoForm->weight.weight;
				if (fWeightAdd > 0.0) {
					instanceData->weight = instanceData->weight + ((float)(int)instanceData->ammoCapacity * fWeightAdd);
				}
			}
			
			// enchantments - ammo type
			if (newAmmoType.enchantments.count > 0) {
				if (!instanceData->enchantments) {
					instanceData->enchantments = new tArray<EnchantmentItem*>();
				}
				for (UInt8 i = 0; i < newAmmoType.enchantments.count; i++) {
					if (instanceData->enchantments->GetItemIndex(newAmmoType.enchantments[i]) < 0) {
						instanceData->enchantments->Push(newAmmoType.enchantments[i]);
					}
				}
			}

			// auto fire sound tweak
			if (ATShared::SharedData->bUseSingleFireAutoSounds) {
				if (instanceData->keywords) {
					for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
						if (instanceData->keywords->keywords[i] == ATShared::SharedData->soundKW_Automatic) {
							instanceData->keywords->keywords[i] = ATShared::SharedData->soundKW_AutomaticDisable;
							i = instanceData->keywords->numKeywords;
						}
					}
				}
			}


		}
	}
	return ammoTypeIndex;
}


// ATWeapon functions:



float ATWeapon::GetMaxCNDMult(TESObjectWEAP::InstanceData *instanceData)
{
	float retVal = 1.0;
	if (instanceData) {
		if (instanceData->keywords) {
			for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
				for (UInt8 j = 0; j < MaxCNDMods.count; j++) {
					ATWeapVarMod tempMod = MaxCNDMods[j];
					if (tempMod.modKW == instanceData->keywords->keywords[i]) {
						retVal += tempMod.modMultAdd;
					}
				}
			}
		}
	}
	return retVal;
}

float ATWeapon::GetWearMult(TESObjectWEAP::InstanceData *instanceData)
{
	float retVal = 1.0;
	if (instanceData) {
		if (instanceData->keywords) {
			for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
				for (UInt8 j = 0; j < WearMods.count; j++) {
					ATWeapVarMod tempMod = WearMods[j];
					if (tempMod.modKW == instanceData->keywords->keywords[i]) {
						retVal += tempMod.modMultAdd;
					}
				}
			}
		}
	}
	return retVal;
}


void ATWeapon::ProcessInstanceData(TESObjectWEAP::InstanceData *instanceData)
{
	if (instanceData) {
		bool bFoundMaxCND = false;
		bool bFoundSkillReq = false;
		bool bFoundInstanceCheck = false;
		bool bFoundHUDIcon = false;

		float maxCNDMult = GetMaxCNDMult(instanceData);
		UInt32 curMaxCND = (UInt32)(int)((float)(int)MaxCNDBase * maxCNDMult);

		UInt32 iSkillReq = 0;
		if (ATShared::SharedData->bEnableSkillRequirements) {
			iSkillReq = (UInt32)(int)max(ATShared::SharedData->fSkillReq_MinReq, min(instanceData->weight / ATShared::SharedData->fSkillReq_MaxWeight * ATShared::SharedData->fSkillReq_MaxReq, ATShared::SharedData->fSkillReq_MaxReq));
		}
		
		if (!instanceData->modifiers) {
			instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>;
		}
		
		if (instanceData->modifiers->count > 0) {
			for (int i = instanceData->modifiers->count - 1; i >= 0; i--) {
				TBO_InstanceData::ValueModifier tempAVypeMod;
				instanceData->modifiers->GetNthItem(i, tempAVypeMod);
				if (tempAVypeMod.avInfo) {
					if (tempAVypeMod.avInfo == ATShared::SharedData->MaxConditionAV) {
						bFoundMaxCND = true;
						if (curMaxCND > 0) {
							tempAVypeMod.unk08 = curMaxCND;
						}
						else {
							instanceData->modifiers->Remove(i);
						}
					}
					else if (tempAVypeMod.avInfo == ATShared::SharedData->SkillReqAV) {
						bFoundSkillReq = true;
						if (iSkillReq > 0) {
							tempAVypeMod.unk08 = iSkillReq;
						}
						else {
							instanceData->modifiers->Remove(i);
						}
					}
					else if (tempAVypeMod.avInfo == ATShared::SharedData->HUDIconAV_Weapon) {
						bFoundHUDIcon = true;
						if (HUDIconIndex > 0) {
							tempAVypeMod.unk08 = HUDIconIndex;
						}
						else {
							instanceData->modifiers->Remove(i);
						}
					}
					else if (tempAVypeMod.avInfo == ATShared::SharedData->HasInstanceAV) {
						bFoundInstanceCheck = true;
					}
				}
			}
		}

		
		TBO_InstanceData::ValueModifier addAVMod;
		if (!bFoundMaxCND) {
			if (curMaxCND > 0) {
				addAVMod.avInfo = ATShared::SharedData->MaxConditionAV;
				addAVMod.unk08 = curMaxCND;
				instanceData->modifiers->Push(addAVMod);
			}
		}
		
		if (!bFoundSkillReq) {
			if (iSkillReq > 0) {
				TBO_InstanceData::ValueModifier addAVypeMod2;
				addAVypeMod2.avInfo = ATShared::SharedData->SkillReqAV;
				addAVypeMod2.unk08 = iSkillReq;
				instanceData->modifiers->Push(addAVypeMod2);
			}
		}

		if (!bFoundHUDIcon) {
			if (HUDIconIndex > 0) {
				TBO_InstanceData::ValueModifier addAVypeMod3;
				addAVypeMod3.avInfo = ATShared::SharedData->HUDIconAV_Weapon;
				addAVypeMod3.unk08 = HUDIconIndex;
				instanceData->modifiers->Push(addAVypeMod3);
			}
		}

		if (!bFoundInstanceCheck) {
			TBO_InstanceData::ValueModifier addAVypeMod4;
			addAVypeMod4.avInfo = ATShared::SharedData->HasInstanceAV;
			addAVypeMod4.unk08 = 1;
			instanceData->modifiers->Push(addAVypeMod4);
		}
		
	}
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



	UInt32 UpdateCaliber(TESObjectWEAP::InstanceData *instanceData)
	{
		if (instanceData->keywords) {
			int caliberIndex = 0;
			for (UInt8 j = 0; j < ATShared::SharedData->index_ATCalibersEquipped.count; j++) {
				BGSKeyword *tempCalKW = ATShared::SharedData->index_ATCalibersEquipped[j];
				if (tempCalKW) {
					for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
						if (instanceData->keywords->keywords[i] == tempCalKW) {
							return j;
						}
					}
				}
			}
		}
		return -1;
	}


	// log weapon stats:

	// gun
	void LogWeaponStats_Gun(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			_MESSAGE("\n Weapon: %s", GetWeapName(thisObj));
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

	// main instance update function - called whenever the instance needs to be reset
	bool UpdateWeaponStats(VMRefOrInventoryObj *thisObj, BGSKeyword *curCaliber, UInt32 curAmmoType)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				TESObjectREFR *tempRef = thisObj->GetObjectReference();
				if (tempRef) {
					int iAmmoType = (int)curAmmoType;
					UInt32 weapID = 0;

					if (tempRef->baseForm) {
						weapID = tempRef->baseForm->formID;
					}

					if (curCaliber) {
						int iCaliber = ATShared::SharedData->index_ATCalibersEquipped.GetItemIndex(curCaliber);
						ATCaliber *newCaliber = nullptr;
						ATShared::SharedData->g_ATCalibers.GetNthItem(iCaliber, newCaliber);
						if (newCaliber) {
							iAmmoType = newCaliber->ProcessInstanceData(instanceData, curAmmoType);
						}
					}

					if (weapID > 0) {
						int weapIndex = ATShared::SharedData->index_ATWeapons.GetItemIndex(weapID);
						ATWeapon *tempWeap = ATShared::SharedData->g_ATWeapons[weapIndex];
						if (tempWeap) {
							tempWeap->ProcessInstanceData(instanceData);
						}
					}

					return (iAmmoType != curAmmoType);
				}
			}
		}
		return false;
	}

	
	// returns the current caliber's identifier keyword (dn_at_AmmoConversion_x)
	BGSKeyword* GetCurCaliber(VMRefOrInventoryObj *thisObj)
	{
		BGSKeyword *retKW = nullptr;
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				int iCaliber = UpdateCaliber(instanceData);
				if (iCaliber > -1) {
					ATShared::SharedData->index_ATCalibersEquipped.GetNthItem(iCaliber, retKW);
				}
			}
		}
		return retKW;
	}

	// returns the current caliber's objectmod
	TESForm* GetCurCaliberMod(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				int iCaliber = UpdateCaliber(instanceData);
				if (iCaliber > -1) {
					ATCaliber *newCaliber = ATShared::SharedData->g_ATCalibers[iCaliber];
					if (newCaliber) {
						return newCaliber->caliberMod;
					}
				}
			}
		}
		return nullptr;
	}

	// returns the current ammo type index (or -1 if none)
	UInt32 GetCurAmmoType(VMRefOrInventoryObj *thisObj, BGSKeyword *curCaliber)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (curCaliber) {
					int iCaliber = ATShared::SharedData->index_ATCalibersEquipped.GetItemIndex(curCaliber);
					if (iCaliber > -1) {
						ATCaliber *newCaliber = ATShared::SharedData->g_ATCalibers[iCaliber];
						TESAmmo *curAmmo = instanceData->ammo;
						if (curAmmo && newCaliber) {
							for (UInt8 i = 0; i < newCaliber->ammoTypes.count; i++) {
								if (newCaliber->ammoTypes[i].ammoForm) {
									if (curAmmo == newCaliber->ammoTypes[i].ammoForm) {
										return i;
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



	float CalcWearPerShot(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				UInt32 weapID = 0;
				TESObjectREFR *tempRef = thisObj->GetObjectReference();
				if (tempRef) {
					if (tempRef->baseForm) {
						weapID = tempRef->baseForm->formID;
					}

					if (weapID > 0) {
						int weapIndex = ATShared::SharedData->index_ATWeapons.GetItemIndex(weapID);
						ATWeapon *tempWeap = ATShared::SharedData->g_ATWeapons[weapIndex];
						if (tempWeap) {
							if (!instanceData->modifiers) {
								instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
							}

							return tempWeap->GetWearMult(instanceData);

							/*
							if (instanceData->modifiers->count > 0) {
								for (UInt8 i = 0; i < instanceData->modifiers->count; i++) {
									TBO_InstanceData::ValueModifier tempAVypeMod;
									instanceData->modifiers->GetNthItem(i, tempAVypeMod);
									if (tempAVypeMod.avInfo) {
										if (tempAVypeMod.avInfo->formID == ATShared::SharedData->MaxConditionAV->formID) {
											instanceData->modifiers->Remove(i);
											i = instanceData->modifiers->count;
										}
									}
								}
							}

							TBO_InstanceData::ValueModifier addAVypeMod;
							addAVypeMod.avInfo = ATShared::SharedData->MaxConditionAV;
							addAVypeMod.unk08 = curMaxCND;
							instanceData->modifiers->Push(addAVypeMod);
							*/
						}
					}
				}

			}
		}
		return 1.0;
	}

	UInt32 GetMaxCND(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (instanceData->modifiers) {
					for (UInt8 i = 0; i < instanceData->modifiers->count; i++) {
						TBO_InstanceData::ValueModifier tempAVypeMod;
						instanceData->modifiers->GetNthItem(i, tempAVypeMod);
						if (tempAVypeMod.avInfo) {
							if (tempAVypeMod.avInfo == ATShared::SharedData->MaxConditionAV) {
								return tempAVypeMod.unk08;
							}
						}
					}
				}
			}
		}
		return 0;
	}

	bool HasInstanceMods(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (instanceData->modifiers) {
					for (UInt8 i = 0; i < instanceData->modifiers->count; i++) {
						TBO_InstanceData::ValueModifier tempAVypeMod;
						instanceData->modifiers->GetNthItem(i, tempAVypeMod);
						if (tempAVypeMod.avInfo) {
							if (tempAVypeMod.avInfo == ATShared::SharedData->HasInstanceAV) {
								return true;
							}
						}
					}
				}
			}
		}
		return false;
	}


	VMArray<TESAmmo*> GetAmmoList(VMRefOrInventoryObj *thisObj)
	{
		VMArray<TESAmmo*> result;
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				UInt32 iCaliber = UpdateCaliber(instanceData);
				if (iCaliber > -1) {
					ATCaliber *newCaliber = ATShared::SharedData->g_ATCalibers[iCaliber];
					if (newCaliber) {
						for (UInt8 i = 0; i < newCaliber->ammoTypes.count; i++) {
							result.Push(&newCaliber->ammoTypes[i].ammoForm);
						}
					}
				}
				else {
					result.Push(&instanceData->ammo);
				}
			}
		}
		return result;
	}

	VMArray<BGSKeyword*> GetWpnKeywords(VMRefOrInventoryObj *thisObj)
	{
		VMArray<BGSKeyword*> result;
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (instanceData->keywords) {
					for (UInt8 i = 0; i < instanceData->keywords->numKeywords; i++) {
						if (instanceData->keywords->keywords[i]) {
							result.Push(&instanceData->keywords->keywords[i]);
						}
					}
				}
			}
		}
		return result;
	}
	

	float GetWpnWeight(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}





}


bool ATWeaponRef::RegisterPapyrus(VirtualMachine* vm)
{
	ATWeaponRef::RegisterFuncs(vm);
	_MESSAGE("Registered ATWeaponRef native functions.");
	return true;
}


void ATWeaponRef::RegisterFuncs(VirtualMachine* vm)
{
	// Weapon update
	vm->RegisterFunction(
		new NativeFunction1 <VMRefOrInventoryObj, void, UInt32>("LogWeaponStats", SCRIPTNAME, ATWeaponRef::LogWeaponStats, vm));
	
	vm->RegisterFunction(
		new NativeFunction2 <VMRefOrInventoryObj, bool, BGSKeyword*, UInt32>("UpdateWeaponStats", SCRIPTNAME, ATWeaponRef::UpdateWeaponStats, vm));
	
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, BGSKeyword*>("GetCurCaliber", SCRIPTNAME, ATWeaponRef::GetCurCaliber, vm));
	vm->RegisterFunction(
		new NativeFunction1 <VMRefOrInventoryObj, UInt32, BGSKeyword*>("GetCurAmmoType", SCRIPTNAME, ATWeaponRef::GetCurAmmoType, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, UInt32>("GetMaxCND", SCRIPTNAME, ATWeaponRef::GetMaxCND, vm));
	
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, bool>("HasInstanceMods", SCRIPTNAME, ATWeaponRef::HasInstanceMods, vm));
	
	

	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, TESForm*>("GetCurCaliberMod", SCRIPTNAME, ATWeaponRef::GetCurCaliberMod, vm));

	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, VMArray<TESAmmo*>>("GetAmmoList", SCRIPTNAME, ATWeaponRef::GetAmmoList, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, VMArray<BGSKeyword*>>("GetWpnKeywords", SCRIPTNAME, ATWeaponRef::GetWpnKeywords, vm));

	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, float>("GetWpnWeight", SCRIPTNAME, ATWeaponRef::GetWpnWeight, vm));

	
	
}

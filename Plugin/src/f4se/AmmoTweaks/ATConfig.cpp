#include "ATConfig.h"


bool ATCaliber::LoadFromIni(const char* caliberFileName)
{
	CSimpleIniA iniAmmo;
	iniAmmo.SetUnicode();

	if (iniAmmo.LoadFile(caliberFileName) > -1) {
		//configPath->clear();
		//configPath->append(caliberFileName);
		configPath = BSFixedString(caliberFileName);

		// caliber keyword
		std::string *keyString = new std::string();
		const char *subTypeID = "";

		const char *keyID = iniAmmo.GetValue("Caliber", "sCaliberKWID", "none");
		objectID = ATShared::GetFormIDFromIdentifier(keyID);

		// casing item
		keyID = iniAmmo.GetValue("Caliber", "sCasingItemID", "none");
		TESObjectMISC *tempCasing = (TESObjectMISC*)ATShared::GetFormFromIdentifier(keyID);
		
		// ammo subtypes:
		int numSubTypes = iniAmmo.GetLongValue("Caliber", "iNumSubTypes", 0);
		if (numSubTypes > 0) {
			for (UInt8 i = 0; i < numSubTypes; i++) {
				ATAmmoType newType;
				keyString->clear();
				keyString->append("sSubtype");
				keyString->append(std::to_string(i));
				subTypeID = iniAmmo.GetValue("Caliber", keyString->c_str(), "none");

				// ammo item
				keyID = iniAmmo.GetValue(subTypeID, "sAmmoID", "none");
				TESForm *tempAmmo = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

				// make sure the ammo form exists before adding to account for DLC/mod-specific ammo types
				if (tempAmmo) {
					newType.modItem = tempAmmo;

					keyID = iniAmmo.GetValue(subTypeID, "sCaliberModID", "none");
					newType.swapMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(keyID);

					if (tempCasing)
						newType.casingItem = tempCasing;

					// Projectile Overrides
					int projectilesCount = iniAmmo.GetLongValue(subTypeID, "iNumProjOverrides", 0);
					if (projectilesCount > 0) {
						for (UInt8 j = 0; j < projectilesCount; j++) {
							ATProjectileOverride tempProjOverride;

							keyString->clear();
							keyString->append("sProjOverride");
							keyString->append(std::to_string(j));
							keyID = iniAmmo.GetValue(subTypeID, keyString->c_str(), "none");
							tempProjOverride.projectile = (BGSProjectile*)ATShared::GetFormFromIdentifier(keyID);

							keyString->clear();
							keyString->append("sProjKeyword");
							keyString->append(std::to_string(j));
							keyID = iniAmmo.GetValue(subTypeID, keyString->c_str(), "none");
							tempProjOverride.projectileKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

							keyString->clear();
							keyString->append("sProjImpact");
							keyString->append(std::to_string(j));
							keyID = iniAmmo.GetValue(subTypeID, keyString->c_str(), "none");
							tempProjOverride.impactData = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(keyID);

							newType.projectiles.Push(tempProjOverride);
						}
					}

					ammoTypes.Push(newType);
				}
			}
			// data was created, return true
			return true;
		}
	}
	return false;
}


bool ATWeapon::LoadFromIni(const char* weapFileName)
{
	CSimpleIniA iniWeap;
	iniWeap.SetUnicode();

	if (iniWeap.LoadFile(weapFileName) > -1) {
		const char *weapID = iniWeap.GetValue("Base", "sWeapID", "none");
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);
		
		if (tempWeapBase) {
			const char *weapName = tempWeapBase->fullName.name.c_str();

			TESObjectWEAP::InstanceData *instanceData = &tempWeapBase->weapData;
			if (instanceData) {
				std::string *keyString = new std::string();

				objectID = tempWeapBase->formID;
				//configPath->clear();
				//configPath->append(weapFileName);
				configPath = BSFixedString(weapFileName);

				// AimModel - recoil/cone of fire edits
				if (instanceData->aimModel) {
					ATAimModel *aimModel = (ATAimModel*)instanceData->aimModel;
					if (aimModel) {
						// diminish spring force
						float springForceMult = (float)min(1.0, max(0.0, iniWeap.GetDoubleValue("Base", "fRecoilSpringForceMult", 1.0)));
						if (springForceMult != 1.0) {
							aimModel->Rec_DimSpringForce = aimModel->Rec_DimSpringForce * springForceMult;
						}
						// recoil
						float recoilMult = iniWeap.GetDoubleValue("Base", "fRecoilMultiplier", 1.0);
						if (recoilMult != 1.0) {
							aimModel->Rec_MaxPerShot = aimModel->Rec_MaxPerShot * recoilMult;
							aimModel->Rec_MinPerShot = aimModel->Rec_MinPerShot * recoilMult;
							aimModel->Rec_ArcMaxDegrees = aimModel->Rec_ArcMaxDegrees * recoilMult;
						}
						// cone of fire
						float cofMult = iniWeap.GetDoubleValue("Base", "fCoFMultiplier", 1.0);
						if (cofMult != 1.0) {
							aimModel->CoF_MaxAngle = aimModel->CoF_MaxAngle * cofMult;
							aimModel->CoF_MinAngle = aimModel->CoF_MinAngle * cofMult;
						}
					}
				}

				// base damage multiplier
				float fDamageMult = iniWeap.GetDoubleValue("Base", "fBaseDamageMult", 1.0);
				if (fDamageMult != 1.0) {
					if (instanceData->baseDamage > 0) {
						instanceData->baseDamage = (UInt16)min((int)max(0.0, (float)(int)instanceData->baseDamage * fDamageMult), 0xFF);
					}
					if (instanceData->damageTypes) {
						if (instanceData->damageTypes->count > 0) {
							for (UInt16 i = 0; i < instanceData->damageTypes->count; i++) {
								TBO_InstanceData::DamageTypes tempDT;
								instanceData->damageTypes->GetNthItem(i, tempDT);
								if (tempDT.value > 0) {
									tempDT.value = (UInt32)min((int)max(0.0, (float)(int)tempDT.value * fDamageMult), 0xFFFF);
								}
							}
						}
					}
				}

				// base range multiplier
				float fRangeMult = iniWeap.GetDoubleValue("Base", "fMaxRangeMult", 1.0);
				if (fRangeMult != 1.0) {
					instanceData->maxRange = instanceData->maxRange * fRangeMult;
				}

				// weapon critical charge/chance multiplier
				float fCritChargeVal = iniWeap.GetDoubleValue("Base", "fCritChargeMult", 1.0);
				if (fCritChargeVal != 1.0) {
					instanceData->critChargeBonus = fCritChargeVal;
				}

				// required skill AV
				const char *reqSkillID = iniWeap.GetValue("Base", "sRequiredSkill", "none");
				ActorValueInfo *reqSkillAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(reqSkillID);
				if (reqSkillAV) {
					instanceData->skill = reqSkillAV;
				}



				// av modifiers
				int avsCount = iniWeap.GetLongValue("ActorValues", "iNumActorValues", 0);
				if (avsCount > 0) {
					if (!instanceData->modifiers) {
						instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
					}
					for (UInt8 i = 0; i < avsCount; i++) {
						keyString->clear();
						keyString->append("sActorValue");
						keyString->append(std::to_string(i));
						const char *avID = iniWeap.GetValue("ActorValues", keyString->c_str(), "none");
						ActorValueInfo *tempAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(avID);
						if (tempAV) {
							keyString->clear();
							keyString->append("iActorValue");
							keyString->append(std::to_string(i));
							UInt32 avAmt = iniWeap.GetLongValue("ActorValues", keyString->c_str(), 0);
							if (avAmt > 0x0) {
								TBO_InstanceData::ValueModifier tempAVMod;
								tempAVMod.avInfo = tempAV;
								tempAVMod.unk08 = avAmt;
								instanceData->modifiers->Push(tempAVMod);
							}
						}
					}
				}

				// enchantments
				int enchCount = iniWeap.GetLongValue("Enchantments", "iNumEnchants", 0);
				if (enchCount > 0) {
					EnchantmentItem *tempEnch = nullptr;
					if (!instanceData->enchantments) {
						instanceData->enchantments = new tArray<EnchantmentItem*>();
					}
					for (UInt8 i = 0; i < enchCount; i++) {
						keyString->clear();
						keyString->append("sEnchant");
						keyString->append(std::to_string(i));
						const char *enchID = iniWeap.GetValue("Enchantments", keyString->c_str(), "none");
						tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
						if (tempEnch) {
							instanceData->enchantments->Push(tempEnch);
						}
					}
				}

				// caliber enable keywords
				int caliberCount = 0;
				int kwCount = iniWeap.GetLongValue("Calibers", "iNumCalibers", 0);
				if (kwCount > 0) {
					BGSKeyword **keywords = new BGSKeyword*[tempWeapBase->keyword.numKeywords + kwCount];
					if (tempWeapBase->keyword.numKeywords > 0) {
						for (UInt8 i = 0; i < tempWeapBase->keyword.numKeywords; i++) {
							keywords[i] = tempWeapBase->keyword.keywords[i];
						}
					}
					for (UInt8 i = 0; i < kwCount; i++) {
						keyString->clear();
						keyString->append("sCaliberKW");
						keyString->append(std::to_string(i));
						const char *kwID = iniWeap.GetValue("Calibers", keyString->c_str(), "none");
						BGSKeyword *tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
						if (tempKW) {
							keywords[i + tempWeapBase->keyword.numKeywords] = tempKW;
							caliberCount += 1;
						}
					}
					// add recipe keywords if needed
					if (caliberCount > 0) {
						tempWeapBase->keyword.numKeywords = tempWeapBase->keyword.numKeywords + kwCount;
						tempWeapBase->keyword.keywords = keywords;
					}
				}

				// swappable mods:
				int iNumModSlots = iniWeap.GetLongValue("SwappableMods", "iNumModSlots", 0);
				if (iNumModSlots > 0) {
					for (UInt8 i = 0; i < iNumModSlots; i++) {
						keyString->clear();
						keyString->append("iNumMods_Slot");
						keyString->append(std::to_string(i));
						kwCount = iniWeap.GetLongValue("SwappableMods", keyString->c_str(), 0);
						if (kwCount > 0) {
							ATModSlot newSlot;
							keyString->clear();
							keyString->append("sSlotName");
							keyString->append(std::to_string(i));
							const char *slotName = iniWeap.GetValue("SwappableMods", keyString->c_str(), " ");
							newSlot.slotName = BSFixedString(slotName);

							for (UInt8 j = 0; j < kwCount; j++) {
								keyString->clear();
								keyString->append("sMod_Slot");
								keyString->append(std::to_string(i));
								keyString->append("_");
								keyString->append(std::to_string(j));
								const char *modID = iniWeap.GetValue("SwappableMods", keyString->c_str(), "none");
								BGSMod::Attachment::Mod *tempMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(modID);
								if (tempMod) {
									ATSwappableMod newMod;
									newMod.swapMod = tempMod;
									keyString->clear();
									keyString->append("sRequiredKW_Slot");
									keyString->append(std::to_string(i));
									keyString->append("_");
									keyString->append(std::to_string(j));
									const char *reqKWID = iniWeap.GetValue("SwappableMods", keyString->c_str(), "none");
									BGSKeyword *tempReqKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(reqKWID);
									if (tempReqKW) {
										newMod.requiredKW = tempReqKW;
									}

									keyString->clear();
									keyString->append("sExcludeKW_Slot");
									keyString->append(std::to_string(i));
									keyString->append("_");
									keyString->append(std::to_string(j));
									const char *excKWID = iniWeap.GetValue("SwappableMods", keyString->c_str(), "none");
									BGSKeyword *tempExcKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(excKWID);
									if (tempExcKW) {
										newMod.excludeKW = tempExcKW;
									}

									newSlot.swappableMods.Push(newMod);
								}
							}

							modSlots.Push(newSlot);
						}
					}
				}

				// damaged mods:
				int iNumDamageSlots = iniWeap.GetLongValue("DamagedMods", "iNumDamageSlots", 0);
				if (iNumDamageSlots > 0) {
					for (UInt8 i = 0; i < iNumDamageSlots; i++) {
						keyString->clear();
						keyString->append("iNumParts_Slot");
						keyString->append(std::to_string(i));
						kwCount = iniWeap.GetLongValue("DamagedMods", keyString->c_str(), 0);
						if (kwCount > 0) {
							ATModSlot newDamSlot;
							
							for (UInt8 j = 0; j < kwCount; j++) {
								keyString->clear();
								keyString->append("sDamagedMod_Slot");
								keyString->append(std::to_string(i));
								keyString->append("_");
								keyString->append(std::to_string(j));
								const char *modID = iniWeap.GetValue("DamagedMods", keyString->c_str(), "none");
								BGSMod::Attachment::Mod *tempMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(modID);
								if (tempMod) {
									ATSwappableMod newMod;
									newMod.swapMod = tempMod;
									keyString->clear();
									keyString->append("sRequiredKW_Slot");
									keyString->append(std::to_string(i));
									keyString->append("_");
									keyString->append(std::to_string(j));
									const char *reqKWID = iniWeap.GetValue("DamagedMods", keyString->c_str(), "none");
									BGSKeyword *tempReqKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(reqKWID);
									if (tempReqKW) {
										newMod.requiredKW = tempReqKW;
									}

									keyString->clear();
									keyString->append("sExcludeKW_Slot");
									keyString->append(std::to_string(i));
									keyString->append("_");
									keyString->append(std::to_string(j));
									const char *excKWID = iniWeap.GetValue("DamagedMods", keyString->c_str(), "none");
									BGSKeyword *tempExcKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(excKWID);
									if (tempExcKW) {
										newMod.excludeKW = tempExcKW;
									}

									newDamSlot.swappableMods.Push(newMod);
								}
							}

							if (newDamSlot.swappableMods.count > 0) {
								damagedMods.Push(newDamSlot);
							}
						}
					}
				}

				// critical failures
				kwCount = iniWeap.GetLongValue("CritFailures", "iNumCritFails", 0);
				if (kwCount > 0) {
					for (UInt8 i = 0; i < kwCount; i++) {
						ATCritEffect newEffect;
						keyString->clear();
						keyString->append("sCritSpell");
						keyString->append(std::to_string(i));
						const char *spellID = iniWeap.GetValue("CritFailures", keyString->c_str(), "none");
						SpellItem *tempSpell = (SpellItem*)ATShared::GetFormFromIdentifier(spellID);
						newEffect.critSpell = tempSpell;
						newEffect.rollMax = iniWeap.GetLongValue("CritFailures", keyString->c_str(), 0);
						critFailureTable.critFailures.Push(newEffect);
					}
				}

				// magazines
				kwCount = iniWeap.GetLongValue("Magazines", "iNumMags", 0);
				iReloadType = iniWeap.GetLongValue("Magazines", "iReloadType", 0);
				if (kwCount > 0) {
					for (UInt8 i = 0; i < kwCount; i++) {
						keyString->clear();
						keyString->append("sMagCheckKW");
						keyString->append(std::to_string(i));
						const char *kwID = iniWeap.GetValue("Magazines", keyString->c_str(), "none");
						BGSKeyword *tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
						if (tempKW) {
							keyString->clear();
							keyString->append("sMagItem");
							keyString->append(std::to_string(i));
							const char *magItemID = iniWeap.GetValue("Magazines", keyString->c_str(), "none");
							TESObjectMISC *tempMagItem = (TESObjectMISC*)ATShared::GetFormFromIdentifier(magItemID);
							if (tempMagItem) {
								ATMagItem newMag;
								newMag.magItem = tempMagItem;
								newMag.magKW = tempKW;
								magazines.Push(newMag);
							}
						}
					}
				}

				// holsters
				kwCount = iniWeap.GetLongValue("Holsters", "iNumHolsters", 0);
				if (kwCount > 0) {

				}

				// NPCsUseAmmo flag
				if (instanceData->flags) {
					//NPCsUseAmmo
					bool bUseAmmo = iniWeap.GetBoolValue("Base", "bNPCsUseAmmo", false);
					if (bUseAmmo)
						instanceData->flags |= 0x0000002;
					else
						instanceData->flags &= 0x0000002;
				}

				_MESSAGE("  Loaded:  0x%08x - %s", objectID, weapName);
				if (caliberCount > 0) {
					_MESSAGE("        %i Calibers", caliberCount);
				}

				for (UInt8 i = 0; i < modSlots.count; i++) {
					ATModSlot tempSlot;
					if (modSlots.GetNthItem(i, tempSlot)) {
						_MESSAGE("        %i %s", tempSlot.swappableMods.count, tempSlot.slotName.c_str());
					}
				}

				return true;
			}
		}
		
	}
	return false;
}



// Config file read operations:
namespace ATFileLoad
{

	// loads turret/robot weapon changes from ini
	bool LoadWeaponBase_TurretRobot(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (tempWeapBase) {
			TESObjectWEAP::InstanceData *instanceData = &tempWeapBase->weapData;
			if (instanceData) {
				const char *tmpAmmoID = iniWeap->GetValue("Base", "sAmmo", "none");
				TESAmmo *tmpAmmoForm = (TESAmmo*)ATShared::GetFormFromIdentifier(tmpAmmoID);
				if (tmpAmmoForm) {
					instanceData->ammo = tmpAmmoForm;
				}

				const char *tmpProjID = iniWeap->GetValue("Base", "sProjectile", "none");
				if (instanceData->firingData) {
					BGSProjectile *tmpProjForm = (BGSProjectile*)ATShared::GetFormFromIdentifier(tmpProjID);
					if (tmpProjForm) {
						instanceData->firingData->projectileOverride = tmpProjForm;
					}
				}

				const char *tmpNPCListID = iniWeap->GetValue("Base", "sAmmoLootList", "none");
				TESLevItem *tmpNPCListForm = (TESLevItem*)ATShared::GetFormFromIdentifier(tmpNPCListID);
				if (tmpNPCListForm) {
					instanceData->addAmmoList = tmpNPCListForm;
				}

				const char *tmpImpactID = iniWeap->GetValue("Base", "sImpactDataSet", "none");
				BGSImpactDataSet *tmpImpactForm = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(tmpImpactID);
				if (tmpImpactForm) {
					instanceData->unk58 = tmpImpactForm;
				}

				UInt16 tmpDamage = iniWeap->GetLongValue("Base", "iDamage", 0);
				instanceData->baseDamage = tmpDamage;

				float tmpRangeMin = iniWeap->GetDoubleValue("Base", "fRangeMin", 64.0);
				instanceData->minRange = tmpRangeMin;

				float tmpRangeMax = iniWeap->GetDoubleValue("Base", "fRangeMax", 256.0);
				instanceData->maxRange = tmpRangeMax;
				
				float tmpOoRMult = iniWeap->GetDoubleValue("Base", "fOoRMult", 0.5);
				instanceData->outOfRangeMultiplier = tmpOoRMult;
				
				// av modifiers
				int avsCount = iniWeap->GetLongValue("Base", "iNumActorValues", 0);
				if (avsCount > 0) {
					if (!instanceData->modifiers) {
						instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
					}
					for (UInt8 i = 0; i < avsCount; i++) {
						std::string *avKeyStr = new std::string();
						avKeyStr->append("sActorValue");
						avKeyStr->append(std::to_string(i));
						const char *avID = iniWeap->GetValue("Base", avKeyStr->c_str(), "none");
						ActorValueInfo *tempAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(avID);
						if (tempAV) {
							avKeyStr->clear();
							avKeyStr->append("iActorValue");
							avKeyStr->append(std::to_string(i));
							UInt32 avAmt = iniWeap->GetLongValue("Base", avKeyStr->c_str(), 0);
							if (avAmt > 0x0) {
								TBO_InstanceData::ValueModifier tempAVMod;
								tempAVMod.avInfo = tempAV;
								tempAVMod.unk08 = avAmt;
								instanceData->modifiers->Push(tempAVMod);
							}
						}
					}
				}
					
				// enchantments
				int enchCount = iniWeap->GetLongValue("Base", "iNumEnchants", 0);
				if (enchCount > 0) {
					EnchantmentItem *tempEnch = nullptr;
					if (!instanceData->enchantments) {
						instanceData->enchantments = new tArray<EnchantmentItem*>();
					}
					for (UInt8 i = 0; i < enchCount; i++) {
						std::string *enchKeyStr = new std::string();
						enchKeyStr->append("sEnchant");
						enchKeyStr->append(std::to_string(i));
						const char *enchID = iniWeap->GetValue("Base", enchKeyStr->c_str(), "none");
						tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
						if (tempEnch) {
							instanceData->enchantments->Push(tempEnch);
						}
					}
				}

				//flags:

				// NPCsUseAmmo
				if (instanceData->flags) {
					//NPCsUseAmmo
					bool bUseAmmo = iniWeap->GetBoolValue("Base", "bUseAmmo", false);
					if (bUseAmmo)
						instanceData->flags |= 0x0000002;
					else
						instanceData->flags &= 0x0000002;
				}

				return true;
			}
		}
		
		return false;
	}

	// loads thrown weapon changes from ini
	bool LoadWeaponBase_Thrown(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (tempWeapBase) {
			TESObjectWEAP::InstanceData *instanceData = &tempWeapBase->weapData;
			if (instanceData) {
				//flags:
				// NPCsUseAmmo
				if (instanceData->flags) {
					//NPCsUseAmmo
					bool bUseAmmo = iniWeap->GetBoolValue("Base", "bNPCsUseAmmo", false);
					if (bUseAmmo)
						instanceData->flags |= 0x0000002;
					else
						instanceData->flags &= 0x0000002;
				}
				return true;
			}
		}
		
		return false;
	}

	// loads melee weapon changes from ini
	bool LoadWeaponBase_Melee(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (tempWeapBase) {
			TESObjectWEAP::InstanceData *instanceData = &tempWeapBase->weapData;
			if (instanceData) {
				//fBaseDamageMult
				float fBaseDamageMult = iniWeap->GetDoubleValue("Base", "fBaseDamageMult", 1.0);
				if (fBaseDamageMult != 1.0) {
					instanceData->baseDamage = (UInt16)min((int)((float)(int)instanceData->baseDamage * fBaseDamageMult), 0xFF);
				}

				//fReachMult
				float fReachMult = iniWeap->GetDoubleValue("Base", "fReachMult", 1.0);
				if (fReachMult != 1.0) {
					instanceData->reach = max(0.0, instanceData->reach * fReachMult);
				}
				
				//fCritChargeMult
				float fCritChargeMult = iniWeap->GetDoubleValue("Base", "fCritChargeMult", 1.0);
				if (fCritChargeMult != 1.0) {
					instanceData->critChargeBonus = fCritChargeMult;
				}

				//iStaggerAmount
				// 0 = none, 1 = small, 2 = medium, 3 = large, 4 = extralarge
				UInt32 iStaggerAmount = iniWeap->GetLongValue("Base", "iStaggerAmount", 1);
				if (iStaggerAmount < 5) {
					instanceData->stagger = iStaggerAmount;
				}

				//fAPCostMult
				float fAPCostMult = iniWeap->GetDoubleValue("Base", "fAPCostMult", 1.0);
				if (fAPCostMult != 1.0) {
					instanceData->actionCost = instanceData->actionCost * fAPCostMult;
				}

				//sRequiredSkill
				const char *reqSkillID = iniWeap->GetValue("Base", "sRequiredSkill", "none");
				ActorValueInfo *reqSkillAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(reqSkillID);
				if (reqSkillAV) {
					instanceData->skill = reqSkillAV;
				}

				// av modifiers
				int avsCount = iniWeap->GetLongValue("ActorValues", "iNumActorValues", 0);
				if (avsCount > 0) {
					if (!instanceData->modifiers) {
						instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
					}
					for (UInt8 i = 0; i < avsCount; i++) {
						std::string *avKeyStr = new std::string();
						avKeyStr->append("sActorValue");
						avKeyStr->append(std::to_string(i));
						const char *avID = iniWeap->GetValue("ActorValues", avKeyStr->c_str(), "none");
						ActorValueInfo *tempAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(avID);
						if (tempAV) {
							avKeyStr->clear();
							avKeyStr->append("iActorValue");
							avKeyStr->append(std::to_string(i));
							UInt32 avAmt = iniWeap->GetLongValue("ActorValues", avKeyStr->c_str(), 0);
							if (avAmt > 0x0) {
								TBO_InstanceData::ValueModifier tempAVMod;
								tempAVMod.avInfo = tempAV;
								tempAVMod.unk08 = avAmt;
								instanceData->modifiers->Push(tempAVMod);
							}
						}
					}
				}

				// enchantments
				int enchCount = iniWeap->GetLongValue("Enchantments", "iNumEnchants", 0);
				if (enchCount > 0) {
					EnchantmentItem *tempEnch = nullptr;
					if (!instanceData->enchantments) {
						instanceData->enchantments = new tArray<EnchantmentItem*>();
					}
					for (UInt8 i = 0; i < enchCount; i++) {
						std::string *enchKeyStr = new std::string();
						enchKeyStr->append("sEnchant");
						enchKeyStr->append(std::to_string(i));
						const char *enchID = iniWeap->GetValue("Enchantments", enchKeyStr->c_str(), "none");
						tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
						if (tempEnch) {
							instanceData->enchantments->Push(tempEnch);
						}
					}
				}

				//"ammo"/attack type


				//damagedmods

			}
		}
		return false;
	}



	void LoadData_Ammo(const char* configDir)
	{
		const char *tempModName = "", *ammoID = "none";
		std::string configAmmoString = "";
		std::string configModDirString = "";
		std::string configSection = "";
		UInt32 caliberCount = 0;
		UInt32 tempID = 0;

		CSimpleIniA iniIndex;
		iniIndex.SetUnicode();

		for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
			configAmmoString.clear();
			configAmmoString.append(configDir);
			configAmmoString.append(tempModName);
			configAmmoString.append("\\_Index.ini");

			if (iniIndex.LoadFile(configAmmoString.c_str()) == 0) {
				caliberCount = iniIndex.GetLongValue("Calibers", "iNumCalibers", 0);
				if (caliberCount > 0) {
					_MESSAGE("\n%s: Loading %i Calibers", tempModName, caliberCount);
					for (UInt8 i = 0; i < caliberCount; i++) {
						configSection.clear();
						configSection.append("sCaliberID");
						configSection.append(std::to_string(i));
						ammoID = iniIndex.GetValue("Calibers", configSection.c_str(), "none");

						if (ammoID != "none") {
							configModDirString.clear();
							configModDirString.append(configDir);
							configModDirString.append(tempModName);
							configModDirString.append("\\");
							configModDirString.append(ammoID);
							configModDirString.append(".ini");

							ATCaliber caliberStore;
							if (caliberStore.LoadFromIni(configModDirString.c_str())) {
								tempID = caliberStore.objectID;
								int foundIndex = ATShared::index_Calibers.GetItemIndex(caliberStore.objectID);
								if (foundIndex > -1) {
									// the caliber already exits - override with newer data
									ATShared::ATCalibers[foundIndex] = caliberStore;
									_MESSAGE("  Override:  %s - 0x%08X", ammoID, caliberStore.objectID);
								}
								else {
									_MESSAGE("  Loaded:    %s - 0x%08X", ammoID, tempID);
									ATShared::ATCalibers.Push(caliberStore);
									ATShared::index_Calibers.Push(tempID);
									
								}
							}
							else {
								_MESSAGE("  !!Failed:  %s (%s)", ammoID, tempModName);
							}
						}
					}
				}
			}
			iniIndex.Reset();
		}
	}

	
	void LoadData_Weapons(const char* configDir)
	{
		const char *tempModName = "", *finalDirName = "", *weaponName = "", *weaponID = "";

		std::string configWeapons = "";
		std::string configModDir = "";
		std::string configSection = "";

		UInt32 weaponCount = 0;
		int weapTypeIndex = -1;

		CSimpleIniA iniIndex;
		iniIndex.SetUnicode();

		CSimpleIniA iniWeap;
		iniWeap.SetUnicode();

		for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
			configWeapons.clear();
			configWeapons.append(configDir);
			configWeapons.append(tempModName);
			configWeapons.append("\\_Index.ini");

			if (iniIndex.LoadFile(configWeapons.c_str()) > -1) {

				// guns
				weaponCount = iniIndex.GetLongValue("Gun", "iNumWeapons", 0);
				if (weaponCount > 0) {
					_MESSAGE("\n%s: Loading %i Guns", tempModName, weaponCount);
					for (UInt8 i = 0; i < weaponCount; i++) {
						configSection.clear();
						configSection.append("sWeapon");
						configSection.append(std::to_string(i));

						weaponName = iniIndex.GetValue("Gun", configSection.c_str(), "");
						if (weaponName != "") {
							configModDir.clear();
							configModDir.append(configDir);
							configModDir.append(tempModName);
							configModDir.append("\\Gun\\");
							configModDir.append(weaponName);
							configModDir.append(".ini");
							finalDirName = configModDir.c_str();
							if (iniWeap.LoadFile(finalDirName) > -1) {
								weaponID = iniWeap.GetValue("Base", "sWeapID", "none");
								
								ATWeapon newWeapon;
								if (newWeapon.LoadFromIni(finalDirName)) {
									int foundIndex = ATShared::index_Weapons.GetItemIndex(newWeapon.objectID);
									if (foundIndex > -1) {
										// weapon already exists - override
										ATShared::ATWeapons[foundIndex] = newWeapon;
									} 
									else {
										ATShared::ATWeapons.Push(newWeapon);
										ATShared::index_Weapons.Push(newWeapon.objectID);
									}
								}
								else {
									_MESSAGE("  x Failed to load %s", finalDirName);
								}
								iniWeap.Reset();
							}
							else {
								_MESSAGE("  xx Not found: %s", finalDirName);
							}
						}
					}
					
				}
				
				/*
				// melee weapons
				weaponCount = iniIndex.GetLongValue("Melee", "iNumWeapons", 0);
				if (weaponCount > 0) {
					_MESSAGE("\n%s: Loading %i Melee Weapons", tempModName, weaponCount);
					for (UInt8 i = 0; i < weaponCount; i++) {
						configSection.clear();
						configSection.append("sWeapon");
						configSection.append(std::to_string(i));

						weaponName = iniIndex.GetValue("Melee", configSection.c_str(), "");
						if (weaponName != "") {
							configModDir.clear();
							configModDir.append(configDir);
							configModDir.append(tempModName);
							configModDir.append("\\Melee\\");
							configModDir.append(weaponName);
							configModDir.append(".ini");
							finalDirName = configModDir.c_str();
							if (iniWeap.LoadFile(finalDirName) > -1) {
								weaponID = iniWeap.GetValue("Base", "sWeapID", "none");
								if (LoadWeaponBase_Melee(&iniWeap, weaponID)) {
									_MESSAGE("  Loaded %s", finalDirName);
								}
								else {
									_MESSAGE("  x Failed to load %s", finalDirName);
								}
								iniWeap.Reset();
							}
							else {
								_MESSAGE("  xx Not found: %s", finalDirName);
							}
						}
					}
					
				}
				*/

				/*
				// thrown weapons
				weaponCount = iniIndex.GetLongValue("Thrown", "iNumWeapons", 0);
				if (weaponCount > 0) {
					_MESSAGE("\n%s: Loading %i Thrown Weapons", tempModName, weaponCount);
					for (UInt8 i = 0; i < weaponCount; i++) {
						configSection.clear();
						configSection.append("sWeapon");
						configSection.append(std::to_string(i));

						// thrown weapons
						weaponName = iniIndex.GetValue("Thrown", configSection.c_str(), "");
						if (weaponName != "") {
							configModDir.clear();
							configModDir.append(configDir);
							configModDir.append(tempModName);
							configModDir.append("\\Thrown\\");
							configModDir.append(weaponName);
							configModDir.append(".ini");
							finalDirName = configModDir.c_str();
							if (iniWeap.LoadFile(finalDirName) > -1) {
								if (LoadWeaponBase_Thrown(&iniWeap, weaponID)) {
									weaponID = iniWeap.GetValue("Base", "sWeapID", "none");
									_MESSAGE("  Loaded %s", finalDirName);
								}
								else {
									_MESSAGE("  x Failed to load %s", finalDirName);
								}
								iniWeap.Reset();
							}
							else {
								_MESSAGE("  xx Not found: %s", finalDirName);
							}
						}
					}
					
				}
				*/

				/*
				// turrets
				weaponCount = iniIndex.GetLongValue("Turret", "iNumWeapons", 0);
				if (weaponCount > 0) {
					_MESSAGE("\n%s: Loading %i Turrets", tempModName, weaponCount);
					for (UInt8 i = 0; i < weaponCount; i++) {
						configSection.clear();
						configSection.append("sWeapon");
						configSection.append(std::to_string(i));

						weaponName = iniIndex.GetValue("Turret", configSection.c_str(), "");
						if (weaponName != "") {
							configModDir.clear();
							configModDir.append(configDir);
							configModDir.append(tempModName);
							configModDir.append("\\Turret\\");
							configModDir.append(weaponName);
							configModDir.append(".ini");
							finalDirName = configModDir.c_str();
							if (iniWeap.LoadFile(finalDirName) > -1) {
								weaponID = iniWeap.GetValue("Base", "sWeapID", "none");
								if (LoadWeaponBase_TurretRobot(&iniWeap, weaponID)) {
									_MESSAGE("  Loaded %s", finalDirName);
								}
								else {
									_MESSAGE("  x Failed to load %s", finalDirName);
								}
								iniWeap.Reset();
							}
							else {
								_MESSAGE("  xx Not found: %s", finalDirName);
							}
						}
					}
					
				}
				*/
				
			}
			iniIndex.Reset();
		}

		configWeapons.clear();
		configModDir.clear();
		configSection.clear();
	}


	void LoadData_CritEffectTables(const char* configDir)
	{
		const char *tempModName = "";
		std::string tempDirStr;

		CSimpleIniA iniCritTables;
		iniCritTables.SetUnicode();

		for (UInt8 k = 0; k < (*g_dataHandler)->modList.loadedMods.count; k++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[k]->name;
			tempDirStr.clear();
			tempDirStr.append(configDir);
			tempDirStr.append(tempModName);
			tempDirStr.append("\\CritEffectTables.ini");

			if (iniCritTables.LoadFile(tempDirStr.c_str()) > -1) {
				int critTablesCount = iniCritTables.GetLongValue("Index", "iNumTables", 0);
				if (critTablesCount > 0) {
					std::string configTableName = "";
					std::string configSpellName = "";

					_MESSAGE("%s: Loading %i Critical Effect Tables", tempModName, critTablesCount);

					for (UInt8 i = 0; i < critTablesCount; i++) {
						configTableName.clear();
						configTableName.append("sTable");
						configTableName.append(std::to_string(i));
						
						const char *sectionName = iniCritTables.GetValue("Index", configTableName.c_str(), "");
						if (sectionName != "") {
							int critSpellsCount = iniCritTables.GetLongValue(sectionName, "iNumEffects", 0);
							if (critSpellsCount > 0) {
								ATCritEffectTable newCritTable;
								for (UInt8 j = 0; j < critSpellsCount; j++) {
									ATCritEffect newCritEffect;

									configSpellName.clear();
									configSpellName.append("sCritSpell");
									configSpellName.append(std::to_string(i));
									const char *spellID = iniCritTables.GetValue(sectionName, configSpellName.c_str(), "none");
									SpellItem *tempSpell = (SpellItem*)ATShared::GetFormFromIdentifier(spellID);
									if (tempSpell) {
										newCritEffect.critSpell = tempSpell;
									}

									configSpellName.clear();
									configSpellName.append("iCritRollMax");
									configSpellName.append(std::to_string(i));
									newCritEffect.rollMax = iniCritTables.GetLongValue(sectionName, configSpellName.c_str(), 0);

									configSpellName.clear();
									configSpellName.append("fCritScale");
									configSpellName.append(std::to_string(i));
									newCritEffect.critScale = iniCritTables.GetDoubleValue(sectionName, configSpellName.c_str(), 1.0);

									newCritTable.critEffects.Push(newCritEffect);
								}

								_MESSAGE("  Loaded %s:  %i Effects", sectionName, newCritTable.critEffects.count);
							}
						}
					}
				}
			}
		}
	}

}



void ATConfig::EditGameData()
{
	const char* configPath_MainINI = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\AmmoTweaks.ini";

	const char* configPath_Weapons = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Weapons\\";
	const char* configPath_Ammo = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Ammo\\";

	const char* configPath_Armor = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Armor\\";
	const char* configPath_Races = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Races\\";

	const char* configPath_Translations = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Translations\\";

	const char* configPath_CritTables = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\CritEffectTables\\";

	const char* sLanguage = "Default";

	//		ini Settings:

	CSimpleIniA mainINI;
	mainINI.SetUnicode();
	mainINI.LoadFile(configPath_MainINI);

	bool bLoadWeaponData = mainINI.GetBoolValue("WeaponTweaks", "bEnable", false);
	bool bLoadArmorData = mainINI.GetBoolValue("ArmorTweaks", "bEnable", false);
	bool bLoadResistData = mainINI.GetBoolValue("ResistTweaks", "bEnable", false);

	
	ATFileLoad::LoadData_CritEffectTables(configPath_CritTables);
	ATFileLoad::LoadData_Ammo(configPath_Ammo);
	ATFileLoad::LoadData_Weapons(configPath_Weapons);
	
	_MESSAGE("\nGame Data finished loading.\n");
}


#include "ATConfigLoader.h"
#include <time.h>


// ----------------------- ATCaliber:

// loads a caliber's data from the given ini file path
bool ATCaliber::ReadIni(const char* configFileName)
{
	bool bRetVal = false;
	CSimpleIniA iniAmmo;
	iniAmmo.SetUnicode();
	iniAmmo.SetMultiKey(true);

	if (iniAmmo.LoadFile(configFileName) > -1) {
		configPath = BSFixedString(configFileName);

		// caliber keyword
		const char *keyID = iniAmmo.GetValue("Caliber", "sCaliberID", "none");
		objectID = ATShared::GetFormIDFromIdentifier(keyID);

		hudName = iniAmmo.GetValue("Caliber", "sName", "none");

		// ammo subtypes:
		CSimpleIni::TNamesDepend values, valuesProj;
		if (iniAmmo.GetAllValues("Caliber", "sAmmoTypes", values)) {
			int ammoTypeCount = values.size();
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				const char *ammoSection = i->pItem;
				// ammo item
				keyID = iniAmmo.GetValue(ammoSection, "sAmmoID", "none");
				TESForm *tempAmmo = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

				// ObjectMod
				keyID = iniAmmo.GetValue(ammoSection, "sModID", "none");
				BGSMod::Attachment::Mod *tempMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(keyID);

				// make sure the ammo form + mod exist
				if (tempAmmo && tempMod) {
					AmmoType newType;

					newType.modItem = tempAmmo;
					newType.swapMod = tempMod;
					
					// casing item
					keyID = iniAmmo.GetValue(ammoSection, "sCasingID", "none");
					newType.casingItem = (TESObjectMISC*)ATShared::GetFormFromIdentifier(keyID);

					// projectiles
					if (iniAmmo.GetAllValues(ammoSection, "sProjectiles", valuesProj)) {
						CSimpleIni::TNamesDepend::const_iterator j = valuesProj.begin();
						for (; j != valuesProj.end(); ++j) {
							const char *projSection = j->pItem;
							ProjectileOverride tempProjOverride;

							// projectile
							keyID = iniAmmo.GetValue(projSection, "sProjectile", "none");
							tempProjOverride.projectile = (BGSProjectile*)ATShared::GetFormFromIdentifier(keyID);
							// keywords:
							keyID = iniAmmo.GetValue(projSection, "sRequiredKW", "none");
							tempProjOverride.projectileKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);
							keyID = iniAmmo.GetValue(projSection, "sExcludeKW", "none");
							tempProjOverride.excludeKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);
							// impactData
							keyID = iniAmmo.GetValue(projSection, "sImpactData", "none");
							tempProjOverride.impactData = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(keyID);

							// add to ammo's projectile overrides
							newType.projectiles.Push(tempProjOverride);
						}
					}
					valuesProj.clear();

					// add ammo type to the caliber list
					ammoTypes.Push(newType);
					bRetVal = true;
				}
			}
		}
		
		values.clear();
	}
	return bRetVal;
}


// ----------------------- ATWeapon:

// loads a weapon's data from the given ini file path
bool ATWeapon::ReadIni(const char* configFileName, bool bSaveData, bool bLoadStatTweaks, bool bFollowerWeapons, bool bNPCsUseAmmo)
{
	CSimpleIniA iniWeap;
	iniWeap.SetUnicode();
	iniWeap.SetMultiKey(true);

	if (iniWeap.LoadFile(configFileName) > -1) {
		const char *weapID = iniWeap.GetValue("Base", "sWeapID", "none");
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);
		
		if (tempWeapBase) {
			TESObjectWEAP::InstanceData *instanceData = &tempWeapBase->weapData;
			if (instanceData) {
				const char *keyID = "";
				CSimpleIni::TNamesDepend values;
				
				objectID = tempWeapBase->formID;
				configPath = BSFixedString(configFileName);
				hudName = iniWeap.GetValue("Base", "sName", "");

				if (bLoadStatTweaks) {
					// Base Damage
					int iDamage = iniWeap.GetLongValue("Base", "iBaseDamage", -1);
					if (iDamage > -1) {
						instanceData->baseDamage = (UInt16)min(max(0, iDamage), 0xFF);
					}

					// Base DamageTypes
					if (iniWeap.GetAllValues("Base", "sBaseDamageTypes", values)) {
						int dtCount = values.size();
						if (dtCount > 0) {
							if (!instanceData->damageTypes) {
								instanceData->damageTypes = new tArray<TBO_InstanceData::DamageTypes>();
							}
							int oldDTCount = instanceData->damageTypes->count;

							CSimpleIni::TNamesDepend::const_iterator i = values.begin();
							for (; i != values.end(); ++i) {
								TBO_InstanceData::DamageTypes tempDT = ATShared::GetDamageTypeFromIdentifier(i->pItem);
								if (tempDT.damageType && (tempDT.value > 0)) {
									int iFound = -1;
									for (UInt16 j = 0; j < oldDTCount; j++) {
										TBO_InstanceData::DamageTypes checkDT;
										if (instanceData->damageTypes->GetNthItem(j, checkDT)) {
											if (checkDT.damageType == tempDT.damageType) {
												checkDT.value = tempDT.value;
												iFound = j;
												break;
											}
										}
									}
									if (iFound < 0) {
										instanceData->damageTypes->Push(tempDT);
									}
								}
							}
						}
					}
					values.clear();

					// Secondary Damage
					float fBaseSecDamage = iniWeap.GetDoubleValue("Base", "fBaseSecDamage", -1.0);
					if (fBaseSecDamage >= 0.0) {
						instanceData->secondary = fBaseSecDamage;
					}

					// AP Cost
					float fAPCost = iniWeap.GetDoubleValue("Base", "fAPCost", -1.0);
					if (fAPCost >= 0.0) {
						instanceData->actionCost = fAPCost;
					}

					// Critical Charge/Chance Multiplier
					float fCritChargeVal = iniWeap.GetDoubleValue("Base", "fCritChargeBonus", -1.0);
					if (fCritChargeVal >= 0.0) {
						instanceData->critChargeBonus = fCritChargeVal;
					}

					// Base Range
					float fRangeMax = iniWeap.GetDoubleValue("Base", "fBaseRangeMax", -1.0);
					if (fRangeMax >= 0.0) {
						instanceData->maxRange = fRangeMax;
					}
					float fRangeMin = iniWeap.GetDoubleValue("Base", "fBaseRangeMin", -1.0);
					if (fRangeMin >= 0.0) {
						instanceData->minRange = fRangeMin;
					}

					// out of range damage mult.
					float fOoRMult = iniWeap.GetDoubleValue("Base", "fOutOfRangeMult", -1.0);
					if (fOoRMult >= 0.0) {
						instanceData->outOfRangeMultiplier = fOoRMult;
					}

					// Melee Reach
					float fReach = iniWeap.GetDoubleValue("Base", "fReach", -1.0);
					if (fReach >= 0.0) {
						instanceData->reach = fReach;
					}

					// Melee Stagger
					int iStaggerAmount = iniWeap.GetLongValue("Base", "iStaggerAmount", -1);
					if ((iStaggerAmount > -1) && (iStaggerAmount < 5)) {
						instanceData->stagger = (UInt32)iStaggerAmount;
					}

					// value
					int iCapsValue = iniWeap.GetDoubleValue("Base", "iBaseValue", -1);
					if (iCapsValue >= 0) {
						instanceData->value = (UInt32)iCapsValue;
					}

					// AimModel:
					if (instanceData->aimModel) {
						ATAimModel *aimModel = (ATAimModel*)instanceData->aimModel;
						if (aimModel) {
							// Recoil
							float springForce = iniWeap.GetDoubleValue("Base", "fRecoilSpringForce", -1.0);
							if (springForce >= 0.0) {
								aimModel->Rec_DimSpringForce = springForce;
							}
							float recoilMin = iniWeap.GetDoubleValue("Base", "fRecoilMin", -1.0);
							if (recoilMin >= 0.0) {
								aimModel->Rec_MinPerShot = recoilMin;
							}
							float recoilMax = iniWeap.GetDoubleValue("Base", "fRecoilMax", -1.0);
							if (recoilMax >= 0.0) {
								aimModel->Rec_MaxPerShot = recoilMax;
							}
							float recoilHipMult = iniWeap.GetDoubleValue("Base", "fRecoilHipMult", -1.0);
							if (recoilHipMult >= 0.0) {
								aimModel->Rec_HipMult = recoilHipMult;
							}

							// Cone of Fire
							float cofMin = iniWeap.GetDoubleValue("Base", "fConeOfFireMin", -1.0);
							if (cofMin >= 0.0) {
								aimModel->CoF_MinAngle = cofMin;
							}
							float cofMax = iniWeap.GetDoubleValue("Base", "fConeOfFireMax", -1.0);
							if (cofMax >= 0.0) {
								aimModel->CoF_MaxAngle = cofMax;
							}
						}
					}


					// default ammo
					keyID = iniWeap.GetValue("Base", "sAmmoID", "none");
					TESAmmo *ammoDefault = (TESAmmo*)ATShared::GetFormFromIdentifier(keyID);
					if (ammoDefault) {
						instanceData->ammo = ammoDefault;
					}

					// NPC ammo list
					keyID = iniWeap.GetValue("Base", "sAmmoLootList", "none");
					TESLevItem *ammoLoot = (TESLevItem*)ATShared::GetFormFromIdentifier(keyID);
					if (ammoLoot) {
						instanceData->addAmmoList = ammoLoot;
					}

					// default projectile override
					if (instanceData->firingData) {
						keyID = iniWeap.GetValue("Base", "sProjectile", "none");
						BGSProjectile *projOvrd = (BGSProjectile*)ATShared::GetFormFromIdentifier(keyID);
						if (projOvrd) {
							instanceData->firingData->projectileOverride = projOvrd;
						}
					}

					// ImpactDataList
					keyID = iniWeap.GetValue("Base", "sImpactDataSet", "none");
					BGSImpactDataSet *tempImpactData = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(keyID);
					if (tempImpactData) {
						instanceData->unk58 = tempImpactData;
					}
				}
				

				// Flags
				if (instanceData->flags) {
					// NPCsUseAmmo
					if (bNPCsUseAmmo) {
						bool bUseAmmo = iniWeap.GetBoolValue("Base", "bNPCsUseAmmo", false);
						if (bUseAmmo)
							instanceData->flags |= ATShared::ATData.wFlag_NPCsUseAmmo;
						else
							instanceData->flags &= ATShared::ATData.wFlag_NPCsUseAmmo;
					}

					// remove the 'not playable' and 'can't drop' flags from follower weapons
					if (bFollowerWeapons) {
						bool bFollowerWeapon = iniWeap.GetBoolValue("Base", "bFollowerWeapon", false);
						if (bFollowerWeapon) {
							instanceData->flags &= ATShared::ATData.wFlag_CantDrop;
							instanceData->flags &= ATShared::ATData.wFlag_NotPlayable;
						}
					}
				}

				

				// ActorValue Modifiers
				if (iniWeap.GetAllValues("Base", "sActorValues", values)) {
					if (values.size() > 0) {
						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}
						CSimpleIni::TNamesDepend::const_iterator i = values.begin();
						for (; i != values.end(); ++i) {
							keyID = i->pItem;
							TBO_InstanceData::ValueModifier newAVMod = ATShared::GetAVModiferFromIdentifer(keyID);
							if ((newAVMod.avInfo) && (newAVMod.unk08 > 0)) {
								instanceData->modifiers->Push(newAVMod);
							}
						}
					}
				}
				values.clear();

				// Enchantments
				if (iniWeap.GetAllValues("Base", "sEnchantments", values)) {
					if (values.size() > 0) {
						if (!instanceData->enchantments) {
							instanceData->enchantments = new tArray<EnchantmentItem*>();
						}
						CSimpleIni::TNamesDepend::const_iterator i = values.begin();
						for (; i != values.end(); ++i) {
							keyID = i->pItem;
							EnchantmentItem *tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(keyID);
							if (tempEnch) {
								instanceData->enchantments->Push(tempEnch);
							}
						}
					}
				}
				values.clear();

				// Keywords
				if (iniWeap.GetAllValues("Base", "sKeywords", values)) {
					int kwCount = values.size();
					if (kwCount > 0) {
						BGSKeyword **keywords = new BGSKeyword*[tempWeapBase->keyword.numKeywords + kwCount];
						if (tempWeapBase->keyword.numKeywords > 0) {
							for (UInt8 i = 0; i < tempWeapBase->keyword.numKeywords; i++) {
								keywords[i] = tempWeapBase->keyword.keywords[i];
							}
						}
						CSimpleIni::TNamesDepend::const_iterator i = values.begin();
						int j = 0;
						for (; i != values.end(); ++i) {
							keyID = i->pItem;
							BGSKeyword *tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);
							if (tempKW) {
								keywords[j + tempWeapBase->keyword.numKeywords] = tempKW;
							}
							j += 1;
						}
						if (j > 0) {
							tempWeapBase->keyword.numKeywords = tempWeapBase->keyword.numKeywords + j;
							tempWeapBase->keyword.keywords = keywords;
						}
					}
				}
				values.clear();


				// create custom data if needed
				if (bSaveData) {
					// Required Skill AV
					keyID = iniWeap.GetValue("Base", "sRequiredSkill", "none");
					requiredSkill = (ActorValueInfo*)ATShared::GetFormFromIdentifier(keyID);

					// crit failure table
					keyID = iniWeap.GetValue("Base", "sCritFailureTable", "none");
					critFailureType = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

					// Ammo Swapping animations
					keyID = iniWeap.GetValue("Base", "sAmmoSwapIdle_1P", "none");
					ammoSwapIdle_1P = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

					keyID = iniWeap.GetValue("Base", "sAmmoSwapIdle_3P", "none");
					ammoSwapIdle_3P = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

					ammoSwapAnimEvent = BSFixedString(iniWeap.GetValue("Base", "sAmmoSwapAnimEvent", ""));

					// Swappable Mods
					if (iniWeap.GetAllValues("Base", "sSwapModSlots", values)) {
						int modSlotCount = values.size();
						if (modSlotCount > 0) {
							CSimpleIni::TNamesDepend sections;
							CSimpleIni::TNamesDepend::const_iterator i = values.begin();
							for (; i != values.end(); ++i) {
								if (iniWeap.GetAllValues(i->pItem, "sMods", sections)) {
									if (sections.size() > 0) {
										ModSlot newSlot;
										newSlot.slotName = BSFixedString(iniWeap.GetValue(i->pItem, "sName", "none"));
										CSimpleIni::TNamesDepend::const_iterator j = sections.begin();
										for (; j != sections.end(); ++j) {
											ATSwappableMod newMod;
											keyID = iniWeap.GetValue(j->pItem, "sModID", "none");
											newMod.swapMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(keyID);

											if (newMod.swapMod) {
												keyID = iniWeap.GetValue(j->pItem, "sRequiredKW", "none");
												newMod.requiredKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

												keyID = iniWeap.GetValue(j->pItem, "sRequiredItem", "none");
												newMod.modItem = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

												
												newMod.bRequireModMisc = iniWeap.GetBoolValue(j->pItem, "bRequireModMisc", false);

												newSlot.swappableMods.Push(newMod);
											}
										}

										keyID = iniWeap.GetValue(i->pItem, "sSwapIdleAnim_1P", "none");
										newSlot.swapIdle_1P = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

										keyID = iniWeap.GetValue(i->pItem, "sSwapIdleAnim_3P", "none");
										newSlot.swapIdle_3P = (TESForm*)ATShared::GetFormFromIdentifier(keyID);

										newSlot.swapAnimEventName = BSFixedString(iniWeap.GetValue(i->pItem, "sSwapAnimEvent", ""));


										modSlots.Push(newSlot);
									}
								}
								sections.clear();
							}
						}
					}
					values.clear();

					// Damaged Mods
					if (iniWeap.GetAllValues("Base", "sDamagedModSlots", values)) {
						int modSlotCount = values.size();
						if (modSlotCount > 0) {
							CSimpleIni::TNamesDepend sections;
							CSimpleIni::TNamesDepend::const_iterator i = values.begin();
							for (; i != values.end(); ++i) {
								if (iniWeap.GetAllValues(i->pItem, "sMods", sections)) {
									if (sections.size() > 0) {
										ModSlot newSlot;
										newSlot.slotName = BSFixedString(iniWeap.GetValue(i->pItem, "sName", "none"));
										CSimpleIni::TNamesDepend::const_iterator j = sections.begin();
										for (; j != sections.end(); ++j) {
											ATSwappableMod newMod;
											keyID = iniWeap.GetValue(j->pItem, "sModID", "none");
											newMod.swapMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(keyID);

											if (newMod.swapMod) {
												keyID = iniWeap.GetValue(j->pItem, "sRequiredKW", "none");
												newMod.requiredKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

												newSlot.swappableMods.Push(newMod);
											}
										}
										damagedMods.Push(newSlot);
									}
								}
								sections.clear();
							}
						}
					}
					values.clear();

					// Magazines
					iReloadType = iniWeap.GetLongValue("Base", "iReloadType", -1);
					if (iReloadType > -1) {
						if (iniWeap.GetAllValues("Base", "sMagazines", values)) {
							int magsCount = values.size();
							if (magsCount > 0) {
								CSimpleIni::TNamesDepend::const_iterator i = values.begin();
								for (; i != values.end(); ++i) {
									DroppableMag newMag;
									keyID = iniWeap.GetValue(i->pItem, "sMagItem", "none");
									newMag.magItem = (TESObjectMISC*)ATShared::GetFormFromIdentifier(keyID);
									if (newMag.magItem) {
										keyID = iniWeap.GetValue(i->pItem, "sRequiredKW", "none");
										newMag.magKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

										magazines.Push(newMag);
									}
								}
							}
						}
					}
					values.clear();

					// Holsters
					if (iniWeap.GetAllValues("Base", "sHolsters", values)) {
						int holstersCount = values.size();
						if (holstersCount > 0) {
							CSimpleIni::TNamesDepend::const_iterator i = values.begin();
							for (; i != values.end(); ++i) {
								HolsterArmor newHolster;
								keyID = iniWeap.GetValue(i->pItem, "sArmorWeapon", "none");
								newHolster.armorWeapon = (TESObjectARMO*)ATShared::GetFormFromIdentifier(keyID);

								if (newHolster.armorWeapon) {
									keyID = iniWeap.GetValue(i->pItem, "sArmorHolster", "none");
									newHolster.armorHolster = (TESObjectARMO*)ATShared::GetFormFromIdentifier(keyID);

									keyID = iniWeap.GetValue(i->pItem, "sRequiredKW", "none");
									newHolster.requiredKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(keyID);

									newHolster.holsterName = BSFixedString(iniWeap.GetValue(i->pItem, "sName", " "));

									holsters.Push(newHolster);
								}
							}
						}
					}
					values.clear();

				}
				return true;
			}
		}
	}
	return false;
}


// ----------------------- ATDataStore:

bool ATDataStore::LoadData_CritEffectTables(const std::string & configDir)
{
	CSimpleIniA iniCritTables;
	iniCritTables.SetUnicode();
	iniCritTables.SetMultiKey(true);

	if (iniCritTables.LoadFile(configDir.c_str()) > -1) {
		CSimpleIni::TNamesDepend sections;
		iniCritTables.GetAllSections(sections);
		
		if (sections.size() > 0) {
			CSimpleIni::TNamesDepend values;
			_MESSAGE("    [Crit Effect Tables]:  %i", sections.size());

			CSimpleIni::TNamesDepend::const_iterator i = sections.begin();
			for (; i != sections.end(); ++i) {
				ATCritEffectTable newTable;
				const char *tableName = i->pItem;
				newTable.hudName = BSFixedString(iniCritTables.GetValue(tableName, "sName", " "));
				const char *keyID = iniCritTables.GetValue(tableName, "sCritKW", "none");
				newTable.objectID = ATShared::GetFormIDFromIdentifier(keyID);
				
				if (iniCritTables.GetAllValues(tableName, "sCritEffects", values)) {
					if (values.size() > 0) {
						CSimpleIni::TNamesDepend::const_iterator j = values.begin();
						// default/fallback crit effects:
						int rollAccum = -255;
						for (; j != values.end(); ++j) {
							ATCritEffect newCritEffect = ATShared::GetCritEffectFromIdentifier(j->pItem);
							newCritEffect.rollMin = rollAccum;
							rollAccum = newCritEffect.rollMax + 1;
							newTable.critEffects.Push(newCritEffect);
						}

						int foundIndex = GetCritTableIndex(newTable.objectID);
						if (foundIndex > -1) {
							CritEffectTables[foundIndex] = newTable;
							_MESSAGE("        #Override#: 0x%08X - %s", newTable.objectID, newTable.hudName.c_str());
						}
						else {
							CritEffectTables.Push(newTable);
							//_MESSAGE("        Loaded  : 0x%08X - %s", newTable.objectID, newTable.hudName.c_str());
						}
					}
				}
				values.clear();
			}
			sections.clear();
			return true;
		}
	}
	return false;
}


bool ATDataStore::LoadData_CritFailureTables(const std::string & configDir)
{
	CSimpleIniA iniCritTables;
	iniCritTables.SetUnicode();
	iniCritTables.SetMultiKey(true);

	if (iniCritTables.LoadFile(configDir.c_str()) > -1) {
		CSimpleIni::TNamesDepend sections;
		iniCritTables.GetAllSections(sections);

		if (sections.size() > 0) {
			CSimpleIni::TNamesDepend values;
			_MESSAGE("    Crit Failure Tables: %i", sections.size());

			CSimpleIni::TNamesDepend::const_iterator i = sections.begin();
			for (; i != sections.end(); ++i) {
				ATCritEffectTable newTable;
				const char *tableName = i->pItem;
				newTable.hudName = BSFixedString(iniCritTables.GetValue(tableName, "sName", " "));
				const char *keyID = iniCritTables.GetValue(tableName, "sCritKW", "none");
				newTable.objectID = ATShared::GetFormIDFromIdentifier(keyID);

				if (iniCritTables.GetAllValues(tableName, "sCritFailures", values)) {
					if (values.size() > 0) {
						CSimpleIni::TNamesDepend::const_iterator j = values.begin();
						int rollAccum = -255;
						for (; j != values.end(); ++j) {
							ATCritEffect newCritEffect = ATShared::GetCritEffectFromIdentifier(j->pItem);
							newCritEffect.rollMin = rollAccum;
							rollAccum = newCritEffect.rollMax + 1;
							newTable.critEffects.Push(newCritEffect);
						}

						int foundIndex = GetCritTableIndex(newTable.objectID);
						if (foundIndex > -1) {
							CritFailureTables[foundIndex] = newTable;
							_MESSAGE("        #Override#: 0x%08X - %s", newTable.objectID, newTable.hudName.c_str());
						}
						else {
							CritFailureTables.Push(newTable);
							//_MESSAGE("        Loaded  : 0x%08X - %s", newTable.objectID, newTable.hudName.c_str());
						}
					}
				}
				values.clear();
			}
			sections.clear();
			return true;
		}
	}
	return false;
}


UInt32 ATDataStore::LoadData_Weapon(const std::string & configDir, bool bSaveData, bool bLoadStatTweaks, bool bFollowerWeapons, bool bNPCsUseAmmo)
{
	ATWeapon newWeapon;
	if (newWeapon.ReadIni(configDir.c_str(), bSaveData, bLoadStatTweaks, bFollowerWeapons, bNPCsUseAmmo)) {
		if (bSaveData) {
			int foundIndex = ATShared::ATData.GetWeaponIndex(newWeapon.objectID);
			if (foundIndex > -1) {
				ATShared::ATData.Weapons[foundIndex] = newWeapon;
				_MESSAGE("        #Override#:        0x%08X - %s", newWeapon.objectID, newWeapon.hudName.c_str());
			}
			else {
				ATShared::ATData.Weapons.Push(newWeapon);
				//_MESSAGE("        Loaded  : 0x%08X - %s", newWeapon.objectID, newWeapon.hudName.c_str());
			}
		}
		return newWeapon.objectID;
	}
	return 0;
}


UInt32 ATDataStore::LoadData_Caliber(const std::string & configDir)
{
	ATCaliber caliberStore;
	if (caliberStore.ReadIni(configDir.c_str())) {
		UInt32 tempID = caliberStore.objectID;
		int foundIndex = ATShared::ATData.GetCaliberIndex(tempID);
		if (foundIndex > -1) {
			ATShared::ATData.Calibers[foundIndex] = caliberStore;
			_MESSAGE("        #Override#:        0x%08X - %s", caliberStore.objectID, caliberStore.hudName.c_str());
		}
		else {
			ATShared::ATData.Calibers.Push(caliberStore);
			//_MESSAGE("        Loaded  : 0x%08X - %s", caliberStore.objectID, caliberStore.hudName.c_str());
		}
		return tempID;
	}
	return 0;
}


bool ATDataStore::LoadData_Plugin(const std::string & configDir, bool bLoadStatTweaks, bool bLoadTurrets, bool bLoadThrownWeapons, bool bFollowerWeapons, bool bNPCsUseAmmo)
{
	std::string tempIniPath = configDir.c_str();
	CSimpleIniA iniIndex;
	CSimpleIni::TNamesDepend values;
	std::string tempLoadPath;
	UInt32 loadMsg = 0;

	iniIndex.SetUnicode();
	iniIndex.SetMultiKey(true);
	
	tempIniPath.append("\\Index.ini");

	if (iniIndex.LoadFile(tempIniPath.c_str()) > -1) {
		_MESSAGE("\n  Loading .ini: %s", tempIniPath.c_str());

		// Ammo
		if (iniIndex.GetAllValues("Calibers", "sCalibers", values)) {
			_MESSAGE("    [Calibers]:            %i", values.size());
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				tempLoadPath += configDir;
				tempLoadPath.append("\\Calibers\\");
				tempLoadPath.append(i->pItem);
				tempLoadPath.append(".ini");
				loadMsg = LoadData_Caliber(tempLoadPath);
				if (loadMsg <= 0) {
					_MESSAGE("      !Failed!:          %s", i->pItem);
					return false;
				}
				tempLoadPath.clear();
			}
		}
		values.clear();

		// Weapons:

		// ranged
		if (iniIndex.GetAllValues("Weapons", "sRangedWeapons", values)) {
			_MESSAGE("    [Ranged Weapons]:      %i", values.size());
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				tempLoadPath += configDir;
				tempLoadPath.append("\\Weapons\\Ranged\\");
				tempLoadPath.append(i->pItem);
				tempLoadPath.append(".ini");
				loadMsg = LoadData_Weapon(tempLoadPath, true, bLoadStatTweaks, bFollowerWeapons, bNPCsUseAmmo);
				if (loadMsg <= 0) {
					_MESSAGE("      !Failed!:  %s", i->pItem);
					return false;
				}
				tempLoadPath.clear();
			}
		}
		values.clear();

		// melee
		if (iniIndex.GetAllValues("Weapons", "sMeleeWeapons", values)) {
			_MESSAGE("    [Melee Weapons]:       %i", values.size());
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				tempLoadPath += configDir;
				tempLoadPath.append("\\Weapons\\Melee\\");
				tempLoadPath.append(i->pItem);
				tempLoadPath.append(".ini");
				loadMsg = LoadData_Weapon(tempLoadPath, true, bLoadStatTweaks, bFollowerWeapons, false);
				if (loadMsg <= 0) {
					_MESSAGE("      !!Failed:  %s", i->pItem);
					return false;
				}
				tempLoadPath.clear();
			}
		}
		values.clear();

		if (bLoadThrownWeapons) {
			if (iniIndex.GetAllValues("Weapons", "sThrownWeapons", values)) {
				_MESSAGE("    [Thrown Weapons]:      %i", values.size());
			}
			values.clear();
		}

		if (bLoadTurrets) {
			if (iniIndex.GetAllValues("Weapons", "sTurrets", values)) {
				_MESSAGE("    [Turrets]:             %i", values.size());
				CSimpleIni::TNamesDepend::const_iterator i = values.begin();
				for (; i != values.end(); ++i) {
					tempLoadPath += configDir;
					tempLoadPath.append("\\Weapons\\Turret\\");
					tempLoadPath.append(i->pItem);
					tempLoadPath.append(".ini");
					loadMsg = LoadData_Weapon(tempLoadPath, false, true, false, false);
					if (loadMsg <= 0) {
						_MESSAGE("      !!Failed:  %s", i->pItem);
						return false;
					}
					tempLoadPath.clear();
				}
			}
			values.clear();
		}

		// critical effect tables
		tempLoadPath += configDir;
		tempLoadPath.append("\\Misc\\CritEffectTables.ini");
		LoadData_CritEffectTables(tempLoadPath);
		tempLoadPath.clear();

		// critical failure tables
		tempLoadPath += configDir;
		tempLoadPath.append("\\Misc\\CritFailureTables.ini");
		LoadData_CritFailureTables(tempLoadPath);
		tempLoadPath.clear();

		return true;
	}
	tempIniPath.clear();
	return false;
}

bool ATDataStore::FinalizeData(bool bEnableLog)
{
	bool bCheckPassed = false;
	_MESSAGE("\nFinalizing data...\n");

	// limit array capacities to save some memory:
	Calibers.capacity = Calibers.count;
	Weapons.capacity = Weapons.count;
	CritEffectTables.capacity = CritEffectTables.count;
	CritFailureTables.capacity = CritFailureTables.count;

	if (bEnableLog)
		_MESSAGE("  [Calibers] (%i):", Calibers.count);

	for (UInt32 i = 0; i < Calibers.count; i++) {
		ATCaliber curCaliber = Calibers[i];
		curCaliber.ammoTypes.capacity = curCaliber.ammoTypes.count;
		for (UInt32 j = 0; j < curCaliber.ammoTypes.count; j++) {
			ATCaliber::AmmoType tempAmmoType = curCaliber.ammoTypes[j];
			tempAmmoType.projectiles.capacity = tempAmmoType.projectiles.count;
		}
		
		if (bEnableLog)
			_MESSAGE("    0x%08X - %s : %i ammo types", curCaliber.objectID, curCaliber.hudName.c_str(), curCaliber.ammoTypes.count);
	}

	if (bEnableLog)
		_MESSAGE("\n  [Crit Effect Tables] (%i):", CritEffectTables.count);

	for (UInt32 i = 0; i < CritEffectTables.count; i++) {
		ATCritEffectTable curCET = CritEffectTables[i];
		//curCET.critEffects.capacity = curCET.critEffects.count;
		if (bEnableLog) {
			_MESSAGE("    0x%08X - %s : %i effects", curCET.objectID, curCET.hudName.c_str(), curCET.critEffects.count);
			for (UInt32 j = 0; j < curCET.critEffects.count; j++) {
				ATCritEffect tempSI = curCET.critEffects[j];
				_MESSAGE("        %i: 0x%08X", j, (tempSI.critSpell != nullptr) ? tempSI.critSpell->formID : 0);
			}
		}
	}

	if (bEnableLog)
		_MESSAGE("\n  [Crit Failure Tables] (%i):", CritFailureTables.count);

	for (UInt32 i = 0; i < CritFailureTables.count; i++) {
		ATCritEffectTable curCFT = CritFailureTables[i];
		//curCFT.critEffects.capacity = curCFT.critEffects.count;
		if (bEnableLog) {
			_MESSAGE("    0x%08X - %s : %i effects", curCFT.objectID, curCFT.hudName.c_str(), curCFT.critEffects.count);
			for (UInt32 j = 0; j < curCFT.critEffects.count; j++) {
				ATCritEffect tempSI = curCFT.critEffects[j];
				_MESSAGE("        %i: 0x%08X", j, (tempSI.critSpell != nullptr) ? tempSI.critSpell->formID : 0);
			}
		}
	}

	if (bEnableLog)
		_MESSAGE("\n  [Weapons] (%i):", Weapons.count);

	for (UInt32 i = 0; i < Weapons.count; i++) {
		ATWeapon curWeap = Weapons[i];
		curWeap.modSlots.capacity = curWeap.modSlots.count;
		curWeap.damagedMods.capacity = curWeap.damagedMods.count;
		curWeap.magazines.capacity = curWeap.magazines.count;
		curWeap.holsters.capacity = curWeap.holsters.count;

		if (bEnableLog)
			_MESSAGE("    0x%08X - %s : %i mod slots, %i damage slots, %i magazines, %i holsters", curWeap.objectID, curWeap.hudName.c_str(), curWeap.modSlots.count, curWeap.damagedMods.count, curWeap.magazines.count, curWeap.holsters.count);
	}

	bCheckPassed = true;

	return bCheckPassed;
}

void ATConfig::LoadGameData()
{
	// seed the rng
	srand(time(NULL));

	CSimpleIniA iniMain;
	iniMain.SetUnicode();

	// load global config
	// - things that require a game restart to kick in
	if (iniMain.LoadFile(".\\Data\\F4SE\\Plugins\\AmmoTweaks.ini") > -1) {

		// load AmmoTweaks configuration data
		// - anything not handled by the base game (caliber lists, swappable mods slots, crit tables, etc.)
		const char *configDataPath = iniMain.GetValue("General", "sConfigFoldersPath", "");
		if (configDataPath != "") {
			bool bLoadStatTweaks = iniMain.GetBoolValue("Weapons", "bLoadStatTweaks", false);
			bool bLoadTurrets = iniMain.GetBoolValue("Weapons", "bLoadTurrets", false);
			bool bLoadThrownWeapons = iniMain.GetBoolValue("Weapons", "bLoadThrownWeapons", false);
			bool bFollowerWeapons = iniMain.GetBoolValue("Weapons", "bFollowerWeapons", false);
			bool bNPCsUseAmmo = iniMain.GetBoolValue("Weapons", "bNPCsUseAmmo", false);

			_MESSAGE("\nLoading Framework Data...");

			// try to load base framework data first
			std::string tempDirStr;
			const char *tempModName = "_Base";
			tempDirStr.append(configDataPath);
			tempDirStr.append(tempModName);
			if (ATShared::ATData.LoadData_Plugin(tempDirStr, bLoadStatTweaks, bLoadTurrets, bLoadThrownWeapons, bFollowerWeapons, bNPCsUseAmmo)) {
				// load per-plugin data
				for (UInt32 i = 0; i < (*g_dataHandler)->modList.loadedMods.count; i++) {
					tempDirStr.clear();
					tempModName = (*g_dataHandler)->modList.loadedMods[i]->name;
					tempDirStr.append(configDataPath);
					tempDirStr.append(tempModName);
					ATShared::ATData.LoadData_Plugin(tempDirStr, bLoadStatTweaks, bLoadTurrets, bLoadThrownWeapons, bFollowerWeapons, bNPCsUseAmmo);
				}

				// finalize lists
				if (ATShared::ATData.FinalizeData(true))
					_MESSAGE("\nData check passed.\n");
				else
					_MESSAGE("\n!!Data check failed.\n");
			}
			else {
				_MESSAGE("\n!!Unable to load _Base config.");
			}
			tempDirStr.clear();


			// load MCM config stuff last
			// - settings that can be edited while the game is running
			//const char *configMCMPath = ".\\Data\\MCM\\Settings\\AmmoTweaks.ini";


		}
		else {
			_MESSAGE("\n!!Unable to load config path.");
		}
	}

}


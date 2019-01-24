#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "SimpleIni/SimpleIni.h"
#include <string>

#include "ATConfigLoad.h"


// Config file read operations:
namespace ATFileIO
{
	// loads gun changes from ini
	void LoadWeaponBase_Gun(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (ATShared::SharedData) {
			if (tempWeapBase) {

				TESObjectWEAP::InstanceData *instanceData = ATShared::GetInstanceData_WeapForm(tempWeapBase);
				if (instanceData) {
					// av modifiers
					int avCount = iniWeap->GetLongValue(weapID, "iNumActorValues", 0);
					if (avCount > 0) {
						TBO_InstanceData::ValueModifier tempAVypeMod;
						ActorValueInfo *tempAV = nullptr;
						UInt32 tempAmount = 0;

						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}

						for (UInt8 i = 0; i < avCount; i++) {
							std::string *avKeyStr = new std::string();
							avKeyStr->append("sActorValue");
							avKeyStr->append(std::to_string(i));
							const char *avID = iniWeap->GetValue(weapID, avKeyStr->c_str(), "none");
							tempAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(avID);

							if (tempAV) {
								tempAVypeMod.avInfo = tempAV;
								avKeyStr->clear();
								avKeyStr->append("iActorValueAmt");
								avKeyStr->append(std::to_string(i));
								tempAmount = iniWeap->GetLongValue(weapID, avKeyStr->c_str(), 0);
								if (tempAmount > 0) {
									tempAVypeMod.unk08 = tempAmount;
									instanceData->modifiers->Push(tempAVypeMod);
								}
							}
							avKeyStr->clear();
						}
					}

					// enchantments
					int enchCount = iniWeap->GetLongValue(weapID, "iNumEnchantments", 0);
					if (enchCount > 0) {
						EnchantmentItem *tempEnch = nullptr;

						if (!instanceData->enchantments) {
							instanceData->enchantments = new tArray<EnchantmentItem*>();
						}

						for (UInt8 i = 0; i < enchCount; i++) {
							std::string *enchKeyStr = new std::string();
							enchKeyStr->append("sEnchantment");
							enchKeyStr->append(std::to_string(i));
							const char *enchID = iniWeap->GetValue(weapID, enchKeyStr->c_str(), "none");
							tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
							if (tempEnch) {
								instanceData->enchantments->Push(tempEnch);
							}
						}
					}

					// caliber keywords
					int kwCount = iniWeap->GetLongValue(weapID, "iNumKeywords", 0);
					if (kwCount > 0) {
						BGSKeyword *tempKW = nullptr;
						BGSKeyword **keywords = new BGSKeyword*[tempWeapBase->keyword.numKeywords + kwCount];

						for (UInt8 i = 0; i < tempWeapBase->keyword.numKeywords; i++) {
							keywords[i] = tempWeapBase->keyword.keywords[i];
						}

						for (UInt8 i = 0; i < kwCount; i++) {
							std::string *kwKeyStr = new std::string();
							kwKeyStr->append("sKeyword");
							kwKeyStr->append(std::to_string(i));
							const char *kwID = iniWeap->GetValue(weapID, kwKeyStr->c_str(), "none");
							tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
							if (tempKW) {
								keywords[i + tempWeapBase->keyword.numKeywords] = tempKW;
							}
						}

						tempWeapBase->keyword.numKeywords = tempWeapBase->keyword.numKeywords + kwCount;
						tempWeapBase->keyword.keywords = keywords;

					}

					//flags:

					// NPCsUseAmmo
					if (instanceData->flags) {
						//NPCsUseAmmo
						if (ATShared::SharedData->bNPCsUseAmmo)
							instanceData->flags |= 0x0000002;
						else
							instanceData->flags &= 0x0000002;
					}

				}
			}
		}
	}

	// loads turret/robot weapon changes from ini
	void LoadWeaponBase_TurretRobot(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (ATShared::SharedData) {
			if (tempWeapBase) {
				TESObjectWEAP::InstanceData *instanceData = ATShared::GetInstanceData_WeapForm(tempWeapBase);
				if (instanceData) {
					UInt16 tmpDamage = iniWeap->GetLongValue("Base", "iDamage", 0);
					UInt32 tmpAP = iniWeap->GetLongValue("Base", "iArmorPenetration", 0);
					float tmpRangeMin = iniWeap->GetDoubleValue("Base", "fRangeMin", 0.0);

					float tmpRangeMax = iniWeap->GetDoubleValue("Base", "fRangeMax", 0.0);
					float tmpOoRMult = iniWeap->GetDoubleValue("Base", "fOoRMult", 0.0);
					float tmpCritChance = iniWeap->GetDoubleValue("Base", "fCritChance", 0.0);

					const char *tmpAmmoID = iniWeap->GetValue("Base", "sAmmo", "none");
					const char *tmpProjID = iniWeap->GetValue("Base", "sProjectile", "none");
					const char *tmpNPCListID = iniWeap->GetValue("Base", "sAmmoLootList", "none");
					const char *tmpImpactID = iniWeap->GetValue("Base", "sImpactDataSet", "none");

					TESAmmo *tmpAmmoForm = (TESAmmo*)ATShared::GetFormFromIdentifier(tmpAmmoID);
					if (tmpAmmoForm) {
						instanceData->ammo = tmpAmmoForm;
						
					}

					if (instanceData->firingData) {
						BGSProjectile *tmpProjForm = (BGSProjectile*)ATShared::GetFormFromIdentifier(tmpProjID);
						if (tmpProjForm) {
							instanceData->firingData->projectileOverride = tmpProjForm;
						}
					}

					TESLevItem *tmpNPCListForm = (TESLevItem*)ATShared::GetFormFromIdentifier(tmpNPCListID);
					if (tmpNPCListForm) {
						instanceData->addAmmoList = tmpNPCListForm;
					}

					BGSImpactDataSet *tmpImpactForm = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(tmpImpactID);
					if (tmpImpactForm) {
						instanceData->unk58 = tmpImpactForm;
					}

					instanceData->baseDamage = tmpDamage;
					instanceData->minRange = tmpRangeMin;
					instanceData->maxRange = tmpRangeMax;
					instanceData->critDamageMult = tmpCritChance;
					instanceData->outOfRangeMultiplier = tmpOoRMult;
					
					// av modifiers
					TBO_InstanceData::ValueModifier tempAVypeMod;
					ActorValueInfo *tempAV = nullptr;
					UInt32 tempAmount = 0;
					// armor penetration
					if (tmpAP > 0x0) {
						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}
						tempAVypeMod.avInfo = ATShared::SharedData->avArmorPenetration;
						tempAVypeMod.unk08 = tmpAP;
						instanceData->modifiers->Push(tempAVypeMod);
					}
					// critical chance
					if (tmpCritChance > 0.0) {
						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}
						tempAVypeMod.avInfo = ATShared::SharedData->avCriticalChance;
						tempAVypeMod.unk08 = UInt32(tmpCritChance);
						instanceData->modifiers->Push(tempAVypeMod);
					}
					
					// enchantments
					int enchCount = iniWeap->GetLongValue(weapID, "iNumEnchantments", 0);
					if (enchCount > 0) {
						EnchantmentItem *tempEnch = nullptr;

						if (!instanceData->enchantments) {
							instanceData->enchantments = new tArray<EnchantmentItem*>();
						}

						for (UInt8 i = 0; i < enchCount; i++) {
							std::string *enchKeyStr = new std::string();
							enchKeyStr->append("sEnchantment");
							enchKeyStr->append(std::to_string(i));
							const char *enchID = iniWeap->GetValue(weapID, enchKeyStr->c_str(), "none");
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
						if (ATShared::SharedData->bNPCsUseAmmo_Turret)
							instanceData->flags |= 0x0000002;
						else
							instanceData->flags &= 0x0000002;
					}

				}
			}
		}
	}

	// loads thrown weapon changes from ini
	void LoadWeaponBase_Thrown(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (ATShared::SharedData) {
			if (tempWeapBase) {
				TESObjectWEAP::InstanceData *instanceData = ATShared::GetInstanceData_WeapForm(tempWeapBase);
				if (instanceData) {
					// projectile
					const char *tmpProjID = iniWeap->GetValue(weapID, "sProjectile", "none");
					if (instanceData->firingData) {
						BGSProjectile *tmpProjForm = (BGSProjectile*)ATShared::GetFormFromIdentifier(tmpProjID);
						if (tmpProjForm) {
							instanceData->firingData->projectileOverride = tmpProjForm;
						}
					}

					//flags:
					// NPCsUseAmmo
					if (instanceData->flags) {
						//NPCsUseAmmo
						if (ATShared::SharedData->bNPCsUseAmmo_Thrown)
							instanceData->flags |= 0x0000002;
						else
							instanceData->flags &= 0x0000002;
					}
				}
			}
		}
	}


}


bool ATWeapon::FillFromIni(const char* weaponFileName, const char* weaponID)
{
	bool bRetVal = false;

	if (ATShared::SharedData) {
		CSimpleIniA iniWeap;
		iniWeap.SetUnicode();

		if (iniWeap.LoadFile(weaponFileName) > -1) {
			weaponFormID = ATShared::GetFormIDFromIdentifier(weaponID);
			
			MaxCNDBase = iniWeap.GetLongValue("Base", "iMaxCNDBase", 0);
			HUDIconIndex = iniWeap.GetLongValue("Base", "iHUDIcon", 0);
			HUDIconIndexAlt = iniWeap.GetLongValue("Base", "iHUDIconAlt", 0);

			BGSKeyword *tempKW = nullptr;

			int maxCNDModsCount = iniWeap.GetLongValue("ValueModifiers", "iNumCNDMods", 0);
			if (maxCNDModsCount > 0) {
				for (UInt8 i = 0; i < maxCNDModsCount; i++) {
					std::string *kwKeyStr = new std::string();
					kwKeyStr->append("sCNDModKW");
					kwKeyStr->append(std::to_string(i));
					const char *kwID = iniWeap.GetValue("ValueModifiers", kwKeyStr->c_str(), "none");
					tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
					if (tempKW) {
						kwKeyStr->clear();
						kwKeyStr->append("fCNDModAdd");
						kwKeyStr->append(std::to_string(i));
						float cndModAmt = iniWeap.GetDoubleValue("ValueModifiers", kwKeyStr->c_str(), 0.0);
						if (cndModAmt > 0.0) {
							ATWeapVarMod tempVarMod;
							tempVarMod.modKW = tempKW;
							tempVarMod.modMultAdd = cndModAmt;
							MaxCNDMods.Push(tempVarMod);
						}
					}
				}
			}
			tempKW = nullptr;
			int maxWearModsCount = iniWeap.GetLongValue("ValueModifiers", "iNumWearMods", 0);
			for (UInt8 i = 0; i < maxWearModsCount; i++) {
				std::string *kwKeyStr = new std::string();
				kwKeyStr->append("sWearModKW");
				kwKeyStr->append(std::to_string(i));
				const char *kwID = iniWeap.GetValue("ValueModifiers", kwKeyStr->c_str(), "none");
				tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
				if (tempKW) {
					kwKeyStr->clear();
					kwKeyStr->append("fWearModAdd");
					kwKeyStr->append(std::to_string(i));
					float cndModAmt = iniWeap.GetDoubleValue("ValueModifiers", kwKeyStr->c_str(), 0.0);
					if (cndModAmt > 0.0) {
						ATWeapVarMod tempVarMod;
						tempVarMod.modKW = tempKW;
						tempVarMod.modMultAdd = cndModAmt;
						WearMods.Push(tempVarMod);
					}
				}
			}

			BGSMod::Attachment::Mod *tempModForm = nullptr;
			tempKW = nullptr;
			int numMuzzleMods = iniWeap.GetLongValue("SwappableMods", "iNumMuzzles", 0);
			if (numMuzzleMods > 0) {
				for (UInt8 i = 0; i < numMuzzleMods; i++) {
					std::string *kwKeyStr = new std::string();
					kwKeyStr->append("sMuzzleMod");
					kwKeyStr->append(std::to_string(i));
					const char *modID = iniWeap.GetValue("SwappableMods", kwKeyStr->c_str(), "none");
					tempModForm = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(modID);
					if (tempModForm) {
						ATWeaponModSwap newModSwap;
						newModSwap.swapMod = tempModForm;
						kwKeyStr->clear();
						kwKeyStr->append("sMuzzleKW");
						kwKeyStr->append(std::to_string(i));
						const char *kwID = iniWeap.GetValue("SwappableMods", kwKeyStr->c_str(), "none");
						tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
						if (tempKW) {
							newModSwap.swapKWExclude = tempKW;
						}
						MuzzleMods.Push(newModSwap);
					}
				}
			}
			
			if (weaponFormID > 0) {
				ATShared::SharedData->index_ATWeapons.Push(weaponFormID);
				bRetVal = true;
			}
		}
	}
	return bRetVal;
}


bool ATCaliber::FillFromIni(const char* caliberFileName)
{
	bool bRetVal = false;
	CSimpleIniA iniAmmo;
	iniAmmo.SetUnicode();

	if (iniAmmo.LoadFile(caliberFileName) > -1) {
		caliberName = new std::string();
		caliberName->append(iniAmmo.GetValue("Caliber", "sName", "none"));

		const char *tempCalNameKWID = iniAmmo.GetValue("Caliber", "sCaliberID", "none");
		kwCaliberID = (BGSKeyword*)ATShared::GetFormFromIdentifier(tempCalNameKWID);

		tempCalNameKWID = iniAmmo.GetValue("Caliber", "sCaliberID_AWKCR", "none");
		kwCaliberID_AWKCR = (BGSKeyword*)ATShared::GetFormFromIdentifier(tempCalNameKWID);

		const char *tempCaliberModKW = iniAmmo.GetValue("Caliber", "sCaliberModID", "none");
		caliberMod = (BGSMod::Attachment::Mod*)ATShared::GetFormFromIdentifier(tempCaliberModKW);

		iArmorPenetration = iniAmmo.GetLongValue("Caliber", "iArmorPenetration", 0);
		fWearMult = iniAmmo.GetDoubleValue("Caliber", "fWearMult", 1.0);
		fCritFailMult = iniAmmo.GetDoubleValue("Caliber", "fCritFailMult", 1.0);


		// Projectiles
		UInt8 projectilesCount = iniAmmo.GetLongValue("Caliber", "iNumProjectiles", 0);
		for (UInt8 j = 0; j < projectilesCount; j++) {
			std::string projectileIndex = "sProjectile";
			projectileIndex.append(std::to_string(j));
			projectiles.Push((BGSProjectile*)ATShared::GetFormFromIdentifier(iniAmmo.GetValue("Caliber", projectileIndex.c_str(), "none")));
			projectileIndex.clear();
		}

		// ImpactDataSets
		UInt8 impactsCount = iniAmmo.GetLongValue("Caliber", "iNumImpacts", 0);
		for (UInt8 j = 0; j < impactsCount; j++) {
			std::string impactIndex = "sImpact";
			impactIndex.append(std::to_string(j));
			impacts.Push((BGSImpactDataSet*)ATShared::GetFormFromIdentifier(iniAmmo.GetValue("Caliber", impactIndex.c_str(), "none")));
			impactIndex.clear();
		}

		// ammo subtypes
		int numSubTypes = iniAmmo.GetLongValue("Caliber", "iNumSubTypes", 0);
		if (numSubTypes > 0) {
			for (UInt8 i = 0; i < numSubTypes; i++) {
				ATAmmoType newType;
				
				std::string subtypeIndex = "sSubtype";
				subtypeIndex.append(std::to_string(i));
				const char *subTypeID = iniAmmo.GetValue("Caliber", subtypeIndex.c_str(), "none");
				
				newType.AmmoName = new std::string();
				newType.AmmoName->append(subTypeID);

				const char *ammoFormID = iniAmmo.GetValue(subTypeID, "sAmmoID", "none");
				TESAmmo *newAmmo = (TESAmmo*)ATShared::GetFormFromIdentifier(ammoFormID);
				if (newAmmo) {
					newType.ammoForm = newAmmo;
					if (ATShared::SharedData->bEditAmmoNames) {
						const char* tmpAmmoItemName = iniAmmo.GetValue(subTypeID, "sAmmoName", "");
						if (tmpAmmoItemName != "") {
							BSFixedString *tmpFullName = &newAmmo->fullName.name;
							CALL_MEMBER_FN(tmpFullName, Set)(tmpAmmoItemName);
						}
					}
				}

				newType.fDamageMult = iniAmmo.GetDoubleValue(subTypeID, "fDamageMult", 1.0);
				newType.fRangeMult = iniAmmo.GetDoubleValue(subTypeID, "fRangeMult", 1.0);

				newType.iArmorPenetration = iniAmmo.GetLongValue(subTypeID, "iArmorPenetration", 0);

				newType.fRecoilMult = iniAmmo.GetDoubleValue(subTypeID, "fRecoilMult", 1.0);
				newType.fCoFMult = iniAmmo.GetDoubleValue(subTypeID, "fCoFMult", 1.0);

				// suppressor/legendary explosive/muzzle brake vars:
				newType.projectileStd = iniAmmo.GetLongValue(subTypeID, "iProjectileStd", 0);
				newType.projectileSup = iniAmmo.GetLongValue(subTypeID, "iProjectileSup", 0);
				newType.projectileExp = iniAmmo.GetLongValue(subTypeID, "iProjectileExp", 0);
				newType.projectileBrk = iniAmmo.GetLongValue(subTypeID, "iProjectileBrk", 0);

				newType.iImpactIndex = iniAmmo.GetLongValue(subTypeID, "iImpactIndex", 0);

				newType.fWearMult = iniAmmo.GetDoubleValue(subTypeID, "fWearMult", 1.0);
				
				newType.iHitEffect = iniAmmo.GetLongValue(subTypeID, "iHitEffect", 3);
				
				newType.iSoundLevel = iniAmmo.GetLongValue(subTypeID, "iNoiseLevel", 1);
				newType.iSoundLevelSup = iniAmmo.GetLongValue(subTypeID, "iNoiseLevelSup", 0);
				newType.iSoundLevelExp = iniAmmo.GetLongValue(subTypeID, "iNoiseLevelExp", 3);

				newType.fCritChanceMult = iniAmmo.GetDoubleValue(subTypeID, "fCritMult", 1.0);
				newType.fCritDmgMult = newType.fCritChanceMult;
				newType.fCritFailMult = iniAmmo.GetDoubleValue(subTypeID, "fCritFailMult", 1.0);
				newType.iCritEffect = iniAmmo.GetLongValue(subTypeID, "iCritEffectTable", 0);


				// enchantments
				int enchCount = iniAmmo.GetLongValue(subTypeID, "iNumEnchantments", 0);
				if (enchCount > 0) {
					EnchantmentItem *tempEnch = nullptr;
					for (UInt8 j = 0; j < enchCount; j++) {
						std::string *enchKeyStr = new std::string();
						enchKeyStr->append("sEnchantment");
						enchKeyStr->append(std::to_string(j));
						const char *enchID = iniAmmo.GetValue(subTypeID, enchKeyStr->c_str(), "none");
						tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
						if (tempEnch) {
							newType.enchantments.Push(tempEnch);
						}
					}
				}

				ammoTypes.Push(newType);

				subtypeIndex.clear();
			}
		}

		bRetVal = true;
	}

	iniAmmo.Reset();
	
	return bRetVal;
}


namespace ATConfig
{
	

	// -------------------- Load ------------------------------------
	
	void LoadData_Ammo(const char* configDir)
	{
		
		const char *tempModName = "", *finalDirName = "", *ammoID = "none";

		std::string configAmmoString = "";
		std::string configModDirString = "";
		std::string configSection = "";

		UInt32 caliberCount = 0;

		CSimpleIniA iniIndex;
		iniIndex.SetUnicode();

		for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
			configAmmoString.clear();
			configAmmoString.append(ATShared::SharedData->configPath_AmmoIndex);
			configAmmoString.append(tempModName);
			configAmmoString.append(".ini");

			if (iniIndex.LoadFile(configAmmoString.c_str()) == 0) {
				caliberCount = iniIndex.GetLongValue("Calibers", "numCalibers", 0);
				if (caliberCount > 0) {
					for (UInt8 i = 0; i < caliberCount; i++) {
						configSection.clear();
						configSection.append("Caliber");
						configSection.append(std::to_string(i));
						ammoID = iniIndex.GetValue("Calibers", configSection.c_str(), "none");

						if (ammoID != "none") {
							configModDirString.clear();
							configModDirString.append(configDir);
							configModDirString.append(tempModName);
							configModDirString.append("\\");
							configModDirString.append(ammoID);
							configModDirString.append(".ini");
							finalDirName = configModDirString.c_str();

							ATCaliber *caliberStore = new ATCaliber();
							if (caliberStore->FillFromIni(finalDirName)) {
								ATShared::SharedData->g_ATCalibers.Push(caliberStore);
								ATShared::SharedData->index_ATCalibersEquipped.Push(caliberStore->kwCaliberID);
								_MESSAGE("Loaded Caliber %s - Name: %s", finalDirName, caliberStore->caliberName);
							}
							else {
								_MESSAGE("Failed to load Caliber %s", finalDirName);
							}
						}

					}
				}
			}
			iniIndex.Reset();
		}

	}


	void LoadData_Melee(const char* configDir)
	{
		// speed
		// stagger
		// firemodes
		// maxcnd
		// crit%
		// critfail%
		// apcost
	}

	void LoadData_Thrown(const char* configDir)
	{
		// projectile
		// 
	}


	void LoadData_Weapons(const char* configDir)
	{
		if (ATShared::SharedData) {
			const char *tempModName = "", *finalDirName = "", *weaponName = "", *weaponID = "";

			std::string configWeaponsString = "";
			std::string configModDirString = "";
			std::string configSection = "";

			UInt32 weaponCount = 0;
			int weapTypeIndex = -1;

			CSimpleIniA iniIndex;
			iniIndex.SetUnicode();

			CSimpleIniA iniWeap;
			iniWeap.SetUnicode();

			for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
				tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
				configWeaponsString.clear();
				configWeaponsString.append(".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Weapons\\");
				configWeaponsString.append(tempModName);
				configWeaponsString.append("\\_Index.ini");

				if (iniIndex.LoadFile(configWeaponsString.c_str()) > -1) {
					weaponCount = iniIndex.GetLongValue("Weapons", "iNumWeapons", 0);
					if (weaponCount > 0) {
						for (UInt8 i = 0; i < weaponCount; i++) {
							configSection.clear();
							configSection.append("sWeapon");
							configSection.append(std::to_string(i));
							weaponName = iniIndex.GetValue("Weapons", configSection.c_str(), "");

							if (weaponName != "") {
								configModDirString.clear();
								configModDirString.append(".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Weapons\\");
								configModDirString.append(tempModName);
								configModDirString.append("\\");
								configModDirString.append(weaponName);
								configModDirString.append(".ini");
								finalDirName = configModDirString.c_str();
								if (iniWeap.LoadFile(finalDirName) > -1) {
									weaponID = iniWeap.GetValue("Base", "sWeapID", "none");
									weapTypeIndex = iniWeap.GetLongValue("Base", "iWeaponType", -1);

									if (weapTypeIndex == 0) {
										// melee:
										//ATShared::SharedData->g_ATWeapons.Push(newWeapon);
									}
									else if (weapTypeIndex == 1) {
										// guns:
										ATFileIO::LoadWeaponBase_Gun(&iniWeap, weaponID);
										ATWeapon *newWeapon = new ATWeapon();
										newWeapon->FillFromIni(finalDirName, weaponID);
										ATShared::SharedData->g_ATWeapons.Push(newWeapon);
									}
									else if (weapTypeIndex == 2) {
										// thrown:
										//ATShared::SharedData->g_ATWeapons.Push(newWeapon);
									}
									else if (weapTypeIndex == 3) {
										// turrets/robots:
										ATFileIO::LoadWeaponBase_TurretRobot(&iniWeap, weaponID);
									}
								}
								else {
									_MESSAGE("weapon type %s not found", weaponName);
								}
							}
						}
						iniWeap.Reset();
					}
				}
				iniIndex.Reset();
			}

			configWeaponsString.clear();
			configModDirString.clear();
			configSection.clear();
		}
	}


}



void ATConfig::EditGameData()
{
	ATShared::SharedData = new ATSharedData();

	//		Keywords:

	// dn_at_HasMuzzle_Amplifier
	ATShared::SharedData->kwMuzAmplifier = (BGSKeyword*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|0049A2");

	// s_30_Auto
	ATShared::SharedData->kwSoundAuto = (BGSKeyword*)LookupFormByID(0x000A191C);
	// at_kw_SoundDummy_Automatic
	ATShared::SharedData->kwSoundAutoDisabled = (BGSKeyword*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001BF9");
	// s_20_Silenced
	ATShared::SharedData->kwSoundSilenced = (BGSKeyword*)LookupFormByID(0x00054A67);
	// at_kw_SoundDummy_Silenced
	ATShared::SharedData->kwSoundSilencedDisabled = (BGSKeyword*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001BF8");

	//		ActorValues:

	// at_av_WpnCNDMax
	ATShared::SharedData->avMaxCondition = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|000AFC");
	
	// at_av_HUDIcon_Weapon
	ATShared::SharedData->avHUDIcon_Weapon = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|000BC9");
	// at_av_WpnSkillReq
	ATShared::SharedData->avSkillReq = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001DF2");
	// at_av_WpnHasInstance
	ATShared::SharedData->avHasInstance = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001C2A");

	// ArmorPenetration
	ATShared::SharedData->avArmorPenetration = (ActorValueInfo*)LookupFormByID(0x00097341);
	// CriticalChance
	ATShared::SharedData->avCriticalChance = (ActorValueInfo*)LookupFormByID(0x000002DD);

	// used as a multiplier for the RNG seed to work around weapons modified at the same time
	ATShared::SharedData->numProcessedWeaps = 1;
	
	//		ini Settings:

	CSimpleIniA mainINI;
	mainINI.SetUnicode();
	mainINI.LoadFile(ATShared::SharedData->configPath_MainINI);

	ATShared::SharedData->bEditAmmoNames = mainINI.GetBoolValue("AmmoTweaks", "bEditAmmoNames", false);
	
	bool bLoadWeaponData = mainINI.GetBoolValue("WeaponTweaks", "bEnable", false);
	if (bLoadWeaponData) {
		ATShared::SharedData->bNPCsUseAmmo = mainINI.GetBoolValue("WeaponTweaks", "bNPCsUseAmmo", false);
		ATShared::SharedData->bNPCsUseAmmo_Thrown = mainINI.GetBoolValue("WeaponTweaks", "bNPCsUseAmmo_Thrown", false);
		ATShared::SharedData->bNPCsUseAmmo_Turret = mainINI.GetBoolValue("WeaponTweaks", "bNPCsUseAmmo_Turret", false);

		ATShared::SharedData->bUseSingleFireAutoSounds = mainINI.GetBoolValue("WeaponTweaks", "bSingleFireAutoSounds", false);

		ATShared::SharedData->bDisableRecoilSpringForce = mainINI.GetBoolValue("WeaponTweaks", "bDisableRecoilSpringForce", false);

		ATShared::SharedData->bTwoShotLegendaryTweak = mainINI.GetBoolValue("WeaponTweaks", "bLegTwoShotDoubleShot", false);
	}

	bool bLoadArmorData = mainINI.GetBoolValue("ArmorTweaks", "bEnable", false);

	bool bLoadResistData = mainINI.GetBoolValue("ResistTweaks", "bEnable", false);

	ATShared::SharedData->bEnableSkillRequirements = mainINI.GetBoolValue("SkillRequirements", "bEnable", false);
	ATShared::SharedData->fSkillReq_MaxWeight = mainINI.GetDoubleValue("SkillRequirements", "fSkillReq_MaxWeight", 30.0);
	ATShared::SharedData->fSkillReq_MinReq = mainINI.GetDoubleValue("SkillRequirements", "fSkillReq_MinReq", 2.0);
	ATShared::SharedData->fSkillReq_MaxReq = mainINI.GetDoubleValue("SkillRequirements", "fSkillReq_MaxReq", 12.0);


	mainINI.Reset();


	LoadData_Ammo(ATShared::SharedData->configPath_Ammo);

	LoadData_Weapons(ATShared::SharedData->configPath_Weapons);
}


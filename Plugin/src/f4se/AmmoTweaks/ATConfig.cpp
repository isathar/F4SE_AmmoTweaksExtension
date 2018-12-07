#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "SimpleIni/SimpleIni.h"
#include <string>

#include "ATConfig.h"


// Config file read/write operations:
namespace ATFileIO
{
	// saves a single weapon base form's stats to ini
	void SaveWeaponBase_Gun(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (tempWeapBase) {
			TESObjectWEAP::InstanceData *instanceData = ATShared::GetInstanceData_WeapForm(tempWeapBase);
			if (instanceData) {
				// read variables:
				const char* weapName = tempWeapBase->GetFullName();
				std::string sectionComment;
				sectionComment.append(";---------------------------- ");
				sectionComment.append(weapName);

				//flags:
				//iniWeap->SetBoolValue(weapID, "NPCsUseAmmo", weapFlag_NPCsUseAmmo, "; Flags:");

				// av modifiers
				if (instanceData->modifiers) {
					iniWeap->SetLongValue(weapID, "numActorValues", instanceData->modifiers->count, "; Actor Values:");
					if (instanceData->modifiers->count > 0) {
						TBO_InstanceData::ValueModifier tempAVMod;
						for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
							if (instanceData->modifiers->GetNthItem(i, tempAVMod)) {
								if (tempAVMod.avInfo) {
									std::string *tempAVKey = new std::string();
									tempAVKey->append("ActorValue");
									tempAVKey->append(std::to_string(i));
									iniWeap->SetValue(weapID, tempAVKey->c_str(), ATShared::GetIdentifierFromFormID(tempAVMod.avInfo->formID));
									tempAVKey->clear();
									tempAVKey->append("ActorValueAmt");
									tempAVKey->append(std::to_string(i));
									iniWeap->SetLongValue(weapID, tempAVKey->c_str(), tempAVMod.unk08);
								}
							}
						}
					}
				}

				// enchantments
				if (instanceData->enchantments) {
					iniWeap->SetLongValue(weapID, "numEnchantments", instanceData->enchantments->count, "; Enchantments:");
					if (instanceData->enchantments->count > 0) {
						EnchantmentItem *tempEnch;
						for (UInt32 i = 0; i < instanceData->enchantments->count; i++) {
							if (instanceData->enchantments->GetNthItem(i, tempEnch)) {
								if (tempEnch) {
									std::string *tempEnchKey = new std::string();
									tempEnchKey->append("Enchantment");
									tempEnchKey->append(std::to_string(i));
									iniWeap->SetValue(weapID, tempEnchKey->c_str(), ATShared::GetIdentifierFromFormID(tempEnch->formID));
									tempEnchKey->clear();
								}
							}
						}
					}
				}

				// keywords
				/*
				iniWeap->SetLongValue(weapID, "numKeywords", tempWeapBase->keyword.numKeywords, "; Keywords:");
				if (tempWeapBase->keyword.numKeywords > 0) {
					for (UInt32 i = 0; i < tempWeapBase->keyword.numKeywords; i++) {
						std::string *tempEnchKey = new std::string();
						tempEnchKey->append("Keyword");
						tempEnchKey->append(std::to_string(i));
						if (tempWeapBase->keyword.keywords[i])
							iniWeap->SetValue(weapID, tempEnchKey->c_str(), ATShared::GetIdentifierFromFormID(tempWeapBase->keyword.keywords[i]->formID));
						else
							iniWeap->SetValue(weapID, tempEnchKey->c_str(), "none");

						tempEnchKey->clear();

					}
				}
				*/
			}
		}
	}

	// loads a single weapon base form's stats from ini
	void LoadWeaponBase_Gun(CSimpleIniA *iniWeap, const char *weapID)
	{
		TESForm *weapForm = ATShared::GetFormFromIdentifier(weapID);
		TESObjectWEAP *tempWeapBase = DYNAMIC_CAST(weapForm, TESForm, TESObjectWEAP);

		if (ATShared::SharedData) {
			if (tempWeapBase) {

				TESObjectWEAP::InstanceData *instanceData = ATShared::GetInstanceData_WeapForm(tempWeapBase);
				if (instanceData) {

					// read variables

					// flags:
					// NPCsUseAmmo
					bool weapFlag_NPCsUseAmmo = ATShared::SharedData->bNPCsUseAmmo;

					//write

					
					// av modifiers
					int avCount = iniWeap->GetLongValue(weapID, "numActorValues", 0);
					if (avCount > 0) {
						TBO_InstanceData::ValueModifier tempAVypeMod;
						ActorValueInfo *tempAV = nullptr;
						UInt32 tempAmount = 0;

						if (!instanceData->modifiers) {
							instanceData->modifiers = new tArray<TBO_InstanceData::ValueModifier>();
						}

						for (UInt8 i = 0; i < avCount; i++) {
							std::string *avKeyStr = new std::string();
							avKeyStr->append("ActorValue");
							avKeyStr->append(std::to_string(i));
							const char *avID = iniWeap->GetValue(weapID, avKeyStr->c_str(), "none");
							tempAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier(avID);

							if (tempAV) {
								tempAVypeMod.avInfo = tempAV;
								avKeyStr->clear();
								avKeyStr->append("ActorValueAmt");
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
					int enchCount = iniWeap->GetLongValue(weapID, "numEnchantments", 0);
					if (enchCount > 0) {
						EnchantmentItem *tempEnch = nullptr;

						if (!instanceData->enchantments) {
							instanceData->enchantments = new tArray<EnchantmentItem*>();
						}

						for (UInt8 i = 0; i < enchCount; i++) {
							std::string *enchKeyStr = new std::string();
							enchKeyStr->append("Enchantment");
							enchKeyStr->append(std::to_string(i));
							const char *enchID = iniWeap->GetValue(weapID, enchKeyStr->c_str(), "none");
							tempEnch = (EnchantmentItem*)ATShared::GetFormFromIdentifier(enchID);
							if (tempEnch) {
								instanceData->enchantments->Push(tempEnch);
							}
						}
					}


					// keywords to add
					int kwCount = iniWeap->GetLongValue(weapID, "numCalibers", 0);

					if (kwCount > 0) {
						BGSKeyword *tempKW = nullptr;
						BGSKeyword **keywords = new BGSKeyword*[tempWeapBase->keyword.numKeywords + kwCount];

						for (UInt8 i = 0; i < tempWeapBase->keyword.numKeywords; i++) {
							keywords[i] = tempWeapBase->keyword.keywords[i];
						}

						for (UInt8 i = 0; i < kwCount; i++) {
							std::string *kwKeyStr = new std::string();
							kwKeyStr->append("Caliber");
							kwKeyStr->append(std::to_string(i));
							const char *kwID = iniWeap->GetValue(weapID, kwKeyStr->c_str(), "none");
							tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
							keywords[i + tempWeapBase->keyword.numKeywords] = tempKW;
						}

						tempWeapBase->keyword.numKeywords = tempWeapBase->keyword.numKeywords + kwCount;
						tempWeapBase->keyword.keywords = keywords;

					}


					//flags:
					if (instanceData->flags) {
						//NPCsUseAmmo
						if (weapFlag_NPCsUseAmmo)
							instanceData->flags |= 0x0000002;
						else
							instanceData->flags &= 0x0000002;
					}



				}
			}
		}
	}


	void LoadCaliberFromINI(const char *finalDirName)
	{
		bool bRetVal = false;
		if (ATShared::SharedData) {
			ATCaliber *caliberStore = new ATCaliber();
			if (caliberStore->FillFromIni(finalDirName)) {
				ATShared::SharedData->g_ATCalibers.Push(caliberStore);
				ATShared::SharedData->index_ATCalibersEquipped.Push(caliberStore->caliberKW);

				_MESSAGE("Loaded Caliber %s - Name: %s", finalDirName, caliberStore->caliberName);
			}
			else {
				_MESSAGE("Failed to load Caliber %s", finalDirName);
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
			
			MaxCNDBase = iniWeap.GetLongValue(weaponID, "MaxCNDBase", 0);
			HUDIconIndex = iniWeap.GetLongValue(weaponID, "HUDIcon", 0);
			HUDIconIndexAlt = iniWeap.GetLongValue(weaponID, "HUDIconAlt", 0);

			BGSKeyword *tempKW = nullptr;

			int maxCNDModsCount = iniWeap.GetLongValue("Shared", "numCNDMods", 0);
			if (maxCNDModsCount > 0) {
				for (UInt8 i = 0; i < maxCNDModsCount; i++) {
					std::string *kwKeyStr = new std::string();
					kwKeyStr->append("CNDModKW");
					kwKeyStr->append(std::to_string(i));
					const char *kwID = iniWeap.GetValue("Shared", kwKeyStr->c_str(), "none");
					tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
					if (tempKW) {
						kwKeyStr->clear();
						kwKeyStr->append("CNDModAdd");
						kwKeyStr->append(std::to_string(i));
						float cndModAmt = iniWeap.GetDoubleValue("Shared", kwKeyStr->c_str(), 0.0);
						if (cndModAmt > 0.0) {
							ATWeapVarMod tempVarMod;
							tempVarMod.modKW = tempKW;
							tempVarMod.modMultAdd = cndModAmt;
							MaxCNDMods.Push(tempVarMod);
						}
					}
				}
			}
			int maxWearModsCount = iniWeap.GetLongValue("Shared", "numWearMods", 0);
			for (UInt8 i = 0; i < maxWearModsCount; i++) {
				std::string *kwKeyStr = new std::string();
				kwKeyStr->append("WearModKW");
				kwKeyStr->append(std::to_string(i));
				const char *kwID = iniWeap.GetValue("Shared", kwKeyStr->c_str(), "none");
				tempKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(kwID);
				if (tempKW) {
					kwKeyStr->clear();
					kwKeyStr->append("WearModAdd");
					kwKeyStr->append(std::to_string(i));
					float cndModAmt = iniWeap.GetDoubleValue("Shared", kwKeyStr->c_str(), 0.0);
					if (cndModAmt > 0.0) {
						ATWeapVarMod tempVarMod;
						tempVarMod.modKW = tempKW;
						tempVarMod.modMultAdd = cndModAmt;
						WearMods.Push(tempVarMod);
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
		caliberName->append(iniAmmo.GetValue("Caliber", "Name", "none"));

		const char *tempCalNameKWID = iniAmmo.GetValue("Caliber", "CaliberKW", "none");
		caliberKW = (BGSKeyword*)ATShared::GetFormFromIdentifier(tempCalNameKWID);
		
		iArmorPenetration = iniAmmo.GetLongValue("Caliber", "ArmorPen", 0);
		fWearMult = iniAmmo.GetDoubleValue("Caliber", "WearMult", 1.0);
		fCritFailMult = iniAmmo.GetDoubleValue("Caliber", "CritFailMult", 1.0);

		const char *tempCaliberModKW = iniAmmo.GetValue("Caliber", "CaliberMod", "none");
		caliberMod = ATShared::GetFormFromIdentifier(tempCaliberModKW);

		// projectile types
		UInt8 projectilesCount = iniAmmo.GetLongValue("Caliber", "numProjectiles", 0);
		for (UInt8 j = 0; j < projectilesCount; j++) {
			std::string projectileIndex = "Projectile";
			projectileIndex.append(std::to_string(j));
			projectiles.Push((BGSProjectile*)ATShared::GetFormFromIdentifier(iniAmmo.GetValue("Caliber", projectileIndex.c_str(), "none")));
			projectileIndex.clear();
		}

		// ammo subtypes
		int numSubTypes = iniAmmo.GetLongValue("Caliber", "numSubTypes", 0);
		if (numSubTypes > 0) {
			for (UInt8 i = 0; i < numSubTypes; i++) {
				ATAmmoType newType;
				
				std::string subtypeIndex = "Subtype";
				subtypeIndex.append(std::to_string(i));
				const char *subTypeID = iniAmmo.GetValue("Caliber", subtypeIndex.c_str(), "none");
				
				newType.AmmoName = new std::string();
				newType.AmmoName->append(iniAmmo.GetValue(subTypeID, "Name", "none"));
				
				const char *ammoFormID = iniAmmo.GetValue(subTypeID, "Ammo", "none");
				TESAmmo *newAmmo = (TESAmmo*)ATShared::GetFormFromIdentifier(ammoFormID);
				newType.ammoForm = newAmmo;

				newType.fDamageMult = iniAmmo.GetDoubleValue(subTypeID, "DamageMult", 1.0);
				newType.fRangeMult = iniAmmo.GetDoubleValue(subTypeID, "RangeMult", 1.0);

				newType.fRecoilMult = iniAmmo.GetDoubleValue(subTypeID, "RecoilMult", 1.0);
				newType.fCoFMult = iniAmmo.GetDoubleValue(subTypeID, "CoFMult", 1.0);

				// suppressor/legendary explosive/muzzle brake vars:
				newType.projectileStd = iniAmmo.GetLongValue(subTypeID, "ProjectileStd", 0);
				newType.projectileSup = iniAmmo.GetLongValue(subTypeID, "ProjectileSup", 0);
				newType.projectileExp = iniAmmo.GetLongValue(subTypeID, "ProjectileExp", 0);
				newType.projectileBrk = iniAmmo.GetLongValue(subTypeID, "ProjectileBrk", 0);

				newType.projImpactData = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(iniAmmo.GetValue(subTypeID, "ImpactOverride", "none"));
				newType.projImpactDataExp = (BGSImpactDataSet*)ATShared::GetFormFromIdentifier(iniAmmo.GetValue("Caliber", "ImpactExp", "none"));

				newType.fWearMult = iniAmmo.GetDoubleValue(subTypeID, "WearMult", 1.0);
				
				newType.iHitEffect = iniAmmo.GetLongValue(subTypeID, "HitEffect", 3);

				newType.iSoundLevel = iniAmmo.GetLongValue(subTypeID, "NoiseLevel", 1);
				newType.iSoundLevelSup = iniAmmo.GetLongValue(subTypeID, "NoiseLevelSup", 0);
				newType.iSoundLevelExp = iniAmmo.GetLongValue(subTypeID, "NoiseLevelExp", 3);

				newType.fCritChanceMult = iniAmmo.GetDoubleValue(subTypeID, "CritMult", 1.0);
				newType.fCritDmgMult = newType.fCritChanceMult;
				newType.iCritEffect = iniAmmo.GetLongValue(subTypeID, "CritEffectTable", 0);


				// enchantments
				int enchCount = iniAmmo.GetLongValue(subTypeID, "numEnchantments", 0);
				if (enchCount > 0) {
					EnchantmentItem *tempEnch = nullptr;
					for (UInt8 j = 0; j < enchCount; j++) {
						std::string *enchKeyStr = new std::string();
						enchKeyStr->append("Enchantment");
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
	// shared directory/section text:

	const char* configPath_Base = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\";

	const char* configPath_MainINI = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\AmmoTweaks.ini";

	const char* configPath_Weapons = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Weapons\\";
	const char* configPath_WeapIndex = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Weapons\\Index_";
	const char *configCategories_Weap[5] = { "Guns", "EnergyGuns", "Heavy", "FollowerGuns", "FollowerEnergyGuns" };

	const char* configPath_Ammo = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Ammo\\";
	const char* configPath_AmmoIndex = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Ammo\\Index_";

	const char* configPath_Firemodes = ".\\Data\\F4SE\\Plugins\\AmmoTweaks\\Firemodes\\";

	bool bNPCsUseAmmo = false;



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
			configAmmoString.append(configPath_AmmoIndex);
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

							ATFileIO::LoadCaliberFromINI(finalDirName);
						}

					}
				}
			}
			iniIndex.Reset();
		}

	}

	void LoadData_Guns_Firemodes(const char* configDir)
	{

	}

	void LoadData_Guns_BaseStats(const char* configDir)
	{
		if (ATShared::SharedData) {
			const char *tempModName = "", *finalDirName = "", *weaponID = "";

			std::string configWeaponsString = "";
			std::string configModDirString = "";
			std::string configSection = "";

			UInt32 weaponCount = 0;

			CSimpleIniA iniIndex;
			iniIndex.SetUnicode();

			CSimpleIniA iniWeap;
			iniWeap.SetUnicode();

			for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
				tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
				configWeaponsString.clear();
				configWeaponsString.append(configPath_WeapIndex);
				configWeaponsString.append(tempModName);
				configWeaponsString.append(".ini");

				if (iniIndex.LoadFile(configWeaponsString.c_str()) > -1) {

					for (UInt8 k = 0; k < 5; k++) {
						weaponCount = iniIndex.GetLongValue(configCategories_Weap[k], "numWeapons", 0);

						if (weaponCount > 0) {
							configModDirString.clear();
							configModDirString.append(configDir);
							configModDirString.append(tempModName);
							configModDirString.append("\\");
							configModDirString.append(configCategories_Weap[k]);
							configModDirString.append(".ini");
							finalDirName = configModDirString.c_str();

							if (iniWeap.LoadFile(finalDirName) > -1) {
								_MESSAGE(finalDirName);
								for (UInt8 i = 0; i < weaponCount; i++) {
									configSection.clear();
									configSection.append("Weapon");
									configSection.append(std::to_string(i));
									weaponID = iniIndex.GetValue(configCategories_Weap[k], configSection.c_str(), "none");
									if (weaponID != "none") {
										ATFileIO::LoadWeaponBase_Gun(&iniWeap, weaponID);
										ATWeapon *newWeapon = new ATWeapon();
										newWeapon->FillFromIni(finalDirName, weaponID);
										ATShared::SharedData->g_ATWeapons.Push(newWeapon);
									}
								}
							}
							else {
								_MESSAGE("%s not found", finalDirName);
							}
							iniWeap.Reset();
						}
					}
				}
				iniIndex.Reset();
			}

			configWeaponsString.clear();
			configModDirString.clear();
			configSection.clear();
		}
	}


	void LoadData_WeapMelee(const char* configDir)
	{
		
	}


	// -------------------- Save ------------------------------------


	void SaveData_Guns_BaseStats(const char* configDir)
	{
		const char *tempModName = "", *finalDirName = "", *weaponID = "";

		std::string configWeaponsString = "";
		std::string configModDirString = "";
		std::string configSection = "";

		UInt32 weaponCount = 0;
		UInt32 accumWeaponCount = 0;

		CreateDirectory(configPath_Base, NULL);
		CreateDirectory(configPath_Weapons, NULL);

		CSimpleIniA iniIndex;
		iniIndex.SetUnicode();

		CSimpleIniA iniWeap;
		iniWeap.SetUnicode();

		for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
			configWeaponsString.clear();
			configWeaponsString.append(configPath_WeapIndex);
			configWeaponsString.append(tempModName);
			configWeaponsString.append(".ini");

			configModDirString.clear();
			configModDirString.append(configPath_Weapons);
			configModDirString.append(tempModName);

			if (iniIndex.LoadFile(configWeaponsString.c_str()) == 0) {
				accumWeaponCount = 0;

				for (UInt8 k = 0; k < 5; k++) {
					weaponCount = iniIndex.GetLongValue(configCategories_Weap[k], "numWeapons", 0);

					if (weaponCount > 0) {
						if (accumWeaponCount < 1) {
							CreateDirectory(configModDirString.c_str(), NULL);
							accumWeaponCount += 1;
						}
						configModDirString.clear();
						configModDirString.append(configPath_Weapons);
						configModDirString.append(tempModName);

						configModDirString.append("\\");
						configModDirString.append(configCategories_Weap[k]);
						configModDirString.append(".ini");
						finalDirName = configModDirString.c_str();

						iniWeap.LoadFile(finalDirName);
						// reset ini file
						iniWeap.SetValue(configCategories_Weap[k], NULL, NULL);

						_MESSAGE(finalDirName);

						for (UInt8 i = 0; i < weaponCount; i++) {
							configSection.clear();
							configSection.append("Weapon");
							configSection.append(std::to_string(i));
							weaponID = iniIndex.GetValue(configCategories_Weap[k], configSection.c_str(), "none");
							ATFileIO::SaveWeaponBase_Gun(&iniWeap, weaponID);
						}

						// save the file, reset
						iniWeap.SaveFile(finalDirName);
						iniWeap.Reset();
					}
				}
			}
			iniIndex.Reset();
		}

		configWeaponsString.clear();
		configModDirString.clear();
		configSection.clear();
	}

	void SaveData_Guns()
	{
		SaveData_Guns_BaseStats(configPath_Weapons);
	}



}




void ATConfig::LoadGameDataFromINI()
{
	
	ATShared::SharedData = new ATSharedData();

	if (ATShared::SharedData) {
		// dn_HasMuzzle_Suppressor
		ATShared::SharedData->suppressorKW = (BGSKeyword*)LookupFormByID(0x0016304E);
		// dn_HasMuzzle_Brake
		ATShared::SharedData->muzBrakeKW = (BGSKeyword*)LookupFormByID(0x0016304D);
		// dn_HasLegendary_ExplosiveBullets
		ATShared::SharedData->legendaryExplKW = (BGSKeyword*)LookupFormByID(0x001E73BC);
		// dn_HasLegendary_TwoShot
		ATShared::SharedData->legendaryTwoShotKW = (BGSKeyword*)LookupFormByID(0x001CF587);
		// dn_HasMuzzle_Compensator
		ATShared::SharedData->compensatorKW = (BGSKeyword*)LookupFormByID(0x0016304C);

		// s_30_automatic
		ATShared::SharedData->soundKW_Automatic = (BGSKeyword*)LookupFormByID(0x000A191C);
		// auto sound dummy
		ATShared::SharedData->soundKW_AutomaticDisable = (BGSKeyword*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001BF9");

		ATShared::SharedData->soundKW_Silenced = (BGSKeyword*)LookupFormByID(0x00054A67);
		ATShared::SharedData->soundKW_SilencedDisable = (BGSKeyword*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001BF8");

		ATShared::SharedData->MaxConditionAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|000AFC");
		ATShared::SharedData->HUDIconAV_Weapon = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|000BC9");
		ATShared::SharedData->SkillReqAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001DF2");
		ATShared::SharedData->HasInstanceAV = (ActorValueInfo*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|001C2A");

		ATShared::SharedData->numProcessedWeaps = 1;
	}
	

	CSimpleIniA mainINI;
	mainINI.SetUnicode();
	mainINI.LoadFile(configPath_MainINI);
	
	if (ATShared::SharedData) {
		ATShared::SharedData->bNPCsUseAmmo = mainINI.GetBoolValue("WeaponTweaks", "NPCsUseAmmo", false);
		ATShared::SharedData->bNPCsUseAmmo_Thrown = mainINI.GetBoolValue("WeaponTweaks", "NPCsUseAmmo_Thrown", false);
		ATShared::SharedData->bNPCsUseAmmo_Turret = mainINI.GetBoolValue("WeaponTweaks", "NPCsUseAmmo_Turret", false);

		ATShared::SharedData->bUseSingleFireAutoSounds = mainINI.GetBoolValue("WeaponTweaks", "UseSingleFireAutoSounds", false);

		ATShared::SharedData->bDisableRecoilSpringForce = mainINI.GetBoolValue("WeaponTweaks", "DisableRecoilSpringForce", false);

		ATShared::SharedData->bTwoShotLegendaryTweak = mainINI.GetBoolValue("WeaponTweaks", "TwoShotLegendaryTweak", false);
		

		ATShared::SharedData->bEnableSkillRequirements = mainINI.GetBoolValue("SkillRequirement", "Enabled", false);
		ATShared::SharedData->fSkillReq_MaxWeight = mainINI.GetDoubleValue("SkillRequirement", "SkillReq_MaxWeight", 30.0);
		ATShared::SharedData->fSkillReq_MinReq = mainINI.GetDoubleValue("SkillRequirement", "SkillReq_MinReq", 2.0);
		ATShared::SharedData->fSkillReq_MaxReq = mainINI.GetDoubleValue("SkillRequirement", "SkillReq_MaxReq", 12.0);

	}


	bool bLoadWeaponData = mainINI.GetBoolValue("General", "ApplyWeaponBaseTweaks", false);

	mainINI.Reset();


	LoadData_Ammo(configPath_Ammo);

	if (bLoadWeaponData) {
		LoadData_Guns_BaseStats(configPath_Weapons);
	}
}


void ATConfig::SaveGameDataToINI()
{
	SaveData_Guns();
	
}


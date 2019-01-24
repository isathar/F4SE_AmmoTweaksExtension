#include "f4se/GameData.h"
#include "f4se/GameRTTI.h"
#include "SimpleIni/SimpleIni.h"
#include <string>

#include "ATConfigSave.h"




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
}


namespace ATConfig
{
	void SaveData_Guns_BaseStats(const char* configDir)
	{
		const char *tempModName = "", *finalDirName = "", *weaponID = "";

		std::string configWeaponsString = "";
		std::string configModDirString = "";
		std::string configSection = "";

		UInt32 weaponCount = 0;
		UInt32 accumWeaponCount = 0;

		CreateDirectory(ATShared::SharedData->configPath_Base, NULL);
		CreateDirectory(ATShared::SharedData->configPath_Weapons, NULL);

		CSimpleIniA iniIndex;
		iniIndex.SetUnicode();

		CSimpleIniA iniWeap;
		iniWeap.SetUnicode();

		for (UInt8 j = 0; j < (*g_dataHandler)->modList.loadedMods.count; j++) {
			tempModName = (*g_dataHandler)->modList.loadedMods[j]->name;
			configWeaponsString.clear();
			configWeaponsString.append(ATShared::SharedData->configPath_WeapIndex);
			configWeaponsString.append(tempModName);
			configWeaponsString.append(".ini");

			configModDirString.clear();
			configModDirString.append(ATShared::SharedData->configPath_Weapons);
			configModDirString.append(tempModName);

			if (iniIndex.LoadFile(configWeaponsString.c_str()) == 0) {
				accumWeaponCount = 0;

				for (UInt8 k = 0; k < 5; k++) {
					weaponCount = iniIndex.GetLongValue(ATShared::SharedData->configCategories_Weap[k], "numWeapons", 0);

					if (weaponCount > 0) {
						if (accumWeaponCount < 1) {
							CreateDirectory(configModDirString.c_str(), NULL);
							accumWeaponCount += 1;
						}
						configModDirString.clear();
						configModDirString.append(ATShared::SharedData->configPath_Weapons);
						configModDirString.append(tempModName);

						configModDirString.append("\\");
						configModDirString.append(ATShared::SharedData->configCategories_Weap[k]);
						configModDirString.append(".ini");
						finalDirName = configModDirString.c_str();

						iniWeap.LoadFile(finalDirName);
						// reset ini file
						iniWeap.SetValue(ATShared::SharedData->configCategories_Weap[k], NULL, NULL);

						_MESSAGE(finalDirName);

						for (UInt8 i = 0; i < weaponCount; i++) {
							configSection.clear();
							configSection.append("Weapon");
							configSection.append(std::to_string(i));
							weaponID = iniIndex.GetValue(ATShared::SharedData->configCategories_Weap[k], configSection.c_str(), "none");
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
		SaveData_Guns_BaseStats(ATShared::SharedData->configPath_Weapons);
	}
}



void ATConfig::SaveGameDataToINI()
{
	SaveData_Guns();

}


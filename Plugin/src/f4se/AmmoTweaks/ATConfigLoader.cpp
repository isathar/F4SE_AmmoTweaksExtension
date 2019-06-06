#include "ATConfigLoader.h"
#include "SimpleIni/SimpleIni.h"
#include <string>
#include <chrono>



// loads a weapon's caliber slots data from ini
bool ATCaliber::ReadIni(const char * configFileName)
{
	CSimpleIniA iniCalibers;
	iniCalibers.SetUnicode();
	iniCalibers.SetMultiKey(true);

	if (iniCalibers.LoadFile(configFileName) > -1) {
		const char *keyID = "", *ammoSection = "", *projSection = "";

		// ---- HUD/Menu Name
		hudName = iniCalibers.GetValue("Caliber", "sName", "none");

		// ---- ID/Recipe Keywords
		keyID = iniCalibers.GetValue("Caliber", "sCaliberID", "none");
		objectID = ATUtilities::GetFormIDFromIdentifier(keyID);
		keyID = iniCalibers.GetValue("Caliber", "sRecipeID", "none");
		recipeKW = (BGSKeyword*)ATUtilities::GetFormFromIdentifier(keyID);

		// ---- Ammo Swapping Animations/Event
		keyID = iniCalibers.GetValue("Caliber", "sAmmoSwapIdle_1P", "none");
		swapIdle_1P = (TESForm*)ATUtilities::GetFormFromIdentifier(keyID);
		keyID = iniCalibers.GetValue("Caliber", "sAmmoSwapIdle_3P", "none");
		swapIdle_3P = (TESForm*)ATUtilities::GetFormFromIdentifier(keyID);
		swapAnimEventName = BSFixedString(iniCalibers.GetValue("Caliber", "sAmmoSwapAnimEvent", ""));

		// ---- Ammo Subtypes
		CSimpleIni::TNamesDepend ammoValues, projValues;
		if (iniCalibers.GetAllValues("Caliber", "sAmmoTypes", ammoValues)) {
			CSimpleIni::TNamesDepend::const_iterator j = ammoValues.begin();
			for (; j != ammoValues.end(); ++j) {
				ATCaliber::AmmoType newAmmoType;
				ammoSection = j->pItem;

				// ---- HUD/Menu Name
				newAmmoType.hudName = iniCalibers.GetValue(ammoSection, "sName", "none");

				// ---- Ammo Item
				keyID = iniCalibers.GetValue(ammoSection, "sAmmoID", "none");
				newAmmoType.ammoItem = (TESAmmo*)ATUtilities::GetFormFromIdentifier(keyID);
					
				// ---- Ammo Mod
				keyID = iniCalibers.GetValue(ammoSection, "sModID", "none");
				newAmmoType.ammoMod = (BGSMod::Attachment::Mod*)ATUtilities::GetFormFromIdentifier(keyID);

				if (newAmmoType.ammoItem && newAmmoType.ammoMod) {
					// ---- Casing item
					keyID = iniCalibers.GetValue(ammoSection, "sCasingID", "none");
					newAmmoType.casingItem = (TESObjectMISC*)ATUtilities::GetFormFromIdentifier(keyID);

					// ---- Projectile Overrides
					if (iniCalibers.GetAllValues(ammoSection, "sProjectiles", projValues)) {
						CSimpleIni::TNamesDepend::const_iterator k = projValues.begin();
						for (; k != projValues.end(); ++k) {
							projSection = k->pItem;
							ATCaliber::ProjectileOverride tempProjOverride;

							// ---- Projectile
							keyID = iniCalibers.GetValue(projSection, "sProjectile", "none");
							tempProjOverride.projectile = (BGSProjectile*)ATUtilities::GetFormFromIdentifier(keyID);
								
							// ---- Required/Exclude Keywords
							keyID = iniCalibers.GetValue(projSection, "sRequiredKW", "none");
							tempProjOverride.projectileKW = ATUtilities::GetFormIDFromIdentifier(keyID);
							keyID = iniCalibers.GetValue(projSection, "sExcludeKW", "none");
							tempProjOverride.excludeKW = ATUtilities::GetFormIDFromIdentifier(keyID);

							// ---- ImpactDataSet
							keyID = iniCalibers.GetValue(projSection, "sImpactData", "none");
							tempProjOverride.impactData = (BGSImpactDataSet*)ATUtilities::GetFormFromIdentifier(keyID);

							newAmmoType.projectiles.Push(tempProjOverride);
						}
					}
					newAmmoType.projectiles.capacity = newAmmoType.projectiles.count;
					ammoTypes.Push(newAmmoType);

					projValues.clear();
				}
				else {
					_MESSAGE("!Missing Ammo item or mod! - Caliber %s: Subtype: %s, ID: 0x%08X", hudName.c_str(), newAmmoType.hudName.c_str());
				}
			}

			if (ammoTypes.count > 0) {
				ammoTypes.capacity = ammoTypes.count;
				ammoValues.clear();
				return true;
			}
		}
		ammoValues.clear();
	}
	return false;
}



// loads a weapon's data from the given ini file path
bool ATWeapon::ReadIni(const char * configFileName)
{
	CSimpleIniA iniWeap;
	iniWeap.SetUnicode();
	iniWeap.SetMultiKey(true);

	std::string iniFileStr;
	iniFileStr.append(configFileName);
	iniFileStr.append("Weapon.ini");

	if (iniWeap.LoadFile(iniFileStr.c_str()) > -1) {
		const char *weapID = iniWeap.GetValue("Base", "sWeapID", "none");
		TESObjectWEAP *weaponBase = (TESObjectWEAP*)ATUtilities::GetFormFromIdentifier(weapID);

		if (weaponBase) {
			const char *keyID = "";
			CSimpleIni::TNamesDepend values;

			objectID = weaponBase->formID;
			configPath = BSFixedString(configFileName);
			hudName = iniWeap.GetValue("Base", "sName", "");

			// ---- Required Skill AV
			keyID = iniWeap.GetValue("Base", "sRequiredSkill", "none");
			requiredSkill = (ActorValueInfo*)ATUtilities::GetFormFromIdentifier(keyID);

			// ---- Critical Failure Table
			if (iniWeap.GetAllValues("Base", "sCritFailures", values)) {
				if (values.size() > 0) {
					CSimpleIni::TNamesDepend::const_iterator j = values.begin();
					for (; j != values.end(); ++j) {
						ATCritEffect newCritEffect;
						if (ATUtilities::GetCritEffectFromIdentifier(j->pItem, newCritEffect)) {
							critFailures.Push(newCritEffect);
						}
					}
				}
			}
			values.clear();
			critFailures.capacity = critFailures.count;

			// ---- Bash Crit Failure Table
			if (iniWeap.GetAllValues("Base", "sBashCritFailures", values)) {
				if (values.size() > 0) {
					CSimpleIni::TNamesDepend::const_iterator j = values.begin();
					for (; j != values.end(); ++j) {
						ATCritEffect newCritEffect;
						if (ATUtilities::GetCritEffectFromIdentifier(j->pItem, newCritEffect)) {
							critFailures_Bash.Push(newCritEffect);
						}
					}
				}
			}
			values.clear();
			critFailures_Bash.capacity = critFailures_Bash.count;

			// ---- Supported Calibers
			CSimpleIni::TNamesDepend caliberValues;
			std::string caliberFileStr;
			if (iniWeap.GetAllValues("Base", "sCalibers", values)) {
				CSimpleIni::TNamesDepend::const_iterator i = values.begin();
				for (; i != values.end(); ++i) {
					const char * caliberName = i->pItem;
					caliberFileStr.append(configFileName);
					caliberFileStr.append("Calibers\\");
					caliberFileStr.append(caliberName);
					caliberFileStr.append(".ini");
					ATCaliber newCaliber;
					if (newCaliber.ReadIni(caliberFileStr.c_str())) {
						calibers.Push(newCaliber);
					}
					caliberFileStr.clear();
				}
			}
			values.clear();
			calibers.capacity = calibers.count;

			// ---- Caliber Worbench Recipe Keywords
			if (calibers.count > 0) {
				tArray<BGSKeyword*> caliberRecipes;
				for (UInt32 j = 0; j < calibers.count; j++) {
					if (calibers[j].recipeKW) {
						caliberRecipes.Push(calibers[j].recipeKW);
					}
				}
				if (caliberRecipes.count > 0) {
					int keywordsCount = weaponBase->keyword.numKeywords;
					if (keywordsCount > 0) {
						for (UInt32 j = 0; j < keywordsCount; j++) {
							if (weaponBase->keyword.keywords[j]) {
								caliberRecipes.Push(weaponBase->keyword.keywords[j]);
							}
						}
						keywordsCount = caliberRecipes.count;
						weaponBase->keyword.numKeywords = keywordsCount;
						weaponBase->keyword.keywords = new BGSKeyword*[keywordsCount];

						for (UInt32 j = 0; j < keywordsCount; j++) {
							weaponBase->keyword.keywords[j] = caliberRecipes[j];
						}
					}
					caliberRecipes.Clear();
				}
			}

			// ---- Swappable Mod Slots
			if (iniWeap.GetAllValues("Base", "sSwapModSlots", values)) {
				if (values.size() > 0) {
					CSimpleIni::TNamesDepend sections;
					CSimpleIni::TNamesDepend::const_iterator i = values.begin();
					for (; i != values.end(); ++i) {
						const char * modSlotName = i->pItem;
						if (iniWeap.GetAllValues(modSlotName, "sMods", sections)) {
							if (sections.size() > 0) {
								ATModSlot newSlot;

								CSimpleIni::TNamesDepend::const_iterator j = sections.begin();
								for (; j != sections.end(); ++j) {
									const char * sectionName = j->pItem;
									ATSwappableMod newMod;
									keyID = iniWeap.GetValue(sectionName, "sModID", "none");
									newMod.swapMod = (BGSMod::Attachment::Mod*)ATUtilities::GetFormFromIdentifier(keyID);

									if (newMod.swapMod) {
										keyID = iniWeap.GetValue(sectionName, "sRequiredKW", "none");
										newMod.requiredKW = ATUtilities::GetFormIDFromIdentifier(keyID);

										keyID = iniWeap.GetValue(sectionName, "sRequiredItem", "none");
										newMod.modItem = (TESForm*)ATUtilities::GetFormFromIdentifier(keyID);

										newMod.bRequireModMisc = iniWeap.GetBoolValue(sectionName, "bRequireModMisc", false);

										newSlot.swappableMods.Push(newMod);
									}
								}
								newSlot.swappableMods.capacity = newSlot.swappableMods.count;

								// add name + push if the slot has any mods
								if (newSlot.swappableMods.count > 0) {
									newSlot.slotName = BSFixedString(iniWeap.GetValue(modSlotName, "sName", "none"));

									keyID = iniWeap.GetValue(modSlotName, "sSwapIdleAnim_1P", "none");
									newSlot.swapIdle_1P = (TESForm*)ATUtilities::GetFormFromIdentifier(keyID);
									keyID = iniWeap.GetValue(modSlotName, "sSwapIdleAnim_3P", "none");
									newSlot.swapIdle_3P = (TESForm*)ATUtilities::GetFormFromIdentifier(keyID);
									newSlot.swapAnimEventName = BSFixedString(iniWeap.GetValue(modSlotName, "sSwapAnimEvent", ""));

									modSlots.Push(newSlot);
								}
							}
						}
						sections.clear();
					}
				}
			}
			values.clear();
			modSlots.capacity = modSlots.count;

			// ---- Damaged Mod Slots
			if (iniWeap.GetAllValues("Base", "sDamagedModSlots", values)) {
				int modSlotCount = values.size();
				if (modSlotCount > 0) {
					CSimpleIni::TNamesDepend sections;
					CSimpleIni::TNamesDepend::const_iterator i = values.begin();
					for (; i != values.end(); ++i) {
						const char * modSlotName = i->pItem;
						if (iniWeap.GetAllValues(modSlotName, "sModID", sections)) {
							if (sections.size() > 0) {
								ATDamagedModSlot newSlot;
								newSlot.slotName = BSFixedString(iniWeap.GetValue(modSlotName, "sName", "none"));
								CSimpleIni::TNamesDepend::const_iterator j = sections.begin();
								for (; j != sections.end(); ++j) {
									keyID = j->pItem;
									BGSMod::Attachment::Mod * swapMod = (BGSMod::Attachment::Mod*)ATUtilities::GetFormFromIdentifier(keyID);
									if (swapMod) {
										newSlot.damagedMods.Push(swapMod);
									}
								}
								newSlot.damagedMods.capacity = newSlot.damagedMods.count;
								if (newSlot.damagedMods.count > 0)
									damagedModSlots.Push(newSlot);
							}
						}
						sections.clear();
					}
				}
			}
			values.clear();
			damagedModSlots.capacity = damagedModSlots.count;

			// ---- Droppable Magazine Items
			iReloadType = iniWeap.GetLongValue("Base", "iReloadType", -1);
			if (iReloadType > -1) {
				if (iniWeap.GetAllValues("Base", "sMagazines", values)) {
					int magsCount = values.size();
					if (magsCount > 0) {
						CSimpleIni::TNamesDepend::const_iterator i = values.begin();
						for (; i != values.end(); ++i) {
							ATMagItem newMag;
							keyID = iniWeap.GetValue(i->pItem, "sMagItem", "none");
							newMag.magItem = (TESObjectMISC*)ATUtilities::GetFormFromIdentifier(keyID);
							if (newMag.magItem) {
								keyID = iniWeap.GetValue(i->pItem, "sRequiredKW", "none");
								newMag.magKW = ATUtilities::GetFormIDFromIdentifier(keyID);
								magazines.Push(newMag);
							}
						}
					}
				}
			}
			values.clear();
			magazines.capacity = magazines.count;

			// ---- Weapon Holstering
			if (iniWeap.GetAllValues("Base", "sHolsters", values)) {
				int holstersCount = values.size();
				if (holstersCount > 0) {
					CSimpleIni::TNamesDepend::const_iterator i = values.begin();
					for (; i != values.end(); ++i) {
						ATHolsterArmor newHolster;
						keyID = iniWeap.GetValue(i->pItem, "sArmorWeapon", "none");
						newHolster.armorWeapon = (TESObjectARMO*)ATUtilities::GetFormFromIdentifier(keyID);

						if (newHolster.armorWeapon) {
							keyID = iniWeap.GetValue(i->pItem, "sRequiredKW", "none");
							newHolster.requiredKW = ATUtilities::GetFormIDFromIdentifier(keyID);

							newHolster.holsterName = BSFixedString(iniWeap.GetValue(i->pItem, "sName", " "));

							holsters.Push(newHolster);
						}
					}
				}
			}
			values.clear();
			holsters.capacity = holsters.count;
			return true;
		}
	}
	return false;
}


// -------- ATGameData functions:

// loads critical effects from ini
bool ATGameData::LoadData_CritEffectTables(const std::string & configDir)
{
	CSimpleIniA iniCritTables;
	iniCritTables.SetUnicode();
	iniCritTables.SetMultiKey(true);

	if (iniCritTables.LoadFile(configDir.c_str()) > -1) {
		CSimpleIni::TNamesDepend sections;
		iniCritTables.GetAllSections(sections);
		
		if (sections.size() > 0) {
			CSimpleIni::TNamesDepend values;
			CSimpleIni::TNamesDepend::const_iterator i = sections.begin();
			for (; i != sections.end(); ++i) {
				ATCritEffectTable newTable;
				const char *tableName = i->pItem;
				newTable.hudName = BSFixedString(iniCritTables.GetValue(tableName, "sName", " "));
				const char *keyID = iniCritTables.GetValue(tableName, "sCritKW", "none");
				newTable.objectID = ATUtilities::GetFormIDFromIdentifier(keyID);
				
				if (iniCritTables.GetAllValues(tableName, "sCritEffects", values)) {
					if (values.size() > 0) {
						CSimpleIni::TNamesDepend::const_iterator j = values.begin();
						// default/fallback crit effects:
						for (; j != values.end(); ++j) {
							ATCritEffect newCritEffect;
							if (ATUtilities::GetCritEffectFromIdentifier(j->pItem, newCritEffect)) {
								newTable.defaultEffects.Push(newCritEffect);
							}
						}
						newTable.defaultEffects.capacity = newTable.defaultEffects.count;

						int foundIndex = GetCritTableIndex(newTable.objectID);
						if (foundIndex == -1) {
							ATCritEffectTables.Push(newTable);
						}
					}
				}
				values.clear();
			}
			sections.clear();
			ATCritEffectTables.capacity = ATCritEffectTables.count;
			return true;
		}
	}
	return false;
}


// loads AmmoTweaks data from ini files at the the specified path
bool ATGameData::LoadData_Template(const std::string & configDir)
{
	std::string tempIniPath = configDir.c_str();
	CSimpleIniA iniIndex;
	CSimpleIni::TNamesDepend values;
	std::string tempLoadPath;
	UInt32 loadMsg = 0;

	iniIndex.SetUnicode();
	iniIndex.SetMultiKey(true);
	
	tempIniPath.append("\\Weapons\\Index.ini");
	if (iniIndex.LoadFile(tempIniPath.c_str()) > -1) {
		const char * tempKey = "";
		// -------- Weapons
		// ranged
		if (iniIndex.GetAllValues("Weapons", "sRangedWeapons", values)) {
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				tempKey = i->pItem;
				tempLoadPath += configDir;
				tempLoadPath.append("\\Weapons\\Ranged\\");
				tempLoadPath.append(tempKey);
				tempLoadPath.append("\\");
				
				loadMsg = -1;
				ATWeapon newWeapon;
				if (newWeapon.ReadIni(tempLoadPath.c_str())) {
					int foundIndex = GetWeaponIndex(newWeapon.objectID);
					if (foundIndex == -1) {
						loadMsg = newWeapon.objectID;
						ATWeapons.Push(newWeapon);
					}
					else {
						ATWeapons[foundIndex] = newWeapon;
					}
				}

				if (loadMsg <= 0) {
					_MESSAGE("      !Failed!:  %s", tempKey);
					return false;
				}
				tempLoadPath.clear();
			}
		}
		values.clear();

		// melee
		if (iniIndex.GetAllValues("Weapons", "sMeleeWeapons", values)) {
			CSimpleIni::TNamesDepend::const_iterator i = values.begin();
			for (; i != values.end(); ++i) {
				tempKey = i->pItem;
				tempLoadPath += configDir;
				tempLoadPath.append("\\Weapons\\Melee\\");
				tempLoadPath.append(tempKey);
				tempLoadPath.append("\\");
				loadMsg = -1;

				ATWeapon newWeapon;
				if (newWeapon.ReadIni(tempLoadPath.c_str())) {
					int foundIndex = GetWeaponIndex(newWeapon.objectID);
					if (foundIndex == -1) {
						loadMsg = newWeapon.objectID;
						ATWeapons.Push(newWeapon);
					}
					else {
						ATWeapons[foundIndex] = newWeapon;
					}
				}

				if (loadMsg <= 0) {
					_MESSAGE("      !!Failed:  %s", tempKey);
					return false;
				}
				tempLoadPath.clear();
			}
		}
		values.clear();
		
		// thrown
		if (iniIndex.GetAllValues("Weapons", "sThrownWeapons", values)) {
			_MESSAGE("    [Thrown Weapons]:      %i", values.size());
		}
		values.clear();

		ATWeapons.capacity = ATWeapons.count;

		// -------- critical effect tables
		tempLoadPath += configDir;
		tempLoadPath.append("\\CritEffectTables.ini");
		LoadData_CritEffectTables(tempLoadPath);
		tempLoadPath.clear();

		return true;
	}
	tempIniPath.clear();
	return false;
}

// debug log
bool ATGameData::LogConfigData()
{
	_MESSAGE("\nStored data:");

	std::string dispStr;
	dispStr.append("\n  [Crit Effect Tables](");
	dispStr.append(std::to_string(ATCritEffectTables.count));
	dispStr.append(")\n");
	for (UInt32 i = 0; i < ATCritEffectTables.count; i++) {
		ATCritEffectTable curCET = ATCritEffectTables[i];
		dispStr.append("    ");
		dispStr.append(std::to_string(curCET.objectID));
		dispStr.append(" - ");
		dispStr.append(curCET.hudName.c_str());
		dispStr.append("\n");
	}
	_MESSAGE(dispStr.c_str());
	dispStr.clear();

	_MESSAGE("\n  [Weapons] (%i):", ATWeapons.count);
	for (UInt32 i = 0; i < ATWeapons.count; i++) {
		ATWeapon curWeap = ATWeapons[i];
		dispStr.append("\n    0x%08X : ");
		dispStr.append(curWeap.hudName.c_str());
		dispStr.append("\n");
		if (curWeap.calibers.count > 0) {
			dispStr.append("      ");
			dispStr.append(std::to_string(curWeap.calibers.count));
			dispStr.append(" Calibers: ");
			for (UInt32 j = 0; j < curWeap.calibers.count; j++) {
				dispStr.append(std::to_string(j));
				dispStr.append(" - ");
				dispStr.append(curWeap.calibers[j].hudName.c_str());
				dispStr.append(", ");
			}
		}
		_MESSAGE(dispStr.c_str(), curWeap.objectID);
		dispStr.clear();
		_MESSAGE("     ModSlots:");
		if (curWeap.modSlots.count > 0) {
			for (UInt32 i = 0; i < curWeap.modSlots.count; i++) {
				dispStr.append("\n      ");
				dispStr.append(std::to_string(curWeap.modSlots[i].swappableMods.count));
				dispStr.append(" ");
				dispStr.append(curWeap.modSlots[i].slotName.c_str());
				dispStr.append(": ");
				for (UInt32 j = 0; j < curWeap.modSlots[i].swappableMods.count; j++) {
					dispStr.append(std::to_string(j));
					dispStr.append(" - ");
					dispStr.append(curWeap.modSlots[i].swappableMods[j].swapMod->fullName.name.c_str());
					dispStr.append(", ");
				}
			}
		}
		_MESSAGE(dispStr.c_str());
		dispStr.clear();
		if (curWeap.damagedModSlots.count > 0) {
			_MESSAGE("     Damaged ModSlots:");
			for (UInt32 i = 0; i < curWeap.damagedModSlots.count; i++) {
				dispStr.append("\n      ");
				dispStr.append(std::to_string(curWeap.damagedModSlots[i].damagedMods.count));
				dispStr.append(" ");
				dispStr.append(curWeap.damagedModSlots[i].slotName.c_str());
				dispStr.append(": ");
				for (UInt32 j = 0; j < curWeap.damagedModSlots[i].damagedMods.count; j++) {
					dispStr.append(std::to_string(j));
					dispStr.append(" - ");
					dispStr.append(curWeap.damagedModSlots[i].damagedMods[j]->fullName.name.c_str());
					dispStr.append(", ");
				}
			}
		}
		_MESSAGE(dispStr.c_str());
		dispStr.clear();
	}
	return true;
}



// loads everything needed for AmmoTweaks to function
void ATGameData::LoadGameData()
{
	CSimpleIniA iniMain;
	iniMain.SetUnicode();

	// seed the RNG once
	//	- nanoseconds since 01-01-1970 is probably overkill, but whatever....
	ATUtilities::ATrng.Seed(std::chrono::high_resolution_clock::now().time_since_epoch().count());

	// load global config
	// - things that require a game restart to kick in
	if (iniMain.LoadFile(".\\Data\\F4SE\\Plugins\\AmmoTweaks.ini") > -1) {
		// load AmmoTweaks configuration data
		const char *configDataPath = iniMain.GetValue("General", "sConfigFoldersPath", "");
		if (configDataPath != "") {
			bool bShowDebugInfo =	iniMain.GetBoolValue("General", "bShowDebugInfo", false);
			bool bLoadedSomething = false;

			std::string tempDirStr;
			const char * templateToLoad = "";

			CSimpleIni::TNamesDepend templates;
			if (iniMain.GetAllValues("General", "sTemplatesToLoad", templates)) {
				
				CSimpleIni::TNamesDepend::const_iterator i = templates.begin();
				for (; i != templates.end(); ++i) {
					templateToLoad = i->pItem;

					_MESSAGE("\nLoading Template: %s...", templateToLoad);

					tempDirStr.append(configDataPath);
					tempDirStr.append(templateToLoad);
					if (LoadData_Template(tempDirStr)) {
						bLoadedSomething = true;
					}
					else
						_MESSAGE("\n!!Unable to load template", templateToLoad);

					tempDirStr.clear();
				}
			}

			if (bLoadedSomething) {
				_MESSAGE("\nAmmoTweaks data is ready.");
				if (bShowDebugInfo) {
					LogConfigData();
				}
			}
		}
		else
			_MESSAGE("\n!!Unable to load config path.");
	}

}


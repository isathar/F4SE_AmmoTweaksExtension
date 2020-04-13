#include "ConfigReader.h"

#include "include/SimpleIni/SimpleIni.h"
#include "include/nlohmann/json.hpp"

#include <chrono>
#include <Windows.h>
#include <fstream>
#include <iostream>


using json = nlohmann::json;


namespace ATConfigReader
{
	// -------- read operations:

	// returns a formID from a formatted string
	UInt32 GetFormIDFromIdentifier(const std::string & formIdentifier)
	{
		UInt32 formId = 0;
		if (formIdentifier.c_str() != "none") {
			std::size_t pos = formIdentifier.find_first_of("|");
			std::string modName = formIdentifier.substr(0, pos);
			std::string modForm = formIdentifier.substr(pos + 1);
			sscanf_s(modForm.c_str(), "%X", &formId);

			if (formId != 0x0) {
				UInt8 modIndex = (*g_dataHandler)->GetLoadedModIndex(modName.c_str());
				if (modIndex != 0xFF) {
					formId |= ((UInt32)modIndex) << 24;
				}
				else {
					UInt16 lightModIndex = (*g_dataHandler)->GetLoadedLightModIndex(modName.c_str());
					if (lightModIndex != 0xFFFF) {
						formId |= 0xFE000000 | (UInt32(lightModIndex) << 12);
					}
					else {
						_MESSAGE("FormID %s not found!", formIdentifier.c_str());
						formId = 0;
					}
				}
			}
		}
		return formId;
	}

	// returns a form from a formatted string
	TESForm * GetFormFromIdentifier(const std::string & formIdentifier)
	{
		UInt32 formId = GetFormIDFromIdentifier(formIdentifier);
		return (formId > 0x0) ? LookupFormByID(formId) : nullptr;
	}

	// returns true if the passed mod is loaded
	bool IsModLoaded(const std::string & modName)
	{
		if ((*g_dataHandler)->GetLoadedModIndex(modName.c_str()) != (UInt8)-1) {
			return true;
		}
		return ((*g_dataHandler)->GetLoadedLightModIndex(modName.c_str()) != (UInt16)-1);
	}

	std::vector<std::string> GetFolderNames(const std::string filePath)
	{
		std::vector<std::string> names;
		std::string search_path = filePath + "/*.*";
		WIN32_FIND_DATA fd;
		std::string tempName;
		std::size_t pos;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
					tempName = fd.cFileName;
					// exclude \. and \..
					pos = tempName.find_first_of(".");
					if (pos != std::string::npos) {
						names.push_back(fd.cFileName);
					}
				}
			} while (::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
		return names;
	}

	// returns a list of json file names at the passed path
	std::vector<std::string> GetFileNames(const std::string & folder)
	{
		std::vector<std::string> names;
		std::string search_path = folder + "/*.json";
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		if (hFind != INVALID_HANDLE_VALUE) {
			do {
				if (!(fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
					names.push_back(fd.cFileName);
				}
			} while (::FindNextFile(hFind, &fd));
			::FindClose(hFind);
		}
		return names;
	}

	// returns true if the passed path/filename exists
	bool IsPathValid(const std::string & path)
	{
		std::string search_path = path + "*";
		WIN32_FIND_DATA fd;
		HANDLE hFind = ::FindFirstFile(search_path.c_str(), &fd);
		return (hFind != INVALID_HANDLE_VALUE);
	}



	// -------- data building functions:
	
	bool BuildData_Caliber(const std::string & dataPath, ATCaliberData & newCaliber, const UInt8 iDebugLevel)
	{
		json caliberData;
		std::ifstream loadedFile(dataPath.c_str());
		loadedFile >> caliberData;
		loadedFile.close();

		if (caliberData.is_null() || caliberData["sCaliberID"].is_null() || caliberData["sCaliberName"].is_null()) {
			_MESSAGE("      WARNING: caliber data, name, or caliberID is null!");
			loadedFile.close();
			return false;
		}
		if (caliberData["ammoTypes"].is_null() || caliberData["ammoTypes"].empty()) {
			_MESSAGE("      WARNING: Caliber is missing ammo types!");
			loadedFile.close();
			return false;
		}
		std::string caliberIDStr = caliberData["sCaliberID"];
		newCaliber.iCaliberKWID = ATConfigReader::GetFormIDFromIdentifier(caliberIDStr);
		if (newCaliber.iCaliberKWID == 0) {
			_MESSAGE("      WARNING: unable to load caliber keyword!");
			loadedFile.close();
			return false;
		}

		std::string caliberNameStr = caliberData["sCaliberName"];
		newCaliber.sCaliberName = caliberNameStr.c_str();
		
		_MESSAGE("      Building caliber data for %s - 0x%08X", caliberIDStr.c_str(), newCaliber.iCaliberKWID);

		if (!caliberData["sRecipeID"].is_null()) {
			std::string recipeIDStr = caliberData["sRecipeID"];
			newCaliber.iRecipeKWID = ATConfigReader::GetFormIDFromIdentifier(recipeIDStr);
			if (newCaliber.iRecipeKWID == 0) {
				_MESSAGE("        WARNING: unable to load recipe keyword!");
			}
		}
		else {
			_MESSAGE("        WARNING: no recipe keyword id given!");
		}

		// base caliber variables
		if (!caliberData["fBaseWear"].is_null()) {
			newCaliber.fBaseWear = caliberData["fBaseWear"];
			_MESSAGE("        Wear:                %.02f", newCaliber.fBaseWear);
		}
		if (!caliberData["fBaseArmorPenetration"].is_null()) {
			newCaliber.fBaseArmorPenetration = caliberData["fBaseArmorPenetration"];
			_MESSAGE("        Armor Penetration:   %.02f", newCaliber.fBaseArmorPenetration);
		}
		if (!caliberData["fBaseTargetArmorMult"].is_null()) {
			newCaliber.fBaseTargetArmorMult = caliberData["fBaseTargetArmorMult"];
			_MESSAGE("        Target Armor Mult:   %.02f", newCaliber.fBaseTargetArmorMult);
		}
		if (!caliberData["fBaseCritFailMult"].is_null()) {
			newCaliber.fBaseCritFailMult = caliberData["fBaseCritFailMult"];
			_MESSAGE("        Crit Fail Mult:      %.02f", newCaliber.fBaseCritFailMult);
		}
		if (!caliberData["fBaseCritMult"].is_null()) {
			newCaliber.fBaseCritMult = caliberData["fBaseCritMult"];
			_MESSAGE("        Crit Mult:           %.02f", newCaliber.fBaseCritMult);
		}

		// ammo subtypes
		json ammoTypesObj;
		for (json::iterator itAmmoTypes = caliberData["ammoTypes"].begin(); itAmmoTypes != caliberData["ammoTypes"].end(); ++itAmmoTypes) {
			ammoTypesObj = *itAmmoTypes;
			if (ammoTypesObj.is_null() || ammoTypesObj.empty()) {
				_MESSAGE("        WARNING: Empty or null ammoType!");
				continue;
			}
			// AmmoTypes require a name, item, and mod. Skip if either doesn't exist
			if (ammoTypesObj["sAmmoID"].is_null() || ammoTypesObj["sName"].is_null() || ammoTypesObj["sModID"].is_null()) {
				_MESSAGE("        WARNING: No ammo name, item ID, or mod ID!");
				ammoTypesObj.clear();
				continue;
			}

			std::string ammoIDStr = ammoTypesObj["sAmmoID"];
			ATCaliberData::AmmoTypesData newAmmoType = ATCaliberData::AmmoTypesData();
			newAmmoType.iAmmoItemID = GetFormIDFromIdentifier(ammoIDStr);
			if (newAmmoType.iAmmoItemID == 0) {
				_MESSAGE("        WARNING: No ammo item!");
				ammoTypesObj.clear();
				continue;
			}
			std::string ammoModIDStr = ammoTypesObj["sModID"];
			newAmmoType.iAmmoModID = GetFormIDFromIdentifier(ammoModIDStr);
			if (newAmmoType.iAmmoModID == 0) {
				_MESSAGE("        WARNING: No mod item!");
				ammoTypesObj.clear();
				continue;
			}

			std::string ammoNameStr = ammoTypesObj["sName"];
			newAmmoType.sAmmoName = ammoNameStr.c_str();
			_MESSAGE("        Creating ammo type - %s", newAmmoType.sAmmoName.c_str());

			// casing item + chance
			std::string casingIDStr = ammoTypesObj["sCasingID"];
			newAmmoType.iCasingItemID = GetFormIDFromIdentifier(casingIDStr);
			if (newAmmoType.iCasingItemID == 0) {
				_MESSAGE("          WARNING: No casing item!");
			}
			else {
				// only get casing chance if a casing item exists
				if (!ammoTypesObj["fCasingChance"].is_null()) {
					newAmmoType.fCasingDropChance = ammoTypesObj["fCasingChance"];
					_MESSAGE("          Casing item:       0x%08X\n          Casing chance:     %.04f", newAmmoType.iCasingItemID, newAmmoType.fCasingDropChance);
				}
			}

			if (!ammoTypesObj["fArmorPenetration"].is_null()) {
				newAmmoType.fArmorPenetration = ammoTypesObj["fArmorPenetration"];
				_MESSAGE("          Armor Penetration: %.02f", newAmmoType.fArmorPenetration);
			}
			if (!ammoTypesObj["fTargetArmorMult"].is_null()) {
				newAmmoType.fTargetArmorMult = ammoTypesObj["fTargetArmorMult"];
				_MESSAGE("          Target Armor Mult: %.02f", newAmmoType.fTargetArmorMult);
			}
			if (!ammoTypesObj["fWearMult"].is_null()) {
				newAmmoType.fWearMult = ammoTypesObj["fWearMult"];
				_MESSAGE("          Wear Multiplier:   %.02f", newAmmoType.fWearMult);
			}
			if (!ammoTypesObj["fCritFailMult"].is_null()) {
				newAmmoType.fCritFailMult = ammoTypesObj["fCritFailMult"];
				_MESSAGE("          Crit Fail Mult:    %.02f", newAmmoType.fCritFailMult);
			}
			if (!ammoTypesObj["fCritMult"].is_null()) {
				newAmmoType.fCritMult = ammoTypesObj["fCritMult"];
				_MESSAGE("          Crit Mult:         %.02f", newAmmoType.fCritMult);
			}
			newCaliber.ammoTypes.push_back(newAmmoType);
		}
		
		// projectile overrides
		if (!caliberData["projectiles"].is_null() || !caliberData["projectiles"].empty()) {
			json projectilesObj;
			for (json::iterator itProjectiles = caliberData["projectiles"].begin(); itProjectiles != caliberData["projectiles"].end(); ++itProjectiles) {
				projectilesObj = *itProjectiles;
				if (projectilesObj.is_null() || projectilesObj.empty()) {
					_MESSAGE("        WARNING: Empty or null projectile override!");
					continue;
				}

				if (projectilesObj["sProjectileID"].is_null()) {
					_MESSAGE("        WARNING: Missing Projectile ID!");
					continue;
				}

				ATCaliberData::ProjectileData newProjOverride = ATCaliberData::ProjectileData();
				std::string projIDStr = projectilesObj["sProjectileID"];
				newProjOverride.iProjectileID = GetFormIDFromIdentifier(projIDStr);
				if (newProjOverride.iProjectileID == 0) {
					_MESSAGE("        WARNING: Can't find projectile form!");
					continue;
				}

				if (!projectilesObj["sImpactDataSetID"].is_null()) {
					std::string impactIDStr = projectilesObj["sImpactDataSetID"];
					newProjOverride.iImpactDataSetID = GetFormIDFromIdentifier(impactIDStr);
					if (newProjOverride.iImpactDataSetID == 0) {
						_MESSAGE("        WARNING: no impactDataSet form!");
					}
				}

				if (!projectilesObj["reqKeywords"].is_null() && projectilesObj["reqKeywords"].is_array() && !projectilesObj["reqKeywords"].empty()) {
					for (json::iterator itKWs = projectilesObj["reqKeywords"].begin(); itKWs != projectilesObj["reqKeywords"].end(); ++itKWs) {
						std::string kwIDStr = *itKWs;
						UInt32 newKWID = GetFormIDFromIdentifier(kwIDStr);
						if (newKWID != 0) {
							newProjOverride.projectileKWIDs.push_back(newKWID);
						}
					}
				}

				_MESSAGE("        Adding projectile: %s", projIDStr.c_str());
				newCaliber.projectileOverrides.push_back(newProjOverride);
			}
		}

		if (!caliberData["iLeveledItemChance"].is_null()) {
			int iLeveledItemChance = caliberData["iLeveledItemChance"];
			UInt8 iActualLLChance = (UInt8)(min(100, max(0, 100 - iLeveledItemChance)));
			if (!caliberData["sLeveledItemLoot"].is_null()) {
				std::string caliberLootIDStr = caliberData["sLeveledItemLoot"];
				TESLevItem * caliberLootLI = (TESLevItem*)GetFormFromIdentifier(caliberLootIDStr);
				if (caliberLootLI) {
					caliberLootLI->leveledList.unk2A = iActualLLChance;
					_MESSAGE("        Loot chance:       %i", iLeveledItemChance);
				}
			}
			if (!caliberData["sLeveledItemScrounger"].is_null()) {
				std::string caliberScroungerIDStr = caliberData["sLeveledItemScrounger"];
				TESLevItem * caliberScroungerLI = (TESLevItem*)GetFormFromIdentifier(caliberScroungerIDStr);
				if (caliberScroungerLI) {
					caliberScroungerLI->leveledList.unk2A = iActualLLChance;
					_MESSAGE("        Scrounger chance:  %i", iLeveledItemChance);
				}
			}
		}

		//loadedFile.close();
		return true;
	}


	// creates a ATCaliber slot from the passed ATWeapon's and this ATCaliberData's stats
	bool CreateWeaponCaliber(ATWeapon & weaponBase, ATCaliberData & caliberBase, ATCaliber & caliberOut)
	{
		if (caliberBase.iCaliberKWID == 0) {
			return false;
		}
		caliberOut.kwCaliberID = (BGSKeyword*)LookupFormByID(caliberBase.iCaliberKWID);
		if (!caliberOut.kwCaliberID) {
			return false;
		}
		if (caliberBase.sCaliberName.empty()) {
			return false;
		}
		if (caliberBase.ammoTypes.empty()) {
			return false;
		}

		caliberOut.sCaliberName = caliberBase.sCaliberName.c_str();
		if (caliberBase.iCraftingPerkID != 0) {
			caliberOut.perkCraftingSkill = (BGSPerk*)LookupFormByID(caliberBase.iCraftingPerkID);
		}

		// ammoTypes
		for (std::vector<ATCaliberData::AmmoTypesData>::iterator ammoTypeIt = caliberBase.ammoTypes.begin(); ammoTypeIt != caliberBase.ammoTypes.end(); ++ammoTypeIt) {
			ATCaliberData::AmmoTypesData ammoData = *ammoTypeIt;
			if (ammoData.iAmmoItemID == 0 || ammoData.iAmmoModID == 0 || ammoData.sAmmoName.empty()) {
				continue;
			}
			ATCaliber::AmmoType newAmmoType = ATCaliber::AmmoType();
			newAmmoType.sAmmoName = ammoData.sAmmoName.c_str();
			newAmmoType.ammoItem = LookupFormByID(ammoData.iAmmoItemID);
			if (!newAmmoType.ammoItem) {
				continue;
			}
			newAmmoType.ammoMod = (BGSMod::Attachment::Mod*)LookupFormByID(ammoData.iAmmoModID);
			if (!newAmmoType.ammoMod) {
				continue;
			}
			if (ammoData.iCasingItemID != 0) {
				newAmmoType.casingItem = (TESObjectMISC*)LookupFormByID(ammoData.iCasingItemID);
				if (newAmmoType.casingItem) {
					newAmmoType.fCasingDropChance = ammoData.fCasingDropChance;
				}
			}
			newAmmoType.modEffects.fCritMult = caliberBase.fBaseCritMult * ammoData.fCritMult;
			newAmmoType.modEffects.fCritFailMult = caliberBase.fBaseCritFailMult * ammoData.fCritFailMult;
			newAmmoType.modEffects.fArmorPenetrationAdd = caliberBase.fBaseArmorPenetration + ammoData.fArmorPenetration;
			newAmmoType.modEffects.fTargetArmorMult = caliberBase.fBaseTargetArmorMult * ammoData.fTargetArmorMult;
			newAmmoType.modEffects.fWearMult = caliberBase.fBaseWear * ammoData.fWearMult;
			
			caliberOut.ammoTypes.push_back(newAmmoType);
		}

		// projectiles
		if (caliberBase.projectileOverrides.empty()) {
			return true;
		}
		for (std::vector<ATCaliberData::ProjectileData>::iterator projectilesIt = caliberBase.projectileOverrides.begin(); projectilesIt != caliberBase.projectileOverrides.end(); ++projectilesIt) {
			ATCaliberData::ProjectileData projData = *projectilesIt;
			if (projData.iProjectileID == 0) {
				continue;
			}
			ATCaliber::ProjectileOverride newProjOverride = ATCaliber::ProjectileOverride();
			newProjOverride.projectile = (BGSProjectile*)LookupFormByID(projData.iProjectileID);
			if (!newProjOverride.projectile) {
				continue;
			}
			if (projData.iImpactDataSetID != 0) {
				newProjOverride.impactData = (BGSImpactDataSet*)LookupFormByID(projData.iImpactDataSetID);
			}
			if (!projData.projectileKWIDs.empty()) {
				for (std::vector<UInt32>::iterator kwIt = projData.projectileKWIDs.begin(); kwIt != projData.projectileKWIDs.end(); ++kwIt) {
					UInt32 newKWID = *kwIt;
					if (newKWID != 0) {
						BGSKeyword * newKWForm = (BGSKeyword*)LookupFormByID(newKWID);
						if (newKWForm) {
							newProjOverride.projectileKWs.push_back(newKWForm);
						}
					}
				}
			}
			caliberOut.projectiles.push_back(newProjOverride);
		}
		return true;
	}


	bool BuildData_Weapon(const std::string & dataPath, ATWeapon & newWeapon, const UInt8 iDebugLevel)
	{
		json weaponData;
		std::ifstream loadedFile(dataPath.c_str());
		loadedFile >> weaponData;
		loadedFile.close();

		if (weaponData.is_null() || weaponData["sWeaponID"].is_null() || weaponData["sWeaponName"].is_null()) {
			_MESSAGE("      WARNING: weapon data, sWeaponID or sWeaponName is null!");
			loadedFile.close();
			return 1;
		}
		std::string weaponID = weaponData["sWeaponID"];
		newWeapon.iWeaponID = GetFormIDFromIdentifier(weaponID);
		if (newWeapon.iWeaponID == 0) {
			_MESSAGE("      WARNING: unable to load weapon from ID!");
			loadedFile.close();
			return 2;
		}

		std::string weaponName = weaponData["sWeaponName"];
		newWeapon.sWeaponName = weaponName.c_str();

		_MESSAGE("      Building weapon Data for 0x%08X (%s)...", newWeapon.iWeaponID, newWeapon.sWeaponName.c_str());

		if (!weaponData["sRequiredValueID"].is_null()) {
			std::string skillIDStr = weaponData["sRequiredValueID"];
			newWeapon.avRequiredSkill = (ActorValueInfo*)GetFormFromIdentifier(skillIDStr);
			_MESSAGE("        Required skill:      0x%08X (%s)", newWeapon.avRequiredSkill->formID, newWeapon.avRequiredSkill->GetFullName());
		}
		if (!weaponData["fBaseMaxCondition"].is_null()) {
			newWeapon.fBaseMaxCondition = weaponData["fBaseMaxCondition"];
			_MESSAGE("        Base max CND:        %.04f", newWeapon.fBaseMaxCondition);
		}
		if (!weaponData["fBaseWearDivisor"].is_null()) {
			newWeapon.fBaseWearDivisor = weaponData["fBaseWearDivisor"];
			_MESSAGE("        Wear divisor:        %.04f", newWeapon.fBaseWearDivisor);
		}

		if (!weaponData["weaponIcons"].is_null() && !weaponData["weaponIcons"].empty()) {
			json iconsObj;
			for (json::iterator itIcons = weaponData["weaponIcons"].begin(); itIcons != weaponData["weaponIcons"].end(); ++itIcons) {
				iconsObj = *itIcons;
				if (iconsObj.is_null() || iconsObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon icon!");
					continue;
				}
				if (iconsObj["iIconIndex"].is_null()) {
					continue;
				}

				ATWeapon::HUDIconMod newHudIconMod = ATWeapon::HUDIconMod();
				newHudIconMod.iIconIndex = iconsObj["iIconIndex"];
				if (!iconsObj["sRequiredKW"].is_null()) {
					std::string kwIDStr = iconsObj["sRequiredKW"];
					newHudIconMod.reqKW = (BGSKeyword*)GetFormFromIdentifier(kwIDStr);
				}
				newWeapon.hudIcons.push_back(newHudIconMod);
			}
			_MESSAGE("        Weapon icons:        %i", newWeapon.hudIcons.size());
		}
		
		if (!weaponData["animations"].is_null() && !weaponData["animations"].empty()) {
			json animsObj;
			for (json::iterator animsIt = weaponData["animations"].begin(); animsIt != weaponData["animations"].end(); ++animsIt) {
				animsObj = *animsIt;
				if (animsObj.is_null() || animsObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon animation data!");
					continue;
				}

				ATWeapon::WeaponAnim newAnim = ATWeapon::WeaponAnim();
				if (!animsObj["iAnimID"].is_null()) {
					newAnim.iAnimID = animsObj["iAnimID"];
				}
				if (newAnim.iAnimID == -1) {
					continue;
				}
				if (!animsObj["sActionID"].is_null()) {
					std::string actionIDStr = animsObj["sActionID"];
					newAnim.animAction = (BGSAction*)GetFormFromIdentifier(actionIDStr);
				}
				if (!animsObj["sIdleID_1P"].is_null()) {
					std::string idle1IDStr = animsObj["sIdleID_1P"];
					newAnim.animIdle_1P = GetFormFromIdentifier(idle1IDStr);
				}
				if (!animsObj["sIdleID_3P"].is_null()) {
					std::string idle3IDStr = animsObj["sIdleID_3P"];
					newAnim.animIdle_3P = GetFormFromIdentifier(idle3IDStr);
				}
				if (!newAnim.animAction && (!newAnim.animIdle_1P || !newAnim.animIdle_3P)) {
					continue;
				}
				if (!animsObj["sFinishedEvent"].is_null()) {
					std::string idleEvtIDStr = animsObj["sFinishedEvent"];
					newAnim.sAnimFinishedEvent = idleEvtIDStr.c_str();
				}
				newWeapon.weaponAnims.push_back(newAnim);
			}
			_MESSAGE("        Animations:          %i", newWeapon.weaponAnims.size());
		}
		
		if (!weaponData["ammo"].is_null()) {
			json ammoObj = weaponData["ammo"];
			if (!ammoObj["calibers"].is_null() && !ammoObj["calibers"].empty()) {
				for (json::iterator calibersIt = ammoObj["calibers"].begin(); calibersIt != ammoObj["calibers"].end(); ++calibersIt) {
					json caliberObj = *calibersIt;
					if (caliberObj.is_null() || caliberObj.empty()) {
						_MESSAGE("        WARNING: Empty or null weapon caliber data!");
						continue;
					}
					if (caliberObj["sCaliberID"].is_null()) {
						continue;
					}
					std::string caliberIDStr = caliberObj["sCaliberID"];
					UInt32 iCaliberID = GetFormIDFromIdentifier(caliberIDStr);
					if (iCaliberID == 0) {
						continue;
					}
					ATCaliberData newCaliberData;
					if (!ATGameData::GetCaliberDataByID(iCaliberID, newCaliberData)) {
						continue;
					}

					ATCaliber newCaliberSlot = ATCaliber();
					if (CreateWeaponCaliber(newWeapon, newCaliberData, newCaliberSlot)) {
						if (!caliberObj["iReloadType"].is_null()) {
							newCaliberSlot.iReloadType = caliberObj["iReloadType"];
						}
						if (!caliberObj["iAnimID"].is_null()) {
							newCaliberSlot.iReloadAnimIndex = caliberObj["iAnimID"];
						}
						_MESSAGE("        Adding caliber:      0x%08X (%s)", newCaliberSlot.kwCaliberID->formID, newCaliberSlot.sCaliberName.c_str());
						newWeapon.calibers.push_back(newCaliberSlot);
					}
				}
			}

			if (!ammoObj["magazines"].is_null() && !ammoObj["magazines"].empty()) {
				for (json::iterator magsIt = ammoObj["magazines"].begin(); magsIt != ammoObj["magazines"].end(); ++magsIt) {
					json magObj = *magsIt;
					if (magObj.is_null() || magObj.empty()) {
						_MESSAGE("        WARNING: Empty or null weapon magazine data!");
						continue;
					}
					if (magObj["sMiscItemID"].is_null() || magObj["sModID"].is_null()) {
						continue;
					}

					ATWeapon::MagazineItem newMagItem = ATWeapon::MagazineItem();
					std::string magItemIDStr = magObj["sMiscItemID"];
					newMagItem.magItem = (TESObjectMISC*)GetFormFromIdentifier(magItemIDStr);
					if (!newMagItem.magItem) {
						continue;
					}
					std::string magModIDStr = magObj["sModID"];
					newMagItem.magMod = (BGSMod::Attachment::Mod*)GetFormFromIdentifier(magModIDStr);
					if (!newMagItem.magMod) {
						continue;
					}
					if (!magObj["iCapacity"].is_null()) {
						newMagItem.iCapacity = magObj["iCapacity"];
					}
					_MESSAGE("        Adding magazine:     0x%08X (%s)", newMagItem.magItem->formID, newMagItem.magMod->GetFullName());
					newWeapon.magazines.push_back(newMagItem);
				}
			}
		}

		if (!weaponData["firemodes"].is_null() && !weaponData["firemodes"].empty()) {
			for (json::iterator firemodesIt = weaponData["firemodes"].begin(); firemodesIt != weaponData["firemodes"].end(); ++firemodesIt) {
				json firemodeObj = *firemodesIt;
				if (firemodeObj.is_null() || firemodeObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon firemode data!");
					continue;
				}
				if (firemodeObj["sName"].is_null() || firemodeObj["sModID"].is_null()) {
					continue;
				}
				ATFiremode newFiremode = ATFiremode();
				std::string fireModeModIDStr = firemodeObj["sModID"];
				newFiremode.firemodeMod = (BGSMod::Attachment::Mod*)GetFormFromIdentifier(fireModeModIDStr);
				if (!newFiremode.firemodeMod) {
					continue;
				}
				std::string fireModeNameStr = firemodeObj["sName"];
				newFiremode.sFiremodeName = fireModeNameStr.c_str();
				_MESSAGE("        Adding firemode:     %s", newFiremode.sFiremodeName.c_str());

				if (!firemodeObj["sRecipeKWID"].is_null()) {
					std::string fireModeRecipeIDStr = firemodeObj["sRecipeKWID"];
					newFiremode.recipeKW = (BGSKeyword*)GetFormFromIdentifier(fireModeRecipeIDStr);
					if (newFiremode.recipeKW) {
						_MESSAGE("          Recipe KW:           0x%08X (%s)", newFiremode.recipeKW->formID, newFiremode.recipeKW->keyword.c_str());
					}
				}

				if (!firemodeObj["iIconIndex"].is_null()) {
					newFiremode.iHudIcon = firemodeObj["iIconIndex"];
					_MESSAGE("          HUD Icon:            %i", newFiremode.iHudIcon);
				}
				if (!firemodeObj["iAnimID"].is_null()) {
					newFiremode.iSwapAnimIndex = firemodeObj["iAnimID"];
					_MESSAGE("          Anim ID:             %i", newFiremode.iSwapAnimIndex);
				}
				if (!firemodeObj["fArmorPenetration"].is_null()) {
					newFiremode.modEffects.bUseEffect = true;
					newFiremode.modEffects.fArmorPenetrationAdd = firemodeObj["fArmorPenetration"];
					_MESSAGE("          Armor Penetration:   %.04f", newFiremode.modEffects.fArmorPenetrationAdd);
				}
				if (!firemodeObj["fTargetArmorMult"].is_null()) {
					newFiremode.modEffects.bUseEffect = true;
					newFiremode.modEffects.fTargetArmorMult = firemodeObj["fTargetArmorMult"];
					_MESSAGE("          Target Armor Mult:   %.04f", newFiremode.modEffects.fTargetArmorMult);
				}
				if (!firemodeObj["fWearMult"].is_null()) {
					newFiremode.modEffects.bUseEffect = true;
					newFiremode.modEffects.fWearMult = firemodeObj["fWearMult"];
					_MESSAGE("          Wear Mult:           %.04f", newFiremode.modEffects.fWearMult);
				}
				if (!firemodeObj["fCritMult"].is_null()) {
					newFiremode.modEffects.bUseEffect = true;
					newFiremode.modEffects.fCritMult = firemodeObj["fCritMult"];
					_MESSAGE("          Crit Mult:           %.04f", newFiremode.modEffects.fCritMult);
				}
				if (!firemodeObj["fCritFailMult"].is_null()) {
					newFiremode.modEffects.bUseEffect = true;
					newFiremode.modEffects.fCritFailMult = firemodeObj["fCritFailMult"];
					_MESSAGE("          Crit Fail Mult:      %.04f", newFiremode.modEffects.fCritFailMult);
				}
				if (!firemodeObj["sCritTableID"].is_null()) {
					std::string fmCritTableIDStr = firemodeObj["sCritTableID"];
					newFiremode.iCritEffectTableID = GetFormIDFromIdentifier(fmCritTableIDStr);
					_MESSAGE("          Crit Table ID:       0x%08X", newFiremode.iCritEffectTableID);
				}

				newWeapon.firemodes.push_back(newFiremode);
			}
		}

		if (!weaponData["swappableMods"].is_null() && !weaponData["swappableMods"].empty()) {
			json curSwapModSlotObj, curSwapModObj;
			for (json::iterator modsSlotsIt = weaponData["swappableMods"].begin(); modsSlotsIt != weaponData["swappableMods"].end(); ++modsSlotsIt) {
				curSwapModSlotObj = *modsSlotsIt;
				if (curSwapModSlotObj.is_null() || curSwapModSlotObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon mod slot data!");
					continue;
				}
				if (curSwapModSlotObj["sSlotName"].is_null() || curSwapModSlotObj["slotMods"].is_null() || curSwapModSlotObj["slotMods"].empty()) {
					continue;
				}

				ATModSlot newModSlot = ATModSlot();
				std::string modSlotName = curSwapModSlotObj["sSlotName"];
				newModSlot.sSlotName = modSlotName.c_str();
				if (!curSwapModSlotObj["iAnimID"].is_null()) {
					int iTempSwapAnim = curSwapModSlotObj["iAnimID"];
					newModSlot.iSwapAnimIndex = (UInt32)iTempSwapAnim;
				}
				if (!curSwapModSlotObj["bSaveAmmoCount"].is_null()) {
					newModSlot.bSaveAmmoCount = curSwapModSlotObj["bSaveAmmoCount"];
				}
				if (!curSwapModSlotObj["bShowInMenu"].is_null()) {
					newModSlot.bShowInMenu = curSwapModSlotObj["bShowInMenu"];
				}
				
				for (json::iterator modsIt = curSwapModSlotObj["slotMods"].begin(); modsIt != curSwapModSlotObj["slotMods"].end(); ++modsIt) {
					curSwapModObj = *modsIt;
					if (curSwapModObj["sName"].is_null() || curSwapModObj["sModID"].is_null()) {
						continue;
					}
					ATModSlot::SwappableMod newSwapMod = ATModSlot::SwappableMod();
					std::string swapModIDStr = curSwapModObj["sModID"];
					newSwapMod.swapMod = (BGSMod::Attachment::Mod*)GetFormFromIdentifier(swapModIDStr);
					if (!newSwapMod.swapMod) {
						continue;
					}

					std::string swapModNameStr = curSwapModObj["sName"];
					newSwapMod.sMenuName = swapModNameStr.c_str();
					if (!curSwapModObj["sRequiredItem"].is_null()) {
						std::string modItemIDStr = curSwapModObj["sRequiredItem"];
						newSwapMod.requiredItem = GetFormFromIdentifier(modItemIDStr);
					}
					if (!curSwapModObj["sRequiredKW"].is_null()) {
						std::string modReqKWStr = curSwapModObj["sRequiredKW"];
						newSwapMod.requiredKW = (BGSKeyword*)GetFormFromIdentifier(modReqKWStr);
					}
					if (!curSwapModObj["bRequireModMisc"].is_null()) {
						newSwapMod.bRequireModMisc = curSwapModObj["bRequireModMisc"];
					}
					newModSlot.swappableMods.push_back(newSwapMod);
				}
				
				if (!newModSlot.swappableMods.empty()) {
					_MESSAGE("        Adding ModSlot:      %s", newModSlot.sSlotName.c_str());
					newWeapon.modSlots.push_back(newModSlot);
				}
			}
		}

		if (!weaponData["damagedMods"].is_null() && !weaponData["damagedMods"].empty()) {
			json curDmgModSlotObj, curDmgModObj;
			for (json::iterator dmgSlotsIt = weaponData["damagedMods"].begin(); dmgSlotsIt != weaponData["damagedMods"].end(); ++dmgSlotsIt) {
				curDmgModSlotObj = *dmgSlotsIt;
				if (curDmgModSlotObj.is_null() || curDmgModSlotObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon CND mod slot data!");
					continue;
				}
				if (!curDmgModSlotObj["sSlotName"].is_null() && !curDmgModSlotObj["slotMods"].is_null() && !curDmgModSlotObj["slotMods"].empty()) {
					ATDamagedModSlot newDmgSlot = ATDamagedModSlot();
					std::string dmgSlotName = curDmgModSlotObj["sSlotName"];
					newDmgSlot.sSlotName = dmgSlotName.c_str();

					if (!curDmgModSlotObj["bRemoveAmmo"].is_null()) {
						newDmgSlot.bRemoveAmmo = curDmgModSlotObj["bRemoveAmmo"];
					}

					for (json::iterator dmgModsIt = curDmgModSlotObj["slotMods"].begin(); dmgModsIt != curDmgModSlotObj["slotMods"].end(); ++dmgModsIt) {
						curDmgModObj = *dmgModsIt;
						if (curDmgModObj["sModID"].is_null()) {
							continue;
						}
						std::string dmgModIDStr = curDmgModObj["sModID"];
						BGSMod::Attachment::Mod * newDmgMod = (BGSMod::Attachment::Mod*)GetFormFromIdentifier(dmgModIDStr);
						if (newDmgMod) {
							newDmgSlot.damagedMods.push_back(newDmgMod);
						}
					}

					if (!newDmgSlot.damagedMods.empty()) {
						_MESSAGE("        Adding CNDModSlot:   %s", newDmgSlot.sSlotName.c_str());
						newWeapon.damagedModSlots.push_back(newDmgSlot);
					}
				}
			}
		}

		if (!weaponData["critFailureTable"].is_null() && !weaponData["critFailureTable"].empty()) {
			json curCFObj;
			for (json::iterator cfIt = weaponData["critFailureTable"].begin(); cfIt != weaponData["critFailureTable"].end(); ++cfIt) {
				curCFObj = *cfIt;
				if (curCFObj.is_null() || curCFObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon crit failure data!");
					continue;
				}
				if (curCFObj["sName"].is_null() || curCFObj["iRollMax"].is_null()) {
					continue;
				}
				ATWeapon::CritFailure newCritFailure = ATWeapon::CritFailure();
				std::string critFailNameStr = curCFObj["sName"];
				newCritFailure.sMenuName = critFailNameStr.c_str();

				if (!curCFObj["sSpellID"].is_null()) {
					std::string critFailSpellStr = curCFObj["sSpellID"];
					newCritFailure.spellCritFailure = (SpellItem*)GetFormFromIdentifier(critFailSpellStr);
				}

				newCritFailure.iRollMax = curCFObj["iRollMax"];

				_MESSAGE("        Adding crit failure: %s", newCritFailure.sMenuName.c_str());
				newWeapon.criticalFailureTable.push_back(newCritFailure);
			}
		}

		if (!weaponData["visibleUnequipped"].is_null() && !weaponData["visibleUnequipped"].empty()) {
			json curVUObj;
			for (json::iterator vuIt = weaponData["visibleUnequipped"].begin(); vuIt != weaponData["visibleUnequipped"].end(); ++vuIt) {
				curVUObj = *vuIt;
				if (curVUObj.is_null() || curVUObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon visible unequipped data!");
					continue;
				}
				if (curVUObj["sName"].is_null() || curVUObj["sArmorID_Weapon"].is_null()) {
					continue;
				}

				ATWeapon::HolsteredArmor newHolsteredArmor = ATWeapon::HolsteredArmor();
				std::string holsterNameStr = curVUObj["sName"];
				newHolsteredArmor.sMenuName = holsterNameStr.c_str();

				std::string weapArmorIDStr = curVUObj["sArmorID_Weapon"];
				newHolsteredArmor.armorWeapon = (TESObjectARMO*)GetFormFromIdentifier(weapArmorIDStr);
				if (!newHolsteredArmor.armorWeapon) {
					continue;
				}
				if (!curVUObj["sArmorID_Holster"].is_null()) {
					std::string holsterArmorIDStr = curVUObj["sArmorID_Holster"];
					newHolsteredArmor.armorHolster = (TESObjectARMO*)GetFormFromIdentifier(weapArmorIDStr);
				}

				if (!curVUObj["reqKeywords"].is_null() && !curVUObj["reqKeywords"].empty()) {
					json reqKWObj;
					for (json::iterator kwIt = curVUObj["reqKeywords"].begin(); kwIt != curVUObj["reqKeywords"].end(); ++kwIt) {
						std::string newKWID = *kwIt;
						BGSKeyword * newReqKW = (BGSKeyword*)GetFormFromIdentifier(newKWID);
						if (newReqKW) {
							newHolsteredArmor.requiredKWs.push_back(newReqKW);
						}
					}
				}

				_MESSAGE("        Adding holster type: %s", newHolsteredArmor.sMenuName.c_str());
				newWeapon.holsters.push_back(newHolsteredArmor);
			}
		}

		if (!weaponData["modEffects"].is_null() && !weaponData["modEffects"].empty()) {
			json curMEObj;
			for (json::iterator modsEffectsIt = weaponData["modEffects"].begin(); modsEffectsIt != weaponData["modEffects"].end(); ++modsEffectsIt) {
				curMEObj = *modsEffectsIt;
				if (curMEObj.is_null() || curMEObj.empty()) {
					_MESSAGE("        WARNING: Empty or null weapon modEffect data!");
					continue;
				}
				ATWeaponModStats newModStat = ATWeaponModStats();
				if (!curMEObj["sRequiredKW"].is_null()) {
					std::string kwIDStr = curMEObj["sRequiredKW"];
					newModStat.checkKW = (BGSKeyword*)GetFormFromIdentifier(kwIDStr);
				}
				if (!curMEObj["fArmorPenetrationAdd"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fArmorPenetrationAdd = curMEObj["fArmorPenetrationAdd"];
				}
				if (!curMEObj["fTargetArmorMult"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fTargetArmorMult = curMEObj["fTargetArmorMult"];
				}
				if (!curMEObj["fWearMult"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fWearMult = curMEObj["fWearMult"];
				}
				if (!curMEObj["fCritMult"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fCritMult = curMEObj["fCritMult"];
				}
				if (!curMEObj["fCritFailMult"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fCritFailMult = curMEObj["fCritFailMult"];
				}
				if (!curMEObj["fMaxConditionMultAdd"].is_null()) {
					newModStat.bUseEffect = true;
					newModStat.fMaxConditionMultAdd = curMEObj["fMaxConditionMultAdd"];
				}
				if (newModStat.bUseEffect) {
					_MESSAGE("        Adding modEffects:   0x%08X (%s)", newModStat.checkKW ? newModStat.checkKW->formID : 0, newModStat.checkKW ? newModStat.checkKW->keyword.c_str() : "none");
					newWeapon.modEffects.push_back(newModStat);
				}
			}
		}

		//loadedFile.close();
		return true;
	}

	bool BuildData_CraftingRecipe(const std::string & dataPath)
	{
		json recipesData;
		std::ifstream loadedFile(dataPath.c_str());
		loadedFile >> recipesData;
		loadedFile.close();

		if (recipesData.is_null() || recipesData["recipes"].is_null() || recipesData["recipes"].empty()) {
			_MESSAGE("      WARNING: recipe data is null!");
			return false;
		}



		return true;
	}


	// -------- config file read functions:

	UInt8 LoadConfig_CaliberData(const std::string & folderPath, const UInt8 iDebugLevel)
	{
		std::string sFolderPathStr, sCurFilePath;
		std::vector<std::string> configPathsList;
		_MESSAGE("\n  Loading calibers from template (%s)", folderPath.c_str());

		sFolderPathStr.append(folderPath);
		sFolderPathStr.append("Calibers\\");
		configPathsList = GetFileNames(sFolderPathStr);
		if (!configPathsList.empty()) {
			_MESSAGE("\n    Loading %i Calibers:", configPathsList.size());
			for (std::vector<std::string>::iterator itFile = configPathsList.begin(); itFile != configPathsList.end(); ++itFile) {
				sCurFilePath.clear();
				sCurFilePath.append(sFolderPathStr);
				sCurFilePath.append(*itFile);
				_MESSAGE("\n      Loading %s", sCurFilePath.c_str());
				//LoadConfig_Caliber(sCurFilePath, iDebugLevel);
				ATCaliberData newCaliber = ATCaliberData();
				if (BuildData_Caliber(sCurFilePath, newCaliber, iDebugLevel)) {
					_MESSAGE("        Created caliber data - %s", newCaliber.sCaliberName.c_str());
					ATConfigData::ATCaliberBase.push_back(newCaliber);
				}
				else {
					_MESSAGE("      WARNING: Failed to create caliber data!");
				}
			}
			configPathsList.clear();
		}
		else {
			_MESSAGE("\n    Template does not contain any calibers.");
		}
		return 0;
	}


	UInt8 LoadConfig_Template(const std::string & folderPath, const UInt8 iDebugLevel)
	{
		std::string sFolderPathStr, sCurFilePath;
		std::vector<std::string> configPathsList;

		_MESSAGE("\n  Loading template - %s", folderPath.c_str());

		sFolderPathStr.append(folderPath);
		sFolderPathStr.append("Weapons\\");
		configPathsList = GetFileNames(sFolderPathStr);
		if (!configPathsList.empty()) {
			_MESSAGE("\n    Loading %i Weapons:", configPathsList.size());
			for (std::vector<std::string>::iterator itFile = configPathsList.begin(); itFile != configPathsList.end(); ++itFile) {
				sCurFilePath.clear();
				sCurFilePath.append(sFolderPathStr);
				sCurFilePath.append(*itFile);
				_MESSAGE("\n      Loading %s", sCurFilePath.c_str());
				ATWeapon newWeapon = ATWeapon();
				if (BuildData_Weapon(sCurFilePath, newWeapon, iDebugLevel)) {
					_MESSAGE("        Created weapon data - %s", newWeapon.sWeaponName.c_str());
					ATConfigData::ATWeapons.push_back(newWeapon);
				}
				else {
					_MESSAGE("      WARNING: Failed to create weapon data!");
				}
			}
			configPathsList.clear();
		}
		else {
			_MESSAGE("\n    Template does not contain any weapons.");
		}
		
		sFolderPathStr.clear();
		sFolderPathStr.append(folderPath);
		sFolderPathStr.append("CraftingRecipes\\");
		configPathsList = GetFileNames(sFolderPathStr);
		if (!configPathsList.empty()) {
			_MESSAGE("\n    Loading %i Crafting Recipes...", configPathsList.size());
			for (std::vector<std::string>::iterator itFile = configPathsList.begin(); itFile != configPathsList.end(); ++itFile) {
				sCurFilePath.clear();
				sCurFilePath.append(sFolderPathStr);
				sCurFilePath.append(*itFile);
				_MESSAGE("      %s", sCurFilePath.c_str());
			}
			configPathsList.clear();
		}
		else {
			_MESSAGE("\n    Template does not contain any crafting recipes.");
		}
		return 0;
	}


}


bool ATConfigReader::LoadGameData()
{
	bool bDataLoaded = false;
	if (!IsModLoaded("F4TweaksFramework.esm")) {
		_MESSAGE("ERROR: F4TweaksFramework.esm is missing.");
		return bDataLoaded;
	}
	if (!IsModLoaded("AmmoTweaks.esm")) {
		_MESSAGE("ERROR: AmmoTweaks.esm is missing.");
		return bDataLoaded;
	}
	
	// benchmark stuff
	int endTime = 0;
	int startTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();

	// defaults:
	const char *sTemplatesPath = ".\\Data\\AmmoTweaks\\";
	UInt8 iDebugLevel = 0;
	CSimpleIniA iniMain;

	// load main ini file to get paths or use defaults
	iniMain.SetUnicode();
	if (iniMain.LoadFile(".\\Data\\F4SE\\Plugins\\AmmoTweaks.ini") > -1) {
		sTemplatesPath = iniMain.GetValue("General", "sTemplatesPath", ".\\Data\\AmmoTweaks\\");
		iDebugLevel = iniMain.GetLongValue("General", "iDebugLevel", 0);
		ATGlobalVars::configDataPath = iniMain.GetValue("General", "sModConfigPath", ".\\Data\\MCM\\Settings\\AmmoTweaks.ini");
		ATGlobalVars::configDataPathBase = iniMain.GetValue("General", "sModConfigBasePath", ".\\Data\\MCM\\Config\\AmmoTweaks\\settings.ini");
		_MESSAGE("\nLoaded main configuration file.\n    Templates Path:   %s\n    Debug Level:      %i\n    Base Config Path: %s\n    Mod Config Path:  %s", sTemplatesPath, iDebugLevel, ATGlobalVars::configDataPathBase, ATGlobalVars::configDataPath);
	}
	else {
		_MESSAGE("\nERROR: Unable to load AmmoTweaks.ini.");
		return bDataLoaded;
	}

	ATConfigData::rng.Seed();

	ATConfigData::AV_WeaponAmmoCount = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|F84A");

	ATConfigData::AV_WeaponCNDMax = (ActorValueInfo*)GetFormFromIdentifier("F4TweaksFramework.esm|3E13");
	ATConfigData::AV_WeaponCNDWear = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|849");
	ATConfigData::AV_WeaponCNDPct = (ActorValueInfo*)GetFormFromIdentifier("F4TweaksFramework.esm|3E11");
	ATConfigData::AV_WeaponDamageLevel = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|1339");

	ATConfigData::AV_CritFailChance = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|B33");
	ATConfigData::AV_CritFailRollMod = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|12B2");
	ATConfigData::AV_CritFailMult = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|3612");

	ATConfigData::AV_RecoilMod = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|370A");
	ATConfigData::AV_SkillReqDiff = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|3170");
	ATConfigData::AV_SkillModActive = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|1DF2");

	ATConfigData::AV_StatusPoisoned = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E39");
	ATConfigData::AV_StatusBleeding = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E3C");
	ATConfigData::AV_StatusRadiated = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E3B");
	ATConfigData::AV_StatusBurning = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E38");
	ATConfigData::AV_StatusChilled = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E3D");
	ATConfigData::AV_StatusIonized = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E3A");
	ATConfigData::AV_StatusArmorDmg = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E3E");
	ATConfigData::AV_StatusBlinded = (ActorValueInfo*)GetFormFromIdentifier("AmmoTweaks.esm|6E2E");

	ATConfigData::AV_ArmorPenetration = (ActorValueInfo*)LookupFormByID(0x97341);
	ATConfigData::AV_ArmorPenetrationThrown = (ActorValueInfo*)GetFormFromIdentifier("F4TweaksFramework.esm|81E");
	ATConfigData::AV_TargetArmorMult = (ActorValueInfo*)GetFormFromIdentifier("F4TweaksFramework.esm|6457");
	ATConfigData::AV_TargetArmorMultThrown = (ActorValueInfo*)GetFormFromIdentifier("F4TweaksFramework.esm|6BF2");
	ATConfigData::AV_Luck = (ActorValueInfo*)LookupFormByID(0x2C8);
	
	ATConfigData::Ammo_Broken = (TESAmmo*)GetFormFromIdentifier("AmmoTweaks.esm|168E");
	ATConfigData::Ammo_Jammed = (TESAmmo*)GetFormFromIdentifier("AmmoTweaks.esm|1805");
	ATConfigData::Ammo_Overheated = (TESAmmo*)GetFormFromIdentifier("AmmoTweaks.esm|E0E9");

	ATConfigData::Mod_NullMagazine = (BGSMod::Attachment::Mod*)GetFormFromIdentifier("AmmoTweaks.esm|136F");

	// global settings
	if (!ATGlobalVars::LoadSettings()) {
		_MESSAGE("\n\nWARNING: MCM custom settings not found. This isn't a problem if you haven't edited any settings using MCM. If you have, check AmmoTweaks.ini for sMCMConfigPath.");
	}

	LoadConfig_CaliberData(sTemplatesPath, iDebugLevel);
	LoadConfig_Template(sTemplatesPath, iDebugLevel);

	endTime = std::chrono::high_resolution_clock::now().time_since_epoch().count();
	_MESSAGE("\nAmmoTweaks data is ready. Time elapsed: %.04f s\n\n", (float)(endTime - startTime) * 0.000000001);
	
	return true;
}

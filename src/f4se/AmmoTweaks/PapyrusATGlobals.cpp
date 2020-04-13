#include "PapyrusATGlobals.h"
#include "include/AimModelEdit.h"


namespace ATGlobals
{
	DECLARE_STRUCT(MagazineData, SCRIPTNAME_Globals)
	DECLARE_STRUCT(ModSwapIdle, SCRIPTNAME_Globals)
	DECLARE_STRUCT(WeaponHolsteredData, SCRIPTNAME_Globals)
	DECLARE_STRUCT(WeaponModData, SCRIPTNAME_Globals)


	// initializes equipped weapon variables (on equip)
	bool InitEquippedWeapon(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			return false;
		}
		ATWeapon tempWeap;
		if (!ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
			return false;
		}
		ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			return false;
		}
		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (!instanceData) {
			return false;
		}

		// -- Skill Requirement check --
		// - do this first since everything else won't need to be calculated if the instance is getting reset
		float fRecoilMult = 0.0;
		float fSkillReqDiff = 0.0;
		if (ATGlobalVars::bSkillRequirement) {
			if (tempWeap.avRequiredSkill) {
				// calculate the skill modifier
				float fCheckAmount = ownerActor->actorValueOwner.GetValue(tempWeap.avRequiredSkill);
				float fSkillReq = ATGameData::CalcSkillRequirement(instanceData->weight);
				fSkillReqDiff = round(min(max(fCheckAmount - fSkillReq, -ATGlobalVars::fSkillReq_MaxSkillDiff), ATGlobalVars::fSkillReq_MaxSkillDiff));
				fRecoilMult = floor((((fSkillReqDiff * (ATGlobalVars::fSkillReq_MaxRecoilDiff / ATGlobalVars::fSkillReq_MaxSkillDiff)))) * -100.0) / 100.0;
			}
		}

		// recoil multiplier - used by the skill requirement perk
		float fRecoilMod = ownerActor->actorValueOwner.GetMod(0, ATConfigData::AV_RecoilMod);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_RecoilMod, fRecoilMult);

		bool bIsPlayer = (ownerActor == (Actor*)*g_player);
		if (bIsPlayer) {
			// -- weapon instance edits: only do these for the player for now
			UInt32 iRecoilCheckInt = (UInt32)(int)floor((fRecoilMult + fRecoilMod) * 1000.0);
			UInt32 iSkillReqCheckInt = ATGameData::GetWeaponInstanceAVModifier(instanceData, ATConfigData::AV_SkillModActive);
			if (iSkillReqCheckInt != 0) {
				if (iRecoilCheckInt != iSkillReqCheckInt) {
					// reset the weapon instance to its default values
					return true;
				}
			}
			else {
				// apply instance edits, re-equip the weapon in script
				if (iRecoilCheckInt != 0) {
					ATGameData::SetWeaponInstanceAVModifier(instanceData, ATConfigData::AV_SkillModActive, iRecoilCheckInt);

					// -- update recoil if needed
					if (instanceData->aimModel) {
						ATAimModel * tempAimModel = (ATAimModel*)instanceData->aimModel;
						if (tempAimModel) {
							tempAimModel->Rec_HipMult = max(0.0, tempAimModel->Rec_HipMult + (tempAimModel->Rec_HipMult * fRecoilMult));
							tempAimModel->Rec_MinPerShot = max(0.0, tempAimModel->Rec_MinPerShot + (tempAimModel->Rec_MinPerShot * fRecoilMult));
							tempAimModel->Rec_MaxPerShot = max(tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot + (tempAimModel->Rec_MaxPerShot * fRecoilMult));
							
							_MESSAGE("\nSkillReq updated - Modifier: %.4f\n    Recoil - Min: %.4f, Max: %.4f, HipMult: %.4f",
								fRecoilMult, tempAimModel->Rec_MinPerShot, tempAimModel->Rec_MaxPerShot, tempAimModel->Rec_HipMult
							);
							return true;
						}
					}
				}
			}
		}

		// -- Weapon Stats Modifiers --
		ATWeaponModStats newWeaponStats = ATWeaponModStats();
		newWeaponStats.fCritMult = 100.0;
		newWeaponStats.fCritFailMult = 100.0;
		newWeaponStats.fWearMult = 100.0;

		// Firemode effects
		ATFiremode curFiremode;
		if (tempWeap.GetCurrentFiremode(extraData, curFiremode)) {
			_MESSAGE("Setting firemode stats");
			newWeaponStats.fCritMult *= curFiremode.modEffects.fCritMult;
			newWeaponStats.fCritFailMult *= curFiremode.modEffects.fCritFailMult;
			newWeaponStats.fArmorPenetrationAdd += curFiremode.modEffects.fArmorPenetrationAdd;
			newWeaponStats.fTargetArmorMult *= curFiremode.modEffects.fTargetArmorMult;
			newWeaponStats.fWearMult *= curFiremode.modEffects.fWearMult;
			newWeaponStats.fMaxConditionMultAdd += curFiremode.modEffects.fMaxConditionMultAdd;
		}

		// Caliber/Ammo effects
		ATCaliber::AmmoType curAmmoType;
		if (tempWeap.GetCurrentAmmoType(instanceData, extraData, curAmmoType)) {
			_MESSAGE("Setting ammoType stats");
			newWeaponStats.fCritMult *= curAmmoType.modEffects.fCritMult;
			newWeaponStats.fCritFailMult *= curAmmoType.modEffects.fCritFailMult;
			newWeaponStats.fArmorPenetrationAdd += curAmmoType.modEffects.fArmorPenetrationAdd;
			newWeaponStats.fTargetArmorMult *= curAmmoType.modEffects.fTargetArmorMult;
			newWeaponStats.fWearMult *= curAmmoType.modEffects.fWearMult;
			newWeaponStats.fMaxConditionMultAdd += curAmmoType.modEffects.fMaxConditionMultAdd;
		}

		// misc mod effects
		if (!tempWeap.modEffects.empty()) {
			for (std::vector<ATWeaponModStats>::iterator modEffectsIt = tempWeap.modEffects.begin(); modEffectsIt != tempWeap.modEffects.end(); ++modEffectsIt) {
				ATWeaponModStats addWeaponStats = *modEffectsIt;
				if (addWeaponStats.checkKW) {
					if (!ATGameData::WeaponInstanceHasKeyword(instanceData, addWeaponStats.checkKW)) {
						continue;
					}
				}
				_MESSAGE("Setting mod stats");
				newWeaponStats.fCritMult *= addWeaponStats.fCritMult;
				newWeaponStats.fCritFailMult *= addWeaponStats.fCritFailMult;
				newWeaponStats.fArmorPenetrationAdd += addWeaponStats.fArmorPenetrationAdd;
				newWeaponStats.fTargetArmorMult *= addWeaponStats.fTargetArmorMult;
				newWeaponStats.fWearMult *= addWeaponStats.fWearMult;
				newWeaponStats.fMaxConditionMultAdd += addWeaponStats.fMaxConditionMultAdd;
			}
		}

		// final stats calculations
		newWeaponStats.fWearMult = newWeaponStats.fWearMult / tempWeap.fBaseWearDivisor;
		newWeaponStats.fCritMult = max(0.0, newWeaponStats.fCritMult);
		newWeaponStats.fCritFailMult = max(0.0, newWeaponStats.fCritFailMult);
		newWeaponStats.fArmorPenetrationAdd = (min(100.0, newWeaponStats.fArmorPenetrationAdd));

		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_CritFailMult, newWeaponStats.fCritFailMult);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_ArmorPenetration, newWeaponStats.fArmorPenetrationAdd);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_TargetArmorMult, newWeaponStats.fTargetArmorMult * 100.0);


		// -- Weapon Condition --
		float fCNDPercent = max(0.0, ATGlobalVars::bConditionSystem ? ATGameData::GetInstanceHealth(extraData, true) : 1.0) * 100.0;
		float fMaxCND = max(1.0, tempWeap.fBaseMaxCondition + (tempWeap.fBaseMaxCondition * newWeaponStats.fMaxConditionMultAdd));
		float fNewCondition = fMaxCND * fCNDPercent;

		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDMax, fMaxCND);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDPct, fCNDPercent);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDWear, newWeaponStats.fWearMult);

		// -- Ammo Count --
		if (instanceData->ammo && instanceData->ammoCapacity > 0) {
			if (ownerActor->middleProcess && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
				UInt32 iAmmoCount = 0;
				if (ATGlobalVars::bConditionSystem && ((int)(floor(fNewCondition)) < 1)) {
					// disable gun if broken
					instanceData->ammo = ATConfigData::Ammo_Broken;
					ATGameData::SetInstanceHealth(extraData, 0.0);
				}
				else {
					// make sure count is positive and owner has enough ammo
					int iAmmoItemCount = ATGameData::GetItemCount(ownerActor, instanceData->ammo->formID);
					int iAmmoWpnCount = ATGameData::GetWeaponInstanceAVModifier(instanceData, ATConfigData::AV_WeaponAmmoCount);
					iAmmoCount = max(0, min(iAmmoWpnCount, min(iAmmoItemCount, instanceData->ammoCapacity)));
				}
				ownerActor->middleProcess->unk08->equipData->equippedData->unk18 = iAmmoCount;
			}
		}

		// -- Projectile Override --
		tempWeap.UpdateProjectile(instanceData);

		// -- Update the HUD overlay --
		if (bIsPlayer) {
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_SkillReqDiff, -fSkillReqDiff);
			ATHUD::ForceHUDWidgetsUpdate();
		}

		return false;
	}


	// sets the equipped weapon's droppable magazine info (on equip)
	MagazineData GetEquippedWeaponMagazineData(StaticFunctionTag*, Actor * ownerActor)
	{
		MagazineData magData;
		if (!ownerActor) {
			_MESSAGE("GetEquippedWeaponMagazineData: No ownerActor");
			magData.SetNone(true);
			return magData;
		}

		ATWeapon tempWeap;
		ExtraDataList * extraData = nullptr;
		if (!ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
			_MESSAGE("GetEquippedWeaponMagazineData: Can't find weapon");
			magData.SetNone(true);
			return magData;
		}
		extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			_MESSAGE("GetEquippedWeaponMagazineData: No extraData!");
			magData.SetNone(true);
			return magData;
		}

		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (!instanceData || !instanceData->ammo) {
			_MESSAGE("GetEquippedWeaponMagazineData: No instanceData or no ammo!");
			magData.SetNone(true);
			return magData;
		}

		TESAmmo * ammoItem = nullptr;
		TESObjectMISC * casingItem = nullptr;
		TESObjectMISC * magItem = nullptr;
		BGSMod::Attachment::Mod * magMod = nullptr;
		TESObjectMISC * magModMisc = nullptr;
		float fCasingChance = 0.0;
		UInt32 iCapacity = instanceData->ammoCapacity;
		UInt32 iReloadType = -1;

		// ---- ammo
		ATCaliber tempCaliber;
		if (tempWeap.GetCurrentCaliber(instanceData, tempCaliber)) {
			ATCaliber::AmmoType tempAmmoType;
			if (tempWeap.GetCurrentAmmoType(instanceData, extraData, tempAmmoType)) {
				ammoItem = (TESAmmo*)tempAmmoType.ammoItem;
				;
				casingItem = tempAmmoType.casingItem;
				fCasingChance = tempAmmoType.fCasingDropChance;
			}
			iReloadType = tempCaliber.iReloadType;
		}
		if (!ammoItem) {
			ammoItem = instanceData->ammo;
		}

		// -- magazine item
		ATWeapon::MagazineItem tempMag;
		if (tempWeap.GetCurMagItem(extraData, tempMag)) {
			magItem = tempMag.magItem;
			magMod = tempMag.magMod;
			auto pair = g_modAttachmentMap->Find(&magMod);
			if (pair) {
				magModMisc = pair->miscObject;
			}
		}

		magData.Set<UInt32>("iReloadType", iReloadType);
		magData.Set<TESObjectMISC*>("MagItem", magItem);
		magData.Set<BGSMod::Attachment::Mod*>("MagMod", magMod);
		magData.Set<TESObjectMISC*>("MagModMisc", magModMisc);
		magData.Set<TESAmmo*>("LoadedAmmo", ammoItem);
		magData.Set<UInt32>("iAmmoCapacity", iCapacity);
		magData.Set<UInt32>("iAmmoCount", -1);
		magData.Set<TESObjectMISC*>("AmmoCasing", casingItem);
		magData.Set<float>("fCasingChance", fCasingChance);
		magData.Set<TESForm*>("ScrapOutput", nullptr);

		return magData;
	}


	// sets unequipped weapon and holster armors (on equip)
	WeaponHolsteredData GetEquippedWeaponHolsterData(StaticFunctionTag*, Actor * ownerActor)
	{
		WeaponHolsteredData holsterData;
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = nullptr;
			if (extraData) {
				instanceData = ATGameData::GetWeaponInstanceData(extraData);
			}
			if (instanceData) {
				ATWeapon tempWeap;
				if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
					TESObjectARMO * holsteredArmor = nullptr;
					TESObjectARMO * holsterArmor = nullptr;

					// ---- holstered weapon
					ATWeapon::HolsteredArmor newHolster;
					if (tempWeap.GetCurWeapArmor(instanceData, newHolster)) {
						holsteredArmor = newHolster.armorWeapon;
						holsterArmor = newHolster.armorHolster;
					}

					// ---- set struct data
					holsterData.Set<TESObjectARMO*>("WeaponHolstered", holsteredArmor);
					holsterData.Set<TESObjectARMO*>("HolsterArmor", holsterArmor);
					return holsterData;
				}
			}
		}
		// ---- reset struct data if the weapon isn't supported/something went wrong
		holsterData.Set<TESObjectARMO*>("WeaponHolstered", nullptr);
		holsterData.Set<TESObjectARMO*>("HolsterArmor", nullptr);

		return holsterData;
	}


	// resets an actor's weapon variables (on unequip)
	bool ResetEquippedWeaponVars(StaticFunctionTag*, Actor * ownerActor)
	{
		if (ownerActor) {
			// reset
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDWear, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDPct, -1.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDMax, -1.0);

			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_ArmorPenetration, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_ArmorPenetrationThrown, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_TargetArmorMult, 100.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_TargetArmorMultThrown, 100.0);

			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_CritFailMult, 100.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_CritFailChance, 0.0);

			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponAmmoCount, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_SkillModActive, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_SkillReqDiff, 0.0);
			ownerActor->actorValueOwner.SetBase(ATConfigData::AV_RecoilMod, 0.0);

			if (ownerActor == (Actor*)*g_player) {
				ATHUD::ForceHUDWidgetsUpdate();
			}

			return true;
		}
		return false;
	}


	// updates weapon variables affected by CND and Luck (on ReloadComplete)
	bool UpdateEquippedWeaponVars(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			return false;
		}

		// ammo count
		float fAmmoCount = 0.0;
		if (ownerActor->middleProcess && ownerActor->middleProcess->unk08 && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
			fAmmoCount = (float)(int)(UInt32)ownerActor->middleProcess->unk08->equipData->equippedData->unk18;
		}
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponAmmoCount, fAmmoCount);

		// condition
		float fCNDPercent = 1.0;
		if (ATGlobalVars::bConditionSystem) {
			fCNDPercent = 0.0;
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			if (extraData) {
				fCNDPercent = ATGameData::GetInstanceHealth(extraData);
			}
		}

		// critical failure chance
		float fLuck = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_Luck);
		float fCritFailChance = 0.0;
		float fCritFailRollMod = 0.0;
		if (ATGlobalVars::bCritFailures) {
			float fCritFailMult = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_CritFailMult);
			if (ATGlobalVars::bConditionSystem) {
				if (fCNDPercent < ATGlobalVars::fMinCritFailureThreshold) {
					fCritFailChance = (((21.0 - (fLuck * fCNDPercent)) / 21.0) * 5.0) * (fCritFailMult * 0.01);
					fCritFailRollMod = 7.0 * ((fLuck * fCNDPercent) - 7.0);
				}
			}
			else {
				fCritFailChance = (((21.0 - fLuck) / 21.0) * 5.0) * (fCritFailMult * 0.01);
				fCritFailRollMod = 7.0 * (fLuck - 7.0);
			}
		}
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_CritFailChance, fCritFailChance);
		ownerActor->actorValueOwner.SetBase(ATConfigData::AV_CritFailRollMod, fCritFailRollMod);

		// update HUD
		if (ownerActor == (Actor*)*g_player) {
			ATHUD::ForceHUDWidgetsUpdate();
		}
		return true;
	}

	
	// performs a critical failure roll and returns the resulting crit failure spell, if player: damages CND, saves CND + ammo count
	SpellItem * WeaponFired(StaticFunctionTag*, Actor * ownerActor)
	{
		//_MESSAGE("WeaponFired");
		if (!ownerActor) {
			return nullptr;
		}

		// -- player only stuff
		if (ownerActor == (Actor*)*g_player) {
			//_MESSAGE("Player WeaponFired event");

			if (ATGlobalVars::bConditionSystem) {
				ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
				if (extraData) {
					float fCNDPercent = 1.0;
					fCNDPercent = ATGameData::GetInstanceHealth(extraData);
					if (fCNDPercent > 0.0) {
						float fWear = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_WeaponCNDWear) * 0.01;
						float fMaxCND = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_WeaponCNDMax);
						if ((fMaxCND > 0.0) || (fWear > 0.0)) {
							float fCurCND = (fMaxCND * fCNDPercent) - fWear;
							fCNDPercent = max(0.0, (fCurCND / fMaxCND));
							ATGameData::SetInstanceHealth(extraData, fCNDPercent);
							if (fCNDPercent <= 0.0) {
								TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
								if (instanceData && instanceData->ammo) {
									instanceData->ammo = ATConfigData::Ammo_Broken;
								}
							}
							ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponCNDPct, fCNDPercent * 100.0);
						}
					}
				}
			}
			if (ownerActor->middleProcess && ownerActor->middleProcess->unk08 && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
				ownerActor->actorValueOwner.SetBase(ATConfigData::AV_WeaponAmmoCount, (float)(int)(UInt32)ownerActor->middleProcess->unk08->equipData->equippedData->unk18);
			}
		}

		// -- player + npcs
		if (ATGlobalVars::bCritFailures) {
			float fCritFailChance = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_CritFailChance);
			if (fCritFailChance > 0.0) {
				ATWeapon tempWeap;
				if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
					int iRollMod = ownerActor->actorValueOwner.GetValue(ATConfigData::AV_CritFailRollMod);
					return tempWeap.GetCritFailureSpell(fCritFailChance, iRollMod);
				}
			}
		}
		return nullptr;
	}


	// -------- Swappable Mods
	
	WeaponModData GetNextWeaponModData(StaticFunctionTag*, Actor * ownerActor, UInt32 iModSlot)
	{
		WeaponModData curModData;
		if (ownerActor) {
			ATWeapon tempWeap;
			if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
				if ((int)iModSlot == -2) {
					// firemode slot

				}
				else if ((int)iModSlot == -1) {
					// caliber slot
					ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
					TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
					if (extraData && instanceData) {
						ATCaliber tempCaliber;
						if (tempWeap.GetCurrentCaliber(instanceData, tempCaliber)) {
							ATCaliber::AmmoType curAmmoType;
							if (tempWeap.GetNextAmmoType(ownerActor, curAmmoType)) {
								curModData.Set<BGSMod::Attachment::Mod*>("swapMod", curAmmoType.ammoMod);
								curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
								curModData.Set<TESForm*>("swapModReqItem", curAmmoType.ammoItem);
								ATWeapon::WeaponAnim tempAnims;
								if (tempWeap.GetWeaponAnim(tempCaliber.iReloadAnimIndex, tempAnims)) {
									curModData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
									curModData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
									curModData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
									curModData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
								}
								return curModData;
							}
						}
					}
				}
				else if (iModSlot < tempWeap.modSlots.size()) {
					ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
					ATModSlot::SwappableMod swapMod;
					if (tempWeap.GetNextSwapMod(ownerActor, iModSlot, swapMod)) {
						auto pair = g_modAttachmentMap->Find(&swapMod.swapMod);
						curModData.Set<BGSMod::Attachment::Mod*>("swapMod", swapMod.swapMod);
						if (pair) {
							curModData.Set<TESObjectMISC*>("swapModMisc", pair->miscObject);
						}
						else {
							curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
						}
						curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
						curModData.Set<TESForm*>("swapModReqItem", swapMod.requiredItem);
						ATWeapon::WeaponAnim tempAnims;
						if (tempWeap.GetWeaponAnim(tempSlot.iSwapAnimIndex, tempAnims)) {
							curModData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
							curModData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
							curModData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
							curModData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
						}
						return curModData;
					}
					
				}
			}
		}
		curModData.Set<BGSMod::Attachment::Mod*>("swapMod", nullptr);
		curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
		curModData.Set<TESForm*>("swapModReqItem", nullptr);
		curModData.Set<float>("fSwapModWait", 0.0);
		curModData.Set<BGSAction*>("actionSwapMod", nullptr);
		curModData.Set<TESForm*>("idleSwapMod_1P", nullptr);
		curModData.Set<TESForm*>("idleSwapMod_3P", nullptr);
		return curModData;
	}


	bool GetWeaponModDataAtIndex(StaticFunctionTag*, WeaponModData modData, Actor * ownerActor, UInt32 iModSlot = -1, UInt32 iModIndex = -1)
	{
		if (!ownerActor || ((int)iModIndex > 0) || modData.IsNone()) {
			return false;
		}

		ATWeapon tempWeap;
		if (!ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
			return false;
		}

		if (iModSlot == -2) {
			// firemode

		}
		else if (iModSlot == -1) {
			// caliber
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			if (extraData && instanceData) {
				ATCaliber tempCaliber;
				if (tempWeap.GetCurrentCaliber(instanceData, tempCaliber)) {
					if (iModIndex < tempCaliber.ammoTypes.size()) {
						ATCaliber::AmmoType curAmmoType = tempCaliber.ammoTypes[iModIndex];
						if (curAmmoType.ammoMod) {
							modData.Set<BGSMod::Attachment::Mod*>("swapMod", curAmmoType.ammoMod);
							modData.Set<TESObjectMISC*>("swapModMisc", nullptr);
							modData.Set<TESForm*>("swapModReqItem", curAmmoType.ammoItem);
							ATWeapon::WeaponAnim tempAnims;
							if (tempWeap.GetWeaponAnim(tempCaliber.iReloadAnimIndex, tempAnims)) {
								modData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
								modData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
								modData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
								modData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
							}
							return true;
						}
					}
				}
			}
		}
		else if (iModSlot < tempWeap.modSlots.size()) {
			ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
			ATModSlot::SwappableMod curSwapMod = tempSlot.swappableMods[iModIndex];
			if (curSwapMod.swapMod) {
				auto pair = g_modAttachmentMap->Find(&curSwapMod.swapMod);
				modData.Set<BGSMod::Attachment::Mod*>("swapMod", curSwapMod.swapMod);
				if (pair) {
					modData.Set<TESObjectMISC*>("swapModMisc", pair->miscObject);
				}
				else {
					modData.Set<TESObjectMISC*>("swapModMisc", nullptr);
				}
				modData.Set<TESForm*>("swapModReqItem", curSwapMod.requiredItem);
				ATWeapon::WeaponAnim tempAnims;
				if (tempWeap.GetWeaponAnim(tempSlot.iSwapAnimIndex, tempAnims)) {
					modData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
					modData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
					modData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
					modData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
				}
				return true;
			}
			
			
		}
		return false;
	}


	WeaponModData GetCurrentWeaponModData(StaticFunctionTag*, Actor * ownerActor, UInt32 iModSlot = -1)
	{
		WeaponModData curModData;
		if (ownerActor) {
			ATWeapon tempWeap;
			if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
				if (iModSlot == -2) {
					// firemode

				}
				else if (iModSlot == -1) {
					// caliber
					ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
					TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
					if (extraData && instanceData) {
						ATCaliber tempCaliber;
						if (tempWeap.GetCurrentCaliber(instanceData, tempCaliber)) {
							ATCaliber::AmmoType curAmmoType;
							if (tempWeap.GetCurrentAmmoType(instanceData, extraData, curAmmoType)) {
								if (curAmmoType.ammoMod) {
									curModData.Set<BGSMod::Attachment::Mod*>("swapMod", curAmmoType.ammoMod);
									curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
									curModData.Set<TESForm*>("swapModReqItem", curAmmoType.ammoItem);
									ATWeapon::WeaponAnim tempAnims;
									if (tempWeap.GetWeaponAnim(tempCaliber.iReloadAnimIndex, tempAnims)) {
										curModData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
										curModData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
										curModData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
										curModData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
									}
									
									return curModData;
								}
							}
						}
					}
				}
				else if (iModSlot < tempWeap.modSlots.size()) {
					// mod slot
					ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
					if (extraData) {
						ATModSlot tempSlot = tempWeap.modSlots[iModSlot];
						ATModSlot::SwappableMod curSwapMod;
						if (tempWeap.GetCurrentSwapMod(extraData, iModSlot, curSwapMod)) {
							if (curSwapMod.swapMod) {
								auto pair = g_modAttachmentMap->Find(&curSwapMod.swapMod);
								curModData.Set<BGSMod::Attachment::Mod*>("swapMod", curSwapMod.swapMod);
								if (pair) {
									curModData.Set<TESObjectMISC*>("swapModMisc", pair->miscObject);
								}
								else {
									curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
								}
								curModData.Set<TESForm*>("swapModReqItem", curSwapMod.requiredItem);
								ATWeapon::WeaponAnim tempAnims;
								if (tempWeap.GetWeaponAnim(tempSlot.iSwapAnimIndex, tempAnims)) {
									curModData.Set<BGSAction*>("actionSwapMod", tempAnims.animAction);
									curModData.Set<TESForm*>("idleSwapMod_1P", tempAnims.animIdle_1P);
									curModData.Set<TESForm*>("idleSwapMod_3P", tempAnims.animIdle_3P);
									curModData.Set<BSFixedString>("sAnimFinishedEvent", BSFixedString(tempAnims.sAnimFinishedEvent.c_str()));
								}
								return curModData;
							}
						}
						
					}
				}
			}
		}
		curModData.Set<BGSMod::Attachment::Mod*>("swapMod", nullptr);
		curModData.Set<TESObjectMISC*>("swapModMisc", nullptr);
		curModData.Set<TESForm*>("swapModReqItem", nullptr);
		curModData.Set<float>("fSwapModWait", 0.0);
		curModData.Set<BGSAction*>("actionSwapMod", nullptr);
		curModData.Set<TESForm*>("idleSwapMod_1P", nullptr);
		curModData.Set<TESForm*>("idleSwapMod_3P", nullptr);
		return curModData;
	}

	
	// ---- equipped magazine count

	// adds iAmount to the equipped weapon's ammo count, returns the difference
	// - standard ammo is added to/removed from ownerActor's inventory, f-core/health-based ammo disappears
	UInt32 LoadEquippedAmmo(StaticFunctionTag*, Actor * ownerActor, UInt32 iAmount = 0)
	{
		if (ownerActor && (iAmount != 0)) {
			if (ownerActor->middleProcess && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
				ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
				TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
				if (extraData && instanceData) {
					int iAmount_s = (int)iAmount;
					int iMaxAmount = instanceData->ammoCapacity;
					// edge case check in case a melee weapon gets through
					if (iMaxAmount > 0) {
						int iCurAmount = (int)ownerActor->middleProcess->unk08->equipData->equippedData->unk18;
						int iNewAmount = (int)min(max(0, iCurAmount + iAmount_s), iMaxAmount);
						ownerActor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)iNewAmount;
						return (UInt32)(iNewAmount - iCurAmount);
					}
				}
			}
		}
		return 0;
	}

	UInt32 GetEquippedAmmoCount(StaticFunctionTag*, Actor * ownerActor)
	{
		if (ownerActor) {
			if (ownerActor->middleProcess && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
				return (UInt32)ownerActor->middleProcess->unk08->equipData->equippedData->unk18;
			}
		}
		return 0;
	}

	bool SetEquippedAmmoCount(StaticFunctionTag*, Actor * ownerActor, UInt32 iAmount = 0)
	{
		if ((int)iAmount > -1) {
			if (ownerActor) {
				if (ownerActor->middleProcess && ownerActor->middleProcess->unk08->equipData && ownerActor->middleProcess->unk08->equipData->equippedData) {
					ownerActor->middleProcess->unk08->equipData->equippedData->unk18 = (UInt64)iAmount;
					return true;
				}
			}
		}
		return false;
	}


	// ---- Equipped ammo

	// returns the equipped ammo form - used for ammo swapping checks and jamming
	TESAmmo * GetInstanceAmmo(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			return nullptr;
		}
		ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			return nullptr;
		}
		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (instanceData) {
			return instanceData->ammo;
		}
		return nullptr;
	}

	// temporarily equips the specified ammo form - used for jamming
	bool SetEquippedAmmo(StaticFunctionTag*, TESAmmo *newAmmo, Actor *ownerActor)
	{
		if (!ownerActor) {
			return false;
		}
		ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			return false;
		}
		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (instanceData) {
			if (instanceData->ammo != newAmmo) {
				instanceData->ammo = newAmmo;
				return true;
			}
		}
		return false;
	}

	// returns the equipped ammo form - used for ammo swapping checks and jamming
	TESAmmo * GetEquippedAmmo(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			return nullptr;
		}
		ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			return nullptr;
		}
		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (!instanceData) {
			return nullptr;
		}
		ATWeapon curWeapon;
		if (!ATGameData::GetEquippedWeapon(ownerActor, curWeapon)) {
			return nullptr;
		}
		ATCaliber::AmmoType curAmmoType;
		if (curWeapon.GetCurrentAmmoType(instanceData, extraData, curAmmoType)) {
			if (curAmmoType.ammoItem->formType == kFormType_AMMO) {
				return (TESAmmo*)curAmmoType.ammoItem;
			}
		}
		return nullptr;
	}

	
	// ---- Projectiles

	BGSProjectile * GetEquippedProjectile(StaticFunctionTag*, Actor * ownerActor)
	{
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			if (extraData && instanceData) {
				if (instanceData->firingData) {
					return instanceData->firingData->projectileOverride;
				}
			}
		}
		return nullptr;
	}

	bool SetEquippedProjectile(StaticFunctionTag*, BGSProjectile * newProjectile, Actor * ownerActor)
	{
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			if (extraData && instanceData) {
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

	bool ResetEquippedProjectile(StaticFunctionTag*, Actor * ownerActor)
	{
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			if (extraData && instanceData) {
				ATWeapon tempWeap;
				if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
					return tempWeap.UpdateProjectile(instanceData);
				}
			}
		}
		return false;
	}


	// ********************** Critical Effects/Failures
	
	// picks a random damaged for the Damage Weapon Part crit failure
	BGSMod::Attachment::Mod *GetRandomDamagedMod(StaticFunctionTag*, Actor * ownerActor, UInt32 iModSlot)
	{
		if (ownerActor) {
			ATWeapon tempWeap;
			if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
				ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
				if (extraData) {
					return tempWeap.GetDamagedMod(extraData, (int)iModSlot);
				}
			}
		}
		return nullptr;
	}


	// ---- CND/Repair Stuff:

	BGSPerk * GetRepairPerk(StaticFunctionTag*, Actor * ownerActor)
	{
		if (ownerActor) {
			ATWeapon tempWeap;
			if (ATGameData::GetEquippedWeapon(ownerActor, tempWeap)) {
				return tempWeap.perkRepairSkill;
			}
		}
		return nullptr;
	}


	bool SetEquippedWeaponSavedCND(StaticFunctionTag*, float fAmount, Actor * ownerActor)
	{
		if (!ownerActor) {
			_MESSAGE("ERROR: SetWeaponRefSavedCND - no ownerActor");
			return false;
		}
		ExtraDataList *tempData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (tempData) {
			return ATGameData::SetInstanceHealth(tempData, max(0.0, fAmount));
		}
		return false;
	}

	float GetEquippedWeaponSavedCND(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			_MESSAGE("ERROR: GetWeaponRefSavedCND - no ownerActor");
			return -1.0;
		}
		ExtraDataList *tempData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (tempData) {
			return ATGameData::GetInstanceHealth(tempData);
		}
		return -1.0;
	}

	bool AddEquippedWeaponSavedCND(StaticFunctionTag*, float fAmount, Actor * ownerActor)
	{
		if (!ownerActor) {
			_MESSAGE("ERROR: SetWeaponRefSavedCND - no ownerActor");
			return false;
		}
		ExtraDataList *tempData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (tempData) {
			float fOldCND = ATGameData::GetInstanceHealth(tempData);
			return ATGameData::SetInstanceHealth(tempData, min(max(0.0, fAmount + fOldCND), 1.0));
		}
		return false;
	}


	bool CheckUnjam(StaticFunctionTag*, Actor * ownerActor)
	{
		if (!ownerActor) {
			return false;
		}
		ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
		if (!extraData) {
			return false;
		}
		TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
		if (!instanceData || !instanceData->ammo) {
			return false;
		}
		if (ATGameData::GetInstanceHealth(extraData) > 0.0) {
			bool bDoUnjam = false;
			if ((instanceData->ammo == ATConfigData::Ammo_Jammed) || (instanceData->ammo == ATConfigData::Ammo_Overheated)) {
				bDoUnjam = true;
			}
			else if (instanceData->ammo == ATConfigData::Ammo_Broken) {
				if (!ATGameData::InstanceHasMod(extraData, ATConfigData::Mod_NullMagazine)) {
					bDoUnjam = true;
				}
			}

			if (bDoUnjam) {
				ATWeapon curWeapon;
				if (ATGameData::GetEquippedWeapon(ownerActor, curWeapon)) {
					ATCaliber::AmmoType curAmmoType;
					if (curWeapon.GetCurrentAmmoType(instanceData, extraData, curAmmoType)) {
						if (curAmmoType.ammoItem && curAmmoType.ammoItem->formType == kFormType_AMMO) {
							instanceData->ammo = (TESAmmo*)curAmmoType.ammoItem;
						}
					}
				}
				return true;
			}
		}
		return false;
	}


	// Syncs AmmoTweaks settings to values set in MCM
	void UpdateGlobalSettings(StaticFunctionTag*)
	{
		ATGlobalVars::LoadSettings();
	}

	// adds a message to the hud message log window
	bool SendHUDOverlayMessage(StaticFunctionTag*, BSFixedString newMessage)
	{
		ATHUD::AddHUDMessage(newMessage);
		return false;
	}


	// -------- Testing/Dev:

	// ---- alt firemode method testing
	bool SetWeaponAutomatic(StaticFunctionTag*, bool bAutomatic, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			if (extraData && instanceData) {
				_MESSAGE("Speed: %f", instanceData->speed);
				bool bHasAutoKW = false;
				tArray<BGSKeyword*> keywordsList;
				int iKWCount = 0;

				// flag
				if (bAutomatic) {
					instanceData->flags |= 0x0008000;
					instanceData->speed = instanceData->speed * 0.75;
					//instanceData->flags |= ATWeapon::wFlag_RepeatableSingleFire;
				}
				else {
					instanceData->flags &= ~0x0008000;
					instanceData->speed = instanceData->speed / 0.75;
					//instanceData->flags &= ~ATWeapon::wFlag_RepeatableSingleFire;
				}

				_MESSAGE("Speed after: %f", instanceData->speed);
			}
		}
		return false;
	}

	// ---- dumps weapon variables
	bool LogEquippedWeaponUnknowns(StaticFunctionTag*, Actor * ownerActor, UInt32 iEquipSlot = 41)
	{
		if (ownerActor) {
			ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(ownerActor, 0);
			TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
			TESObjectWEAP * curWeapon = ATGameData::GetEquippedWeaponForm(ownerActor, 0);
			
			if (extraData && instanceData && curWeapon) {
				// random variables
				_MESSAGE(
					"Weapon ints:\n   unk18: %i\n   unk20: %i\n   unk50: %i\n  unk100: %i\n  unk114: %i\n  unk118: %i\n  unk11C: %i\n  unk134: %i",
					instanceData->unk18, instanceData->unk20, instanceData->unk50, instanceData->unk100, instanceData->unk114,
					instanceData->unk118, instanceData->unk11C, instanceData->unk134
				);

				_MESSAGE(
					"Weapon floats:\n   speed: %f\n   attack delay: %f\n   unkC0: %f\n   unkD8: %f\n   unkEC: %f\n   firingData.unk00: %f\n   firingData.unk18: %f\n   firingData.unk1C: %f\n",
					instanceData->speed, instanceData->attackDelay,
					instanceData->unkC0, instanceData->unkD8, instanceData->unkEC,
					instanceData->firingData->unk00, instanceData->firingData->unk18, instanceData->firingData->unk1C
				);

				// ---- AimModel
				if (instanceData->aimModel) {
					ATAimModel *tempAM = (ATAimModel*)instanceData->aimModel;
					if (tempAM) {
						_MESSAGE(
							"AimModel:\n  Cone of Fire:\n    Min Angle: %f, Max Angle: %f\n    Increase/Shot: %f, Decrease/Sec: %f, Decrease Delay: %i\n",
							tempAM->CoF_MinAngle, tempAM->CoF_MaxAngle, tempAM->CoF_IncrPerShot, tempAM->CoF_DecrPerSec, tempAM->CoF_DecrDelayMS
						);
						_MESSAGE(
							"  Recoil:\n    Min/Shot: %f, Max/Shot: %f, Hip Multiplier: %f, Diminish Spring Force: %f",
							tempAM->Rec_MinPerShot, tempAM->Rec_MaxPerShot, tempAM->Rec_HipMult, tempAM->Rec_DimSpringForce, tempAM->CoF_DecrDelayMS
						);
					}
				}

				TESObjectWEAP::InstanceData * testInstance = (TESObjectWEAP::InstanceData*)ownerActor->middleProcess->unk08->equipData->instanceData;
				if (testInstance) {
					ATAimModel *tempAM2 = (ATAimModel*)testInstance->aimModel;
					tempAM2->CoF_MaxAngle *= 2.0;
					tempAM2->CoF_MinAngle *= 2.0;
					_MESSAGE("\nEquipData has instanceData");
				}
				else {
					_MESSAGE("\nEquipData does not have instanceData");
				}

				// ---- Equipped mods
				BGSObjectInstanceExtra::Data * data = ownerActor->equipData->slots[iEquipSlot].extraData->data;
				if (data) {
					UInt32 dataSize = data->blockSize / sizeof(BGSObjectInstanceExtra::Data::Form);
					_MESSAGE("\nEquipped Mods:");
					for (UInt32 j = 0; j < dataSize; j++) {
						_MESSAGE("    0x%08X", data->forms[j].formId);
					}
				}


			}
		}
		return false;
	}


}


bool ATGlobals::RegisterPapyrus(VirtualMachine* vm)
{
	ATGlobals::RegisterFuncs(vm);
	return true;
}


void ATGlobals::RegisterFuncs(VirtualMachine* vm)
{
	// ---- Equipped Weapon Init

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("InitEquippedWeapon", SCRIPTNAME_Globals, InitEquippedWeapon, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("ResetEquippedWeaponVars", SCRIPTNAME_Globals, ResetEquippedWeaponVars, vm));

	
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, MagazineData, Actor*>("GetEquippedWeaponMagazineData", SCRIPTNAME_Globals, GetEquippedWeaponMagazineData, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, WeaponHolsteredData, Actor*>("GetEquippedWeaponHolsterData", SCRIPTNAME_Globals, GetEquippedWeaponHolsterData, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("UpdateEquippedWeaponVars", SCRIPTNAME_Globals, UpdateEquippedWeaponVars, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, SpellItem*, Actor*>("WeaponFired", SCRIPTNAME_Globals, WeaponFired, vm));
	

	// ---- Swappable Mods
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, WeaponModData, Actor*, UInt32>("GetNextWeaponModData", SCRIPTNAME_Globals, GetNextWeaponModData, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, bool, WeaponModData, Actor*, UInt32, UInt32>("GetWeaponModDataAtIndex", SCRIPTNAME_Globals, GetWeaponModDataAtIndex, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, WeaponModData, Actor*, UInt32>("GetCurrentWeaponModData", SCRIPTNAME_Globals, GetCurrentWeaponModData, vm));

	
	// ---- Ammo
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESAmmo*, Actor*>("GetInstanceAmmo", SCRIPTNAME_Globals, GetInstanceAmmo, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESAmmo*, Actor*>("GetEquippedAmmo", SCRIPTNAME_Globals, GetEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, TESAmmo*, Actor*>("SetEquippedAmmo", SCRIPTNAME_Globals, SetEquippedAmmo, vm));
	

	// ---- Projectiles
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, BGSProjectile*, Actor*>("SetEquippedProjectile", SCRIPTNAME_Globals, SetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, BGSProjectile*, Actor*>("GetEquippedProjectile", SCRIPTNAME_Globals, GetEquippedProjectile, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("ResetEquippedProjectile", SCRIPTNAME_Globals, ResetEquippedProjectile, vm));


	// ---- Crit Effects/Failures
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, BGSMod::Attachment::Mod*, Actor*, UInt32>("GetRandomDamagedMod", SCRIPTNAME_Globals, GetRandomDamagedMod, vm));


	// ---- Magazine Ammo Count
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Actor*, UInt32>("LoadEquippedAmmo", SCRIPTNAME_Globals, LoadEquippedAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Actor*>("GetEquippedAmmoCount", SCRIPTNAME_Globals, GetEquippedAmmoCount, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("SetEquippedAmmoCount", SCRIPTNAME_Globals, SetEquippedAmmoCount, vm));


	// ---- Global Settings
	vm->RegisterFunction(
		new NativeFunction0 <StaticFunctionTag, void>("UpdateGlobalSettings", SCRIPTNAME_Globals, UpdateGlobalSettings, vm));

	// ---- CND
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, float, Actor*>("SetEquippedWeaponSavedCND", SCRIPTNAME_Globals, SetEquippedWeaponSavedCND, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Actor*>("GetEquippedWeaponSavedCND", SCRIPTNAME_Globals, GetEquippedWeaponSavedCND, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, float, Actor*>("AddEquippedWeaponSavedCND", SCRIPTNAME_Globals, AddEquippedWeaponSavedCND, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, Actor*>("CheckUnjam", SCRIPTNAME_Globals, CheckUnjam, vm));

	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, bool, BSFixedString>("SendHUDOverlayMessage", SCRIPTNAME_Globals, SendHUDOverlayMessage, vm));


	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, bool, bool, Actor*, UInt32>("SetWeaponAutomatic", SCRIPTNAME_Globals, SetWeaponAutomatic, vm));

	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Actor*, UInt32>("LogEquippedWeaponUnknowns", SCRIPTNAME_Globals, LogEquippedWeaponUnknowns, vm));


	_MESSAGE("Registered native functions for %s", SCRIPTNAME_Globals);
}


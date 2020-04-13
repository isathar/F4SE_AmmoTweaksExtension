#include "CustomMenusAT.h"
#include "include/AimModelEdit.h"


const char * ATHUDMenu::sMenuName = "AmmoTweaks/ATHUDMenu";
bool ATHUDMenu::bForceUpdateBaseVars = false;
bool ATHUDMenu::bForceWeaponStatsUpdate = false;
bool ATHUDMenu::bHideHUDWidgets = false;
bool ATHUDMenu::bPipboyOpen = false;


std::vector<ATMenuInvItem> ATMenuInventory::invData_Weapons = std::vector<ATMenuInvItem>();
std::vector<ATMenuInvItem> ATMenuInventory::invData_Armors = std::vector<ATMenuInvItem>();
std::vector<ATMenuInvItem> ATMenuInventory::invData_Ammo = std::vector<ATMenuInvItem>();

int ATMenuInventory::iEquippedWeapon = -1;
int ATMenuInventory::iEquippedGrenade = -1;
std::vector<int>  ATMenuInventory::iEquippedArmors = std::vector<int>();

int ATMenuInventory::iDistanceUnit = 0;
int ATMenuInventory::iLastItemCount = 0;

int ATMenuInventory::iPipboyTab = -1;
int ATMenuInventory::iSelectedItem = -1;

int ATMenuInventory::iPipboyPage = -1;
int ATMenuInventory::iSortMode = 0;


const UInt32 DTID_Physical = 0x60A87;
const UInt32 DTID_Energy = 0x60A81;
const UInt32 DTID_Fire = 0x60A82;
const UInt32 DTID_Cryo = 0x60A83;
const UInt32 DTID_Shock = 0x60A80;
const UInt32 DTID_Poison = 0x60A84;
const UInt32 DTID_Rad = 0x60A85;


// -------------------------------- HUD Overlay

// constructor
ATHUDMenu::ATHUDMenu() : GameMenuBase()
{
	flags = kFlag_DoNotPreventGameSave;
	context = 0x0022;
	if (CALL_MEMBER_FN((*g_scaleformManager), LoadMovie)(this, movie, "AmmoTweaks/ATHUDMenu", "root1", 2)) {
		_MESSAGE("ATHUDMenu: menu loaded");
		stage.SetMember("showShadowEffect", &GFxValue(true));
		CreateBaseShaderTarget(shaderTarget, stage);
	}
}

// delete - clean up weapon menu cache
ATHUDMenu::~ATHUDMenu(){}

// kept clear like the HUDExtension's since I'm not sure what to do with this
void ATHUDMenu::RegisterFunctions(){}


// updates the HUD overlay's base color
void ATHUDMenu::UpdateBaseColor()
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	GFxValue args[4];
	args[0].SetNumber(ATGlobalVars::fHUDBaseColorR);
	args[1].SetNumber(ATGlobalVars::fHUDBaseColorB);
	args[2].SetNumber(ATGlobalVars::fHUDBaseColorG);
	args[3].SetBool(ATGlobalVars::bHUDEnableColorChanges);
	stage.Invoke("UpdateBaseMenuVars", nullptr, args, 4);
}

// sets the selected widget's transforms
void ATHUDMenu::UpdateWidgetTS(UInt32 iIndex, float fNewX, float fNewY, float fNewScale)
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	GFxValue args[4];
	args[0].SetInt((int)iIndex);
	args[1].SetNumber(fNewX);
	args[2].SetNumber(fNewY);
	args[3].SetNumber(fNewScale);
	stage.Invoke("InitWidget", nullptr, args, 4);
}

// resets base color, all widgets' transforms, CND->Dirt display
void ATHUDMenu::InitializeWidgets()
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	UpdateWidgetTS(0, ATGlobalVars::fHUDWeapStatsTX, ATGlobalVars::fHUDWeapStatsTY, ATGlobalVars::fHUDWeapStatsTS);
	UpdateWidgetTS(1, ATGlobalVars::fHUDWeapMenuTX, ATGlobalVars::fHUDWeapMenuTY, ATGlobalVars::fHUDWeapMenuTS);
	UpdateWidgetTS(2, ATGlobalVars::fHUDStatusFXTX, ATGlobalVars::fHUDStatusFXTY, ATGlobalVars::fHUDStatusFXTS);
	UpdateWidgetTS(3, ATGlobalVars::fHUDMsgBoxTX, ATGlobalVars::fHUDMsgBoxTY, ATGlobalVars::fHUDMsgBoxTS);
	UpdateBaseColor();
}

void ATHUDMenu::SetWidgetVisible(UInt32 iIndex, bool bWidgetVisible)
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	GFxValue args[2];
	args[0].SetInt(iIndex);
	args[1].SetBool(bWidgetVisible);
	stage.Invoke("ToggleVisibility", nullptr, args, 2);
}

void ATHUDMenu::SetHUDWidgetsVisible(bool bWidgetVisible)
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	GFxValue args[1];
	args[0].SetBool(bWidgetVisible);
	stage.Invoke("SetHUDWidgetsVisibe", nullptr, args, 1);
}

// adds a message to the message list
void ATHUDMenu::AddHUDMessage(BSFixedString newMessage)
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	if (newMessage.c_str() != "") {
		GFxValue args[1];
		args[0].SetString(newMessage.c_str());
		stage.Invoke("AddCustomHUDMessage", nullptr, args, 1);
	}
}

// updates the weapon widget's CND bar 
void ATHUDMenu::UpdateWidgetCNDPercent()
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	Actor * playerActor = *g_player;
	if (!playerActor) {
		return;
	}

	float fCNDCur = -1.0;
	ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(playerActor, 0);
	if (extraData) {
		fCNDCur = ATGameData::GetInstanceHealth(extraData);
	}
	
	int iPlayerWeaponCND = (int)floor(fCNDCur * 100.0);
	if (iPlayerWeaponCND != (int)iCurCondition) {
		iCurCondition = iPlayerWeaponCND;
		GFxValue args[1];
		args[0].SetNumber(fCNDCur);
		stage.Invoke("UpdateWeaponCondition", nullptr, args, 1);
	}
}

// updates all of the weapon widget's variables
void ATHUDMenu::UpdateWeaponStatsWidget()
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	PlayerCharacter * playerActor = *g_player;
	if (!playerActor) {
		return;
	}

	int iWeapIcon = -1;
	int iFMIcon = -1;
	const char * ammoName = "";
	int iSkillReq = 0;
	int iWpnDmg = 0;

	ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(playerActor, 0);
	TESObjectWEAP::InstanceData * instanceData = ATGameData::GetWeaponInstanceData(extraData);
	if (extraData && instanceData) {
		ATWeapon weaponData;
		if (ATGameData::GetEquippedWeapon(playerActor, weaponData)) {
			iWeapIcon = weaponData.GetHUDIcon(instanceData);
			iFMIcon = weaponData.GetFiremodeIcon(extraData);
			ammoName = "PLACEHOLDER";
			iSkillReq = (int)playerActor->actorValueOwner.GetValue(ATConfigData::AV_SkillReqDiff);
			iWpnDmg = (int)playerActor->actorValueOwner.GetValue(ATConfigData::AV_WeaponDamageLevel);
			ATCaliber::AmmoType tempAmmoType;
			if (weaponData.GetCurrentAmmoType(instanceData, extraData, tempAmmoType)) {
				ammoName = tempAmmoType.sAmmoName.c_str();
			}
		}
	}
	
	GFxValue args[5];
	args[0].SetInt(iWeapIcon);
	args[1].SetInt(iFMIcon);
	args[2].SetString(ammoName);
	args[3].SetInt(iSkillReq);
	args[4].SetInt(iWpnDmg);
	stage.Invoke("UpdateWeaponStats", nullptr, args, 5);
}

// updates all status effect indicators
void ATHUDMenu::UpdateStatusEffects()
{
	if (!stage.IsDisplayObject()) {
		return;
	}
	PlayerCharacter * playerActor = *g_player;
	if (!playerActor) {
		return;
	}

	float fArmorDmg = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusArmorDmg);
	float fBleeding = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusBleeding);
	float fBlinded = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusBlinded);
	float fBurning = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusBurning);
	float fChilled = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusChilled);
	float fIonized = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusIonized);
	float fPoisoned = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusPoisoned);
	float fRadiated = playerActor->actorValueOwner.GetValue(ATConfigData::AV_StatusRadiated);

	GFxValue args[8];
	args[0].SetNumber(fPoisoned);
	args[1].SetNumber(fBleeding);
	args[2].SetNumber(fRadiated);
	args[3].SetNumber(fBurning);
	args[4].SetNumber(fChilled);
	args[5].SetNumber(fIonized);
	args[6].SetNumber(fArmorDmg);
	args[7].SetNumber(fBlinded);

	stage.Invoke("UpdateStatusEffects", nullptr, args, 8);
	
}

// clears the weapon quick menu
void ATHUDMenu::ResetWeaponMenu()
{
	if (!stage.IsDisplayObject()) {
		return;
	}

	stage.Invoke("ResetWeaponMenu", nullptr, nullptr, 0);
	// tbd: clear weapon menu data
}


// things to execute every frame
void ATHUDMenu::DrawNextFrame(float unk0, void * unk1)
{
	if (!stage.IsDisplayObject()) {
		return;
	}

	if (bPipboyOpen) {
		if (bHideHUDWidgets && !bHUDWidgetsHidden) {
			bHideHUDWidgets = true;
			bForceUpdateBaseVars = false;
			bHUDWidgetsHidden = true;
			SetHUDWidgetsVisible(false);
		}
	}
	else {
		// check if colors/widget transforms need to be reset - do nothing else this frame
		if (bForceUpdateBaseVars) {
			bForceUpdateBaseVars = false;
			InitializeWidgets();
		}
		else {
			if (!bHideHUDWidgets) {
				// check if widgets need to be made visible
				if (bHUDWidgetsHidden) {
					bHUDWidgetsHidden = false;
					SetHUDWidgetsVisible(true);
				}
				else {
					if (*g_player) {
						if (!(*g_player)->actorState.IsWeaponDrawn()) {
							bHideHUDWidgets = true;
						}
					}
				}

				// count frames so values aren't checked every 0.03s or so
				iFrameCounter_CND += 1;
				iFrameCounter_StatusFX += 1;

				// set weapon stats every time a weapon is equipped/unequipped/reloaded
				if (bForceWeaponStatsUpdate) {
					bForceWeaponStatsUpdate = false;
					UpdateWeaponStatsWidget();
				}
				else {
					// check status effect icons
					if (iFrameCounter_StatusFX > iUpdateThreshold_StatusFX) {
						iFrameCounter_StatusFX = 0;
						UpdateStatusEffects();
					} 
					else {
						// check the condition bar
						if (iFrameCounter_CND > iUpdateThreshold_CND) {
							iFrameCounter_CND = 0;
							UpdateWidgetCNDPercent();
						}
					}
				}
			}
			else {
				// hide the hud widgets if needed
				if (!bHUDWidgetsHidden) {
					bHUDWidgetsHidden = true;
					SetHUDWidgetsVisible(false);
				}
				else {
					if (*g_player) {
						if ((*g_player)->actorState.IsWeaponDrawn()) {
							bHideHUDWidgets = false;
						}
					}
				}
			}
		}
	}
	__super::DrawNextFrame(unk0, unk1);
}




// ---------------- Pipboy Overlay

bool ATMenuInventory::GetSelectedMenuItemData()
{
	GameMenuBase * pipboyMenuVar = (GameMenuBase*)((*g_ui)->GetMenu(BSFixedString("PipboyMenu")));
	if (!pipboyMenuVar) {
		_MESSAGE("No PipboyMenu");
	}
	else {
		ATMenuInvItem curItem = ATMenuInvItem();
		ATMenuInvItem equippedItem = ATMenuInvItem();
		bool bFoundSelected = false;

		if (iPipboyTab == 0) {
			// weapons tab
			if (iSelectedItem >= 0 && (iSelectedItem < ATMenuInventory::invData_Weapons.size())) {
				curItem = ATMenuInventory::invData_Weapons[iSelectedItem];
				bFoundSelected = true;
				if (iEquippedWeapon >= 0 && (iEquippedWeapon < ATMenuInventory::invData_Weapons.size())) {
					equippedItem = ATMenuInventory::invData_Weapons[iEquippedWeapon];
				}
			}
		}
		else if (iPipboyTab == 1) {
			// armors tab
			if (iSelectedItem >= 0 && (iSelectedItem < ATMenuInventory::invData_Armors.size())) {
				curItem = ATMenuInventory::invData_Armors[iSelectedItem];
				bFoundSelected = true;
			}
		} 
		else if (iPipboyTab == 6) {
			// ammo tab
			if (iSelectedItem >= 0 && (iSelectedItem < ATMenuInventory::invData_Ammo.size())) {
				curItem = ATMenuInventory::invData_Ammo[iSelectedItem];
				bFoundSelected = true;
			}
		}

		if (bFoundSelected) {
			pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.ResetInfoObjs", nullptr, nullptr, 0);

			// ---- all tabs:

			bool bMadeChanges = false;
			float fDifference = 0.0;
			int iDifference = 0;
			float fDiffVal = 0.0;
			if (curItem.fCondition > -1.0) {
				GFxValue cndData[5];
				if (equippedItem.fCondition > -1.0) {
					fDifference = equippedItem.fCondition - curItem.fCondition;
				}
				else {
					fDifference = 0.0;
				}
				cndData[0] = &GFxValue(curItem.fCondition);
				cndData[1] = &GFxValue(curItem.fConditionMax);
				iDifference = (int)fDifference;
				if (iDifference < 0) {
					fDiffVal = 1.0;
					if (fDifference < (-0.66 * curItem.fConditionMax)) {
						fDiffVal = 3.0;
					} 
					else if (fDifference < (-0.33 * curItem.fConditionMax)) {
						fDiffVal = 2.0;
					}
				}
				else if (iDifference > 0) {
					fDiffVal = -1.0;
					if (fDifference > (0.66 * curItem.fConditionMax)) {
						fDiffVal = -3.0;
					}
					else if (fDifference > (0.33 * curItem.fConditionMax)) {
						fDiffVal = -2.0;
					}
				}
				cndData[2] = &GFxValue((float)iDifference);
				cndData[3] = &GFxValue(fDiffVal);
				cndData[4] = &GFxValue(0.0);
				pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewCondition", nullptr, cndData, 5);
				bMadeChanges = true;
			}

			if (iPipboyTab == 0) {
				// ---- weapons tab

				if (curItem.iNumProjectiles > 1) {
					GFxValue projCountData[1];
					float fNumProjectiles = (float)curItem.iNumProjectiles;
					projCountData[0] = &GFxValue(fNumProjectiles);
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.UpdateProjectileCount", nullptr, projCountData, 1);
					bMadeChanges = true;
				}

				if (curItem.iSkillReq != 0 && curItem.sReqSkill.size() != 0) {
					GFxValue skillReqData[5];
					iDifference = 0;
					fDiffVal = 0.0;
					std::string skillReqStr = curItem.sReqSkill.c_str();
					skillReqStr.append(" Req");
					BSFixedString skillReqDataStr = BSFixedString(skillReqStr.c_str());
					skillReqData[0] = &GFxValue(skillReqDataStr);
					skillReqData[1] = &GFxValue((float)curItem.iSkillReq);
					skillReqData[2] = &GFxValue((float)iDifference);
					skillReqData[3] = &GFxValue(fDiffVal);
					skillReqData[4] = &GFxValue(0.0);
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, skillReqData, 5);
					bMadeChanges = true;
				}

				if (curItem.iArmorPenetration > 0) {
					GFxValue penData[5];
					iDifference = curItem.iArmorPenetration - equippedItem.iArmorPenetration;
					fDiffVal = 0.0;
					penData[0] = &GFxValue("Penetration");
					penData[1] = &GFxValue((float)curItem.iArmorPenetration);
					penData[4] = &GFxValue(0.0);

					if (iDifference < 0) {
						fDiffVal = -1.0;
						if (iDifference < -66) {
							fDiffVal = -3.0;
						}
						else if (iDifference < -33) {
							fDiffVal = -2.0;
						}
					}
					else if (iDifference > 0) {
						fDiffVal = 1.0;
						if (fDifference > 66) {
							fDiffVal = 3.0;
						}
						else if (fDifference > 33) {
							fDiffVal = 2.0;
						}
					}
					penData[2] = &GFxValue((float)iDifference);
					penData[3] = &GFxValue(fDiffVal);
					
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, penData, 5);
					bMadeChanges = true;
				}

				if (curItem.fTargetArmorMult > 1.0) {
					GFxValue penData[5];
					iDifference = (int)((curItem.fTargetArmorMult - equippedItem.fTargetArmorMult) * 100.0);
					fDiffVal = 0.0;
					penData[0] = &GFxValue("Armor Mult");
					penData[1] = &GFxValue(curItem.fTargetArmorMult);
					penData[4] = &GFxValue(2.0);
					
					if (iDifference < 0) {
						fDiffVal = -1.0;
						if (iDifference < -66) {
							fDiffVal = -3.0;
						}
						else if (iDifference < -33) {
							fDiffVal = -2.0;
						}
					}
					else if (iDifference > 0) {
						fDiffVal = 1.0;
						if (fDifference > 66) {
							fDiffVal = 3.0;
						}
						else if (fDifference > 33) {
							fDiffVal = 2.0;
						}
					}
					penData[2] = &GFxValue((float)iDifference);
					penData[3] = &GFxValue(fDiffVal);

					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, penData, 5);
					bMadeChanges = true;
				}

				if (curItem.fWear > 0.0) {
					iDifference = 0;
					fDiffVal = 0.0;
					GFxValue wearData[5];
					wearData[0] = &GFxValue("Wear");
					wearData[1] = &GFxValue(curItem.fWear);
					wearData[2] = &GFxValue((float)iDifference);
					wearData[3] = &GFxValue(fDiffVal);
					wearData[4] = &GFxValue(2.0);
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, wearData, 5);
					bMadeChanges = true;
				}

				if (curItem.fCritMult > 0.0) {
					iDifference = 0;
					fDiffVal = 0.0;
					GFxValue critData[5];
					critData[0] = &GFxValue("Crit Mult");
					critData[1] = &GFxValue(curItem.fCritMult);
					critData[2] = &GFxValue((float)iDifference);
					critData[3] = &GFxValue(fDiffVal);
					critData[4] = &GFxValue(2.0);
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, critData, 5);
					bMadeChanges = true;
				}

				if (curItem.fCritFailMult > 0.0) {
					iDifference = 0;
					fDiffVal = 0.0;
					GFxValue critFailData[5];
					critFailData[0] = &GFxValue("Crit Failure Mult");
					critFailData[1] = &GFxValue(curItem.fCritFailMult);
					critFailData[2] = &GFxValue((float)iDifference);
					critFailData[3] = &GFxValue(fDiffVal);
					critFailData[4] = &GFxValue(2.0);
					pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.PushNewStats", nullptr, critFailData, 5);
					bMadeChanges = true;
				}
			}

			// update the InfoObj
			//if (bMadeChanges) {
				pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.UpdateInvInfo", nullptr, nullptr, 0);
			//}
		}
	}
	return true;
}


void ATGFxFunc_PipboyMenu_at_setSelectedItem::Invoke(Args * args)
{
	if (args->numArgs > 1) {
		ATMenuInventory::iSelectedItem = args->args[0].GetInt();
		ATMenuInventory::iPipboyTab = args->args[1].GetInt();
		if (ATMenuInventory::iSelectedItem != -1 && ATMenuInventory::iPipboyTab != -1) {
			ATMenuInventory::GetSelectedMenuItemData();
		}
		_MESSAGE("called at_setSelectedItem:  idx: %i, tab: %i", ATMenuInventory::iSelectedItem, ATMenuInventory::iPipboyTab);
	}
}

void ATGFxFunc_PipboyMenu_at_useSelectedItem::Invoke(Args * args)
{
	if (args->numArgs > 1) {
		ATMenuInventory::iSelectedItem = args->args[0].GetInt();
		ATMenuInventory::iPipboyTab = args->args[1].GetInt();
		_MESSAGE("called at_useSelectedItem:  idx: %i, tab: %i", ATMenuInventory::iSelectedItem, ATMenuInventory::iPipboyTab);
		if (ATMenuInventory::BuildInventoryList()) {
			ATMenuInventory::GetSelectedMenuItemData();
			ATMenuInventory::UpdatePipboyStatus();
		}
	}
}

void ATGFxFunc_PipboyMenu_at_setSelectedMenu::Invoke(Args * args)
{
	if (args->numArgs != 0) {
		int iNextPage = args->args[0].GetInt();
		_MESSAGE("called at_setSelectedMenu:  Page %i", iNextPage);
		if (iNextPage == -1) {
			ATMenuInventory::iPipboyPage = -1;
			return;
		}
		if ((ATMenuInventory::iPipboyPage == -1) || (ATMenuInventory::iPipboyPage != iNextPage)) {
			ATMenuInventory::iPipboyPage = iNextPage;
			if (iNextPage == 0) {
				ATMenuInventory::UpdatePipboyStatus();
				return;
			}
			if (iNextPage == 1) {
				ATMenuInventory::UpdatePipboyStatus();
				ATMenuInventory::GetSelectedMenuItemData();
			}
			return;
		}
	}
}


void ATGFxFunc_PipboyMenu_at_setSortMode::Invoke(Args * args)
{
	if (args->numArgs >= 1) {
		int iNewSortMode = args->args[0].GetInt();
		_MESSAGE("called at_setSortMode  %i", iNewSortMode);
		if (ATMenuInventory::iSortMode != iNewSortMode) {
			ATMenuInventory::iSortMode = iNewSortMode;
			ATMenuInventory::SortInventoryList();
		}
	}
}



bool ATMenuInventory::UpdatePipboyStatus()
{
	GameMenuBase * pipboyMenuVar = (GameMenuBase*)((*g_ui)->GetMenu(BSFixedString("PipboyMenu")));
	if (!pipboyMenuVar) {
		_MESSAGE("No PipboyMenu");
		return false;
	}

	Actor * playerActor = (Actor*)*g_player;
	if (!playerActor) {
		return false;
	}

	float fCNDPercent = 0.0;
	float fArmorPenetration = playerActor->actorValueOwner.GetValue(ATConfigData::AV_ArmorPenetration);
	float fCritFailChance = playerActor->actorValueOwner.GetValue(ATConfigData::AV_CritFailChance);
	const char * sSkillReqName = "";
	float fSkillReqDiff = 0.0;

	ExtraDataList * extraData = ATGameData::GetEquippedWeaponExtraData(playerActor, 0);
	if (extraData) {
		fCNDPercent = ATGameData::GetInstanceHealth(extraData) * 100.0;
		ATWeapon curWeapon;
		if (ATGameData::GetEquippedWeapon(playerActor, curWeapon)) {
			if (curWeapon.avRequiredSkill) {
				sSkillReqName = ATGameData::GetAVShortName(curWeapon.avRequiredSkill->formID);
				fSkillReqDiff = playerActor->actorValueOwner.GetValue(ATConfigData::AV_SkillReqDiff);
			}
		}
	}

	float fLuck = playerActor->actorValueOwner.GetValue(ATConfigData::AV_Luck);
	TESObjectWEAP::InstanceData * weapInstance = ATGameData::GetEquippedWeaponInstance(playerActor, 0);
	float fCritChance = fLuck;
	if (weapInstance) {
		fCritChance = fLuck * weapInstance->critChargeBonus * fCNDPercent;
	}

	GFxValue args[6];
	args[0].SetNumber(fCNDPercent);
	args[1].SetNumber(fArmorPenetration);
	args[2].SetNumber(fCritChance);
	args[3].SetNumber(fCritFailChance);
	args[4].SetString(sSkillReqName);
	args[5].SetNumber(-fSkillReqDiff);
	pipboyMenuVar->movie->movieRoot->Invoke("root.ATPipboyOverlay.content.UpdateWidget_Status", nullptr, args, 6);

	return true;
}



// -------------- Menu Events:

static IMenu * CreateATHUDMenu()
{
	return new ATHUDMenu();
}


class ATMenuVisibilityHandler : public BSTEventSink<MenuOpenCloseEvent>
{
public:
	virtual ~ATMenuVisibilityHandler() { };

	virtual	EventResult	ReceiveEvent(MenuOpenCloseEvent * evn, void * dispatcher) override
	{
		const BSFixedString sMenuName_HUDMenu = "HUDMenu";
		const BSFixedString sMenuName_PowerArmorHUDMenu = "PowerArmorHUDMenu";
		const BSFixedString sMenuName_faderMenu = "faderMenu";
		const BSFixedString sMenuName_PipboyMenu = "PipboyMenu";
		const BSFixedString sMenuName_DialogueMenu = "DialogueMenu";
		const BSFixedString sMenuName_BarterMenu = "BarterMenu";
		const BSFixedString sMenuName_LockpickingMenu = "LockpickingMenu";

		const BSFixedString sMenuName_ATHUDMenu = "AmmoTweaks/ATHUDMenu";


		if (evn->isOpen) {
			if (evn->menuName.c_str() == sMenuName_ATHUDMenu) {
				_MESSAGE("HUD Overlay opened");
				ATHUDMenu::bForceUpdateBaseVars = true;
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_HUDMenu) {
				if (!((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu))) {
					_MESSAGE("HUDMenu opened: opening HUD Overlay");
					ATHUDMenu::OpenMenu();
				}
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_PowerArmorHUDMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("PowerArmorHUDMenu opened: rearranging HUD Overlay");
				}
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_PipboyMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("Pip Boy opened: hiding HUD widget");
					ATHUDMenu::bHideHUDWidgets = true;
					ATHUDMenu::bPipboyOpen = true;
				}
				_MESSAGE("Pip Boy opened: enabling pipboy stuff");
				if (ATMenuInventory::BuildInventoryList()) {
					ATMenuInventory::GetSelectedMenuItemData();
				}
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_DialogueMenu || evn->menuName == sMenuName_BarterMenu || evn->menuName == sMenuName_LockpickingMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("Misc menu opened: hiding widget");
					ATHUDMenu::bHideHUDWidgets = true;
				}
			}
		}
		else {
			if (evn->menuName == sMenuName_ATHUDMenu) {
				_MESSAGE("HUD Overlay closed");
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_faderMenu) {
				if (!((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu))) {
					_MESSAGE("faderMenu closed: opening HUD Overlay");
					ATHUDMenu::OpenMenu();
				}
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_PowerArmorHUDMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("PowerArmorHUDMenu closed: rearranging HUD Overlay");
				}
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_PipboyMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("Pip Boy closed: showing hud widgets");
					
					ATHUDMenu::bHideHUDWidgets = false;
					ATHUDMenu::bPipboyOpen = false;
				}
				_MESSAGE("Pip Boy closed: disabling pipboy stuff");
				ATMenuInventory::invData_Ammo.clear();
				ATMenuInventory::invData_Armors.clear();
				ATMenuInventory::invData_Weapons.clear();
				return kEvent_Continue;
			}
			if (evn->menuName == sMenuName_DialogueMenu || evn->menuName == sMenuName_BarterMenu || evn->menuName == sMenuName_LockpickingMenu) {
				if ((*g_ui)->IsMenuOpen(sMenuName_ATHUDMenu)) {
					_MESSAGE("Misc menu closed: hiding hud widgets");
					ATHUDMenu::bHideHUDWidgets = false;
				}
			}
		}
		return kEvent_Continue;
	};
};



ATMenuVisibilityHandler g_openCloseHandler;

// -------------- Menu Functions:

void ATHUD::RegisterMenus()
{
	if (*g_ui) {
		BSFixedString menuNameHUD = ATHUDMenu::sMenuName;
		if (!(*g_ui)->menuTable.Find(&menuNameHUD)) {
			(*g_ui)->Register(menuNameHUD.c_str(), CreateATHUDMenu);
			if ((*g_ui)->IsMenuRegistered(menuNameHUD)) {
				_MESSAGE("Registered menu: %s", menuNameHUD.c_str());
			}
		}
		(*g_ui)->menuOpenCloseEventSource.AddEventSink(&g_openCloseHandler);
	}
}

void ATHUD::ForceHUDWidgetsUpdate()
{
	ATHUDMenu::bForceWeaponStatsUpdate = true;
}


void ATHUD::AddHUDMessage(BSFixedString newMessage)
{
	if (newMessage.c_str() != "") {
		if (*g_ui) {
			ATHUDMenu * menu = (ATHUDMenu*)((*g_ui)->GetMenu(BSFixedString(ATHUDMenu::sMenuName)));
			if (menu) {
				menu->AddHUDMessage(newMessage);
			}
		}
	}
}


bool ATMenuInventory::RepairItemByInvItemID(UInt32 itemUID)
{
	bool bUnequipped = false;
	bool bEquipped = false;
	Actor * playerActor = (Actor*)*g_player;
	if (!playerActor) {
		return false;
	}
	if (playerActor->inventoryList && (playerActor->inventoryList->items.count > 0)) {
		playerActor->inventoryList->inventoryLock.LockForReadAndWrite();
		for (UInt32 i = 0; i < playerActor->inventoryList->items.count; i++) {
			BGSInventoryItem inventoryItem;
			if (playerActor->inventoryList->items.GetNthItem(i, inventoryItem)) {
				if (inventoryItem.form->formType == kFormType_WEAP) {
					inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
					{
						_MESSAGE("0x%08X - refCount: %i, flags: 0x%08X", inventoryItem.form->formID, stack->m_refCount, stack->flags);
						ExtraDataList * stackDataList = stack->extraData;
						if (stackDataList && stackDataList->m_data) {
							_MESSAGE("    data_unk10: 0x%04X, unk13: 0x%02X, unk14: 0x%08X", stack->extraData->m_data->unk10, stack->extraData->m_data->unk13, stack->extraData->m_data->unk14);
						}
						else {
							_MESSAGE("   no extradata");
						}
						return true;
					});
				}
			}
		}
	}
	return bEquipped;
}




void ATMenuInventory::SortInventoryList()
{
	// Sort lists:
	switch (iSortMode) {
	case 1:
		// -- By Damage/Resistances: weapons and armors
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByDmg());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByDmg());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByDmg());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByName());
		}
		break;
	case 2:
		// -- By Speed: weapons
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByRoF());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByRoF());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByName());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByName());
		}
		break;
	case 3:
		// -- By Range: weapons
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByRange());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByRange());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByName());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByName());
		}
		break;
	case 4:
		// -- By Accuracy: weapons
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByAccuracy());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByAccuracy());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByName());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByName());
		}
		break;
	case 5:
		// -- By Value: all
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByValue());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByValue());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByValue());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByValue());
		}
		break;
	case 6:
		// -- By Weight: all
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByWeight());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByWeight());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByWeight());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByWeight());
		}
		break;
	default:
		// -- By Name: all
		//std::sort(inventoryData.begin(), inventoryData.end(), Comp_ATMenuInvItem_ByName());
		if (invData_Weapons.size() > 1) {
			std::sort(invData_Weapons.begin(), invData_Weapons.end(), Comp_ATMenuInvItem_ByName());
		}
		if (invData_Armors.size() > 1) {
			std::sort(invData_Armors.begin(), invData_Armors.end(), Comp_ATMenuInvItem_ByName());
		}
		if (invData_Ammo.size() > 1) {
			std::sort(invData_Ammo.begin(), invData_Ammo.end(), Comp_ATMenuInvItem_ByName());
		}
	}

	int iDispCounter = 0;

	_MESSAGE("\n    Weapons (%i):", invData_Weapons.size());
	if (invData_Weapons.size() > 0) {
		for (std::vector<ATMenuInvItem>::iterator itItems2 = invData_Weapons.begin(); itItems2 != invData_Weapons.end(); ++itItems2) {
			itItems2->iMenuListIndex = iDispCounter;
			_MESSAGE("      %i. %i - %s", iDispCounter, itItems2->iInvListIndex, itItems2->sItemName.c_str());
			iDispCounter += 1;
		}
	}

	_MESSAGE("\n    Armors (%i):", invData_Armors.size());
	iDispCounter = 0;
	if (invData_Armors.size() > 0) {
		for (std::vector<ATMenuInvItem>::iterator itItems2 = invData_Armors.begin(); itItems2 != invData_Armors.end(); ++itItems2) {
			itItems2->iMenuListIndex = iDispCounter;
			_MESSAGE("      %i. %s", iDispCounter, itItems2->sItemName.c_str());
			iDispCounter += 1;
		}
	}

	_MESSAGE("\n    Ammo (%i):", invData_Ammo.size());
	iDispCounter = 0;
	if (invData_Ammo.size() > 0) {
		for (std::vector<ATMenuInvItem>::iterator itItems2 = invData_Ammo.begin(); itItems2 != invData_Ammo.end(); ++itItems2) {
			_MESSAGE("      %i. - %s", iDispCounter, itItems2->sItemName.c_str());
			iDispCounter += 1;
		}
	}
}




bool ATMenuInventory::BuildInventoryList()
{
	_MESSAGE("BuildInventoryList:");

	if (!invData_Weapons.empty()) {
		invData_Weapons.clear();
	}
	if (!invData_Armors.empty()) {
		invData_Armors.clear();
	}
	if (!invData_Ammo.empty()) {
		invData_Ammo.clear();
	}
	
	iEquippedWeapon = -1;
	iEquippedGrenade = -1;
	if (!iEquippedArmors.empty()) {
		iEquippedArmors.clear();
	}

	Actor * ownerActor = (Actor*)*g_player;
	if (!ownerActor || !ownerActor->inventoryList) {
		_MESSAGE("    WTF event happened");
		return false;
	}

	BGSInventoryList * inventory = ownerActor->inventoryList;
	// list + variables are already reset, return
	if (!inventory || inventory->items.count < 1) {
		_MESSAGE("    inventory was empty");
		return true;
	}
	
	int iCounter_Instance = 0;
	UInt32 iFlagUnplayable = 0x00000004;
	UInt16 iFlagEquipped = 0x0001;
	UInt32 iCapsID = 0x0000000F;

	// cached stuff
	bool bAmmoCheck = false;
	int iInvTab = -1;
	UInt32 iCurFormID = 0;
	bool bFoundAmmoName = false;
	UInt8 actualNumProjectiles = 1;
	UInt32 tempDTID = 0;

	inventory->inventoryLock.LockForRead();
	iLastItemCount = inventory->items.count;

	// loop through the inventory
	for (UInt32 i = 0; i < inventory->items.count; i++) {
		BGSInventoryItem inventoryItem;
		if (inventory->items.GetNthItem(i, inventoryItem)) {
			if (!inventoryItem.form || !inventoryItem.stack) {
				_MESSAGE("    item %i has no stack or form", i);
				continue;
			}

			bAmmoCheck = false;

			// inventory tab sorting
			iInvTab = -1;
			switch (inventoryItem.form->formType) {
				case kFormType_WEAP:
					iInvTab = 0;
					break;
				case kFormType_ARMO:
					iInvTab = 1;
					break;
				case kFormType_AMMO:
					iInvTab = 6;
					break;
			}
			
			iCurFormID = inventoryItem.form->formID;
			// no need to process vars if the item won't show up in the inventory list (unplayable flag or caps)
			if (((inventoryItem.form->flags & iFlagUnplayable) == iFlagUnplayable) || (iCurFormID == iCapsID)) {
				_MESSAGE("  0x%08x is not playable", iCurFormID);
				continue;
			}

			// go through the item's stack for extradata thingies
			inventoryItem.stack->Visit([&](BGSInventoryItem::Stack * stack)
			{
				ExtraDataList * stackData = stack->extraData;
				if (!stackData) {
					_MESSAGE("    0x%08X: no extraDataList", iCurFormID);
					return true;
				}
				// fix for ammo stacks being counted multiple times
				if (iInvTab == 6) {
					if (ATGameData::GetInstanceHealth(stackData, false) <= 0.0) {
						if (bAmmoCheck) {
							return true;
						}
						bAmmoCheck = true;
					}
				}

				ATMenuInvItem newItem;

				// sorting id
				newItem.iInvListIndex = iCounter_Instance;
				
				// name
				newItem.sItemName = inventoryItem.form->GetFullName();
				BSExtraData * extraDataText = stackData->GetByType(ExtraDataType::kExtraData_TextDisplayData);
				if (extraDataText) {
					ExtraTextDisplayData * textData = DYNAMIC_CAST(extraDataText, BSExtraData, ExtraTextDisplayData);
					if (textData) {
						newItem.sItemName = textData->name.c_str();
					}
				}
				
				// equipped state
				if ((stack->flags & iFlagEquipped) == iFlagEquipped) {
					newItem.iEquippedState = 1;
				}

				newItem.iItemCount = inventoryItem.stack->count;
				
				if (iInvTab > -1) {
					// -- specific item types:
					if (iInvTab == 0) {
						// -------------------- Weapons
						TESObjectWEAP * weaponForm = (TESObjectWEAP*)inventoryItem.form;
						TESObjectWEAP::InstanceData * weaponInstance = ATGameData::GetWeaponInstanceData(stackData);
						if (weaponForm && !weaponInstance) {
							weaponInstance = (TESObjectWEAP::InstanceData*)&weaponForm->weapData;
						}
						if (weaponForm->enchantable.enchantment) {
							_MESSAGE("Weapon has enchantment: %s (0x%08X) with %i max charge", 
								weaponForm->enchantable.enchantment->GetFullName(), weaponForm->enchantable.enchantment->formID, weaponForm->enchantable.maxCharge);

							if (weaponForm->enchantable.enchantment->keywordForm.numKeywords != 0) {
								_MESSAGE("  Weapon enchantment keywords:");
								for (UInt32 i = 0; i < weaponForm->enchantable.enchantment->keywordForm.numKeywords; i++) {
									BGSKeyword * tempKW = weaponForm->enchantable.enchantment->keywordForm.keywords[i];
									if (tempKW) {
										_MESSAGE("    %i: %s", i, tempKW->keyword.c_str());
									}
									else {
										_MESSAGE("    %i: null", i);
									}
								}
							}
							else {
								_MESSAGE("  Weapon enchantment has no keywords.");
							}
						}
						if (weaponForm->equipType.equipSlot) {
							_MESSAGE("weapon equipslot: 0x%08X", weaponForm->equipType.equipSlot->formID);
						}
						else {
							_MESSAGE("no weapon equipslot");
						}

						if (weaponInstance) {
							newItem.fWeight = weaponInstance->weight;
							newItem.iValue = weaponInstance->value;
							newItem.fCritMult = weaponInstance->critChargeBonus;

							// damage
							newItem.iDmg_Phys = weaponInstance->baseDamage;
							if (weaponInstance->damageTypes && (weaponInstance->damageTypes->count != 0)) {
								for (UInt32 i = 0; i < weaponInstance->damageTypes->count; i++) {
									TBO_InstanceData::DamageTypes tempDT;
									if (weaponInstance->damageTypes->GetNthItem(i, tempDT)) {
										tempDTID = tempDT.damageType->formID;
										switch (tempDTID) {
											case DTID_Shock:
												newItem.iDmg_Shock = tempDT.value;
												break;
											case DTID_Energy:
												newItem.iDmg_Energy = tempDT.value;
												break;
											case DTID_Fire:
												newItem.iDmg_Fire = tempDT.value;
												break;
											case DTID_Cryo:
												newItem.iDmg_Cryo = tempDT.value;
												break;
											case DTID_Poison:
												newItem.iDmg_Poison = tempDT.value;
												break;
											case DTID_Rad:
												newItem.iDmg_Rad = tempDT.value;
												break;
											case DTID_Physical:
												newItem.iDmg_Phys += tempDT.value;
												break;
										}
									}
								}
							}

							// Range
							newItem.fRange = weaponInstance->maxRange;

							// -- guns
							if (weaponInstance->ammo) {
								newItem.iAmmoCapacity = weaponInstance->ammoCapacity;
								newItem.iAmmoCount = ATGameData::GetItemCount(ownerActor, weaponInstance->ammo->formID);
								if (weaponInstance->firingData) {
									newItem.iNumProjectiles = weaponInstance->firingData->numProjectiles;
								}

								newItem.fAccuracy = 100.0;
								if (weaponInstance->aimModel) {
									ATAimModel * tempAM = (ATAimModel*)weaponInstance->aimModel;
									if (tempAM) {
										newItem.fAccuracy = (100.0 - (((tempAM->CoF_MinAngle + tempAM->Rec_MinPerShot) * tempAM->Rec_HipMult))) * 0.5;
									}
								}

								// fire rate
								if (weaponInstance->attackDelay > 0.0) {
									newItem.fSpeed = weaponInstance->speed * (60.0 / weaponInstance->attackDelay);
								}

								if (weaponInstance->firingData) {
									actualNumProjectiles = (UInt8)((weaponInstance->firingData->numProjectiles << 12) >> 12);
									newItem.iNumProjectiles = (int)actualNumProjectiles;
								}
							}

							// -- custom variables
							ATWeapon weaponData;
							if (ATGameData::GetWeaponByID(iCurFormID, weaponData)) {
								ATWeaponModStats modStats = weaponData.GetModStats(stackData, weaponInstance);
								
								if (modStats.fMaxConditionMultAdd > 0.0) {
									_MESSAGE("Wpn CND");
									newItem.fConditionMax = modStats.fMaxConditionMultAdd * weaponData.fBaseMaxCondition;
									newItem.fCondition = ATGameData::GetInstanceHealth(stackData) * newItem.fConditionMax;
									
									newItem.fWear = modStats.fWearMult * 0.01;
								}

								newItem.fCritMult = modStats.fCritMult * 0.01;
								newItem.fCritFailMult = (float)(int)(modStats.fCritFailMult * 100.0) * 0.0001;
								newItem.iArmorPenetration = (int)modStats.fArmorPenetrationAdd;

								if (weaponData.avRequiredSkill) {
									newItem.sReqSkill = weaponData.avRequiredSkill->GetFullName();
									newItem.iSkillReq = (int)ATGameData::CalcSkillRequirement(weaponInstance->weight);
									_MESSAGE("STR req: %i", newItem.iSkillReq);
								}

								newItem.iItemIcon = weaponData.GetHUDIcon(weaponInstance);
								newItem.iFiremodeIcon = weaponData.GetFiremodeIcon(stackData);
								
								if (newItem.iAmmoCapacity > 0) {
									bFoundAmmoName = false;
									ATCaliber tempCaliber;
									if (weaponData.GetCurrentCaliber(weaponInstance, tempCaliber)) {
										ATCaliber::AmmoType tempAmmoType;
										if (weaponData.GetCurrentAmmoType(weaponInstance, stackData, tempAmmoType)) {
											if (weaponInstance->ammo->formID == tempAmmoType.ammoItem->formID) {
												newItem.sAmmoName.append(tempCaliber.sCaliberName.c_str());
												newItem.sAmmoName.append(" ");
												newItem.sAmmoName.append(tempAmmoType.sAmmoName.c_str());
												bFoundAmmoName = true;
											}
										}
									}
									if (!bFoundAmmoName) {
										newItem.sAmmoName = weaponInstance->ammo->GetFullName();
										
									}
								}
							}

							invData_Weapons.push_back(newItem);
						}
						else {
							_MESSAGE("    Weapon 0x%08X: no instanceData", iCurFormID);
						}
					}
					else if (iInvTab == 1) {
						// -------------------- Armors
						TESObjectARMO * armorForm = (TESObjectARMO*)inventoryItem.form;
						TESObjectARMO::InstanceData * armorInstance = ATGameData::GetArmorInstanceData(stackData);
						if (armorForm && !armorInstance) {
							armorInstance = (TESObjectARMO::InstanceData*)&armorForm->instanceData;
						}
						
						// BipedObjectForm enum:
						// 0x00000001 - 30 - HairTop (Hair/Helmets/Hats)
						// 0x00000002 - 31 - HairLong (Hair/Helmet Linings/Under Helmet)
						// 0x00000004 - 32 - FaceGen Head
						// 0x00000008 - 33 - Body (full outfits)
						// 0x00000010 - 34 - Left Hand
						// 0x00000020 - 35 - Right Hand
						// 0x00000040 - 36 - Torso [U] (Arm addons/Melee weapons on back)
						// 0x00000080 - 37 - Left Arm [U] (torso layer/jackets)
						// 0x00000100 - 38 - Right Arm [U] (separate tops)
						// 0x00000200 - 39 - Left Leg [U] (hip holster/tights/stocking)
						// 0x00000400 - 40 - Right Leg [U] (separate pants/shorts/skirts)
						// 0x00000800 - 41 - Torso [A]
						// 0x00001000 - 42 - Left Arm [A]
						// 0x00002000 - 43 - Right Arm [A]
						// 0x00004000 - 44 - Left Leg [A]
						// 0x00008000 - 45 - Right Leg [A]
						// 0x00010000 - 46 - Headband (earrings/masks/some helmets+hats)
						// 0x00020000 - 47 - Eyes (glasses/eyepatches)
						// 0x00040000 - 48 - Beard (lower masks)
						// 0x00080000 - 49 - Mouth (lower masks)
						// 0x00100000 - 50 - Neck (necklaces/scarves)
						// 0x00200000 - 51 - Ring
						// 0x00400000 - 52 - Scalp
						// 0x00800000 - 53 - Decapitation
						// 0x01000000 - 54 - Unnamed/Custom - Backpack/Cape/Cloak
						// 0x02000000 - 55 - Unnamed/Custom - Belt/Holster/Satchel
						// 0x04000000 - 56 - Unnamed/Custom - Shoulder Harness/Bandolier
						// 0x08000000 - 57 - Unnamed/Custom - Plate carrier/Cargo pack/Cloak backup
						// 0x10000000 - 58 - Unnamed/Custom - Piercings/Gun on back
						// 0x20000000 - 59 - Shield
						// 0x40000000 - 60 - Pipboy
						// 0x80000000 - 61 - FX
						UInt32 iArmorFlags = armorForm->bipedObject.data.parts;
						if ((iArmorFlags & 0x1) == 0x1) {
							_MESSAGE("    %s - Head", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x4) == 0x4) {
							_MESSAGE("    %s - Body", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x800) == 0x800) {
							_MESSAGE("    %s - Torso", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x1000) == 0x1000) {
							_MESSAGE("    %s - Left Arm", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x2000) == 0x2000) {
							_MESSAGE("    %s - Right Arm", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x4000) == 0x4000) {
							_MESSAGE("    %s - Left Leg", newItem.sItemName.c_str());
						}
						else if ((iArmorFlags & 0x8000) == 0x8000) {
							_MESSAGE("    %s - Right Leg", newItem.sItemName.c_str());
						}
						
						
						if (armorInstance) {
							newItem.iValue = armorInstance->value;
							newItem.fWeight = armorInstance->weight;

							if (armorInstance->health != 0) {
								newItem.fConditionMax = (float)(int)armorInstance->health;
								newItem.fCondition = ATGameData::GetInstanceHealth(stackData) * newItem.fConditionMax;
							}

							// resistances
							newItem.iDmg_Phys = armorInstance->armorRating;
							if (armorInstance->damageTypes && (armorInstance->damageTypes->count != 0)) {
								for (UInt32 i = 0; i < armorInstance->damageTypes->count; i++) {
									TBO_InstanceData::DamageTypes tempDT;
									if (armorInstance->damageTypes->GetNthItem(i, tempDT)) {
										tempDTID = tempDT.damageType->formID;
										switch (tempDTID) {
										case DTID_Shock:
											newItem.iDmg_Shock = tempDT.value;
											break;
										case DTID_Energy:
											newItem.iDmg_Energy = tempDT.value;
											break;
										case DTID_Fire:
											newItem.iDmg_Fire = tempDT.value;
											break;
										case DTID_Cryo:
											newItem.iDmg_Cryo = tempDT.value;
											break;
										case DTID_Poison:
											newItem.iDmg_Poison = tempDT.value;
											break;
										case DTID_Rad:
											newItem.iDmg_Rad = tempDT.value;
											break;
										case DTID_Physical:
											newItem.iDmg_Phys += tempDT.value;
											break;
										}
									}
								}
							}
							invData_Armors.push_back(newItem);
						}
						else {
							_MESSAGE("    Armor 0x%08X: no instanceData!", iCurFormID);
						}
						
					}
					else if (iInvTab == 6) {
						// -------------------- Ammo
						TESAmmo * tempAmmo = (TESAmmo*)inventoryItem.form;
						newItem.iValue = tempAmmo->value.value;
						newItem.fWeight = tempAmmo->weight.weight;
						// charge
						if ((int)tempAmmo->unk160[1] > 1) {
							newItem.fConditionMax = (float)(int)tempAmmo->unk160[1];
							newItem.fCondition = ATGameData::GetInstanceHealth(stackData) * newItem.fConditionMax;
						}
						invData_Ammo.push_back(newItem);
					}
				}
				iCounter_Instance += 1;
				return true;
			});
			
		}
	}
	inventory->inventoryLock.Unlock();

	SortInventoryList();
	
	return true;
}



// more credit to shavkacagarikia
bool ATHUD::RegisterScaleform(GFxMovieView * view, GFxValue * f4se_root)
{
	GFxMovieRoot* movieRoot = view->movieRoot;

	GFxValue currentSWFPathPip;
	std::string currentSWFPathStringPip = "";
	if (movieRoot->GetVariable(&currentSWFPathPip, "root.loaderInfo.url")) {
		currentSWFPathStringPip = currentSWFPathPip.GetString();
		if (currentSWFPathStringPip.find("PipboyMenu.swf") != std::string::npos)
		{
			if (!movieRoot) {
				return false;
			}
			GFxValue loaderPip, urlRequestPip, rootPip;
			movieRoot->GetVariable(&rootPip, "root");
			movieRoot->CreateObject(&loaderPip, "flash.display.Loader");
			movieRoot->CreateObject(&urlRequestPip, "flash.net.URLRequest", &GFxValue("AmmoTweaks/Widgets/ATPipboyOverlay.swf"), 1);
			rootPip.SetMember("ATPipboyOverlay", &loaderPip);
			
			GFxValue codeObj;
			movieRoot->GetVariable(&codeObj, "root.Menu_mc.BGSCodeObj");
			if (!codeObj.IsUndefined()) {
				RegisterFunction<ATGFxFunc_PipboyMenu_at_setSelectedItem>(&codeObj, movieRoot, "at_setSelectedItem");
				RegisterFunction<ATGFxFunc_PipboyMenu_at_useSelectedItem>(&codeObj, movieRoot, "at_useSelectedItem");
				RegisterFunction<ATGFxFunc_PipboyMenu_at_setSelectedMenu>(&codeObj, movieRoot, "at_setSelectedMenu");
				RegisterFunction<ATGFxFunc_PipboyMenu_at_setSortMode>(&codeObj, movieRoot, "at_setSortMode");
			}
			
			if (movieRoot->Invoke("root.ATPipboyOverlay.load", nullptr, &urlRequestPip, 1)) {
				if (movieRoot->Invoke("root.Menu_mc.addChild", nullptr, &loaderPip, 1)) {
					return true;
				}
				else {
					_MESSAGE("RegisterScaleform Error: failed to add pipboy widget (root.Menu_mc.addChild)");
				}
			}
			else {
				_MESSAGE("RegisterScaleform Error: failed to load pipboy widget (root.ATPipboyOverlay.load)");
			}
		}
	}
	return true;
}



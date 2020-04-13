#include "GlobalSettings.h"
#include "include/SimpleIni/SimpleIni.h"



// -------- ATGlobalVars defaults:

// ---- Editable settings:
const char * ATGlobalVars::configDataPath = ".\\Data\\MCM\\Settings\\AmmoTweaks.ini"; // MCM ini path
const char * ATGlobalVars::configDataPathBase = ".\\Data\\MCM\\Config\\AmmoTweaks\\settings.ini";

// -- Weapon Condition
bool ATGlobalVars::bConditionSystem = true;				// toggles the condition system
float ATGlobalVars::fMinStartingCND = 0.1;				// minimum condition weapons can start at
bool ATGlobalVars::bAllowEquipBroken = false;			
bool ATGlobalVars::bCNDAffectsValue = true;				

// -- Critical Hits/Failures
bool ATGlobalVars::bClassicCrits = true;				// toggles F3/NV simple crits
bool ATGlobalVars::bReallyClassicCrits = true;			// toggles crit effect tables
bool ATGlobalVars::bEnableSavingRolls = true;
bool ATGlobalVars::bCritFailures = true;				// toggles critical failures
float ATGlobalVars::fMinCritFailureThreshold = 0.65;	// minimum CND percentage at which crit failures are possible
bool ATGlobalVars::bShowCritMessages = true;			// enables displaying messages for critical hits
bool ATGlobalVars::bShowCritFailMessages = true;		// enables displaying messages for critical failures

// -- Weapon holstering
bool ATGlobalVars::bWeaponHolstering = true;			// toggles visible weapon holstering
bool ATGlobalVars::bWeaponHolstering_NPCs = true;		// toggles visible weapon holstering for humanoid NPCs
bool ATGlobalVars::bAutoEquipHolster = true;			// adds the weapon's specified holster on equip

// -- Drop magazines on reload
bool ATGlobalVars::bDropMagsCasingsOnReload = true;

// -- Skill Requirements
bool ATGlobalVars::bSkillRequirement = true;			// toggles the skill requirement system
bool ATGlobalVars::bSkillReqAffectsRecoil = true;		// whether or not skill requirements affect recoil
float ATGlobalVars::fSkillReq_MaxWeight = 30.0;		// weight at which the skill requirement is at its maximum
float ATGlobalVars::fSkillReq_MaxAmount = 12.0;		// maximum skill requirement value
float ATGlobalVars::fSkillReq_MaxSkillDiff = 5.0;		// the number of skill levels above or below the requirement that can affect things
float ATGlobalVars::fSkillReq_MaxRecoilDiff = 0.25;	// max. recoil difference from skill requirements

// -- HUD Overlay
bool ATGlobalVars::bHUDSwitchCNDToDirt = false;
bool ATGlobalVars::bHUDEnableColorChanges = true;
bool ATGlobalVars::bHUDUseCustomBaseColor = true;
float ATGlobalVars::fHUDBaseColorR = 0.08;
float ATGlobalVars::fHUDBaseColorG = 1.00;
float ATGlobalVars::fHUDBaseColorB = 0.09;

// -- HUD Widget transforms
// Weapon Stats
float ATGlobalVars::fHUDWeapStatsTX = 978.0;
float ATGlobalVars::fHUDWeapStatsTY = 567.0;
float ATGlobalVars::fHUDWeapStatsTS = 1.0;
// Weapon Menu
float ATGlobalVars::fHUDWeapMenuTX = 790.0;
float ATGlobalVars::fHUDWeapMenuTY = 486.0;
float ATGlobalVars::fHUDWeapMenuTS = 1.0;
// Status Effects
float ATGlobalVars::fHUDStatusFXTX = 980.0;
float ATGlobalVars::fHUDStatusFXTY = 627.0;
float ATGlobalVars::fHUDStatusFXTS = 1.0;
// Message List
float ATGlobalVars::fHUDMsgBoxTX = 976.0;
float ATGlobalVars::fHUDMsgBoxTY = 326.0;
float ATGlobalVars::fHUDMsgBoxTS = 1.0;



// ---- loads global settings
bool ATGlobalVars::LoadSettings()
{
	CSimpleIniA iniMCM;

	iniMCM.SetUnicode();

	bool bConditionSystemBase = true, bClassicCritsBase = true, bReallyClassicCritsBase = true, bCritFailuresBase = true;
	bool bSkillRequirementBase = true, bSkillReqAffectsRecoilBase = true, bDTFCalculateAPBase = true, bHUDSwitchCNDToDirtBase = false;
	bool bHUDEnableColorChangesBase = true, bHUDUseCustomBaseColorBase = true;
	float fMinStartingCNDBase = 0.1, fMinCritFailureThresholdBase = 0.65, fSkillReq_MaxWeightBase = 30.0, fSkillReq_MaxAmountBase = 12.0;
	float fSkillReq_MaxSkillDiffBase = 5.0, fSkillReq_MaxRecoilDiffBase = 0.25, fHUDBaseColorRBase = 0.08, fHUDBaseColorGBase = 1.0, fHUDBaseColorBBase = 0.09;

	// read base values for settings first
	if (iniMCM.LoadFile(configDataPathBase) != -1) {
		// ---- Weapon Condition
		bConditionSystemBase = iniMCM.GetBoolValue("WeaponCondition", "bEnableWpnCND", true);
		fMinStartingCNDBase = iniMCM.GetDoubleValue("WeaponCondition", "fMinStartingCND", 0.1);
		fMinCritFailureThresholdBase = iniMCM.GetDoubleValue("WeaponCondition", "fMinCritFailCND", 0.65);

		// ---- Critical Hits/Failures
		bClassicCritsBase = iniMCM.GetBoolValue("CriticalHits", "bUseClassicCrits", true);
		bReallyClassicCritsBase = iniMCM.GetBoolValue("CriticalHits", "bUseReallyClassicCrits", true);
		bCritFailuresBase = iniMCM.GetBoolValue("CriticalHits", "bUseCritFailureTables", true);

		// ---- Skill Requirements
		bSkillRequirementBase = iniMCM.GetBoolValue("SkillRequirements", "bEnableSkillReqs", true);
		bSkillReqAffectsRecoilBase = iniMCM.GetBoolValue("SkillRequirements", "bSkillReqAffectsRecoil", true);
		fSkillReq_MaxWeightBase = iniMCM.GetDoubleValue("SkillRequirements", "fMaxWeight", 30.0);
		fSkillReq_MaxAmountBase = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillReq", 12.0);
		fSkillReq_MaxSkillDiffBase = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillDiff", 5.0);
		fSkillReq_MaxRecoilDiffBase = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillDiff", 0.25);

		// ---- HUD Overlay
		bHUDSwitchCNDToDirtBase = iniMCM.GetBoolValue("HUD", "bHUDSwitchCNDToDirt", false);
		bHUDEnableColorChangesBase = iniMCM.GetBoolValue("HUD", "bHUDEnableColorChanges", true);
		bHUDUseCustomBaseColorBase = iniMCM.GetBoolValue("HUD", "bHUDUseCustomBaseColor", true);
		fHUDBaseColorRBase = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorR", 0.08);
		fHUDBaseColorGBase = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorG", 1.00);
		fHUDBaseColorBBase = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorB", 0.09);

		iniMCM.Reset();
	}

	// read settings overridden by MCM
	if (iniMCM.LoadFile(configDataPath) != -1) {
		// ---- Weapon Condition
		bConditionSystem = iniMCM.GetBoolValue("WeaponCondition", "bEnableWpnCND", bConditionSystemBase);
		fMinStartingCND = iniMCM.GetDoubleValue("WeaponCondition", "fMinStartingCND", fMinStartingCNDBase);
		fMinCritFailureThreshold = iniMCM.GetDoubleValue("WeaponCondition", "fMinCritFailCND", fMinCritFailureThresholdBase);

		// ---- Critical Hits/Failures
		bClassicCrits = iniMCM.GetBoolValue("CriticalHits", "bUseClassicCrits", bClassicCritsBase);
		bReallyClassicCrits = iniMCM.GetBoolValue("CriticalHits", "bUseReallyClassicCrits", bReallyClassicCritsBase);
		bCritFailures = iniMCM.GetBoolValue("CriticalHits", "bUseCritFailureTables", bCritFailuresBase);

		// ---- Skill Requirements
		bSkillRequirement = iniMCM.GetBoolValue("SkillRequirements", "bEnableSkillReqs", bSkillRequirementBase);
		bSkillReqAffectsRecoil = iniMCM.GetBoolValue("SkillRequirements", "bSkillReqAffectsRecoil", bSkillReqAffectsRecoilBase);
		fSkillReq_MaxWeight = iniMCM.GetDoubleValue("SkillRequirements", "fMaxWeight", fSkillReq_MaxWeightBase);
		fSkillReq_MaxAmount = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillReq", fSkillReq_MaxAmountBase);
		fSkillReq_MaxSkillDiff = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillDiff", fSkillReq_MaxSkillDiffBase);
		fSkillReq_MaxRecoilDiff = iniMCM.GetDoubleValue("SkillRequirements", "fMaxSkillDiff", fSkillReq_MaxRecoilDiffBase);

		// ---- HUD Overlay
		bHUDSwitchCNDToDirt = iniMCM.GetBoolValue("HUD", "bHUDSwitchCNDToDirt", bHUDSwitchCNDToDirtBase);
		bHUDEnableColorChanges = iniMCM.GetBoolValue("HUD", "bHUDEnableColorChanges", bHUDEnableColorChangesBase);
		bHUDUseCustomBaseColor = iniMCM.GetBoolValue("HUD", "bHUDUseCustomBaseColor", bHUDUseCustomBaseColorBase);
		fHUDBaseColorR = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorR", fHUDBaseColorRBase);
		fHUDBaseColorG = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorG", fHUDBaseColorGBase);
		fHUDBaseColorB = iniMCM.GetDoubleValue("HUD", "fHUDBaseColorB", fHUDBaseColorBBase);

		iniMCM.Reset();

		return true;
	}
	return false;
}
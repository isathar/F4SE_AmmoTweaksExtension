#pragma once


// -- Global settings
class ATGlobalVars
{
public:
	static const char *configDataPath;	// MCM edited ini path
	static const char *configDataPathBase;

	// -- Weapon Condition
	static bool bConditionSystem;			// toggles the condition system
	static float fMinStartingCND;			// minimum condition weapons can start at
	static bool bAllowEquipBroken;
	static bool bCNDAffectsValue;

	// -- Critical Hits/Failures
	static bool bClassicCrits;				// toggles F3/NV simple crits
	static bool bReallyClassicCrits;		// toggles crit effect tables
	static bool bCritFailures;				// toggles critical failures
	static float fMinCritFailureThreshold;	// minimum CND percentage at which crit failures are possible
	static bool bShowCritMessages;			// enables displaying messages for critical hits
	static bool bShowCritFailMessages;		// enables displaying messages for critical failures
	static bool bEnableSavingRolls;

	// -- Weapon holstering
	static bool bWeaponHolstering;			// toggles visible weapon holstering
	static bool bWeaponHolstering_NPCs;		// toggles visible weapon holstering for humanoid NPCs
	static bool bAutoEquipHolster;			// adds the weapon's specified holster on equip

	// -- Drop magazines on reload
	static bool bDropMagsCasingsOnReload;

	// -- Skill Requirements
	static bool bSkillRequirement;			// toggles the skill requirement system
	static bool bSkillReqAffectsRecoil;		// whether or not skill requirements affect recoil
	static float fSkillReq_MaxWeight;		// weight at which the skill requirement is at its maximum
	static float fSkillReq_MaxAmount;		// maximum skill requirement value
	static float fSkillReq_MaxSkillDiff;		// the number of skill levels above or below the requirement that can affect things
	static float fSkillReq_MaxRecoilDiff;	// max. recoil difference from skill requirements

	// -- HUD Overlay
	static bool bHUDSwitchCNDToDirt;
	static bool bHUDEnableColorChanges;
	static bool bHUDUseCustomBaseColor;
	static float fHUDBaseColorR;
	static float fHUDBaseColorG;
	static float fHUDBaseColorB;

	// -- HUD Widget transforms
	// Weapon Stats
	static float fHUDWeapStatsTX;
	static float fHUDWeapStatsTY;
	static float fHUDWeapStatsTS;
	// Weapon Menu
	static float fHUDWeapMenuTX;
	static float fHUDWeapMenuTY;
	static float fHUDWeapMenuTS;
	// Status Effects
	static float fHUDStatusFXTX;
	static float fHUDStatusFXTY;
	static float fHUDStatusFXTS;
	// Message List
	static float fHUDMsgBoxTX;
	static float fHUDMsgBoxTY;
	static float fHUDMsgBoxTS;


	// gets settings from config files - used to update settings when the MCM is closed
	static bool LoadSettings();
};

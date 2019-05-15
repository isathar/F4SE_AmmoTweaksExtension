#pragma once
#include <string>
#include "SimpleIni/SimpleIni.h"

#include "f4se/GameData.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusUtilities.h"



// *********************************************** Base Classes:

// for vm stuff:
struct StaticFunctionTag;
class VirtualMachine;


// AimModel - affects spread, recoil and aiming stability
// - somewhat safe usage: cast AimModel as ATAimModel, edit result
// - any AimModel edits require the edited weapon to be unequipped to take effect

// 110
class ATAimModel : public BaseFormComponent
{
public:
	virtual			~ATAimModel();

	// not sure about these first few...
	// - they seem to be similar to TESForm's first few variables
	// - using UInt32s to fill in the bytes before the actual AimModel variables start

	UInt32			unk00;					//00
	UInt32			unk08;					//08
	UInt32			unk10;					//10

	UInt32			formID;					//18 - gets set to 0 when any values are edited by omods or plugins

	UInt32			unk20;					//20
	UInt32			unk28;					//28

	// *cof* = spread/cone of fire, *rec* = recoil:

	float			CoF_MinAngle;			//30 - min. spread angle (crosshair size)
	float			CoF_MaxAngle;			//38 - max. spread angle
	float			CoF_IncrPerShot;		//40 - spread increase per shot
	float			CoF_DecrPerSec;			//48 - spread decrease per second (after delay)
	UInt32			CoF_DecrDelayMS;		//50 - delay in ms before spread starts to decrease after firing
	float			CoF_SneakMult;			//58 - multiplier applied to spread while sneaking/crouched
	float			Rec_DimSpringForce;		//60 - amount of automatic aim correction after recoil
	float			Rec_DimSightsMult;		//68 - amount of automatic aim correction after recoil while aiming
	float			Rec_MaxPerShot;			//70 - max. amount of recoil per shot
	float			Rec_MinPerShot;			//78 - min. amount of recoil per shot
	float			Rec_HipMult;			//80 - multiplier applied to recoil while firing from the hip
	UInt32			Rec_RunawayShots;		//88 - the number of shots before recoil becomes unbearable?
	float			Rec_ArcMaxDegrees;		//90 - max. difference from the base recoil angle per shot in degrees
	float			Rec_ArcRotate;			//98 - angle for the recoil direction (clock-wise from 12:00)
	float			CoF_IronSightsMult;		//100 - multiplier applied to spread while aiming without a scope
	float			BaseStability;			//108 - multiplier applied to the amount of camera movement while using a scope
};


// TESSpellList's unk08: 
// - used by TESRace and TESActorBase for default abilities, TESNPC for an actor's current abilities

//20
class ATSpellListEntries
{
public:
	SpellItem**			spells;		//00 - array of SpellItems
	void*				unk08;		//08
	void*				unk10;		//10
	UInt32				numSpells;	//18 - length of spells - set manually
};


// container for ATSpellListEntries (TESSpellList):
// - probably completely unsafe usage: cast TESSpellList as ATTESSpellList, edit

// 10
class ATTESSpellList : public BaseFormComponent
{
public:
	ATSpellListEntries	* unk08;	// 08
};


// *********************************************** Custom Classes

/** base class for weapons, calibers, and crit tables
		- stores UID and config file path */
class ATConfigObject
{
public:
	UInt32 objectID = 0x0;
	BSFixedString hudName = "";
	BSFixedString configPath = "";
};


/** Swappable Mod
		swapMod = mod to attach
		requiredKW = required keyword
		modItem = required item or ammo
		bRequireMiscMod = if true, require the owner to have the miscmod item
*/
struct ATSwappableMod
{
	BGSMod::Attachment::Mod *swapMod = nullptr;
	BGSKeyword *requiredKW = nullptr;
	TESForm *modItem = nullptr;
	bool bRequireModMisc = false;
};


// Caliber object
class ATCaliber : public ATConfigObject
{
public:
	struct ProjectileOverride
	{
		BGSKeyword *projectileKW = nullptr;
		BGSKeyword *excludeKW = nullptr;
		BGSProjectile *projectile = nullptr;
		BGSImpactDataSet *impactData = nullptr;
	};
	struct AmmoType : ATSwappableMod
	{
		tArray<ProjectileOverride> projectiles;
		TESObjectMISC *casingItem = nullptr;
	};

	tArray<AmmoType> ammoTypes;

	TESGlobal *chanceNoneGlobal;


	bool ReadIni(const char* configFileName);
};


// Weapon object
class ATWeapon : public ATConfigObject
{
public:
	// holds data for a swappable mod
	struct ModSlot
	{
		BSFixedString slotName = BSFixedString("");
		tArray<ATSwappableMod> swappableMods;

		// idle animation to play before swapping to mods in this slot
		TESForm *swapIdle_1P = nullptr;
		TESForm *swapIdle_3P = nullptr;

		// event to listen to begin a mod swap
		BSFixedString swapAnimEventName = "";
	};

	// holds a magazine/casing/cell item
	struct DroppableMag
	{
		BGSKeyword *magKW = nullptr;
		TESObjectMISC *magItem = nullptr;
	};

	// holds holster/weapon armors for drawn/sheathed states
	struct HolsterArmor
	{
		BSFixedString holsterName = "";
		BGSKeyword *requiredKW = nullptr;
		TESObjectARMO *armorWeapon = nullptr;
		TESObjectARMO *armorHolster = nullptr;
	};

	// swappable mod slots
	tArray<ModSlot> modSlots;

	// damaged mod slots
	tArray<ModSlot> damagedMods;

	// critical failure table
	BGSKeyword *critFailureType = nullptr;

	// list of droppable magazine items associated with this weapon
	tArray<DroppableMag> magazines;

	/* reload type for dropped mag/casing timing
		 -1=disabled, 0=magazine, 1=energy cell, 2=revolver, 3=bolt-action */
	int iReloadType = -1;

	// holster armors/required keywords
	tArray<HolsterArmor> holsters;

	// animation idle to play when swapping ammo types
	TESForm *ammoSwapIdle_1P = nullptr;
	TESForm *ammoSwapIdle_3P = nullptr;
	// animation event to listen for to perform an ammo swap
	BSFixedString ammoSwapAnimEvent = BSFixedString("");

	ActorValueInfo *requiredSkill = nullptr;


	/** gets a random damaged mod
		iModType: -1 = random slot, 0+ = specific slot index */
	BGSMod::Attachment::Mod *GetDamagedMod(Actor *thisActor, UInt32 iEquipSlot, int iSlot);

	// loads weapon data from ini (ATConfig)
	bool ReadIni(const char* configFileName, bool bSaveData, bool bLoadStatTweaks, bool bFollowerWeapons, bool bNPCsUseAmmo);
};



// -------- Critical Effect Tables:

// a critical effect
struct ATCritEffect
{
	// spells to apply if the crit roll succeeds
	SpellItem *critSpell = nullptr;
	// upper threshold for the crit roll
	int rollMin = 0;
	int rollMax = 0;
};

class ATCritEffectTable : public ATConfigObject
{
public:
	tArray<ATCritEffect> critEffects, critEffects_Human, critEffects_Robot;

	// returns a random critical effect spell based on iRollMod
	SpellItem *GetCritSpell(UInt32 iRollMod = 0, UInt32 iTargetType = 0);

	// returns a random critical failure spell based on iRollMod and Luck
	SpellItem *GetCritFailureSpell(UInt32 iRollMod = 0, UInt32 iLuck = 0);
};


class ATDataStore
{
public:
	// fixed weapon flags:
	enum ATWeaponFlags
	{
		//Flag_PlayerOnly			= 0x0000001,
		wFlag_NPCsUseAmmo			= 0x0000002,
		//Flag_NoJamAfterReload		= 0x0000004,
		//Flag_ChargingReload		= 0x0000008,
		//Flag_MinorCrime			= 0x0000010,
		//Flag_FixedRange			= 0x0000020,
		//Flag_NotUsedInNormalCombat= 0x0000040,
		//Flag_CritEffectonDeath	= 0x0000100,
		//Flag_ChargingAttack		= 0x0000200,
		//Flag_HoldInputToPower		= 0x0000800,
		//Flag_NonHostile			= 0x0001000,
		//Flag_BoundWeapon			= 0x0002000,
		//Flag_IgnoresNormalResist	= 0x0004000,
		wFlag_Automatic				= 0x0008000,
		//Flag_RepeatableSingleFire	= 0x0010000,
		wFlag_CantDrop				= 0x0020000,
		//Flag_HideBackpack			= 0x0040000,
		//Flag_EmbeddedWeapon		= 0x0080000,
		wFlag_NotPlayable			= 0x0100000,
		wFlag_HasScope				= 0x0200000,
		//wFlag_BoltAction			= 0x0400000,
		//wFlag_SecondaryWeapon		= 0x0800000,
		//wFlag_DisableShells		= 0x1000000
	};

	tArray<ATWeapon> Weapons;
	tArray<ATCaliber> Calibers;

	tArray<ATCritEffectTable> CritEffectTables;
	tArray<ATCritEffectTable> CritFailureTables;


	// ---- Data cache handling:

	int GetWeaponIndex(UInt32 formID);
	bool GetWeaponByID(UInt32 formID, ATWeapon weaponData);

	int GetCaliberIndex(UInt32 formID);
	bool GetCaliberByID(UInt32 formID, ATCaliber caliberData);

	int GetCritTableIndex(UInt32 formID);
	int GetCritFailTableIndex(UInt32 formID);

	// ---- Config file handling:

	bool LoadData_CritEffectTables(const std::string & configDir);
	bool LoadData_CritFailureTables(const std::string & configDir);

	UInt32 LoadData_Caliber(const std::string & configDir);
	UInt32 LoadData_Weapon(const std::string & configDir, bool bSaveData, bool bLoadStatTweaks, bool bFollowerWeapons, bool bNPCsUseAmmo);

	bool LoadData_Plugin(const std::string & configDir, bool bLoadStatTweaks, bool bLoadTurrets, bool bLoadThrownWeapons, bool bFollowerWeapons, bool bNPCsUseAmmo);

	bool FinalizeData(bool bEnableLog = false);
};



namespace ATShared
{
	// Global data cache
	extern ATDataStore ATData;


	// ---- FormID/Form <-> Identifier:

	const char* GetPluginNameFromFormID(UInt32 formID);

	/** Standard Identifer format:
			PluginName.extension|last 3 bytes of FormID
			ex. Fallout4.esm|000000
	*/
	const char* GetIdentifierFromFormID(UInt32 formID, const char* strSplit = "|");
	UInt32 GetFormIDFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");
	TESForm * GetFormFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");

	/** ValueModifier/DamageType Identifier format:
			PluginName.extension|last 3 bytes of AV/DT FormID|value
			ex. Fallout4.esm|000000|99
	*/
	TBO_InstanceData::ValueModifier GetAVModiferFromIdentifer(const std::string & formIdentifier);
	TBO_InstanceData::DamageTypes GetDamageTypeFromIdentifier(const std::string & formIdentifier);

	/** ATCritEffect Identifier format:
			PluginName.extension|last 3 bytes of KW FormID|minRoll|maxRoll
			ex. AmmoTweaks.esm|000000|0|20
	*/
	ATCritEffect GetCritEffectFromIdentifier(const std::string & formIdentifier);
	
	// ---- Equipped Item:

	UInt32 GetEquippedItemFormID(Actor *thisActor, UInt32 iSlot = 41);
	TESObjectWEAP *GetEquippedWeapon(Actor *thisActor, UInt32 iSlot = 41);
	
	// ---- InstanceData:

	TESObjectWEAP::InstanceData *GetWeapRefInstanceData(VMRefOrInventoryObj *curObj);
	TESObjectWEAP::InstanceData *GetEquippedInstanceData(Actor *thisActor, UInt32 iSlot = 41);

	BGSKeyword* GetInstanceCaliber(TESObjectWEAP::InstanceData *instanceData);


	// ---- Initialization:

	void RegisterPapyrus(VirtualMachine * vm);

}


namespace ATConfig
{
	void LoadGameData();
}

namespace ATWeaponRef
{
	void RegisterPapyrus(VirtualMachine * vm);
}

namespace ATGlobals
{
	void RegisterPapyrus(VirtualMachine * vm);
}


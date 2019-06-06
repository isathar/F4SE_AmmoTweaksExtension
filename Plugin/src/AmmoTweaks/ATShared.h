#pragma once
#include "f4se/GameData.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusUtilities.h"


struct StaticFunctionTag;
class VirtualMachine;


// *********************************************** Base Classes:

/* (AimModel)
- affects spread, recoil and aiming stability
- somewhat safe usage: cast AimModel as ATAimModel, edit result
- any AimModel edits require affected weapons to be re-equipped to take effect
110 */
class ATAimModel : public BaseFormComponent
{
public:
	virtual			~ATAimModel();

	// not sure about these first few...
	// - they seem to be similar to TESForm's first few variables
	// - using UInt32s to fill in the bytes before the actual AimModel variables start

	UInt32				unk00;						//00
	UInt32				unk08;						//08
	UInt32				unk10;						//10
	UInt32				formID;						//18 - gets set to 0 when any values are edited by omods or plugins
	UInt32				unk20;						//20
	UInt32				unk28;						//28

	// *cof* = spread/cone of fire, *rec* = recoil:

	float				CoF_MinAngle;				//30 - min. spread angle (crosshair size)
	float				CoF_MaxAngle;				//38 - max. spread angle
	float				CoF_IncrPerShot;			//40 - spread increase per shot
	float				CoF_DecrPerSec;				//48 - spread decrease per second (after delay)
	UInt32				CoF_DecrDelayMS;			//50 - delay in ms before spread starts to decrease after firing
	float				CoF_SneakMult;				//58 - multiplier applied to spread while sneaking/crouched
	float				Rec_DimSpringForce;			//60 - amount of automatic aim correction after recoil
	float				Rec_DimSightsMult;			//68 - amount of automatic aim correction after recoil while aiming
	float				Rec_MaxPerShot;				//70 - max. amount of recoil per shot
	float				Rec_MinPerShot;				//78 - min. amount of recoil per shot
	float				Rec_HipMult;				//80 - multiplier applied to recoil while firing from the hip
	UInt32				Rec_RunawayShots;			//88 - the number of shots before recoil becomes unbearable?
	float				Rec_ArcMaxDegrees;			//90 - max. difference from the base recoil angle per shot in degrees
	float				Rec_ArcRotate;				//98 - angle for the recoil direction (clock-wise from 12:00)
	float				CoF_IronSightsMult;			//100 - multiplier applied to spread while aiming without a scope
	float				BaseStability;				//108 - multiplier applied to the amount of camera movement while using a scope
};


// *********************************************** Custom Classes:

// base class for AmmoTweaks data structures
class ATConfigObject
{
public:
	UInt32					objectID = 0x0;					// unique ID (usually a keyword or weapon formID)
	BSFixedString			hudName = "";					// name shown in menus/HUD
	BSFixedString			configPath = "";				// ini file path
};


// a critical effect spell and its maximum roll value
struct ATCritEffect
{
	SpellItem		* critSpell = nullptr;			// default spell to apply if the crit roll succeeds
	int				rollMax = 0;					// upper threshold for the crit roll
};

// Critical Effect Table
class ATCritEffectTable : public ATConfigObject
{
public:
	// race-specific crit effect table variation
	struct AltCritTable
	{
		tArray<ATCritEffect>	critEffects;		// list of crit spells
		tArray<UInt32>			critRaces;			// races this crit effect table applies to
	};
	tArray<AltCritTable>	critVariations;			// crit table variations based on target race
	tArray<ATCritEffect>	defaultEffects;			// critical effects to use if no AltCritTable conditions are met
	

	SpellItem *GetCritSpell(UInt32 iRollMod = 0, UInt32 iRaceID = 0);	// returns a random critical effect spell


	bool ReadIni(const char * configFileName);
};


// Caliber info
class ATCaliber : public ATConfigObject
{
public:
	struct ProjectileOverride
	{
		UInt32					projectileKW = 0;			// required keyword id
		UInt32					excludeKW = 0;				// exclude keyword id
		BGSProjectile			* projectile = nullptr;		// projectile override
		BGSImpactDataSet		* impactData = nullptr;		// ImpactDataSet
	};
	struct AmmoType
	{
		BSFixedString			hudName = "";				// name shown in HUD menus
		TESAmmo					* ammoItem = nullptr;		// required item or ammo (if AmmoType)
		BGSMod::Attachment::Mod	* ammoMod = nullptr;		// mod to attach
		TESObjectMISC			* casingItem = nullptr;		// casing
		tArray<ProjectileOverride> projectiles;				// projectile overrides + check keywords
	};
	tArray<AmmoType>		ammoTypes;						// supported ammo types

	BGSKeyword				* recipeKW = nullptr;			// weapon workbench recipe keyword

	TESForm					* swapIdle_1P = nullptr;		// 1st person idle to play when swapping mods
	TESForm					* swapIdle_3P = nullptr;		// 3rd person idle to play when swapping mods
	BSFixedString			swapAnimEventName = "";			// event to listen for to begin a mod swap


	bool ReadIni(const char * configFileName);
};


// -------- weapon data:

// Weapon info
class ATWeapon : public ATConfigObject
{
public:
	// updated weapon flags
	enum ATWeaponFlags
	{
		wFlag_PlayerOnly			= 0x0000001,
		wFlag_NPCsUseAmmo			= 0x0000002,
		wFlag_NoJamAfterReload		= 0x0000004,
		wFlag_ChargingReload		= 0x0000008,
		wFlag_MinorCrime			= 0x0000010,
		wFlag_FixedRange			= 0x0000020,
		wFlag_NotUsedInNormalCombat = 0x0000040,
		wFlag_CritEffectonDeath		= 0x0000100,
		wFlag_ChargingAttack		= 0x0000200,
		wFlag_HoldInputToPower		= 0x0000800,
		wFlag_NonHostile			= 0x0001000,
		wFlag_BoundWeapon			= 0x0002000,
		wFlag_IgnoresNormalResist	= 0x0004000,
		wFlag_Automatic				= 0x0008000,
		wFlag_RepeatableSingleFire	= 0x0010000,
		wFlag_CantDrop				= 0x0020000,
		wFlag_HideBackpack			= 0x0040000,
		wFlag_EmbeddedWeapon		= 0x0080000,
		wFlag_NotPlayable			= 0x0100000,
		wFlag_HasScope				= 0x0200000,
		wFlag_BoltAction			= 0x0400000,
		wFlag_SecondaryWeapon		= 0x0800000,
		wFlag_DisableShells			= 0x1000000
	};

	// Swappable Mod Info
	struct ATSwappableMod
	{
		BSFixedString			menuName = "";				// HUD/Menu name for this mod
		BGSMod::Attachment::Mod	* swapMod = nullptr;		// mod to attach
		UInt32					requiredKW = 0;			// required wepon keyword id
		TESForm					* modItem = nullptr;		// required item or ammo (if AmmoType)
		bool					bRequireModMisc = false;	// require the owner to have the miscmod item
	};
	struct ATModSlot
	{
		BSFixedString			slotName = "";				// HUD/Menu name for this slot
		TESForm					* swapIdle_1P = nullptr;	// 1st person idle to play when swapping mods
		TESForm					* swapIdle_3P = nullptr;	// 3rd person idle to play when swapping mods
		BSFixedString			swapAnimEventName = "";		// event to listen to begin a mod swap
		tArray<ATSwappableMod>	swappableMods;				// list of mods supported by this slot
	};

	// damaged mod info
	struct ATDamagedModSlot
	{
		BSFixedString			slotName = "";				// HUD/Menu name for this slot
		tArray<BGSMod::Attachment::Mod*> damagedMods;		// list of mods supported by this slot
	};

	struct ATMagItem
	{
		TESObjectMISC			* magItem = nullptr;		// magazine misc item
		UInt32					magKW = 0;					// required keyword ID
	};
	struct ATHolsterArmor
	{
		BSFixedString			holsterName = "";			// HUD/Menu name
		UInt32					requiredKW = 0;				// required keyword ID
		TESObjectARMO			* armorWeapon = nullptr;	// sheathed weapon armor
	};

	tArray<ATCaliber>		calibers;						// supported ammo types
	tArray<ATModSlot>		modSlots;						// swappable mod slots
	tArray<ATDamagedModSlot> damagedModSlots;				// damaged mod slots

	tArray<ATCritEffect>	critFailures;					// critical failure table
	tArray<ATCritEffect>	critFailures_Bash;				// bashing critical failure table

	int						iReloadType = -1;				// -1=none, 0=mag, 1=cell, 2=revolver, 3=boltaction
	tArray<ATMagItem>		magazines;						// list of magazine items + keywords
	
	tArray<ATHolsterArmor>	holsters;						// holster/sheathed weapon armors

	ActorValueInfo			* requiredSkill = nullptr;		// required skill AV


	bool GetInstanceCaliber(TESObjectWEAP::InstanceData * instanceData, ATCaliber & thisCaliber);
	int GetCaliberDataIndex(UInt32 formID);
	bool GetCaliberDataByID(UInt32 caliberID, ATCaliber & retCaliber);

	// gets a random damaged mod (random slot if iSlot == -1)
	BGSMod::Attachment::Mod * GetDamagedMod(Actor * ownerActor, UInt32 iEquipSlot, int iModSlot);

	// config file loading:
	bool ReadIni(const char * configFileName);
	bool ReadINI_Caliber(const char * configFileName, TESObjectWEAP * weaponBase);
};


// Crafting Menu Category
class ATCraftingCategory
{
public:
	struct RecipeCompo
	{
		TESForm * compo = nullptr;
		UInt32 iCount = 0;
	};
	struct RecipeRequirement
	{
		UInt32 reqForm = 0;
		UInt32 iValue = 0;
	};

	struct Recipe
	{
		BSFixedString sName = "";
		UInt32 outputID = 0, outputCount = 0, outputType = 0;
		float fHoursToCraft = 0.0;

		tArray<RecipeCompo> inputCompos;
		tArray<RecipeRequirement> requiredItems, requiredAVs, requiredPerks;

		bool bRequireContraption = false;
	};

	struct SubCategory
	{
		BSFixedString sName = "";
		tArray<Recipe> recipes;
	};

	BSFixedString sName = "";
	tArray<SubCategory> subCategories;
};


/* random number generator:
- implementation of xoroshiro128+ (http://xoshiro.di.unimi.it/xoroshiro128plus.c)
*/
class ATxoroshiro128p
{
	const float fMaxUInt32 = (float)0xFFFFFFFF;				// cached max UInt32 as a float
	static const int a = 24, b = 16, c = 37, d = 64;		// xoroshiro parameters
	uint64_t state[2];										// current state

	// rotating left shift
	static inline uint64_t rotl(const uint64_t x, int k)
	{
		return (x << k) | (x >> (d - k));
	}

	// get the next random number, advance the state
	UInt32 Advance()
	{
		const uint64_t s0 = state[0];
		uint64_t s1 = state[1];
		const uint64_t result = s0 + s1;
		s1 ^= s0;
		state[0] = rotl(s0, a) ^ s1 ^ (s1 << b);
		state[1] = rotl(s1, c);
		return result;
	}

public:
	// simplified rng state seed
	void Seed(uint64_t newSeed) 
	{
		state[0] = newSeed;
		state[1] = newSeed / 3 + 1;
	}

	// returns a random int between minVal and maxVal (inclusive)
	int RandomInt(int minVal = 0, int maxVal = 100)
	{
		return (int)(((float)Advance() / fMaxUInt32) * (float)(maxVal + 1 - minVal)) + minVal;
	}
};



namespace ATUtilities
{
	// random number generator
	extern ATxoroshiro128p ATrng;

	// ---- FormID/Form <-> Identifier:

	const char * GetPluginNameFromFormID(UInt32 formID);

	/** Standard Identifer format:
			PluginName.extension|last 3 bytes of FormID
			ex. "AmmoTweaks.esm|000000"
	*/
	const char * GetIdentifierFromFormID(UInt32 formID);
	UInt32 GetFormIDFromIdentifier(const std::string & formIdentifier);
	TESForm * GetFormFromIdentifier(const std::string & formIdentifier);

	/** ATCritEffect Identifier format:
			PluginName.extension|last 3 bytes of KW FormID, maxRoll
			ex. "AmmoTweaks.esm|000000, 20"
	*/
	bool GetCritEffectFromIdentifier(const std::string & formIdentifier, ATCritEffect & tempEffect);

	// gets the FormID of ownerActor's weapon equipped at iEquipSlot
	UInt32 GetEquippedItemFormID(Actor * ownerActor, UInt32 iEquipSlot = 41);

	// gets ownerActor's weapon equipped at iEquipSlot
	TESObjectWEAP *GetEquippedWeapon(Actor * ownerActor, UInt32 iEquipSlot = 41);

	// gets a weapon reference/inventory object's InstanceData
	TESObjectWEAP::InstanceData * GetWeapRefInstanceData(VMRefOrInventoryObj * curObj);

	// gets the InstanceData of ownerActor's weapon equipped at iEquipSlot
	TESObjectWEAP::InstanceData * GetEquippedInstanceData(Actor * ownerActor, UInt32 iEquipSlot = 41);

}


// global data cache interactions
namespace ATGameData
{
	// weapons list:
	extern tArray<ATWeapon> ATWeapons;
	int GetWeaponIndex(UInt32 formID);
	bool GetWeaponByID(UInt32 formID, ATWeapon & weaponData);

	// crit effect tables list:
	extern tArray<ATCritEffectTable> ATCritEffectTables;
	int GetCritTableIndex(UInt32 formID);
	bool GetCritTableByID(UInt32 formID, ATCritEffectTable & critTable);
	bool LoadData_CritEffectTables(const std::string & configDir);

	// crafting recipe list:
	extern tArray<ATCraftingCategory> ATCraftingData;
	UInt32 GetRecipeOutput(const char * sCategory, const char * sSubCategory, const char * sRecipeName);
	
	// loads all data from a config template
	bool LoadData_Template(const std::string & configDir);

	// writes weapon + crit effects data to log
	bool LogConfigData();

	// starts the loading process
	void LoadGameData();
}


// -------- Papyrus initialization:

// ATPapyrusGlobals:
namespace ATGlobals
{
	bool RegisterPapyrus(VirtualMachine * vm);
}

// ATPapyrusWeaponRef:
namespace ATWeaponRef
{
	bool RegisterPapyrus(VirtualMachine * vm);
}

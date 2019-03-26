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
// usage: cast AimModel to ATAimModel

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
// usage: cast TESSpellList as ATTESSpellList

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
	//std::string *configPath;
	BSFixedString configPath = BSFixedString("");
};

// contains everything needed for basic mod swapping
struct ATSwappableMod
{
	// ObjectMod to attach
	BGSMod::Attachment::Mod *swapMod = nullptr;
	// Keyword required to enable swapping to this mod
	BGSKeyword *requiredKW = nullptr;
	// Keyword that disables swapping to this mod
	BGSKeyword *excludeKW = nullptr;
	// an item associated with this mod - Ammo, MiscItem, etc.
	TESForm *modItem = nullptr;
	
	// TBD: index of the Idle to play when attaching this mod - set in scripts
	int idleAnimIndex = -1;
	// TBD: chance to pick this mod during RNG check
	int chanceToPick = 100;
};



// caliber info
class ATCaliber : public ATConfigObject
{
public:
	// keyword:projectile pair for keyword-specific projectile overrides
	struct ATProjectileOverride
	{
		BGSKeyword *projectileKW = nullptr;
		BGSProjectile *projectile = nullptr;
		BGSImpactDataSet *impactData = nullptr;
	};

	// an ammo subtype
	struct ATAmmoType : ATSwappableMod
	{
		// projectile overrides/impactData for this ammo type
		tArray<ATProjectileOverride> projectiles;

		TESObjectMISC *casingItem = nullptr;
	};

	tArray<ATAmmoType> ammoTypes;


	// gets caliber data from ini (ATConfig)
	bool LoadFromIni(const char* caliberFileName);

	// TBD: writes weapon data to ini (ATConfig)
	bool SaveToIni();
};


// a critical effect
struct ATCritEffect
{
	// spell to apply if the crit roll succeeds
	SpellItem *critSpell = nullptr;
	// message to show
	BGSMessage *critMessage = nullptr;
	// upper threshold for the crit roll
	int rollMax = 0;
	// effect scale
	float critScale = 1.0;
};

// Critical Effect Table
class ATCritEffectTable : public ATConfigObject
{
public:
	tArray<ATCritEffect> critEffects;

	SpellItem *GetCritSpell(UInt32 iRollMod = 0);
	bool LoadFromINI(const char *configPath);
};

// Critical Failure Table
class ATCritFailureTable : public ATConfigObject
{
public:
	tArray<ATCritEffect> critFailures;

	SpellItem *GetCritFailureSpell(UInt32 iRollMod = 0);
	bool LoadFromINI(const char *configPath);
};


// weapon info - swappable mods, crit failure table, magazines, etc.
class ATWeapon : public ATConfigObject
{
public:
	// holds the UI name for a swappable mod category and the list of mods in it
	struct ATModSlot
	{
		BSFixedString slotName = BSFixedString("");
		tArray<ATSwappableMod> swappableMods;
	};

	// holds a droppable magazine/casing/cell item
	struct ATMagItem
	{
		BGSKeyword *magKW = nullptr;
		TESObjectMISC *magItem = nullptr;
	};

	// holds holster armor for drawn/sheathed states and a check keyword
	struct ATHolsterItem
	{
		BGSKeyword *requiredKW = nullptr;
		TESObjectARMO *armorItem_Sheathed = nullptr;
		TESObjectARMO *armorItem_Drawn = nullptr;
	};

	/* swappable mod slots:
		Guns:  0=Firemode, 1=Muzzle, 2=Scope, 3=Bayonet
		Melee:  0=AttackType/Firemode	*/
	tArray<ATModSlot> modSlots;

	/* damaged mod slots:
		Guns:  0=Muzzle, 1=Barrel, 1=Grip/Stock, 3=Receiver/Capacitor, 4=Magazine/Cell
		Launcher:  0=Barrel, 1=Grip
		Melee:  0=Blade/Top, 1=Grip		*/
	tArray<ATModSlot> damagedMods;

	// critical failure table
	ATCritFailureTable critFailureTable;

	// list of droppable magazine items associated with this weapon
	tArray<ATMagItem> magazines;

	/* reload type for dropped mag/casing timing
		 -1=disabled, 0=magazine, 1=energy cell, 2=revolver, 3=bolt-action */
	int iReloadType = -1;

	// holster armors/required keywords
	tArray<ATHolsterItem> holsters;


	// defined in ATConfig:
	
	// loads weapon data from ini 
	bool LoadFromIni(const char* weapFileName);

	// defined in ATWeaponRef:

	/** gets a random damaged mod
		iModType: -1 = random slot, 0+ = specific slot index */
	BGSMod::Attachment::Mod *GetDamagedMod(TESObjectWEAP::InstanceData* instanceData, int iSlot);
};





//*********************************************** Shared Functions:

namespace ATShared
{
	// returns a form's plugin's name
	const char* GetPluginNameFromFormID(UInt32 formID);

	// returns a formatted string containing (string=pluginName|UInt=formID without loadorder) 
	const char* GetIdentifierFromFormID(UInt32 formID, const char* strSplit = "|");

	// returns a formID from a formatted string
	UInt32 GetFormIDFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");

	// returns a form from a formatted string
	TESForm * GetFormFromIdentifier(const std::string & formIdentifier, const char* strSplit = "|");


	bool RegisterPapyrus(VirtualMachine * vm);


	extern tArray<UInt32> index_Weapons;
	extern tArray<ATWeapon> ATWeapons;

	extern tArray<UInt32> index_Calibers;
	extern tArray<ATCaliber> ATCalibers;
	
	extern tArray<ATCritEffectTable> ATCritEffectTables;
}


namespace ATConfig
{
	void EditGameData();
}


namespace ATWeaponRef
{
	bool RegisterPapyrus(VirtualMachine * vm);
}

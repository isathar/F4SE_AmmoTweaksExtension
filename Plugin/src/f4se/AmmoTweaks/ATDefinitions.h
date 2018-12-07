#pragma once
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"



/********************************************/
//	Class definitions, shared structs:




/****************************************************************/
//		base classes:


// AimModel - affects spread, recoil and aiming stability

// 110
class ATAimModel : public BaseFormComponent
{
public:
	virtual			~ATAimModel();

	// *cof* = spread/cone of fire, *rec* = recoil

	UInt32			unk00;					//00
	UInt32			unk04;					//08
	UInt32			unk08;					//10

	UInt32			formID;					//18 - gets set to 0 when any values are edited by omods or plugins

	UInt32			unk10;					//20
	UInt32			unk14;					//28

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
// - structure is a lot like a tArray, but can't be cast to one

//20
class ATSpellListEntries
{
public:
	SpellItem**			spells;		//00 - array of SpellItems
	void*				unk08;		//08
	void*				unk0C;		//10
	UInt32				numSpells;	//18 - spells array length
};


// 10
class ATTESSpellList : public BaseFormComponent
{
public:
	ATSpellListEntries	* unk08;	// 08
};




/******************************************/
//		 custom definitions:

// crit effect tables:



class ATCritEffectTable
{
	std::string *tableName;
	UInt8 rollTable[6] = { 20, 45, 70, 90, 100, 255 };
	SpellItem *critSpells[6];

public:
	SpellItem* GetCritEffectForRoll(UInt8 roll)
	{
		for (UInt8 i = 0; i < 6; i++) {
			if (roll <= rollTable[i])
				return critSpells[i];
		}
		return nullptr;
	}

	bool FillFromIni(const char* critTableName);
};



struct ATAmmoRecipeCompo
{
	BGSComponent *compo;
	UInt32 compoCount;
};


// used similarly to an omod's DamageType modifiers + stores dt name for menus
struct ATDamageType
{
	const char* damageTypeName;
	BGSDamageType *damageType;
	UInt32 iDamageAdd;
	float fDamageMult;
};



struct ATFiremode
{
	std::string *firemodeID;
	std::string *firemodeName;

	tArray<BGSKeyword*> keywordsAdd;
	tArray<BGSKeyword*> keywordsRemove;

	float fRecoilMult, fCoFMult, fWearMult;
};


struct ATWeapVarMod
{
	BGSKeyword* modKW = nullptr;
	float modMultAdd = 0.0;
};


// ammo subtypes - FMJ, JHP, etc.
struct ATAmmoType
{
	// HUD name
	std::string *AmmoName;
	
	float fDamageMult = 1.0, fRangeMult = 1.0, fOoRMult = 1.0;
	float fCritDmgMult = 1.0, fCritChanceMult = 1.0, fCritFailMult = 1.0, fWearMult = 1.0;
	float fCapacityMult = 1.0;

	// aimmodel manipulation
	float fRecoilMult = 1.0, fCoFMult = 1.0;

	UInt32 iArmorPenetration = 0, iNumProjectiles = 1;

	// projectile variations for different attachments
	UInt8 projectileStd = 0, projectileSup = 0, projectileExp = 0, projectileBrk = 0;

	// sound detection levels (0=loud,1=normal,2=silent,3=very loud) - standard, suppressed, legendary explosive
	UInt32 iSoundLevel = 0, iSoundLevelSup = 1, iSoundLevelExp = 3;

	// hit effect (0=normal,1=dismember,2=explode,3=no dismember/explode)
	UInt32 iHitEffect = 0;

	// critical effect table to use
	UInt32 iCritEffect = 0;


	// ammo item
	TESAmmo *ammoForm = nullptr;

	// standard/explosive impactdataset
	BGSImpactDataSet *projImpactData = nullptr, *projImpactDataExp = nullptr;

	// additional damage types
	tArray<ATDamageType> damageTypes;
	// additional enchantments
	tArray<EnchantmentItem*> enchantments;
	// additional keywords
	tArray<BGSKeyword*> keywords;

};


// holds the stats and ammo type list for a caliber
class ATCaliber
{
	float fCapacityMult;
	UInt32 iArmorPenetration;
	
	float fCritFailMult;
	float fWearMult;
	
	tArray<ATDamageType> damageTypes;
	tArray<BGSKeyword*> keywords;

	tArray<BGSProjectile*> projectiles;
	tArray<EnchantmentItem*> critEffectTables;

public:
	std::string *caliberName;
	BGSKeyword *caliberKW;

	TESForm *caliberMod;

	tArray<ATAmmoType> ammoTypes;

	TESObjectMISC *casingItem;


	// defined in ATConfig.cpp
	bool FillFromIni(const char* caliberFileName);
	
	// defined in ATWeaponRef.cpp
	UInt32 ProcessInstanceData(TESObjectWEAP::InstanceData *instanceData, int ammoTypeIndex);

};



class ATWeapon
{

public:
	UInt32 weaponFormID;

	tArray<ATWeapVarMod> MaxCNDMods;
	tArray<ATWeapVarMod> WearMods;

	UInt32 MaxCNDBase;
	UInt32 HUDIconIndex, HUDIconIndexAlt;


	// defined in ATConfig.cpp
	bool FillFromIni(const char* weaponFileName, const char* weaponID);

	// defined in ATWeaponRef.cpp:
	float GetMaxCNDMult(TESObjectWEAP::InstanceData *instanceData);
	float GetWearMult(TESObjectWEAP::InstanceData *instanceData);

	void ProcessInstanceData(TESObjectWEAP::InstanceData *instanceData);
};



// misc shared forms and text
class ATSharedData
{
public:
	bool bNPCsUseAmmo = false;
	bool bNPCsUseAmmo_Thrown = false;
	bool bNPCsUseAmmo_Turret = false;

	bool bUseSingleFireAutoSounds = false;

	bool bDisableRecoilSpringForce = false;

	bool bTwoShotLegendaryTweak = false;

	bool bEnableSkillRequirements = false;
	float fSkillReq_MaxWeight = 30.0;
	float fSkillReq_MinReq = 2.0;
	float fSkillReq_MaxReq = 12.0;

	// frequently used kws and avs:
	
	BGSKeyword *suppressorKW;
	BGSKeyword *muzBrakeKW;
	BGSKeyword *compensatorKW;
	BGSKeyword *legendaryExplKW;
	BGSKeyword *legendaryTwoShotKW;

	BGSKeyword *soundKW_Automatic;
	BGSKeyword *soundKW_AutomaticDisable;
	BGSKeyword *soundKW_Silenced;
	BGSKeyword *soundKW_SilencedDisable;

	ActorValueInfo *ArmorPenetrationAV;
	ActorValueInfo *MaxConditionAV;
	ActorValueInfo *WearPerShotAV;
	ActorValueInfo *SkillReqAV;

	ActorValueInfo *HUDIconAV_Weapon;
	ActorValueInfo *HasInstanceAV;


	// weapon + caliber ID index for load order independence:

	tArray<UInt32> index_ATWeapons;
	tArray<BGSKeyword*> index_ATCalibersEquipped;

	// weapon + caliber data:

	tArray<ATWeapon*> g_ATWeapons;
	tArray<ATCaliber*> g_ATCalibers;



	UInt8 numProcessedWeaps;
};

#pragma once
#include "f4se/GameData.h"
#include "f4se/GameExtraData.h"
#include "f4se/GameFormComponents.h"
#include "f4se/GameObjects.h"
#include "f4se/GameReferences.h"
#include "f4se/GameRTTI.h"
#include "f4se/GameTypes.h"
#include "f4se/PapyrusNativeFunctions.h"
#include "f4se/PapyrusVM.h"
#include "f4se/PapyrusScaleformAdapter.h"
#include "f4se/ScaleformLoader.h"
#include "f4se/CustomMenu.h"
#include <string>
#include <vector>
#include <set>
#include <memory>
#include <algorithm>
#include "include/RNG.h"

#include "GlobalSettings.h"



#define SCRIPTNAME_Globals "AT:Globals"
#define SCRIPTNAME_MenuCrafting "AT:UI:CraftingMenu"


struct StaticFunctionTag;
class VirtualMachine;


/** native equivalent of HasKeyword
		credit: shavkacagarikia (https://github.com/shavkacagarikia/ExtraItemInfo) **/
typedef bool(*_IKeywordFormBase_HasKeyword)(IKeywordFormBase* keywordFormBase, BGSKeyword* keyword, UInt32 unk3);

template <typename T>
T GetVirtualFunction(void* baseObject, int vtblIndex) {
	uintptr_t* vtbl = reinterpret_cast<uintptr_t**>(baseObject)[0];
	return reinterpret_cast<T>(vtbl[vtblIndex]);
}


// -- Keyword modifiers for damage types, actor values, hud icons
struct ATKWValueMod
{
	float			fValueAdd =				0.0;
	float			fValueMultAdd =			0.0;
	BGSKeyword		* reqKW = nullptr;
};

struct ActorValueShortName
{
	UInt32 avID = 0;
	BSFixedString avName = "";
};



// additional stat effects from a weapon mod
struct ATWeaponModStats
{
	bool				bUseEffect =			false;
	BGSKeyword *		checkKW =				nullptr;	// if not null, keyword to check for
	float				fArmorPenetrationAdd =	0.0;		// Armor Penetration % to add
	float				fTargetArmorMult =		1.0;		// Target DR/DT Multiplier
	float				fWearMult =				1.0;		// Wear per shot multiplier
	float				fCritMult =				1.0;		// Critical % multiplier
	float				fCritRollAdd =			0.0;		// value added to critical effect table rolls
	float				fCritFailMult =			1.0;		// Critical Failure % multiplier
	float				fCritFailRollAdd =		0.0;		// value added to critical failure table rolls
	float				fChargePerShotMult =	1.0;		// ammo consumption per shot multiplier
	float				fMaxConditionMultAdd =	0.0;		// max condition modifier
};


// Weapon caliber mod slot
class ATCaliber
{
public:
	// projectiles/impactDataSets to switch to depending on keywords
	struct ProjectileOverride
	{
		BGSProjectile		* projectile =			nullptr;	// projectile override
		BGSImpactDataSet	* impactData =			nullptr;	// ImpactDataSet
		std::vector<BGSKeyword*> projectileKWs;					// required keyword ids
	};

	// ammotype data
	struct AmmoType
	{
		TESForm				* ammoItem =			nullptr;
		BGSMod::Attachment::Mod * ammoMod =			nullptr;
		TESObjectMISC		* casingItem =			nullptr;	// casing scrap item
		TESObjectMISC		* droppedCasingItem =	nullptr;	// dummy dropped casing item
		float				fCasingDropChance =		0.0;		// chance of dropping casings (final value depends on craftingPerk)
		std::string			sAmmoName =				"";
		ATWeaponModStats	modEffects;
	};

	BGSKeyword			* kwCaliberID =			nullptr;	// caliber ID keyword - unique to this caliber's ammo mods
	BGSPerk				* perkCraftingSkill =	nullptr;	// perk associated with crafting/collecting casings for this caliber

	UInt32				iReloadType =			-1;			// -1=none, 0=mag, 1=cell, 2=revolver, 3=boltaction
	UInt32				iReloadAnimIndex =		-1;

	std::string			sCaliberName =			"";

	std::vector<AmmoType> ammoTypes;						// supported ammo types
	std::vector<ProjectileOverride> projectiles;			// projectile/impact overrides + kws
};


// -- Firemodes
struct ATFiremode
{
	BGSMod::Attachment::Mod * firemodeMod =		nullptr;
	BGSKeyword			* recipeKW =			nullptr;	// weapon workbench recipe keyword
	BGSKeyword			* requiredKW =			nullptr;	// keyword required to swap to this firemode
	UInt32				iHudIcon =				-1;			// firemode icon index
	UInt32				iSwapAnimIndex =		-1;			// weapon animation index to play on swap
	UInt32				iCritEffectTableID =	0x0;
	std::string			sFiremodeName =			"";
	ATWeaponModStats	modEffects;
};


// -- generic weapon mod slots
class ATModSlot
{
public:
	// Swappable Mod Info
	struct SwappableMod
	{
		BGSMod::Attachment::Mod	* swapMod =			nullptr;	// mod to attach
		BGSKeyword			* requiredKW =			nullptr;	// required weapon keyword id
		TESForm				* requiredItem =		nullptr;	// required inventory item
		bool				bRequireModMisc =		false;		// require the owner to have the miscmod item
		std::string			sMenuName =				"";			// HUD/Menu name for this mod
		ATWeaponModStats	modEffects;
	};

	UInt32				iSwapAnimIndex =		-1;			// weapon animation index to play on swap
	bool				bShowInMenu =			true;
	bool				bSaveAmmoCount =		true;		// save weapon ammo count when swapping to this type of mod (for magazines, launchers, etc.)
	std::string			sSlotName =				"";			// HUD/Menu name for this slot

	std::vector<SwappableMod> swappableMods;				// list of mods supported by this slot
};


// -- Damaged mod slots
class ATDamagedModSlot
{
public:
	std::string			sSlotName =				"";			// HUD/Menu name for this slot
	bool				bRemoveAmmo =			false;		// true for magazines to remove equipped ammo when the mod is attached
	std::vector<BGSMod::Attachment::Mod*> damagedMods;		// list of mods supported by this slot
};


// -- Weapon Data
class ATWeapon
{
public:
	// Magazine data
	struct MagazineItem
	{
		TESObjectMISC		* magItem =				nullptr;	// magazine misc item
		BGSMod::Attachment::Mod * magMod =			nullptr;	// required mod
		UInt32				iCapacity =				0x0;		// TBD - magazine capacity
	};
	// weapon and holster armor
	struct HolsteredArmor
	{
		TESObjectARMO		* armorWeapon =			nullptr;	// sheathed weapon 'armor'
		TESObjectARMO		* armorHolster =		nullptr;	// holster armor
		std::vector<BGSKeyword*> requiredKWs;					// required keyword IDs
		std::string			sMenuName =				"";			// HUD/Menu name
	};
	// idle/action animation data
	struct WeaponAnim
	{
		UInt32				iAnimID =				-1;
		BGSAction			* animAction =			nullptr;	// action to try to play first
		TESForm				* animIdle_1P =			nullptr;	// 1st person idle to play if action fails
		TESForm				* animIdle_3P =			nullptr;	// 3rd person idle to play if action fails
		std::string			sAnimFinishedEvent =	"";			// event to listen for before swapping a mod
	};
	// critical failure effect
	struct CritFailure
	{
		int					iRollMax =				0;			// upper threshold for the crit roll
		SpellItem			* spellCritFailure =	nullptr;	// critical effect spell
		std::string			sMenuName =				"";
	};
	// hud icons
	struct HUDIconMod
	{
		UInt32				iIconIndex =			-1;
		BGSKeyword			* reqKW =				nullptr;
	};
	
	UInt32				iWeaponID =				0x0;		// unique ID (weapon formID)
	
	float				fBaseWearDivisor =		1.0;		// divisor for wear - should be the default caliber's base wear
	float				fBaseMaxCondition =		0.0;

	ActorValueInfo		* avRequiredSkill =		nullptr;	// required skill AV
	BGSPerk				* perkRepairSkill =		nullptr;	// perk that influences repair/casing chances

	std::string			sWeaponName =			"";			// short name to show in menus/HUD

	std::vector<CritFailure> criticalFailureTable;

	std::vector<WeaponAnim>	weaponAnims;					// list of possible animations for mod swaps, etc.

	std::vector<ATFiremode> firemodes;						// firing modes/melee attacks
	std::vector<ATCaliber> calibers;						// caliber data + ammo types
	std::vector<ATModSlot> modSlots;						// swappable mod slots
	std::vector<ATDamagedModSlot> damagedModSlots;			// damaged mod slots

	std::vector<MagazineItem> magazines;					// droppable magazine items
	std::vector<HolsteredArmor> holsters;					// holster/weapon armors

	std::vector<HUDIconMod> hudIcons;						// hud weapon icons

	std::vector<ATWeaponModStats> modEffects;


	// calculates and returns the instance's HUD weapon icon index
	UInt32 GetHUDIcon(TESObjectWEAP::InstanceData * instanceData);
	// calculates and returns the instance's HUD firemode icon index
	UInt32 GetFiremodeIcon(ExtraDataList * weapExtraData);
	
	bool GetCurMagItem(ExtraDataList * extraData, MagazineItem & magItem);
	bool GetCurWeapArmor(TESObjectWEAP::InstanceData * instanceData, ATWeapon::HolsteredArmor & newHolster);

	bool GetWeaponAnim(UInt32 iIndex, WeaponAnim & newAnim);

	bool GetCurrentCaliber(TESObjectWEAP::InstanceData * instanceData, ATCaliber & curCaliber);

	bool GetNextAmmoType(Actor * ownerActor, ATCaliber::AmmoType & nextAmmoType);
	bool GetCurrentAmmoType(TESObjectWEAP::InstanceData * instanceData, ExtraDataList * extraData, ATCaliber::AmmoType & ammoMod);

	bool GetNextFiremode(Actor * ownerActor, ATFiremode & nextFiremode);
	bool GetCurrentFiremode(ExtraDataList * extraData, ATFiremode & curFiremode);

	bool GetNextSwapMod(Actor * ownerActor, UInt32 iModSlot, ATModSlot::SwappableMod & nextSwapMod);
	bool GetCurrentSwapMod(ExtraDataList * extraData, UInt32 iModSlot, ATModSlot::SwappableMod & curSwapMod);

	// critical failure roll - returns the resulting crit failure spell
	SpellItem * GetCritFailureSpell(float fCritFailChance, UInt32 iRollMod = 0);

	// gets a random damaged mod (random slot if iSlot == -1)
	BGSMod::Attachment::Mod * GetDamagedMod(ExtraDataList * extraData, int iModSlot = -1);

	// updates the passed instance's projectile and impactData
	bool UpdateProjectile(TESObjectWEAP::InstanceData * instanceData);

	ATWeaponModStats GetModStats(ExtraDataList * extraData, TESObjectWEAP::InstanceData * instanceData);
};


class ATThrownWeapon
{
public:
	UInt32				iWeaponID =				0x0;		// unique ID (weapon formID)
	UInt32				iHudIcon =				0x0;

	UInt32				iCritEffectTableID =	0x0;
};


// stores base caliber data used to calculate weapon caliber slot stats and to display in menus
class ATCaliberData
{
public:
	struct AmmoTypesData
	{
		UInt32				iAmmoItemID =			0x0;
		UInt32				iAmmoModID =			0x0;
		UInt32				iCasingItemID =			0x0;
		UInt32				iDroppedCasingID =		0x0;
		float				fCritMult =				1.0;
		float				fCritFailMult =			1.0;
		float				fArmorPenetration =		0.0;
		float				fTargetArmorMult =		1.0;
		float				fWearMult =				1.0;
		float				fCasingDropChance =		0.0;
		std::string			sAmmoName =				"";
	};
	struct ProjectileData
	{
		UInt32				iProjectileID =			0x0;
		UInt32				iImpactDataSetID =		0x0;
		std::vector<UInt32> projectileKWIDs;
	};

	UInt32				iCaliberKWID =			0x0;		// caliber check keyword (formID = caliberID) - should be a weapon keyword added by the caliber objectmods
	UInt32				iRecipeKWID =			0x0;		// weapon workbench recipe keyword - ma_* keyword that enables recipes for this caliber's subtypes
	UInt32				iCraftingPerkID =		0x0;		// perk associated with crafting/collecting casings for this caliber
	float				fBaseCritMult =			1.0;		// caliber Critical Multiplier
	float				fBaseCritFailMult =		1.0;		// caliber Critical Failure Multiplier
	float				fBaseArmorPenetration = 0.0;
	float				fBaseTargetArmorMult =	1.0;
	float				fBaseWear =				1.0;
	std::string			sCaliberName =			"";

	std::vector<AmmoTypesData> ammoTypes;
	std::vector<ProjectileData> projectileOverrides;
};


class ATCraftingData
{
public:
	struct RecipeResult
	{
		TESForm * outputItem = nullptr;	// formType determines invItem or ObjectMod
		UInt32 iOutputCount = 0x0;
		float fTimeToCraft = 0.0;
	};
	struct RecipeComponent
	{
		TESForm * itemOrCompo = nullptr;	// formType determines compo or invItem
		UInt32 iCount = 0x0;
	};
	struct RecipeCondition
	{
		TESForm * reqForm = nullptr;	// formType determines Global, AV, Perk, InvItem, WorkbenchKW
		float fValue = 0.0;		// used as check value (float), item count (int), hasPerk/KW (bool)
		std::string sValueOp = "==";		// "==", "!=", ">=", "<=", ">", "<"
		std::string sCondOp = "AND";		// "AND", "OR"
	};

	struct CraftingRecipe
	{
		std::string sMenuName = "";
		std::string sMenuDesc = "";
		std::vector<RecipeResult> outputs;
		std::vector<RecipeComponent> inputs;
		std::vector<RecipeCondition> conditions;
	};

	struct CraftingCategory
	{
		std::string sMenuName = "";
		std::vector<CraftingRecipe> recipes;
	};

	std::vector<CraftingCategory> categories;


	bool GetRecipe(CraftingRecipe & recipeData, UInt32 iCategory = -1, UInt32 iRecipe = -1);
	bool CheckRecipeIngredients(CraftingRecipe & recipeData, TESObjectREFR * workbenchRef = nullptr);
	bool CheckRecipeConditions(CraftingRecipe & recipeData, TESObjectREFR * workbenchRef = nullptr);
};


class ATConfigData
{
public:
	// -- RNG
	static ATxoroshiro128p rng;

	// -- Data Lists:
	static std::vector<ActorValueShortName> AVNames;

	static std::vector<ATWeapon> ATWeapons;						// main weapons list
	static std::vector<ATThrownWeapon> ATThrownWeapons;			// thrown weapons list
	static std::vector<ATCaliberData> ATCaliberBase;			// calibers list

	// ---- Cached Forms/FormIDs:

	static ActorValueInfo		* AV_WeaponAmmoCount;			// Saved weapon ammo count
	static ActorValueInfo		* AV_WeaponAmmoCountSecondary;	// Saved weapon secondary ammo count (launcher)

	static ActorValueInfo		* AV_WeaponCNDMax;				// Weapon Condition (at_av_WpnCND)
	static ActorValueInfo		* AV_WeaponCNDPct;				// CND percentage
	static ActorValueInfo		* AV_WeaponCNDWear;				// Wear per Shot (at_av_WpnCND_Wear)

	static ActorValueInfo		* AV_WeaponDamageLevel;			// at_av_StatusEffect_DamagedWeapon

	static ActorValueInfo		* AV_CritFailChance;			// Critical Failure Chance (at_av_CritFailChance)
	static ActorValueInfo		* AV_CritFailRollMod;			// Critical Hit Chance (at_av_CritFailRollMod)
	static ActorValueInfo		* AV_CritFailMult;				// Critical Failure Multiplier (at_av_CritFailMult)

	static ActorValueInfo		* AV_RecoilMod;					// Recoil Modifier (at_av_RecoilMod)
	static ActorValueInfo		* AV_SkillReqDiff;				// Difference between Skill Requirement and the Actor's AV (at_av_WpnReqDiff)
	static ActorValueInfo		* AV_SkillModActive;			// AV added to weapons that have had their instances edited in memory

	static ActorValueInfo		* AV_StatusPoisoned;
	static ActorValueInfo		* AV_StatusBleeding;
	static ActorValueInfo		* AV_StatusRadiated;
	static ActorValueInfo		* AV_StatusBurning;
	static ActorValueInfo		* AV_StatusChilled;
	static ActorValueInfo		* AV_StatusIonized;
	static ActorValueInfo		* AV_StatusArmorDmg;
	static ActorValueInfo		* AV_StatusBlinded;

	static ActorValueInfo		* AV_ArmorPenetration;			// Armor Penetration
	static ActorValueInfo		* AV_ArmorPenetrationThrown;	// Armor Penetration for thrown weapons
	static ActorValueInfo		* AV_TargetArmorMult;			// Target dr/dt mult
	static ActorValueInfo		* AV_TargetArmorMultThrown;		// same for thrown weapons
	static ActorValueInfo		* AV_Luck;						// Luck

	static TESAmmo				* Ammo_Broken;					// ammo for broken guns
	static TESAmmo				* Ammo_Jammed;
	static TESAmmo				* Ammo_Overheated;

	static BGSMod::Attachment::Mod * Mod_NullMagazine;

};


// ---- game data interactions
namespace ATGameData
{
	int GetWeaponIndex(UInt32 formID);
	bool GetWeaponByID(UInt32 formID, ATWeapon & weaponData);
	bool GetEquippedWeapon(Actor * ownerActor, ATWeapon & weaponData);

	int GetCaliberDataIndex(UInt32 formID);
	bool GetCaliberDataByID(UInt32 formID, ATCaliberData & caliberData);

	TESForm * GetEquippedFormBySlot(Actor * ownerActor, UInt32 iEquipSlot);
	TBO_InstanceData * GetEquippedInstanceBySlot(Actor * ownerActor, UInt32 iEquipSlot);

	TESObjectWEAP * GetEquippedWeaponForm(Actor * ownerActor, UInt32 iItemType = 0);
	TESObjectARMO * GetEquippedArmorForm(Actor * ownerActor, UInt32 iEquipSlot = 0);
	
	ExtraDataList * GetEquippedWeaponExtraData(Actor * ownerActor, UInt32 iItemType = 0);
	TESObjectWEAP::InstanceData * GetEquippedWeaponInstance(Actor * ownerActor, UInt32 iItemType = 0);

	TESObjectWEAP::InstanceData * GetWeaponInstanceData(ExtraDataList * extraData);
	TESObjectARMO::InstanceData * GetArmorInstanceData(ExtraDataList * extraData);

	bool InstanceHasMod(ExtraDataList * extraData, BGSMod::Attachment::Mod * checkMod);
	bool EquippedItemAtSlotHasMod(BGSMod::Attachment::Mod * checkMod, Actor * ownerActor, UInt32 iEquipSlot);

	float GetInstanceHealth(ExtraDataList * extraData, bool bRandomIfNone = false);
	bool SetInstanceHealth(ExtraDataList * extraData, float fNewCND = 0.0);

	bool WeaponInstanceHasKeyword(TESObjectWEAP::InstanceData * instanceData, BGSKeyword * checkKW);
	bool ArmorInstanceHasKeyword(TESObjectARMO::InstanceData * instanceData, BGSKeyword * checkKW);

	bool SetWeaponInstanceAVModifier(TESObjectWEAP::InstanceData * instanceData, ActorValueInfo * avToAdd, UInt32 iAmount);
	UInt32 GetWeaponInstanceAVModifier(TESObjectWEAP::InstanceData * instanceData, ActorValueInfo * avToGet);

	int GetItemCount(Actor * ownerActor, UInt32 itemID);
	bool TargetHasKeyword(Actor * targetActor, BGSKeyword * checkKW);
	float CalcSkillRequirement(float fWeight);

	const char * GetAVShortName(UInt32 avFormID);
}


namespace ATHUD
{
	void RegisterMenus();
	bool RegisterScaleform(GFxMovieView * view, GFxValue * f4se_root);

	void ForceHUDWidgetsUpdate();
	void AddHUDMessage(BSFixedString newMessage);

}


namespace ATGlobals
{
	bool RegisterPapyrus(VirtualMachine * vm);
}


namespace ATConfigReader
{
	bool LoadGameData();
}
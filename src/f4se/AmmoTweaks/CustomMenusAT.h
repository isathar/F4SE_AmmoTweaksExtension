#pragma once
#include "ATShared.h"
#include "f4se_common/Relocation.h"
#include "HookUtil.h"



// stores all information needed for a single inventory item dislayed in the pip boy
struct ATMenuInvItem
{
	int iInvListIndex = -1;
	int iMenuListIndex = -1;

	std::string sItemName = "";	
	std::string sItemDescription = "";

	int iEquippedState = 0;		// 0=unequipped, 1=equipped

	int iItemCount = 0;			// item stack count
	int iValue = 0;				// caps value
	float fWeight = 0.0;		// weight

	float fCondition = -1.0;	// cnd percentage
	float fConditionMax = 0.0;	// max cnd percentage
	float fWear = 1.0;

	int iItemIcon = -1;			// item icon index
	int iFiremodeIcon = -1;		// firemode icon index
	
	std::string sAmmoName = "";	// equipped ammo name
	int iAmmoInvIndex = -1;		// index of the equipped ammo type in inventory list
	int iAmmoCount = 0;			// equipped ammo count
	int iAmmoCapacity = 0;		// equipped mag/cell capacity
	int iNumProjectiles = 1;	// projectiles per shot

	// damage types:
	int iDmg_Phys = 0;
	int iDmg_Energy = 0;
	int iDmg_Fire = 0;
	int iDmg_Cryo = 0;
	int iDmg_Shock = 0;
	int iDmg_Poison = 0;
	int iDmg_Rad = 0;

	int iArmorPenetration = 0;	// armor penetration for DT
	float fTargetArmorMult = 1.0;	// target dt/dr multiplier

	float fRange = 0;			// weapon effective range

	float fAccuracy = 0;		// accuracy score
	float fSpeed = 0;			// firing speed

	float fCritMult = 1.0;		// weapon critical multiplier
	float fCritFailMult = 1.0;	// weapon critical failure multiplier

	std::string sReqSkill = "";	// display name of the required skill
	int iSkillReq = 0;			// skill requirement value

	int iItemSlot = -1;			// equip slot
};


class ATMenuInventory
{
public:
	// sort comps - estimated item sorting algorithms (still off sometimes)

	// 0 - Name
	struct Comp_ATMenuInvItem_ByName {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 1 - Total Damage/DR
	struct Comp_ATMenuInvItem_ByDmg {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 2 - Rate of Fire/Speed
	struct Comp_ATMenuInvItem_ByRoF {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 3 - Range/Reach
	struct Comp_ATMenuInvItem_ByRange {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 4 - Accuracy
	struct Comp_ATMenuInvItem_ByAccuracy {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 5 - Value
	struct Comp_ATMenuInvItem_ByValue {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	// 6 - Weight
	struct Comp_ATMenuInvItem_ByWeight {
		bool operator()(const ATMenuInvItem & a, const ATMenuInvItem & b) const {
			// weight
			if ((int)(a.fWeight * 1000.0) != (int)(b.fWeight * 1000.0)) return (a.fWeight > b.fWeight);
			// weapon name
			int iNameCmpVal = a.sItemName.compare(b.sItemName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// value
			if (a.iValue != b.iValue) return (a.iValue > b.iValue);
			// range
			if (((int)(a.fRange * 1000.0) != (int)(b.fRange * 1000.0))) return (a.fRange > b.fRange);
			// speed
			if (((int)(a.fSpeed * 1000.0) != (int)(b.fSpeed * 1000.0))) return (a.fSpeed > b.fSpeed);
			// damage
			if (a.iDmg_Phys != b.iDmg_Phys) return (a.iDmg_Phys > b.iDmg_Phys);
			if (a.iDmg_Poison != b.iDmg_Poison) return (a.iDmg_Poison > b.iDmg_Poison);
			if (a.iDmg_Fire != b.iDmg_Fire) return (a.iDmg_Fire > b.iDmg_Fire);
			if (a.iDmg_Energy != b.iDmg_Energy) return (a.iDmg_Energy > b.iDmg_Energy);
			if (a.iDmg_Cryo != b.iDmg_Cryo) return (a.iDmg_Cryo > b.iDmg_Cryo);
			if (a.iDmg_Rad != b.iDmg_Rad) return (a.iDmg_Rad > b.iDmg_Rad);
			if (a.iDmg_Shock != b.iDmg_Shock) return (a.iDmg_Shock > b.iDmg_Shock);
			// accuracy
			if ((int)(a.fAccuracy * 1000.0) != (int)(b.fAccuracy * 1000.0)) return (a.fAccuracy > b.fAccuracy);
			// ammo name
			iNameCmpVal = a.sAmmoName.compare(b.sAmmoName.c_str());
			if (iNameCmpVal != 0) return (iNameCmpVal < 0);
			// equipped state
			if (a.iEquippedState != b.iEquippedState) return (a.iEquippedState < b.iEquippedState);
			// original index in inventory list
			return (a.iInvListIndex < b.iInvListIndex);
		}
	};

	//static std::vector<ATMenuInvItem> inventoryData;

	static std::vector<ATMenuInvItem> invData_Weapons;
	static std::vector<ATMenuInvItem> invData_Armors;

	static std::vector<ATMenuInvItem> invData_Ammo;

	static int iDistanceUnit;
	static int iLastItemCount;

	static int iEquippedWeapon;
	static int iEquippedGrenade;
	static std::vector<int> iEquippedArmors;

	static int iPipboyTab;
	static int iPipboyPage;
	static int iSortMode;
	static int iSelectedItem;


	static void SortInventoryList();
	static bool BuildInventoryList();

	static bool GetSelectedMenuItemData();
	static bool UpdatePipboyStatus();

	static bool RepairItemByInvItemID(UInt32 itemUID);

};


// ---- HUD Overlay
class ATHUDMenu : public GameMenuBase
{
public:
	ATHUDMenu();
	virtual ~ATHUDMenu();

	static const char * sMenuName;
	static bool bForceUpdateBaseVars;
	static bool bForceWeaponStatsUpdate;	// set true to force the weapon stats widget to update during the next cycle
	static bool bPipboyOpen;
	static bool bHideHUDWidgets;

	bool bHUDWidgetsHidden = false;
	// frame counters/update thresholds - prevent updating everything every 0.03s (ATHUDOverlay.swf = 30 fps):
	UInt32 iUpdateThreshold_CND = 30;		// frames between condition updates		~1s
	UInt32 iUpdateThreshold_StatusFX = 90;	// frames between status effect updates	~3s
	UInt32 iUpdateThreshold_HideWeapStats = 300; // frames before hiding hud widgets ~10s
	UInt32 iFrameCounter_CND = 0;
	UInt32 iFrameCounter_StatusFX = 0;
	UInt32 iCurCondition = -100;


	static void OpenMenu()
	{
		
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(BSFixedString(sMenuName), kMessage_Open);
	}

	static void CloseMenu()
	{
		CALL_MEMBER_FN((*g_uiMessageManager), SendUIMessage)(BSFixedString(sMenuName), kMessage_Close);
	}

	virtual void RegisterFunctions() override;
	virtual void DrawNextFrame(float unk0, void * unk1) override final;
	
	
	void UpdateBaseColor();
	void UpdateWidgetTS(UInt32 iIndex = -1, float fNewX = 0.0, float fNewY = 0.0, float fNewScale = 1.0);
	void InitializeWidgets();

	void SetWidgetVisible(UInt32 iIndex, bool bWidgetVisible);
	void SetHUDWidgetsVisible(bool bWidgetVisible);

	void AddHUDMessage(BSFixedString newMessage = "");

	void UpdateWidgetCNDPercent();
	void UpdateWeaponStatsWidget();
	void UpdateStatusEffects();

	void ResetWeaponMenu();

};


class ATGFxFunc_PipboyMenu_at_setSelectedItem : public GFxFunctionHandler
{
public:
	virtual void    Invoke(Args * args);
};
class ATGFxFunc_PipboyMenu_at_useSelectedItem : public GFxFunctionHandler
{
public:
	virtual void    Invoke(Args * args);
};
class ATGFxFunc_PipboyMenu_at_setSelectedMenu : public GFxFunctionHandler
{
public:
	virtual void    Invoke(Args * args);
};
class ATGFxFunc_PipboyMenu_at_setSortMode : public GFxFunctionHandler
{
public:
	virtual void    Invoke(Args * args);
};

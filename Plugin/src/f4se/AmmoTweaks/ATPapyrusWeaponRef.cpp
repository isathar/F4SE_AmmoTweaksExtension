#include "ATPapyrusWeaponRef.h"

#define SCRIPTNAME "AT:WeaponRefBase"



namespace ATWeaponRef
{
	// returns the passed weapon ref's current ammo
	TESAmmo* GetRefAmmo(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->ammo;
				
			}
		}
		return nullptr;
	}

	// returns the passed weapon ref's max. range
	float GetRefMaxRange(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->maxRange;
			}
		}
		return 0.0;
	}

	// used to calculate DT for damage threshold framework
	UInt32 GetRefAttackDamage(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				if (instanceData->baseDamage >= 0)
					return (UInt32)instanceData->baseDamage;
				else {

				}
			}
		}
		return 0;
	}
	
	// returns the passed weapon ref's weight
	float GetRefWeight(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->weight;
			}
		}
		return 0.0;
	}

	// returns the passed weapon ref's required skill/attribute AV
	ActorValueInfo* GetRefReqSkill(VMRefOrInventoryObj *thisObj)
	{
		if (thisObj) {
			TESObjectWEAP::InstanceData *instanceData = ATUtilities::GetWeapRefInstanceData(thisObj);
			if (instanceData) {
				return instanceData->skill;
			}
		}
		return nullptr;
	}
	
}


bool ATWeaponRef::RegisterPapyrus(VirtualMachine* vm)
{
	ATWeaponRef::RegisterFuncs(vm);
	return true;
}


void ATWeaponRef::RegisterFuncs(VirtualMachine* vm)
{
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, TESAmmo*>("GetRefAmmo", SCRIPTNAME, ATWeaponRef::GetRefAmmo, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, float>("GetRefMaxRange", SCRIPTNAME, ATWeaponRef::GetRefMaxRange, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, UInt32>("GetRefAttackDamage", SCRIPTNAME, ATWeaponRef::GetRefAttackDamage, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, float>("GetRefWeight", SCRIPTNAME, ATWeaponRef::GetRefWeight, vm));
	vm->RegisterFunction(
		new NativeFunction0 <VMRefOrInventoryObj, ActorValueInfo*>("GetRefReqSkill", SCRIPTNAME, ATWeaponRef::GetRefReqSkill, vm));
	
	_MESSAGE("Registered native functions for %s", SCRIPTNAME);
}

#include "f4se/GameExtraData.h"
#include "f4se/GameRTTI.h"
#include "f4se/PapyrusNativeFunctions.h"

#include "ATInstanceData.h"


namespace ATInstanceData 
{
	#define SCRIPTNAME "AmmoTweaks:ATInstanceData"

	DECLARE_STRUCT(Owner, "InstanceData");




	TESObjectWEAP::InstanceData * GetWeapInstanceData(TESForm *form, UInt32 iSlotIndex)
	{
		ExtraDataList * extraDataList = nullptr;
		TBO_InstanceData * instanceData = nullptr;
		TESObjectWEAP::InstanceData * weapInstanceData = nullptr;

		// Passed a reference directly, just get the instance data directly
		TESObjectREFR * refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR);
		if (refr)
			extraDataList = refr->extraDataList;

		if (extraDataList) {
			BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_InstanceData);
			if (extraData) {
				ExtraInstanceData * objectModData = DYNAMIC_CAST(extraData, BSExtraData, ExtraInstanceData);
				if (objectModData) {
					weapInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(objectModData->instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
					if (weapInstanceData)
						return weapInstanceData;
				}
			}
			else {
				TESBoundObject * boundObject = DYNAMIC_CAST(form, TESForm, TESBoundObject);

				if (boundObject) {
					instanceData = boundObject->CloneInstanceData(nullptr);
					if (instanceData) {
						ExtraInstanceData * objectModData = ExtraInstanceData::Create(form, instanceData);
						if (objectModData) {
							extraDataList->Add(ExtraDataType::kExtraData_InstanceData, objectModData);
							weapInstanceData = (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
							if (weapInstanceData)
								return weapInstanceData;
						}
					}
				}
			}
		}

		TESObjectWEAP * weapon = DYNAMIC_CAST(form, TESForm, TESObjectWEAP);
		if (weapon) {
			weapInstanceData = &weapon->weapData;
			if (weapInstanceData)
				return &weapon->weapData;
		}

		Actor * actor = DYNAMIC_CAST(form, TESForm, Actor);
		if (actor) {
			if (iSlotIndex < 0)
				return nullptr;

			// Not a weapon
			if (iSlotIndex != 41)
				return nullptr;

			ActorEquipData * equipData = actor->equipData;
			if (!equipData)
				return nullptr;

			// Make sure there is an item in this slot
			auto item = equipData->slots[iSlotIndex].item;
			if (!item)
				return nullptr;

			weapInstanceData = (TESObjectWEAP::Data*)Runtime_DynamicCast(equipData->slots[iSlotIndex].instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__Data);
			if (weapInstanceData)
				return weapInstanceData;
		}

		return nullptr;
	}



	TESObjectWEAP::InstanceData * GetWeaponInstanceData(Owner* thisInstance)
	{
		if (!thisInstance || thisInstance->IsNone())
			return nullptr;

		TESForm * form = nullptr;
		UInt32 iSlotIndex = -1;

		if (!thisInstance->Get("owner", &form))
			return nullptr;

		// Must be an inventory object, or doesn't exist
		if (!form) {
			VMRefOrInventoryObj ref;
			if (!thisInstance->Get("owner", &ref))
				return nullptr;

			// Try to get ExtraDataList instead
			ExtraDataList * extraDataList = nullptr;
			ref.GetExtraData(&form, &extraDataList);
		}

		Actor * actor = DYNAMIC_CAST(form, TESForm, Actor);
		if (actor) {
			if (!thisInstance->Get("slotIndex", &iSlotIndex))
				return nullptr;
		}

		return GetWeapInstanceData(form, iSlotIndex);
	}


	//*******************************************************************************
	//						Papyrus functions:

	//*******************************************************************************
	// ImpactDataSet:
	TESForm* GetImpactDataSet(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->unk58) {
				_MESSAGE("Current ImpactDataSet: %08X", ((TESForm*)instanceData->unk58)->formID);
				return (TESForm*)instanceData->unk58;
			}
		}
		_MESSAGE("Error: Failed to get ImpactDataSet");
		return nullptr;
	}
	void SetImpactDataSet(StaticFunctionTag*, Owner thisInstance, TESForm* impactDataForm)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		BGSImpactDataSet * newImpactData = DYNAMIC_CAST(impactDataForm, TESForm, BGSImpactDataSet);
		if (instanceData && instanceData->unk58 && newImpactData) {
			_MESSAGE("New ImpactDataSet: %08X", impactDataForm->formID);
			instanceData->unk58 = newImpactData;
		}
	}

	//*******************************************************************************
	// ZoomData:
	TESForm* GetZoomData(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->zoomData) {
				_MESSAGE("Current ZoomData: %08X", ((TESForm*)instanceData->zoomData)->formID);
				return (TESForm*)instanceData->zoomData;
			}
		}
		_MESSAGE("Error: Failed to get ZoomData");
		return nullptr;
	}
	void SetZoomData(StaticFunctionTag*, Owner thisInstance, TESForm* zoomDataForm)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		BGSZoomData * newZoomData = DYNAMIC_CAST(zoomDataForm, TESForm, BGSZoomData);
		if (instanceData && instanceData->zoomData && newZoomData) {
			_MESSAGE("New ImpactDataSet: %08X", zoomDataForm->formID);
			instanceData->zoomData = newZoomData;
		}
	}

	//*******************************************************************************
	// AimModel:
	TESForm* GetAimModel(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->aimModel) {
				_MESSAGE("\nCurrent AimModel: %08X", ((TESForm*)instanceData->aimModel)->formID);
				return (TESForm*)instanceData->aimModel;
			} else
				_MESSAGE("\nCurrent AimModel: *edited*");
		} else
			_MESSAGE("Error: Failed to get AimModel");
		return nullptr;
	}
	void SetAimModel(StaticFunctionTag*, Owner thisInstance, TESForm* aimModelForm)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		BGSAimModel * newAimModel = DYNAMIC_CAST(aimModelForm, TESForm, BGSAimModel);
		if (instanceData && newAimModel) {
			_MESSAGE("New AimModel: %08X", aimModelForm->formID);
			instanceData->aimModel = newAimModel;
		}
		else
			_MESSAGE("Error: Failed to set AimModel");
	}
	void LogAimModelVars(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			ATAimModel *tempAT = (ATAimModel*)instanceData->aimModel;
			if (tempAT->formID > 0)
				_MESSAGE("\nAimModel: %08X - \n Cone of Fire -", tempAT->formID);
			else
				_MESSAGE("\nAimModel: *edited* - \n Cone of Fire -");

			_MESSAGE("    Angle:                  %f - %f", tempAT->CoF_MinAngle, tempAT->CoF_MaxAngle);
			_MESSAGE("    Increase per Shot:      %f \n    Decrease per Second:    %f \n    Decrease Delay ms:      %i", tempAT->CoF_IncrPerShot, tempAT->CoF_DecrPerSec, tempAT->CoF_DecrDelayMS);
			_MESSAGE("    Sneak Multiplier:       %f \n    Iron Sights Multiplier: %f", tempAT->CoF_SneakMult, tempAT->CoF_IronSightsMult);
			_MESSAGE(" Recoil - \n    Diminish Spring Force:  %f \n    Diminish Sights:        %f", tempAT->Rec_DimSpringForce, tempAT->Rec_DimSightsMult);
			_MESSAGE("    Amount per Shot:        %f - %f", tempAT->Rec_MinPerShot, tempAT->Rec_MaxPerShot);
			_MESSAGE("    Arc:                    %f \n    Arc Rotate:             %f", tempAT->Rec_ArcMaxDegrees, tempAT->Rec_ArcRotate);
			_MESSAGE("    Hip Multiplier:         %f \n    Base Stability:         %f", tempAT->Rec_HipMult, tempAT->BaseStability);
			_MESSAGE("    Shots/Runaway:          %i", tempAT->Rec_RunawayShots);
			
			_MESSAGE(
				"    Unknowns: \n          %08X \n          %08X \n          %08X \n\n          %08X \n          %08X", 
				tempAT->unk00, tempAT->unk04, tempAT->unk08, tempAT->unk10, tempAT->unk14
			);
		}
		else
			_MESSAGE("No instance or AimModel");
	}

	//--------------------------------------------------------------------------
	//   - Cone of Fire - 

	//          - Max Angle
	float GetConeOfFire_MaxAngle(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_MaxAngle;
		}
		return 0.0;
	}
	void SetConeOfFire_MaxAngle(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_MaxAngle = max(0.0, fNewVal);
		}
	}

	//          - Min Angle
	float GetConeOfFire_MinAngle(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_MinAngle;
		}
		return 0.0;
	}
	void SetConeOfFire_MinAngle(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_MinAngle = max(0.0, fNewVal);
		}
	}

	//			- Increase Per Shot
	float GetConeOfFire_IncreasePerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_IncrPerShot;
		}
		return 0.0;
	}
	void SetConeOfFire_IncreasePerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_IncrPerShot = max(0.0, fNewVal);
		}
	}

	//			- Decrease Per Second
	float GetConeOfFire_DecreasePerSec(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_DecrPerSec;
		}
		return 0.0;
	}
	void SetConeOfFire_DecreasePerSec(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_DecrPerSec = max(0.0, fNewVal);
		}
	}

	//			- Decrease Delay (ms)
	UInt32 GetConeOfFire_DecreaseDelayms(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_DecrDelayMS;
		}
		return 0;
	}
	void SetConeOfFire_DecreaseDelayms(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_DecrDelayMS = max(0, iNewVal);
		}
	}

	//			- Sneak Multiplier
	float GetConeOfFire_SneakMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_SneakMult;
		}
		return 0.0;
	}
	void SetConeOfFire_SneakMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_SneakMult = max(0.0, fNewVal);
		}
	}

	//			- Iron Sights Multiplier
	float GetConeOfFire_IronSightsMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->CoF_IronSightsMult;
		}
		return 0.0;
	}
	void SetConeOfFire_IronSightsMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			((ATAimModel*)instanceData->aimModel)->CoF_IronSightsMult = max(0.0, fNewVal);
		}
	}

	//--------------------------------------------------------------------------
	//   - Recoil - 

	//           - Max per Shot
	float GetRecoil_MaxPerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_MaxPerShot;
		}
		return 0.0;
	}
	void SetRecoil_MaxPerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_MaxPerShot = max(0.0, fNewVal);
	}

	//           - Min per Shot
	float GetRecoil_MinPerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_MinPerShot;
		}
		return 0.0;
	}
	void SetRecoil_MinPerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_MinPerShot = max(0.0, fNewVal);
	}

	//           - Arc Max Degrees
	float GetRecoil_ArcMaxDegrees(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_ArcMaxDegrees;
		}
		return 0.0;
	}
	void SetRecoil_ArcMaxDegrees(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_ArcMaxDegrees = max(0.0, fNewVal);
	}

	//           - Arc Rotate
	float GetRecoil_ArcRotate(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_ArcRotate;
		}
		return 0.0;
	}
	void SetRecoil_ArcRotate(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_ArcRotate = max(0.0, fNewVal);
	}

	//           - Base Stability
	float GetRecoil_BaseStability(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->BaseStability;
		}
		return 0.0;
	}
	void SetRecoil_BaseStability(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->BaseStability = max(0.0, fNewVal);
	}

	//           - Hip-fire Multiplier
	float GetRecoil_HipMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_HipMult;
		}
		return 0.0;
	}
	void SetRecoil_HipMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_HipMult = max(0.0, fNewVal);
	}

	//           - Diminish Spring Force
	float GetRecoil_DimSpringForce(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_DimSpringForce;
		}
		return 0.0;
	}
	void SetRecoil_DimSpringForce(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_DimSpringForce = max(0.0, fNewVal);
	}

	//           - Diminish Sights Multiplier
	float GetRecoil_DimSightsMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_DimSightsMult;
		}
		return 0.0;
	}
	void SetRecoil_DimSightsMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_DimSightsMult = max(0.0, fNewVal);
	}

	//           - Runaway Shots
	UInt32 GetRecoil_RunawayShots(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return ((ATAimModel*)instanceData->aimModel)->Rec_RunawayShots;
		}
		return 0;
	}
	void SetRecoil_RunawayShots(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			((ATAimModel*)instanceData->aimModel)->Rec_RunawayShots = max(0, iNewVal);
	}


	//*******************************************************************************
	// ActorValue Modifiers:

	void LogAVModifiers(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->modifiers) {
				if (instanceData->modifiers->count > 0) {
					TBO_InstanceData::ValueModifier tempValMod;
					_MESSAGE("\nActorValue Mods:");
					for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
						if (instanceData->modifiers->GetNthItem(i, tempValMod))
							_MESSAGE("        %s : %i", tempValMod.avInfo->avName, tempValMod.unk08);
					}
				}
			}
		}
	}

	UInt32 GetAVModifier(StaticFunctionTag*, Owner thisInstance, ActorValueInfo *AVToGet)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		UInt32 iRetVal = 0;
		if (instanceData) {
			if (instanceData->modifiers) {
				if (instanceData->modifiers->count > 0) {
					TBO_InstanceData::ValueModifier tempValMod;
					_MESSAGE("    ActorValues -");
					for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
						if (instanceData->modifiers->GetNthItem(i, tempValMod))
							_MESSAGE("        %s -    %i", tempValMod.avInfo->avName, tempValMod.unk08);
					}
				}
			}
		}
		return iRetVal;
	}

	void SetAVModifier(StaticFunctionTag*, Owner thisInstance, ActorValueInfo *AVToSet, UInt32 iValue)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			TBO_InstanceData::ValueModifier tempValMod;
			tArray<TBO_InstanceData::ValueModifier> *modifiers = new tArray<TBO_InstanceData::ValueModifier>();

			if (instanceData->modifiers) {
				if (instanceData->modifiers->count > 0) {
					for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
						if (instanceData->modifiers->GetNthItem(i, tempValMod)) {
							if (tempValMod.avInfo->avName != AVToSet->avName)
								modifiers->Push(tempValMod);
						}
					}
				}
			}

			if (iValue > 0) {
				tempValMod.avInfo = AVToSet;
				tempValMod.unk08 = iValue;
				modifiers->Push(tempValMod);
			}
			instanceData->modifiers = modifiers;
		}
	}
	void AddAVModifier(StaticFunctionTag*, Owner thisInstance, ActorValueInfo *AVToAdd, UInt32 iValue)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			TBO_InstanceData::ValueModifier tempValMod;
			tArray<TBO_InstanceData::ValueModifier> *modifiers = new tArray<TBO_InstanceData::ValueModifier>();
			UInt32 iBaseVal = 0;

			if (instanceData->modifiers) {
				if (instanceData->modifiers->count > 0) {
					for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
						if (instanceData->modifiers->GetNthItem(i, tempValMod)) {
							if (tempValMod.avInfo->avName != AVToAdd->avName)
								modifiers->Push(tempValMod);
							else
								iBaseVal = tempValMod.unk08;
						}
					}
				}
			}

			UInt32 iFinalVal = iBaseVal + iValue;
			if (iFinalVal > 0) {
				tempValMod.avInfo = AVToAdd;
				tempValMod.unk08 = iFinalVal;
				modifiers->Push(tempValMod);
			}
			instanceData->modifiers = modifiers;

			_MESSAGE("AVMod %s set to %i", AVToAdd->avName, iValue);
		}
	}
	void ModAVModifier(StaticFunctionTag*, Owner thisInstance, ActorValueInfo *AVToMod, float fValue)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			TBO_InstanceData::ValueModifier tempValMod;
			tArray<TBO_InstanceData::ValueModifier> *modifiers = new tArray<TBO_InstanceData::ValueModifier>();
			UInt32 iBaseVal = 0;
			
			if (instanceData->modifiers) {
				if (instanceData->modifiers->count > 0) {
					for (UInt32 i = 0; i < instanceData->modifiers->count; i++) {
						if (instanceData->modifiers->GetNthItem(i, tempValMod)) {
							if (tempValMod.avInfo->avName != AVToMod->avName)
								modifiers->Push(tempValMod);
							else
								iBaseVal = tempValMod.unk08;
						}
					}
				}
			}

			UInt32 iFinalVal = iBaseVal + UInt32((float)iBaseVal * fValue);
			if (iFinalVal > 0) {
				tempValMod.avInfo = AVToMod;
				tempValMod.unk08 = iFinalVal;
				modifiers->Push(tempValMod);
			}
			instanceData->modifiers = modifiers;

			_MESSAGE("AVMod %s set to %i", AVToMod->avName, iFinalVal);
		}
	}
	

	//********************************************************************************
	// Keywords:

	bool HasWeapKeyword(StaticFunctionTag*, Owner thisInstance, BGSKeyword *kwToCheck)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->keywords) {
				if (instanceData->keywords->HasKeyword(kwToCheck)) {
					return true;
				}
			}
		}
		return false;
	}

	void AddWeapKeyword(StaticFunctionTag*, Owner thisInstance, BGSKeyword *kwToAdd)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			BGSKeywordForm * keywordForm = instanceData->keywords;
			if (keywordForm) {
				if (!keywordForm->HasKeyword(kwToAdd)) {
					keywordForm->numKeywords = keywordForm->numKeywords + 1;
					keywordForm->keywords[keywordForm->numKeywords - 1] = kwToAdd;
				}
			}
		}
	}
	void RemoveWeapKeyword(StaticFunctionTag*, Owner thisInstance, BGSKeyword *kwToRemove)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			BGSKeywordForm * keywordForm = instanceData->keywords;
			if (keywordForm) {
				UInt32 j = 0;
				UInt32 iNewCount = keywordForm->numKeywords - 1;
				BGSKeyword **kwds = (BGSKeyword**)Heap_Allocate(sizeof(BGSKeyword*) * iNewCount);
				
				for (UInt32 i = 0; i < keywordForm->numKeywords; i++) {
					BGSKeyword * kwd = keywordForm->keywords[i];
					if (kwd != kwToRemove) {
						kwds[j] = kwd;
						j += 1;
					}
				}

				if (keywordForm->keywords) {
					Heap_Free(keywordForm->keywords);
					keywordForm->keywords = nullptr;
					keywordForm->numKeywords = 0;
				}

				keywordForm->keywords = (BGSKeyword**)Heap_Allocate(sizeof(BGSKeyword*) * iNewCount);
				keywordForm->numKeywords = iNewCount;

				for (UInt32 i = 0; i < iNewCount; i++) {
					BGSKeyword * kwd = kwds[i];
					keywordForm->keywords[i] = kwd;
				}
			}
		}
	}


	//********************************************************************************
	// DamageTypes:

	void LogDamageTypes(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			if (instanceData->damageTypes) {
				if (instanceData->damageTypes->count > 0) {
					TBO_InstanceData::DamageTypes tempValMod;
					_MESSAGE("\nDamageTypes:");
					for (UInt32 i = 0; i < instanceData->damageTypes->count; i++) {
						if (instanceData->damageTypes->GetNthItem(i, tempValMod))
							_MESSAGE("        %08X : %i", tempValMod.damageType->formID, tempValMod.value);
					}
				}
			}
		}
	}


	//********************************************************************************
	// SoundLevel (0-4) 0=loud,1=normal,2=silent,3=very loud

	UInt32 GetSoundLevel(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			return instanceData->unk114;
		}
		return 0;
	}
	void SetSoundLevel(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData)
			instanceData->unk114 = (UInt32)min(max(0, iNewVal), 4);
	}


	//********************************************************************************
	// HitEffect (0-3) 0=normal,1=dismember,2=explode,3=no dismember/explode
	
	UInt32 GetHitEffect(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData) {
			return instanceData->unk118;
		}
		return 0;
	}
	void SetHitEffect(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData)
			instanceData->unk118 = (UInt32)min(max(0, iNewVal), 3);
	}
}



bool ATInstanceData::RegisterPapyrus(VirtualMachine * vm)
{
	RegisterFuncs(vm);
	_MESSAGE("Registered ATInstanceData native functions.");
	return true;
}


void ATInstanceData::RegisterFuncs(VirtualMachine* vm)
{
	// ImpactDataSet:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetImpactDataSet", SCRIPTNAME, ATInstanceData::GetImpactDataSet, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetImpactDataSet", SCRIPTNAME, ATInstanceData::SetImpactDataSet, vm));
	
	// ZoomData:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetZoomData", SCRIPTNAME, ATInstanceData::GetZoomData, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetZoomData", SCRIPTNAME, ATInstanceData::SetZoomData, vm));
	
	// Aim Model:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, Owner>("LogAimModelVars", SCRIPTNAME, ATInstanceData::LogAimModelVars, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetAimModel", SCRIPTNAME, ATInstanceData::GetAimModel, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetAimModel", SCRIPTNAME, ATInstanceData::SetAimModel, vm));

	//		AimModel - Cone of Fire:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_MaxAngle", SCRIPTNAME, ATInstanceData::GetConeOfFire_MaxAngle, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_MaxAngle", SCRIPTNAME, ATInstanceData::SetConeOfFire_MaxAngle, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_MinAngle", SCRIPTNAME, ATInstanceData::GetConeOfFire_MinAngle, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_MinAngle", SCRIPTNAME, ATInstanceData::SetConeOfFire_MinAngle, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_IncreasePerShot", SCRIPTNAME, ATInstanceData::GetConeOfFire_IncreasePerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_IncreasePerShot", SCRIPTNAME, ATInstanceData::SetConeOfFire_IncreasePerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_DecreasePerSec", SCRIPTNAME, ATInstanceData::GetConeOfFire_DecreasePerSec, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_DecreasePerSec", SCRIPTNAME, ATInstanceData::SetConeOfFire_DecreasePerSec, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetConeOfFire_DecreaseDelayms", SCRIPTNAME, ATInstanceData::GetConeOfFire_DecreaseDelayms, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetConeOfFire_DecreaseDelayms", SCRIPTNAME, ATInstanceData::SetConeOfFire_DecreaseDelayms, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_SneakMult", SCRIPTNAME, ATInstanceData::GetConeOfFire_SneakMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_SneakMult", SCRIPTNAME, ATInstanceData::SetConeOfFire_SneakMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_IronSightsMult", SCRIPTNAME, ATInstanceData::GetConeOfFire_IronSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_IronSightsMult", SCRIPTNAME, ATInstanceData::SetConeOfFire_IronSightsMult, vm));

	//		AimModel - Recoil:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_MaxPerShot", SCRIPTNAME, ATInstanceData::GetRecoil_MaxPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_MaxPerShot", SCRIPTNAME, ATInstanceData::SetRecoil_MaxPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_MinPerShot", SCRIPTNAME, ATInstanceData::GetRecoil_MinPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_MinPerShot", SCRIPTNAME, ATInstanceData::SetRecoil_MinPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_ArcMaxDegrees", SCRIPTNAME, ATInstanceData::GetRecoil_ArcMaxDegrees, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_ArcMaxDegrees", SCRIPTNAME, ATInstanceData::SetRecoil_ArcMaxDegrees, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_ArcRotate", SCRIPTNAME, ATInstanceData::GetRecoil_ArcRotate, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_ArcRotate", SCRIPTNAME, ATInstanceData::SetRecoil_ArcRotate, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_BaseStability", SCRIPTNAME, ATInstanceData::GetRecoil_BaseStability, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_BaseStability", SCRIPTNAME, ATInstanceData::SetRecoil_BaseStability, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_HipMult", SCRIPTNAME, ATInstanceData::GetRecoil_HipMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_HipMult", SCRIPTNAME, ATInstanceData::SetRecoil_HipMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_DimSpringForce", SCRIPTNAME, ATInstanceData::GetRecoil_DimSpringForce, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_DimSpringForce", SCRIPTNAME, ATInstanceData::SetRecoil_DimSpringForce, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_DimSightsMult", SCRIPTNAME, ATInstanceData::GetRecoil_DimSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_DimSightsMult", SCRIPTNAME, ATInstanceData::SetRecoil_DimSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetRecoil_RunawayShots", SCRIPTNAME, ATInstanceData::GetRecoil_RunawayShots, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetRecoil_RunawayShots", SCRIPTNAME, ATInstanceData::SetRecoil_RunawayShots, vm));
	
	// ActorValue Modifiers:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, Owner>("LogAVModifiers", SCRIPTNAME, ATInstanceData::LogAVModifiers, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, UInt32, Owner, ActorValueInfo*>("GetAVModifier", SCRIPTNAME, ATInstanceData::GetAVModifier, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, void, Owner, ActorValueInfo*, UInt32>("SetAVModifier", SCRIPTNAME, ATInstanceData::SetAVModifier, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, void, Owner, ActorValueInfo*, UInt32>("AddAVModifier", SCRIPTNAME, ATInstanceData::AddAVModifier, vm));
	vm->RegisterFunction(
		new NativeFunction3 <StaticFunctionTag, void, Owner, ActorValueInfo*, float>("ModAVModifier", SCRIPTNAME, ATInstanceData::ModAVModifier, vm));
	
	// Keywords:
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, bool, Owner, BGSKeyword*>("HasWeapKeyword", SCRIPTNAME, ATInstanceData::HasWeapKeyword, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, BGSKeyword*>("AddWeapKeyword", SCRIPTNAME, ATInstanceData::AddWeapKeyword, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, BGSKeyword*>("RemoveWeapKeyword", SCRIPTNAME, ATInstanceData::RemoveWeapKeyword, vm));

	// DamageTypes:
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, Owner>("LogDamageTypes", SCRIPTNAME, ATInstanceData::LogDamageTypes, vm));
	
	// SoundLevel
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetSoundLevel", SCRIPTNAME, ATInstanceData::GetSoundLevel, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetSoundLevel", SCRIPTNAME, ATInstanceData::SetSoundLevel, vm));
	
	// HitEffect
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetHitEffect", SCRIPTNAME, ATInstanceData::GetHitEffect, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetHitEffect", SCRIPTNAME, ATInstanceData::SetHitEffect, vm));


}

#include "../f4se/GameObjects.h"
#include "../f4se/GameReferences.h"
#include "../f4se/GameRTTI.h"
#include "../f4se/GameExtraData.h"

#include "../f4se/PapyrusVM.h"
#include "../f4se/PapyrusNativeFunctions.h"
#include "../f4se/PapyrusInstanceData.h"

#include "Config.h"
#include "PapyrusAT.h"


#define SCRIPT_NAME "AmmoTweaks:ATInstanceData"



bool PapyrusAmmoTweaks::RegisterPapyrus(VirtualMachine * vm)
{
	RegisterFuncs(vm);
	_MESSAGE("Registered Papyrus native functions.");
	return true;
}


namespace PapyrusAmmoTweaks {
	DECLARE_STRUCT(Owner, "InstanceData");
	DECLARE_STRUCT(WeaponStats, "AmmoTweaks:ATInstanceData");


	TBO_InstanceData * GetInstanceData(Owner* thisInstance)
	{
		if (!thisInstance || thisInstance->IsNone())
			return nullptr;
		
		TESForm * form = nullptr;
		ExtraDataList * extraDataList = nullptr;

		if (!thisInstance->Get("owner", &form))
			return nullptr;

		// Must be an inventory object, or doesn't exist
		if (!form) {
			VMRefOrInventoryObj ref;
			if (!thisInstance->Get("owner", &ref))
				return nullptr;

			// Try to get ExtraDataList instead
			ref.GetExtraData(&form, &extraDataList);
		}

		// Passed a reference directly, just get the instance data directly
		TESObjectREFR * refr = DYNAMIC_CAST(form, TESForm, TESObjectREFR);
		if (refr)
			extraDataList = refr->extraDataList;

		if (extraDataList) {
			BSExtraData * extraData = extraDataList->GetByType(ExtraDataType::kExtraData_InstanceData);
			if (extraData) {
				ExtraInstanceData * objectModData = DYNAMIC_CAST(extraData, BSExtraData, ExtraInstanceData);
				if (objectModData)
					return objectModData->instanceData;
			}
			else {
				TESBoundObject * boundObject = DYNAMIC_CAST(form, TESForm, TESBoundObject);
				TBO_InstanceData * instanceData = nullptr;
				if (boundObject) {
					instanceData = boundObject->CloneInstanceData(nullptr);
					if (instanceData) {
						ExtraInstanceData * objectModData = ExtraInstanceData::Create(form, instanceData);
						if (objectModData) {
							extraDataList->Add(ExtraDataType::kExtraData_InstanceData, objectModData);
							return instanceData;
						}
					}
				}
			}
		}

		TESObjectWEAP * weapon = DYNAMIC_CAST(form, TESForm, TESObjectWEAP);
		if (weapon) {
			return &weapon->weapData;
		}

		Actor * actor = DYNAMIC_CAST(form, TESForm, Actor);
		if (actor) {
			UInt32 iSlotIndex = 0;
			if (!thisInstance->Get("slotIndex", &iSlotIndex))
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

			return equipData->slots[iSlotIndex].instanceData;
		}

		return nullptr;
	}

	TESObjectWEAP::InstanceData * GetWeaponInstanceData(Owner* thisInstance)
	{
		TBO_InstanceData * instanceData = GetInstanceData(thisInstance);
		if (!instanceData)
			return nullptr;
		
		return (TESObjectWEAP::InstanceData*)Runtime_DynamicCast(instanceData, RTTI_TBO_InstanceData, RTTI_TESObjectWEAP__InstanceData);
	}


	//*******************************************************************************

	//*******************************************************************************
	// Exported functions:

	// papyrus-side version check
	UInt32 GetVersionCode(StaticFunctionTag* base) {
		return PLUGIN_VERSION;
	}


	//*******************************************************************************
	// ImpactDataSet:

	TESForm* GetImpactDataSet(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->unk58) {
			if (instanceData->unk58->formID > 0) {
				_MESSAGE("Current ImpactDataSet: %08X", instanceData->unk58->formID);
				return instanceData->unk58;
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
			_MESSAGE("New ImpactDataSet: %08X", newImpactData->formID);
			instanceData->unk58 = newImpactData;
		}
	}

	//*******************************************************************************
	// AimModel -
	TESForm* GetAimModel(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			if (instanceData->aimModel->formID > 0) {
				_MESSAGE("Current AimModel: %08X", instanceData->aimModel->formID);
				return instanceData->aimModel;
			} else
				_MESSAGE("Current AimModel: *edited*", instanceData->aimModel->formID);
		}
		_MESSAGE("Error: Failed to get AimModel");
		return nullptr;
	}
	void SetAimModel(StaticFunctionTag*, Owner thisInstance, TESForm* aimModelForm)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		BGSAimModel * newAimModel = DYNAMIC_CAST(aimModelForm, TESForm, BGSAimModel);
		if (instanceData && newAimModel) {
			_MESSAGE("New AimModel: %08X", newAimModel->formID);
			instanceData->aimModel = newAimModel;
		}
		else
			_MESSAGE("Error: Failed to set AimModel");
	}
	void LogAimModelVars(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			_MESSAGE("\nCurrent AimModel: %08X - \n Cone of Fire -", instanceData->aimModel->formID);
			_MESSAGE("    Min Angle:              %f \n    Max Angle:              %f", instanceData->aimModel->fCoF_MinAngle, instanceData->aimModel->fCoF_MaxAngle);
			_MESSAGE("    Increase per Shot:      %f \n    Decrease per Second:    %f \n    Decrease Delay ms:      %i", instanceData->aimModel->fCoF_IncrPerShot, instanceData->aimModel->fCoF_DecrPerSec, instanceData->aimModel->iCoF_DecrDelayMS);
			_MESSAGE("    Sneak Multiplier:       %f \n    Iron Sights Multiplier: %f", instanceData->aimModel->fCoF_SneakMult, instanceData->aimModel->fCoF_IronSightsMult);
			_MESSAGE(" Recoil - \n    Max per Shot:           %f \n    Min per Shot:           %f", instanceData->aimModel->fRec_MaxPerShot, instanceData->aimModel->fRec_MinPerShot);
			_MESSAGE("    Arc:                    %f \n    Arc Rotate:             %f", instanceData->aimModel->fRec_ArcMaxDegrees, instanceData->aimModel->fRec_ArcRotate);
			_MESSAGE("    Diminish Spring Force:  %f \n    Diminish Sights:        %f", instanceData->aimModel->fRec_DimSpringForce, instanceData->aimModel->fRec_DimSightsMult);
			_MESSAGE("    Hip Multiplier:         %f \n    Base Stability:         %f", instanceData->aimModel->fRec_HipMult, instanceData->aimModel->fBaseStability);
			_MESSAGE("    Runaway Shots:          %i", instanceData->aimModel->iRec_RunawayShots);
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
			return instanceData->aimModel->fCoF_MaxAngle;
		}
		return 0.0;
	}
	void SetConeOfFire_MaxAngle(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_MaxAngle = max(0.0, fNewVal);
		}
	}

	//          - Min Angle
	float GetConeOfFire_MinAngle(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fCoF_MinAngle;
		}
		return 0.0;
	}
	void SetConeOfFire_MinAngle(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_MinAngle = max(0.0, fNewVal);
		}
	}

	//			- Increase Per Shot
	float GetConeOfFire_IncreasePerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fCoF_IncrPerShot;
		}
		return 0.0;
	}
	void SetConeOfFire_IncreasePerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_IncrPerShot = max(0.0, fNewVal);
		}
	}

	//			- Decrease Per Second
	float GetConeOfFire_DecreasePerSec(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fCoF_DecrPerSec;
		}
		return 0.0;
	}
	void SetConeOfFire_DecreasePerSec(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_DecrPerSec = max(0.0, fNewVal);
		}
	}

	//			- Decrease Delay (ms)
	UInt32 GetConeOfFire_DecreaseDelayms(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->iCoF_DecrDelayMS;
		}
		return 0;
	}
	void SetConeOfFire_DecreaseDelayms(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->iCoF_DecrDelayMS = max(0, iNewVal);
		}
	}

	//			- Sneak Multiplier
	float GetConeOfFire_SneakMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fCoF_SneakMult;
		}
		return 0.0;
	}
	void SetConeOfFire_SneakMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_SneakMult = max(0.0, fNewVal);
		}
	}

	//			- Iron Sights Multiplier
	float GetConeOfFire_IronSightsMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fCoF_IronSightsMult;
		}
		return 0.0;
	}
	void SetConeOfFire_IronSightsMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			instanceData->aimModel->fCoF_IronSightsMult = max(0.0, fNewVal);
		}
	}

	//--------------------------------------------------------------------------
	//   - Recoil - 

	//           - Max per Shot
	float GetRecoil_MaxPerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_MaxPerShot;
		}
		return 0.0;
	}
	void SetRecoil_MaxPerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_MaxPerShot = max(0.0, fNewVal);
	}

	//           - Min per Shot
	float GetRecoil_MinPerShot(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_MinPerShot;
		}
		return 0.0;
	}
	void SetRecoil_MinPerShot(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_MinPerShot = max(0.0, fNewVal);
	}

	//           - Arc Max Degrees
	float GetRecoil_ArcMaxDegrees(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_ArcMaxDegrees;
		}
		return 0.0;
	}
	void SetRecoil_ArcMaxDegrees(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_ArcMaxDegrees = max(0.0, fNewVal);
	}

	//           - Arc Rotate
	float GetRecoil_ArcRotate(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_ArcRotate;
		}
		return 0.0;
	}
	void SetRecoil_ArcRotate(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_ArcRotate = max(0.0, fNewVal);
	}

	//           - Base Stability
	float GetRecoil_BaseStability(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fBaseStability;
		}
		return 0.0;
	}
	void SetRecoil_BaseStability(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fBaseStability = max(0.0, fNewVal);
	}

	//           - Hip-fire Multiplier
	float GetRecoil_HipMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_HipMult;
		}
		return 0.0;
	}
	void SetRecoil_HipMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_HipMult = max(0.0, fNewVal);
	}

	//           - Diminish Spring Force
	float GetRecoil_DimSpringForce(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_DimSpringForce;
		}
		return 0.0;
	}
	void SetRecoil_DimSpringForce(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_DimSpringForce = max(0.0, fNewVal);
	}

	//           - Diminish Sights Multiplier
	float GetRecoil_DimSightsMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->fRec_DimSightsMult;
		}
		return 0.0;
	}
	void SetRecoil_DimSightsMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->fRec_DimSightsMult = max(0.0, fNewVal);
	}

	//           - Runaway Shots
	UInt32 GetRecoil_RunawayShots(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel) {
			return instanceData->aimModel->iRec_RunawayShots;
		}
		return 0;
	}
	void SetRecoil_RunawayShots(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
			instanceData->aimModel->iRec_RunawayShots = max(0, iNewVal);
	}


	//------------------------------------------------------------------
	// Combo Functions

	// Mod Recoil by Percentage (like omod's mult. and add)
	void ModRecoil_Percent(StaticFunctionTag*, Owner thisInstance, float fPercent)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
		{
			BGSAimModel* aimModel = instanceData->aimModel;
			float fRecoilMin = aimModel->fRec_MinPerShot;
			float fRecoilMax = aimModel->fRec_MaxPerShot;
			float fHipMult = aimModel->fRec_HipMult;
			aimModel->fRec_MinPerShot = max(0.0, fRecoilMin + (fRecoilMin * fPercent));
			aimModel->fRec_MaxPerShot = max(0.0, fRecoilMax + (fRecoilMax * fPercent));
			aimModel->fRec_HipMult = max(0.0, fHipMult + (fHipMult * fPercent));
		}
	}

	// Mod Accuracy by Percentage (like omod's mult. and add)
	void ModAccuracy_Percent(StaticFunctionTag*, Owner thisInstance, float fPercent)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->aimModel)
		{
			BGSAimModel* aimModel = instanceData->aimModel;
			float fCoFMin = instanceData->aimModel->fCoF_MinAngle;
			float fCoFMax = instanceData->aimModel->fCoF_MaxAngle;
			aimModel->fCoF_MinAngle = max(0.0, fCoFMin + (fCoFMin * fPercent));
			aimModel->fCoF_MaxAngle = max(0.0, fCoFMax + (fCoFMax * fPercent));
		}
	}


	//*******************************************************************************
	// ZoomData:
	TESForm* GetZoomData(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			if (instanceData->zoomData->formID > 0) {
				_MESSAGE("Current ZoomData: %08X", instanceData->zoomData->formID);
				return instanceData->zoomData;
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
			_MESSAGE("New ZoomData: %08X", newZoomData->formID);
			instanceData->zoomData = newZoomData;
		}
	}
	void LogZoomDataVars(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			_MESSAGE("\nCurrent ZoomData: %08X -", instanceData->zoomData->formID);
			_MESSAGE("    FoV Multiplier:        %f", instanceData->zoomData->fovMultiplier);
			_MESSAGE("    HUD Overlay:           %i", instanceData->zoomData->overlayIndex);
			if (instanceData->zoomData->imageSpaceMod)
				_MESSAGE("    Image Space Mod:       %08X", instanceData->zoomData->imageSpaceMod->formID);
			else
				_MESSAGE("    Image Space Mod:       none");
			_MESSAGE("    Cam Offset:");
			_MESSAGE("              - X:         %f", instanceData->zoomData->camOffset_X);
			_MESSAGE("              - Y:         %f", instanceData->zoomData->camOffset_Y);
			_MESSAGE("              - Z:         %f", instanceData->zoomData->camOffset_Z);
		}
		else
			_MESSAGE("Error: Failed to get ZoomData");
	}

	//           - FoV Multiplier
	float GetZoomData_FoVMult(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			return instanceData->zoomData->fovMultiplier;
		}
		return 0.0;
	}
	void SetZoomData_FoVMult(StaticFunctionTag*, Owner thisInstance, float fNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData)
			instanceData->zoomData->fovMultiplier = max(0.0, fNewVal);
	}

	//           - HUD Overlay Index
	UInt32 GetZoomData_OverlayIndex(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			return instanceData->zoomData->overlayIndex;
		}
		return 0;
	}
	void SetZoomData_OverlayIndex(StaticFunctionTag*, Owner thisInstance, UInt32 iNewVal)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData)
			instanceData->zoomData->overlayIndex = max(0, iNewVal);
	}

	//			- ImageSpace Modifier
	TESForm* GetZoomData_ImageSpaceMod(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			if (instanceData->zoomData->imageSpaceMod->formID > 0) {
				_MESSAGE("Current ZoomData:ImageSpaceMod: %08X", instanceData->zoomData->imageSpaceMod->formID);
				return instanceData->zoomData->imageSpaceMod;
			}
		}
		_MESSAGE("Error: Failed to get ZoomData:ImageSpaceMod");
		return nullptr;
	}
	void SetZoomData_ImageSpaceMod(StaticFunctionTag*, Owner thisInstance, TESForm* imageSpaceForm)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		TESImageSpaceModifier * newZoomData = DYNAMIC_CAST(imageSpaceForm, TESForm, TESImageSpaceModifier);
		if (instanceData && instanceData->zoomData && newZoomData) {
			_MESSAGE("New ZoomData:ImageSpaceMod: %08X", newZoomData->formID);
			instanceData->zoomData->imageSpaceMod = newZoomData;
		}
	}

	//           - Camera Offset
	float GetZoomData_CamOffset_X(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			return instanceData->zoomData->camOffset_X;
		}
		return 0.0;
	}
	float GetZoomData_CamOffset_Y(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			return instanceData->zoomData->camOffset_Y;
		}
		return 0.0;
	}
	float GetZoomData_CamOffset_Z(StaticFunctionTag*, Owner thisInstance)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			return instanceData->zoomData->camOffset_Z;
		}
		return 0.0;
	}
	void SetZoomData_CamOffset(StaticFunctionTag*, Owner thisInstance, float fNewValX, float fNewValY, float fNewValZ)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && instanceData->zoomData) {
			instanceData->zoomData->camOffset_X = fNewValX;
			instanceData->zoomData->camOffset_Y = fNewValY;
			instanceData->zoomData->camOffset_Z = fNewValZ;
			
		}
	}


	//*******************************************************************************
	// Weapon Update

	void UpdateWeaponStats_Gun(StaticFunctionTag*, Owner thisInstance, WeaponStats* statsUpdate)
	{
		auto instanceData = GetWeaponInstanceData(&thisInstance);
		if (instanceData && statsUpdate) {
			float fDamageMult = 1.0;
			float fRangeMult = 1.0;
			float fRecoilMult = 1.0;
			float fCoFMult = 1.0;
			UInt32 iNumProjectiles = 1;
			TESAmmo* AmmoItem = nullptr;
			BGSProjectile* ProjectileOverride = nullptr;
			BGSAimModel* aimModel = instanceData->aimModel;

			if (statsUpdate->Get("AmmoItem", &AmmoItem)) {
				if (AmmoItem)
					instanceData->ammo = AmmoItem;
			}

			if (statsUpdate->Get("fDamageMult", &fDamageMult))
				instanceData->baseDamage = max(0, min((UInt32)(fDamageMult * instanceData->baseDamage), 0xFFFF));

			if (statsUpdate->Get("fRangeMult", &fRangeMult))
				instanceData->maxRange = max(0.0, fRangeMult * instanceData->maxRange);

			if (instanceData->firingData) {
				if (statsUpdate->Get("ProjectileOverride", &ProjectileOverride)) {
					if (ProjectileOverride)
						instanceData->firingData->projectileOverride = ProjectileOverride;
				}

				if (statsUpdate->Get("iNumProjectiles", &iNumProjectiles))
					instanceData->firingData->numProjectiles = iNumProjectiles;
			}

			if (aimModel) {
				if (statsUpdate->Get("fRecoilMult", &fRecoilMult)) {
					float fRecoilMin = aimModel->fRec_MinPerShot;
					float fRecoilMax = aimModel->fRec_MaxPerShot;
					float fHipMult = aimModel->fRec_HipMult;
					aimModel->fRec_MinPerShot = max(0.0, fRecoilMin + (fRecoilMin * fRecoilMult));
					aimModel->fRec_MaxPerShot = max(0.0, fRecoilMax + (fRecoilMax * fRecoilMult));
					aimModel->fRec_HipMult = max(0.0, fHipMult + (fHipMult * fRecoilMult));
				}

				if (statsUpdate->Get("fCoFMult", &fCoFMult)) {
					float fCoFMin = aimModel->fCoF_MinAngle;
					float fCoFMax = aimModel->fCoF_MaxAngle;
					aimModel->fCoF_MinAngle = max(0.0, fCoFMin + (fCoFMin * fCoFMult));
					aimModel->fCoF_MaxAngle = max(0.0, fCoFMax + (fCoFMax * fCoFMult));
				}
			}

		}
	}
	


}



void PapyrusAmmoTweaks::RegisterFuncs(VirtualMachine* vm) {
	// Get Version
	vm->RegisterFunction(
		new NativeFunction0 <StaticFunctionTag, UInt32>("GetVersionCode", SCRIPT_NAME, PapyrusAmmoTweaks::GetVersionCode, vm));
	
	// Get/Set Form Functions
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetImpactDataSet", SCRIPT_NAME, PapyrusAmmoTweaks::GetImpactDataSet, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetImpactDataSet", SCRIPT_NAME, PapyrusAmmoTweaks::SetImpactDataSet, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetAimModel", SCRIPT_NAME, PapyrusAmmoTweaks::GetAimModel, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetAimModel", SCRIPT_NAME, PapyrusAmmoTweaks::SetAimModel, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetZoomData", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetZoomData", SCRIPT_NAME, PapyrusAmmoTweaks::SetZoomData, vm));
	
	// Variable Log Functions
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, Owner>("LogAimModelVars", SCRIPT_NAME, PapyrusAmmoTweaks::LogAimModelVars, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, Owner>("LogZoomDataVars", SCRIPT_NAME, PapyrusAmmoTweaks::LogZoomDataVars, vm));

	// AimModel - Cone of Fire
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_MaxAngle", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_MaxAngle, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_MaxAngle", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_MaxAngle, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_MinAngle", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_MinAngle, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_MinAngle", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_MinAngle, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_IncreasePerShot", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_IncreasePerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_IncreasePerShot", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_IncreasePerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_DecreasePerSec", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_DecreasePerSec, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_DecreasePerSec", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_DecreasePerSec, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetConeOfFire_DecreaseDelayms", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_DecreaseDelayms, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetConeOfFire_DecreaseDelayms", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_DecreaseDelayms, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_SneakMult", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_SneakMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_SneakMult", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_SneakMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetConeOfFire_IronSightsMult", SCRIPT_NAME, PapyrusAmmoTweaks::GetConeOfFire_IronSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetConeOfFire_IronSightsMult", SCRIPT_NAME, PapyrusAmmoTweaks::SetConeOfFire_IronSightsMult, vm));

	// AimModel - Recoil
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_MaxPerShot", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_MaxPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_MaxPerShot", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_MaxPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_MinPerShot", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_MinPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_MinPerShot", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_MinPerShot, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_ArcMaxDegrees", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_ArcMaxDegrees, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_ArcMaxDegrees", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_ArcMaxDegrees, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_ArcRotate", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_ArcRotate, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_ArcRotate", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_ArcRotate, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_BaseStability", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_BaseStability, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_BaseStability", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_BaseStability, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_HipMult", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_HipMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_HipMult", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_HipMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_DimSpringForce", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_DimSpringForce, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_DimSpringForce", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_DimSpringForce, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetRecoil_DimSightsMult", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_DimSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetRecoil_DimSightsMult", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_DimSightsMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetRecoil_RunawayShots", SCRIPT_NAME, PapyrusAmmoTweaks::GetRecoil_RunawayShots, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetRecoil_RunawayShots", SCRIPT_NAME, PapyrusAmmoTweaks::SetRecoil_RunawayShots, vm));

	// AimModel - Recoil/CoF Combo
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("ModRecoil_Percent", SCRIPT_NAME, PapyrusAmmoTweaks::ModRecoil_Percent, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("ModAccuracy_Percent", SCRIPT_NAME, PapyrusAmmoTweaks::ModAccuracy_Percent, vm));

	// ZoomData
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetZoomData_FoVMult", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_FoVMult, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, float>("SetZoomData_FoVMult", SCRIPT_NAME, PapyrusAmmoTweaks::SetZoomData_FoVMult, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, UInt32, Owner>("GetZoomData_OverlayIndex", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_OverlayIndex, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, UInt32>("SetZoomData_OverlayIndex", SCRIPT_NAME, PapyrusAmmoTweaks::SetZoomData_OverlayIndex, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, TESForm*, Owner>("GetZoomData_ImageSpaceMod", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_ImageSpaceMod, vm));
	vm->RegisterFunction(
		new NativeFunction2 <StaticFunctionTag, void, Owner, TESForm*>("SetZoomData_ImageSpaceMod", SCRIPT_NAME, PapyrusAmmoTweaks::SetZoomData_ImageSpaceMod, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetZoomData_CamOffset_X", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_CamOffset_X, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetZoomData_CamOffset_Y", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_CamOffset_Y, vm));
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, float, Owner>("GetZoomData_CamOffset_Z", SCRIPT_NAME, PapyrusAmmoTweaks::GetZoomData_CamOffset_Z, vm));
	vm->RegisterFunction(
		new NativeFunction4 <StaticFunctionTag, void, Owner, float, float, float>("SetZoomData_CamOffset", SCRIPT_NAME, PapyrusAmmoTweaks::SetZoomData_CamOffset, vm));

	
}

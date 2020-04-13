#pragma once
#include "f4se/GameFormComponents.h"


/* (AimModel)
- affects spread, recoil and aiming stability
- somewhat safe usage: cast AimModel as ATAimModel, edit result
- any AimModel edits require affected weapons to be re-equipped to take effect */
class ATAimModel : public BaseFormComponent
{
public:
	virtual			~ATAimModel();

	/* not sure about these first few - they could be completely wrong, but the rest seems to work, so...
	- similar to TESForm's first few variables
	- using UInt32 to fill in the bytes before the AimModel variables start */

	UInt32				unk00;						//00
	UInt32				unk08;						//08
	UInt32				unk10;						//10
	UInt32				formID;						//18 - gets set to 0 when any values are edited by omods or plugins
	UInt32				unk20;						//20
	UInt32				unk28;						//28

	// CoF_ = spread/cone of fire, Rec_ = recoil:
	// - initializing everything to the CK's default values for AimModels

	float			CoF_MinAngle = 2.0000;			//30 - min. spread angle (crosshair size)
	float			CoF_MaxAngle = 8.0000;			//38 - max. spread angle
	float			CoF_IncrPerShot = 0.3000;		//40 - spread increase per shot
	float			CoF_DecrPerSec = 2.0000;		//48 - spread decrease per second (after delay)
	UInt32			CoF_DecrDelayMS = 60;			//50 - delay in ms before spread starts to decrease after firing
	float			CoF_SneakMult = 0.8000;			//58 - multiplier applied to spread while sneaking/crouched
	float			Rec_DimSpringForce = 125.0000;	//60 - amount of automatic aim correction after recoil
	float			Rec_DimSightsMult = 1.0000;		//68 - amount of automatic aim correction after recoil while aiming
	float			Rec_MaxPerShot = 2.0000;		//70 - max. amount of recoil per shot
	float			Rec_MinPerShot = 0.2500;		//78 - min. amount of recoil per shot
	float			Rec_HipMult = 1.0000;			//80 - multiplier applied to recoil while firing from the hip
	UInt32			Rec_RunawayShots = 10;			//88 - the number of shots before recoil becomes unbearable?
	float			Rec_ArcMaxDegrees = 60.0000;	//90 - max. difference from the base recoil angle per shot in degrees
	float			Rec_ArcRotate = 0.0000;			//98 - angle for the recoil direction (clock-wise from 12:00)
	float			CoF_IronSightsMult = 0.0000;	//100 - multiplier applied to spread while aiming without a scope
	float			BaseStability = 1.0000;			//108 - multiplier applied to the amount of camera movement while using a scope
}; // 110


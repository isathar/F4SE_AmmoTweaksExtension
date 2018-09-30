#pragma once
#include "../f4se/GameFormComponents.h"
#include "../f4se/GameForms.h"

struct StaticFunctionTag;
class VirtualMachine;


// this is probably not a clean way to do it, but seems to work so far...
// - allows BGSAimModel, BGSImpactDataSet, and BGSZoomData to be passed as forms
// - variable source: hex dump
// - no idea what to do about offset/size checks - the first one works, I guess


class BGSAimModel : public TESForm
{
public:
	enum { kTypeID = kFormType_AMDL };

	// DNAM - Data							byte offsets, hopefully
	// cof = cone of fire, rec = recoil
	float			fCoF_MinAngle;			//20 - start at 20 since TESForm ends here
	float			fCoF_MaxAngle;			//24
	float			fCoF_IncrPerShot;		//28
	float			fCoF_DecrPerSec;		//2C
	UInt32			iCoF_DecrDelayMS;		//30
	float			fCoF_SneakMult;			//34
	float			fRec_DimSpringForce;	//38
	float			fRec_DimSightsMult;		//3C
	float			fRec_MaxPerShot;		//40
	float			fRec_MinPerShot;		//44
	float			fRec_HipMult;			//48
	UInt32			iRec_RunawayShots;		//4C
	float			fRec_ArcMaxDegrees;		//50
	float			fRec_ArcRotate;			//54
	float			fCoF_IronSightsMult;	//58
	float			fBaseStability;		//5C - total = 96
};
STATIC_ASSERT(sizeof(BGSAimModel) == 0x60);


class BGSZoomData : public TESForm
{
public:
	enum { kTypeID = kFormType_ZOOM };

	// GNAM - Data
	float					fovMultiplier;	//20
	UInt32					overlayIndex;	//24
	TESImageSpaceModifier *	imageSpaceMod;	//28
	float					camOffset_X;	//30
	float					camOffset_Y;	//34
	float					camOffset_Z;	//38 - total = 3C?
};

// this seems to be all that's needed for these to work
// - need to define variables at some point
class BGSImpactDataSet : public TESForm
{
public:
	enum { kTypeID = kFormType_IPDS };
};

class TESImageSpaceModifier : public TESForm
{
public:
	enum { kTypeID = kFormType_IMAD };
};



namespace PapyrusAmmoTweaks
{
    bool RegisterPapyrus(VirtualMachine *vm);
    void RegisterFuncs(VirtualMachine* vm);
}


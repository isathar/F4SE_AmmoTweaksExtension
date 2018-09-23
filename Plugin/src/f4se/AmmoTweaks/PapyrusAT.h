#pragma once
#include "../f4se/GameFormComponents.h"
#include "../f4se/GameForms.h"

struct StaticFunctionTag;
class VirtualMachine;


// this is probably not a clean way to do it, but seems to work so far...
// - allows BGSAimModel, BGSImpactDataSet, and BGSZoomData to be passed as forms
// - variable source: hex dump
// - no idea what to do about offset/sanity checks

class BGSAimModel : public TESForm
{
public:
	enum { kTypeID = kFormType_AMDL };

	// DNAM - Data							byte offsets, hopefully
	// cof = cone of fire, rec = recoil
	float			fCoF_MinAngle;			//20 - start at 20 since TESForm ends here
	float			fCoF_MaxAngle;			//24
	float			fCoF_IncrPerShot;		//28
	float			fCoF_DecrPerSec;		//32
	UInt32			iCoF_DecrDelayMS;		//36
	float			fCoF_SneakMult;			//40
	float			fRec_DimSpringForce;	//44
	float			fRec_DimSightsMult;		//48
	float			fRec_MaxPerShot;		//52
	float			fRec_MinPerShot;		//56
	float			fRec_HipMult;			//60
	UInt32			iRec_RunawayShots;		//64
	float			fRec_ArcMaxDegrees;		//68
	float			fRec_ArcRotate;			//72
	float			fCoF_IronSightsMult;	//76
	float			fBaseStability;		//80 - so the total is 84 bytes?
};

class BGSZoomData : public TESForm
{
public:
	enum { kTypeID = kFormType_ZOOM };

	// GNAM - Data
	float					fovMultiplier;
	UInt32					overlayIndex;
	TESImageSpaceModifier *	imageSpaceMod;
	float					camOffset_X;
	float					camOffset_Y;
	float					camOffset_Z;
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


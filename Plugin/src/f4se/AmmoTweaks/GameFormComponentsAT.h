#pragma once
#include "../f4se/GameFormComponents.h"

// *cof* = cone of fire, *rec* = recoil
class BGSAimModel : public BaseFormComponent
{
public:
	virtual			~BGSAimModel();

	UInt32			unk00;					//00
	UInt32			unk04;					//04
	UInt32			unk08;					//08

	UInt32			formID;					//0C

	UInt32			unk10;					//10
	UInt32			unk14;					//14

	float			CoF_MinAngle;			//18
	float			CoF_MaxAngle;			//1C
	float			CoF_IncrPerShot;		//20
	float			CoF_DecrPerSec;			//24
	UInt32			CoF_DecrDelayMS;		//28
	float			CoF_SneakMult;			//2C
	float			Rec_DimSpringForce;		//30
	float			Rec_DimSightsMult;		//34
	float			Rec_MaxPerShot;			//38
	float			Rec_MinPerShot;			//3C
	float			Rec_HipMult;			//40
	UInt32			Rec_RunawayShots;		//44
	float			Rec_ArcMaxDegrees;		//48
	float			Rec_ArcRotate;			//4C
	float			CoF_IronSightsMult;		//50
	float			BaseStability;			//54
};

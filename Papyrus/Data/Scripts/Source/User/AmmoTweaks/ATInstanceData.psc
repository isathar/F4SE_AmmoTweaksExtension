scriptname AmmoTweaks:ATInstanceData native hidden
{adds additional functions to use with weapon instances}

import InstanceData


;****************************  ImpactDataSet  **********************************

; returns the instance's ImpactDataSet
Form Function GetImpactDataSet(Owner akOwner) native global

; sets the instance's ImpactDataSet
; - takes effect immediately
Function SetImpactDataSet(Owner akOwner, Form newImpactData) native global


;****************************  ZoomData  **********************************

; returns the instance's ZoomData
Form Function GetZoomData(Owner akOwner) native global

; sets the instance's ZoomData
Function SetZoomData(Owner akOwner, Form newZoomData) native global


;****************************   AimModel   **********************************
; - all changes to the AimModel require the weapon to be unequipped to take effect
; - partially from the CK wiki:
;--------------------------------------------------------------------------------------
; Cone of Fire/Spread:
;    Min angle: 				min. spread angle (crosshair size)
;    Max angle:					max. spread angle (crosshair size)
;    Increase per shot: 		spread increase per shot
;    Decrease per sec: 			spread decrease per second (after delay)
;    Decrease delay MS: 		delay in ms after firing until spread starts to decrease
;    Sneak multiplier: 			multiplier applied to spread while sneaking/crouche.
;    Iron Sights Multiplier: 	multiplier applied to spread while aiming without a scope
;
; Recoil:
;    Arc: 						max. difference from the base recoil angle per shot in degrees
;								-	0 means recoil will always kick in the same direction. 
;								-	360 means recoil will kick in an entirely random direction.
;    Arc rotate: 				angle for the recoil direction (clock-wise from 12:00)
;    Diminish spring force:		amount of automatic aim correction after recoil
;    Diminish sights mult: 		amount of automatic aim correction after recoil while aiming
;    Max per shot: 				max. amount of recoil per shot
;    Min per shot: 				min. amount of recoil per shot
;    Hip mult: 					multiplier applied to recoil while firing from the hip
;    Runaway Recoil shots: 		the number of shots before recoil becomes unbearable?
;    Base Stability: 			multiplier applied to the amount of camera sway while using a scope
;--------------------------------------------------------------------------------------

; returns the instance's AimModel 
; - any changes to the aimmodel through objectmods, etc will cause this to return none
Form Function GetAimModel(Owner akOwner) native global

; sets the instance's AimModel 
;     Warning: also reverts any scripted modifiers/objectmod changes to AimModel
Function SetAimModel(Owner akOwner, Form newAimModel) native global

; prints the instance's AimModel variables to the log
Function LogAimModelVars(Owner akOwner) native global

; Cone of Fire:
; 		- Max Angle
float Function GetConeOfFire_MaxAngle(Owner akOwner) native global
Function SetConeOfFire_MaxAngle(Owner akOwner, float fNewVal) native global

; 		- Min Angle
float Function GetConeOfFire_MinAngle(Owner akOwner) native global
Function SetConeOfFire_MinAngle(Owner akOwner, float fNewVal) native global

; 		- Increase per Shot
float Function GetConeOfFire_IncreasePerShot(Owner akOwner) native global
Function SetConeOfFire_IncreasePerShot(Owner akOwner, float fNewVal) native global

; 		- Decrease per Second
float Function GetConeOfFire_DecreasePerSec(Owner akOwner) native global
Function SetConeOfFire_DecreasePerSec(Owner akOwner, float fNewVal) native global

; 		- Decrease Delay (in milliseconds)
int Function GetConeOfFire_DecreaseDelayms(Owner akOwner) native global
Function SetConeOfFire_DecreaseDelayms(Owner akOwner, int iNewVal) native global

; 		- Sneak Multiplier
float Function GetConeOfFire_SneakMult(Owner akOwner) native global
Function SetConeOfFire_SneakMult(Owner akOwner, float fNewVal) native global

; 		- Iron Sights Multiplier
float Function GetConeOfFire_IronSightsMult(Owner akOwner) native global
Function SetConeOfFire_IronSightsMult(Owner akOwner, float fNewVal) native global

;--------------------------------------------------------------------------------------
; Recoil:
; 		- Min per Shot
float Function GetRecoil_MinPerShot(Owner akOwner) native global
Function SetRecoil_MinPerShot(Owner akOwner, float fNewVal) native global

; 		- Max per Shot
float Function GetRecoil_MaxPerShot(Owner akOwner) native global
Function SetRecoil_MaxPerShot(Owner akOwner, float fNewVal) native global

; 		- Arc (Max. angle)
float Function GetRecoil_ArcMaxDegrees(Owner akOwner) native global
Function SetRecoil_ArcMaxDegrees(Owner akOwner, float fNewVal) native global

; 		- Arc Rotate (center)
float Function GetRecoil_ArcRotate(Owner akOwner) native global
Function SetRecoil_ArcRotate(Owner akOwner, float fNewVal) native global

; 		- Base Stability
float Function GetRecoil_BaseStability(Owner akOwner) native global
Function SetRecoil_BaseStability(Owner akOwner, float fNewVal) native global

; 		- Hip-fire multiplier
float Function GetRecoil_HipMult(Owner akOwner) native global
Function SetRecoil_HipMult(Owner akOwner, float fNewVal) native global

; 		- Diminish Spring Force
float Function GetRecoil_DimSpringForce(Owner akOwner) native global
Function SetRecoil_DimSpringForce(Owner akOwner, float fNewVal) native global

; 		- Diminish Sighted Multiplier
float Function GetRecoil_DimSightsMult(Owner akOwner) native global
Function SetRecoil_DimSightsMult(Owner akOwner, float fNewVal) native global

; 		- Runaway Shots
int Function GetRecoil_RunawayShots(Owner akOwner) native global
Function SetRecoil_RunawayShots(Owner akOwner, int iNewVal) native global


;****************************   ActorValueModifiers   **********************************
; - instance actorvalue modifiers (omod avActorValues)
; - these have to be non-zero integers
; - the plugin prevents the vanilla issue of ActorValue modifers freaking out when set to 0 by removing them from the instance

; prints all AV modifiers to the log
Function LogAVModifiers(Owner akOwner) native global

; returns the value of an existing modifier (or 0 if none)
int Function GetAVModifier(Owner akOwner, ActorValue AVToGet) native global

; sets an ActorValue modifier directly
Function SetAVModifier(Owner akOwner, ActorValue AVToSet, int iValue) native global
; adds iValue to an existing ActorValue modifier, adding the modifier if needed
Function AddAVModifier(Owner akOwner, ActorValue AVToAdd, int iValue) native global
; modifies an existing Actorvalue modifier by a percentage between 0.0-1.0
Function ModAVModifier(Owner akOwner, ActorValue AVToMod, float fValue) native global


;****************************   Keywords   **********************************
; - functions for the manipulation of single instance-added keywords

; checks if a keyword has been added by an omod
bool Function HasWeapKeyword(Owner akOwner, Keyword kwToCheck) native global

; adds a keyword to a weapon instance
Function AddWeapKeyword(Owner akOwner, Keyword kwToAdd) native global
; removes a keyword from a weapon instance
Function RemoveWeapKeyword(Owner akOwner, Keyword kwToRemove) native global


;****************************   DamageTypes   **********************************

; prints all DamageTypes to the log
Function LogDamageTypes(Owner akOwner) native global


;****************************   SoundLevel   **********************************
; 0=loud, 1=normal, 2=silent, 3=very loud

int Function GetSoundLevel(Owner akOwner) native global
Function SetSoundLevel(Owner akOwner, int iNewVal) native global


;****************************   HitEffect   **********************************
; 0=normal, 1=dismember only, 2=explode only, 3=no dismember/explode

int Function GetHitEffect(Owner akOwner) native global
Function SetHitEffect(Owner akOwner, int iNewVal) native global


scriptname AmmoTweaks:ATInstanceData native hidden
{adds additional functions to use with weapon instances}


; AimModel Info from the CK wiki (with edits):
;
; Cone of Fire
;    Min angle: Fully focused angle for cone of fire.
;    Max angle: Maximum angle for cone of fire.
;    Increase per shot: Cone of fire increases by this amount per shot.
;    Decrease per sec: Cone of fire decreases by this amount per second, after the delay.
;    Decrease delay MS: Delay before cone of fire begins to decrease. (In milliseconds)
;    Sneak multiplier: Cone of fire is multiplied by this amount while sneaking.
;    Iron Sights Multiplier: Cone of fire is multiplied by this amount while sighted.
;
; Recoil
;    Arc: Recoil will kick in a random direction within this arc. (In degrees)
;			0 means recoil will always kick in the same direction. 
;			360 means recoil will kick in an entirely random direction.
;    Arc rotate: What angle (in degrees, clockwise) is the center of the recoil arc. 
;			0 is straight up, 90 is straight right, 180 is straight down, etc...
;    Diminish spring force: How fast/much the crosshair returns to its original position after shooting.
;    Diminish sights mult: multiplier applied to diminish spring force while sighted?
;    Max per shot: Maximum amount of recoil per shot.
;    Min per shot: Minimum amount of recoil per shot.
;    Hip mult: Multiplier for recoil when not aiming down sights.
;    Runaway Recoil shots: number of shots before recoil becomes overwhelming?
;    Base Stability: Higher number reduces the scoped sway. Not sure why this is grouped with recoil.


; Structs
; - used to pass weapon stats that need to be modified to the plugin

; stores a weapon's 'clean' stats (without scripted modifiers)
Struct WeaponStats
	Form 	AmmoItem = 			none
	Form	ProjOverride = 		none
	Form	ImpactDataForm = 	none
	Form	NPCAmmoList = 		none
	
	float 	fDamage = 			1.0
	float 	fCritDmgMult =		1.0
	float 	fCritChanceMult =	1.0
	float 	fProjectileCount = 	1.0
	
	float 	fMaxRange = 		256.0
	float 	fMinRange = 		64.0
	
	float 	fRecoilMax = 		1.0
	float 	fRecoilMin = 		1.0
	float 	fCoFMax = 			1.0
	float 	fCoFMin = 			1.0
EndStruct

; stores scripted overrides and multipliers to a weapon's stats
Struct WeaponStatsMod
	Form 	AmmoItem = 			none
	Form	ProjOverride = 		none
	Form	ImpactDataForm = 	none
	Form	NPCAmmoList = 		none
	
	float 	fDamageMult = 		1.0
	float 	fCritDmgMult =		1.0
	float 	fCritChanceMult =	1.0
	float 	fProjectileMult = 	1.0
	
	float 	fRangeMult = 		1.0
	
	float 	fRecoilMult = 		1.0
	float 	fCoFMult = 			1.0
EndStruct



; returns the F4SE plugin's (AmmoTweaks.dll) version
int Function GetVersionCode() native global


; prints the instance's AimModel variables to the log
Function LogAimModelVars(InstanceData:Owner akOwner) native global

; prints the instance's ZoomData variables to the log
Function LogZoomDataVars(InstanceData:Owner akOwner) native global

Function LogWeaponStats_Gun(InstanceData:Owner akOwner) native global




;****************************  ImpactDataSet  **********************************

; returns the instance's ImpactDataSet
Form Function GetImpactDataSet(InstanceData:Owner akOwner) native global

; sets the instance's ImpactDataSet
; - takes effect immediately
Function SetImpactDataSet(InstanceData:Owner akOwner, Form newImpactData) native global


;****************************   AimModel   **********************************
; - all changes to the AimModel require the weapon to be unequipped to take effect

; returns the instance's AimModel 
; - any changes to the aimmodel through objectmods, etc will cause this to return none
Form Function GetAimModel(InstanceData:Owner akOwner) native global

; sets the instance's AimModel 
;     Warning: also reverts any scripted modifiers/objectmod changes to AimModel
Function SetAimModel(InstanceData:Owner akOwner, Form newAimModel) native global


;--------------------------------------------------------------------------------------
; Cone of Fire:
; 		- Max Angle
float Function GetConeOfFire_MaxAngle(InstanceData:Owner akOwner) native global
Function SetConeOfFire_MaxAngle(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Min Angle
float Function GetConeOfFire_MinAngle(InstanceData:Owner akOwner) native global
Function SetConeOfFire_MinAngle(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Increase per Shot
float Function GetConeOfFire_IncreasePerShot(InstanceData:Owner akOwner) native global
Function SetConeOfFire_IncreasePerShot(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Decrease per Second
float Function GetConeOfFire_DecreasePerSec(InstanceData:Owner akOwner) native global
Function SetConeOfFire_DecreasePerSec(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Decrease Delay (in milliseconds)
int Function GetConeOfFire_DecreaseDelayms(InstanceData:Owner akOwner) native global
Function SetConeOfFire_DecreaseDelayms(InstanceData:Owner akOwner, int iNewVal) native global

; 		- Sneak Multiplier
float Function GetConeOfFire_SneakMult(InstanceData:Owner akOwner) native global
Function SetConeOfFire_SneakMult(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Iron Sights Multiplier
float Function GetConeOfFire_IronSightsMult(InstanceData:Owner akOwner) native global
Function SetConeOfFire_IronSightsMult(InstanceData:Owner akOwner, float fNewVal) native global


;--------------------------------------------------------------------------------------
; Recoil:
; 		- Min per Shot
float Function GetRecoil_MinPerShot(InstanceData:Owner akOwner) native global
Function SetRecoil_MinPerShot(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Max per Shot
float Function GetRecoil_MaxPerShot(InstanceData:Owner akOwner) native global
Function SetRecoil_MaxPerShot(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Arc (Max. angle)
float Function GetRecoil_ArcMaxDegrees(InstanceData:Owner akOwner) native global
Function SetRecoil_ArcMaxDegrees(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Arc Rotate (center)
float Function GetRecoil_ArcRotate(InstanceData:Owner akOwner) native global
Function SetRecoil_ArcRotate(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Base Stability
float Function GetRecoil_BaseStability(InstanceData:Owner akOwner) native global
Function SetRecoil_BaseStability(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Hip-fire multiplier
float Function GetRecoil_HipMult(InstanceData:Owner akOwner) native global
Function SetRecoil_HipMult(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Diminish Spring Force
float Function GetRecoil_DimSpringForce(InstanceData:Owner akOwner) native global
Function SetRecoil_DimSpringForce(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Diminish Sighted Multiplier
float Function GetRecoil_DimSightsMult(InstanceData:Owner akOwner) native global
Function SetRecoil_DimSightsMult(InstanceData:Owner akOwner, float fNewVal) native global

; 		- Runaway Shots (?)
int Function GetRecoil_RunawayShots(InstanceData:Owner akOwner) native global
Function SetRecoil_RunawayShots(InstanceData:Owner akOwner, int iNewVal) native global


;--------------------------------------------------------------------------------------
; Combined AimModel Functions:
;		- Mod the instance's Recoil variables by a percentage (as 0.0-1.0)
Function ModRecoil_Percent(InstanceData:Owner akOwner, float fPercent) native global

; 		- Mod the instance's Cone of Fire variables by a percentage (as 0.0-1.0)
Function ModAccuracy_Percent(InstanceData:Owner akOwner, float fPercent) native global


;****************************   ZoomData   **********************************
; - all changes to ZoomData require the weapon to be unequipped to take effect

; returns the instance's ZoomData
; - any changes to the ZoomData will cause this to return none
Form Function GetZoomData(InstanceData:Owner akOwner) native global

; sets the instance's ZoomData 
;     Warning: also reverts any scripted modifiers to ZoomData
Function SetZoomData(InstanceData:Owner akOwner, Form newZoomData) native global

; 		- FoV Multiplier
float Function GetZoomData_FoVMult(InstanceData:Owner akOwner) native global
Function SetZoomData_FoVMult(InstanceData:Owner akOwner, float fNewVal) native global

; 		- HUD Overlay Index
int Function GetZoomData_OverlayIndex(InstanceData:Owner akOwner) native global
Function SetZoomData_OverlayIndex(InstanceData:Owner akOwner, int iNewVal) native global

; 		- ImageSpace Modifier
Form Function GetZoomData_ImageSpaceMod(InstanceData:Owner akOwner) native global
Function SetZoomData_ImageSpaceMod(InstanceData:Owner akOwner, Form newImageSpace) native global

; 		- Camera Offset
float Function GetZoomData_CamOffset_X(InstanceData:Owner akOwner) native global
float Function GetZoomData_CamOffset_Y(InstanceData:Owner akOwner) native global
float Function GetZoomData_CamOffset_Z(InstanceData:Owner akOwner) native global
Function SetZoomData_CamOffset(InstanceData:Owner akOwner, float fNewValX, float fNewValY, float fNewValZ) native global



WeaponStats Function GetWeaponBaseStats_Gun(InstanceData:Owner akOwner, WeaponStats CurStats) native global

Function UpdateWeaponStats_Gun(InstanceData:Owner akOwner, WeaponStatsMod ModStats, WeaponStats CurStats) native global

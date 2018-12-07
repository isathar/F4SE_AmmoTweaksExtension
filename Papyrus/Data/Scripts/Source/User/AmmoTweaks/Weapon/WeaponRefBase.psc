scriptname AmmoTweaks:Weapon:WeaponRefBase extends ObjectReference native
{base class for WeaponReference}


; contains a mod, its miscobject, keywords and anims for dynamic attachment
Struct WeaponModInfo
	ObjectMod 	WpnMod = none
	MiscObject	WpnModItem = none
	Keyword		RequiredKW = none
	Keyword		ExcludeKW = none
	
	Idle 		pSwapIdle_1P = none
	Idle 		pSwapIdle_3P = none
	String  	sSwapAnimEvent = "reloadStateExit"
EndStruct


Struct WpnVarMod
	Keyword RequiredKW = none
	{Keyword needed to apply these values}
	float fModAdd = 0.0
	{value to add}
	float fModMult = 0.0
	{value to multiply the final value by}
EndStruct



; native functions:

Function LogWeaponStats(int weapType) native

bool Function UpdateWeaponStats(Keyword curCaliber, int curAmmoType) native

Keyword Function GetCurCaliber() native
Form Function GetCurCaliberMod() native
int Function GetCurAmmoType(Keyword curCaliber) native

Ammo[] Function GetAmmoList() native
float Function GetWpnWeight() native
Keyword[] Function GetWpnKeywords() native

float Function CalcWearPerShot() native
int Function GetMaxCND() native

bool Function HasInstanceMods() native

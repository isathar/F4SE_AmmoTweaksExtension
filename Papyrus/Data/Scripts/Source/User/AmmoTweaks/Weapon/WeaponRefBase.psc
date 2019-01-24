scriptname AmmoTweaks:Weapon:WeaponRefBase extends ObjectReference native
{base class for WeaponReference}


; native functions:

Function LogWeaponStats(int weapType) native

bool Function UpdateWeaponStats(Keyword curCaliber, int curAmmoType) native

Keyword Function GetCurCaliber() native
ObjectMod Function GetCurCaliberMod() native
int Function GetCurAmmoType(Keyword curCaliber) native

Ammo[] Function GetAmmoList() native
Keyword[] Function GetWpnKeywords() native

float Function CalcWearPerShot() native
int Function GetMaxCND() native

bool Function HasInstanceMods() native

scriptname AT:Items:WeaponReferenceBase extends ObjectReference native hidden
{base class for Weapon ObjectReferences}


; debug
Function LogWeaponStats(int weaponType) native

; HUD display:
Ammo 		Function GetRefAmmo() 			native
float 		Function GetRefMaxRange() 		native
int 		Function GetRefAttackDamage() 	native
float 		Function GetRefWeight() 		native
ActorValue 	Function GetRefReqSkill() 		native
String 		Function GetWeaponName(string oldName) 		native

int Function UpdateRefRecoil(float fRecoilMult = 1.0) native

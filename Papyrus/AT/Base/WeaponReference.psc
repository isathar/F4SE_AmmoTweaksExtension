scriptname AT:Base:WeaponReference extends ObjectReference native hidden
{base class for Weapon ObjectReferences}


; HUD display:
Ammo 		Function GetRefAmmo() 			native
float 		Function GetRefMaxRange() 		native
int 		Function GetRefAttackDamage() 	native
float 		Function GetRefWeight() 		native
ActorValue 	Function GetRefReqSkill() 		native
String 		Function GetWeaponName(string oldName) 		native

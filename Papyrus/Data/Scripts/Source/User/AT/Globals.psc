Scriptname AT:Globals native
{shared functions}


; **** F4SE functions:

; -------- Ammo --------

; returns the dn_at_HasAmmo_X keyword for the equipped caliber
Keyword Function GetEquippedCaliber(Actor ownerActor, int iSlot = 41) native global

; returns the list of supported ammo subtype mods for a caliber
ObjectMod[] Function GetCaliberAmmoMods(Keyword caliberKW) native global

; returns the ammo subtype mod at the given index for a caliber
ObjectMod Function GetAmmoModAtIndex(Keyword caliberKW, int iAmmoIndex) native global

; returns the list of ammo items usable by a caliber
Ammo[] Function GetCaliberAmmoTypes(Keyword caliberKW) native global

; returns the equipped weapon's ammo index in it's caliber's list
int Function GetEquippedAmmoIndex(Keyword caliberKW, Actor ownerActor, int iSlot = 41) native global

; returns the equipped weapon's set ammo type
Ammo Function GetEquippedAmmo(Actor ownerActor, int iSlot = 41) native global

bool Function SetEquippedAmmo(Ammo newAmmo, Actor ownerActor, int iSlot = 41) native global

; returns the casing item at the given index for a caliber
MiscObject Function GetCurrentCasing(Keyword caliberKW, int iAmmoIndex) native global



; -------- Swappable Mods --------

; returns a list of the available weapon attachments for the given mod slot
ObjectMod[] Function GetModsForSlot(Weapon checkWeapon, int iModSlot) native global

; returns the weapon attachment at the given mod slot and index
ObjectMod Function GetModAtIndex(Weapon checkWeapon, int iModSlot, int iModIndex) native global

; returns the index of the equipped weapon attachment in the mod slot's list
int Function GetEquippedModIndex(Actor ownerActor, int iEquipSlot = 41, int iModSlot = 0) native global

Keyword Function GetSwapModRequiredKeyword(Actor ownerActor, int iEquipSlot = 41, int iModSlot = 0, int iModIndex = 0) native global
Keyword Function GetSwapModExcludeKeyword(Actor ownerActor, int iEquipSlot = 41, int iModSlot = 0, int iModIndex = 0) native global



; -------- Projectiles --------

; changes an equipped weapon's projectile, returns true if the projectile was found+updated
bool Function SetEquippedProjectile(Projectile newProjectile, Actor ownerActor, int iSlot = 41) native global

; picks a new projectile depending on the equipped weapon's caliber/ammo mod and muzzle/barrel keywords
bool Function UpdateEquippedProjectile(Keyword caliberKW, Actor ownerActor, int iSlot = 41) native global

Projectile Function GetEquippedProjectile(Actor ownerActor, int iSlot = 41) native global


; -------- Misc Weapon Stats --------

; DT Framework compatibility: DTF_ArmorPiercing = AttackDamage * at_av_ArmorPenetration * 0.01
int Function GetEquippedAttackDamage(Actor ownerActor, int iSlot = 41) native global

; crit charge bonus: used as the weapon critical multiplier
float Function GetEquippedCritChargeMult(Actor ownerActor, int iSlot = 41) native global

; used to calculate skill requirements:
; 	SkillReq = Max(MinReq, (Weight / MaxWeight) * MaxReq)
; 	def: MinReq = 2, MaxReq = 12, MaxWeight = 30
; 	WARS: MaxWeight = 65.0
float Function GetEquippedWeight(Actor ownerActor, int iSlot = 41) native global

; returns the required skill for the equipped weapon
ActorValue Function GetEquippedReqSkill(Actor ownerActor, int iSlot = 41) native global

; returns the equipped weapon's dropped magazine item
MiscObject Function GetEquippedMagItem(Actor ownerActor, int iSlot = 41) native global


int Function UpdateEquippedRecoil(Actor ownerActor, int iSlot = 41, float fRecoilMult = 1.0) native global

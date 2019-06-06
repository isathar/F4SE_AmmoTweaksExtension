Scriptname AT:Globals native hidden
{shared functions}


;-------- Hotkey functions:

Function Hotkey_SwitchModSlot() Global
	
EndFunction

; attaches the mod at iModIndex in slot iModType
Function Hotkey_SwapMod(int iModType=-1, int iModIndex=-1) Global
	Form tempForm = Game.GetFormFromFile(0x00083F, "AmmoTweaks.esm")
	ScriptObject managerQuest = tempForm.CastAs("AT:Managers:EventManager")
	var[] params = new var[2]
	params[0] = iModType
	params[1] = iModIndex
	managerQuest.CallFunction("SendPlayerSwapModEvent", params)
EndFunction

; clears a jammed weapon
Function Hotkey_UnJamWeapon() Global
	Form tempForm = Game.GetFormFromFile(0x00083F, "AmmoTweaks.esm")
	ScriptObject managerQuest = tempForm.CastAs("AT:Managers:EventManager")
	var[] params = new var[0]
	managerQuest.CallFunction("SendPlayerUnJamEvent", params)
EndFunction

; opens the weapon quickmenu
Function Hotkey_ShowWeaponMenu() Global
	Form tempForm = Game.GetFormFromFile(0x002FDA, "AmmoTweaks.esm")
	ScriptObject hudQuest = tempForm.CastAs("AT:HUD:Menu_HUDOverlay")
	var[] params = new var[0]
	hudQuest.CallFunction("ShowWeaponMenu", params)
EndFunction

; separate hotkey to throw grenades
Function Hotkey_ThrowGrenade() Global
	
EndFunction

; separate power attack hotkey
Function Hotkey_WeaponBash() Global
	
EndFunction



; ******** F4SE functions:

; -------- Weapon Stats:

; gets the equipped weapon's name (as set in ini)
String Function GetEquippedWeapName(Actor ownerActor, int iSlot = 41) native global

; DT Framework compatibility: 
; - formula: DTF_ArmorPiercing = max(0, AttackDamage * (at_av_ArmorPenetration - at_av_ArmorPenetrationNegative) * 0.01)
int Function GetEquippedAttackDamage(Actor ownerActor, int iSlot = 41) native global

; crit charge bonus: used as the weapon critical multiplier
float Function GetEquippedCritChargeMult(Actor ownerActor, int iSlot = 41) native global

; used to calculate skill requirements:
; - formula: SkillReq = Max(MinReq, (Weight / MaxWeight) * MaxReq)
; - defaults: MinReq = 2, MaxReq = 12, MaxWeight = 30; WARS: MaxWeight = 65.0
float Function GetEquippedWeight(Actor ownerActor, int iSlot = 41) native global

; returns the required skill for the equipped weapon
ActorValue Function GetEquippedReqSkill(Actor ownerActor, int iSlot = 41) native global

; returns the equipped weapon's current magazine item
MiscObject Function GetEquippedMagItem(Actor ownerActor, int iSlot = 41) native global


; -------- Caliber/Ammo Slot:

; returns the dn_at_HasAmmo_X keyword for the equipped caliber
Keyword Function GetEquippedCaliber(Actor ownerActor, int iSlot = 41) native global

; returns the equipped weapon's ammo index in it's caliber's list
int Function GetEquippedAmmoIndex(Keyword caliberKW, Actor ownerActor, int iSlot = 41) native global

; returns the name of the currently equipped ammo item
String Function GetEquippedAmmoName(Keyword caliberKW, Actor ownerActor, int iSlot = 41) native global

; returns the number of ammo types supported by a caliber
int Function GetNumAmmoTypesForCaliber(Weapon curWeapon, Keyword caliberKW) native global

; fills passed arrays with HUD display data for a caliber slot
bool Function GetHUDDataForCaliber(Actor ownerActor, Weapon curWeapon, Keyword caliberKW, String[] modNames, String[] modDescriptions, int[] modItemCounts, bool[] modAllowedList) native global

; returns the casing item at the given index for a caliber
MiscObject Function GetCurrentCasing(Weapon curWeapon, Keyword caliberKW, int iAmmoIndex) native global

; returns the list of ammo items usable by a caliber
Ammo[] Function GetCaliberAmmoTypes(Weapon curWeapon, Keyword caliberKW) native global

; returns the ammo subtype mod at the given index for a caliber
ObjectMod Function GetAmmoModAtIndex(Weapon curWeapon, Keyword caliberKW, int iAmmoIndex) native global


; -------- Swappable Mods:

; returns the number of active mod slots for a weapon
int Function GetNumSwapModSlots(Weapon checkWeapon) native global

; returns the number of mods supported by a weapon's specific mod slot
int Function GetNumModsForSlot(Weapon checkWeapon, int iModSlot) native global

; returns the name if a weapon's mod slot at the specified index
String Function GetModSlotName(Weapon checkWeapon, int iModSlot) native global

; returns the index of the equipped weapon attachment in the mod slot's list
int Function GetEquippedModIndex(Actor ownerActor, int iEquipSlot = 41, int iModSlot = 0) native global

; fills passed arrays with HUD display data for a mod slot
bool Function GetHUDDataForModSlot(Actor ownerActor, int iEquipSlot, int iModSlot, String[] modNames, String[] modDescriptions, int[] modItemCounts, bool[] modAllowedList) native global

; returns the weapon attachment at the given mod slot and index
ObjectMod Function GetModAtIndex(Weapon checkWeapon, int iModSlot, int iModIndex) native global

; returns a list containing the required item for each mod in a slot
Form[] Function GetModSlotRequiredItems(Weapon checkWeapon, int iModSlot) native global


; -------- Ammo:

; gets/sets the equipped weapon's set ammo type:
Ammo Function GetEquippedAmmo(Actor ownerActor, int iSlot = 41) native global
bool Function SetEquippedAmmo(Ammo newAmmo, Actor ownerActor, int iSlot = 41) native global


; -------- Projectiles:

; gets/sets an equipped weapon's projectile, returns true if the projectile was found+updated
Projectile Function GetEquippedProjectile(Actor ownerActor, int iSlot = 41) native global
bool Function SetEquippedProjectile(Projectile newProjectile, Actor ownerActor, int iSlot = 41) native global


; -------- Crit effect/failure tables:

; gets a random crit effect from the table linked to critTableKW, influenced by iRollMod
; - formula: iRoll = (rand() % 86) + iRollMod
Spell Function GetCritEffect(Keyword critTableKW, Actor targetActor, int iRollMod = 0) native global

; does the crit failure RNG check
; - formula:
;	- fCritMult = (CritFailMult - CritFailMultNeg) * (1.0 - WeaponCNDPercent)
; 	- chance = (((21 - Luck) / 21) * 5) * fCritMult)
; 		- if passed, gets a random crit failure effect from the table linked to checkWeapon, influenced by iRollMod and iLuck
; 		- formula: iRoll = ((rand() % 101) + iRollMod) - (7 * (iLuck - 7))
Spell Function GetCritFailure(Weapon checkWeapon, int iRollMod = 0, int iLuck = 0, float fCritFailMult = 1.0) native global

; returns a random damaged mod from damaged slot iModSlot or a random slot if iModSlot < 0
ObjectMod Function GetRandomDamagedMod(Actor ownerActor, int iEquipSlot = 41, int iModSlot = -1) native global


; -------- Holstered weapons:

; if !weapHolster: returns the sheathed weapon armor for a weapon
; if weapHolster: returns the holster for a weapon, if any
Armor Function GetHolsterArmor(Actor ownerActor, int iSlot = 41) native global


; ---- Weapon updates:

; picks a new projectile depending on the equipped weapon's caliber/ammo mod and muzzle/barrel keywords
bool Function UpdateEquippedProjectile(Keyword caliberKW, Actor ownerActor, int iSlot = 41) native global

; updates recoil based on the passed multiplier
int Function UpdateEquippedRecoil(Actor ownerActor, int iSlot = 41, float fRecoilMult = 1.0) native global


; ---- Animations:

; returns a weapon's idle animation to play while swapping ammo
Form Function GetAmmoSwapIdle_1P(Actor ownerActor, int iSlot = 41) native global
Form Function GetAmmoSwapIdle_3P(Actor ownerActor, int iSlot = 41) native global

; returns a weapon's animation event to listen for when swapping mods
String Function GetAmmoSwapAnimEvent(Actor ownerActor, int iSlot = 41) native global


; -------- debug/dev:

; dump unknown weapon data
bool Function LogEquippedWeaponUnknowns(Actor ownerActor, int iSlot = 41) native global

Function TestWeaponLog() global
	Actor tempActor = Game.GetPlayer()
	int iSlotNum = tempActor.GetEquippedItemType(0) + 32
	AT:Globals.LogEquippedWeaponUnknowns(tempActor, iSlotNum)
EndFunction


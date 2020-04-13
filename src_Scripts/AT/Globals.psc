Scriptname AT:Globals native hidden
{shared functions}


; used to get and store initial weapon stats on equip
struct WeaponEquipData
	Weapon		WeaponForm = 	none
	Armor 		HolsteredArmor = none
	int 		iEquipSlot = 	41
	bool 		bMeleeWeapon = 	false
	int 		iHUDIcon = 		-1
	int 		iFiremodeIcon = -1
	
	Keyword 	CaliberKW = 	none
	Ammo 		AmmoItem = 		none
	MiscObject 	CasingItem = 	none
	float 		fCasingChance = 0.0
	
	MiscObject 	MagazineItem = 	none
	int 		iReloadType = 	-1
	int 		iAmmoCapacity = 0
	
	Keyword		CritFailKW = 	none
endStruct


struct WeaponHolsteredData
	Weapon		WeaponForm = none
	Armor 		WeaponHolstered = none
	Armor 		HolsterArmor = none
endStruct


; mod swapping idle animations and event to listen for
struct ModSwapIdle
	Action actionSwapMod = 	none
	Idle idleSwapMod_1P =	none
	Idle idleSwapMod_3P =	none
	String sAnimFinishedEvent = ""
endStruct


struct MagazineItem
	Weapon MagWeapon = none
	
	MiscObject MagItem = none
	ObjectMod MagMod = none
	int iCapacity = 0
	
	Ammo LoadedAmmo = none
	int iAmmoCount = 0
	
	Form ScrapOutput = none
endStruct



struct WeaponModData
	ObjectMod swapMod = none
	
	MiscObject swapModMisc = none
	Form swapModReqItem = none
	
	float fSwapModWait = 0.0
	Action actionSwapMod = none
	Idle idleSwapMod_1P = none
	Idle idleSwapMod_3P = none
endStruct


; used by guns and magazine items
struct MagazineData
	int iReloadType = -1
	MiscObject MagItem = none
	
	ObjectMod MagMod = none
	MiscObject MagModMisc = none
	
	Ammo LoadedAmmo = none
	int iAmmoCapacity = 0
	int iAmmoCount = -1
	
	MiscObject AmmoCasing = none
	float fCasingChance = 0.0
	
	Form ScrapOutput = none
endStruct



; **************** Hotkey functions:

; attaches the mod at iModIndex in slot iModType
Function Hotkey_SwapMod(int iModType=-1, int iModIndex=-1) Global
	Form tempForm = Game.GetFormFromFile(0x00083F, "AmmoTweaks.esm")
	ScriptObject managerQuest = tempForm.CastAs("AT:EventManager")
	var[] params = new var[2]
	params[0] = iModType
	params[1] = iModIndex
	managerQuest.CallFunction("SendPlayerSwapModEvent", params)
EndFunction

; clears a jammed weapon
Function Hotkey_UnJamWeapon() Global
	Form tempForm = Game.GetFormFromFile(0x00083F, "AmmoTweaks.esm")
	ScriptObject managerQuest = tempForm.CastAs("AT:EventManager")
	var[] params = new var[0]
	managerQuest.CallFunction("SendPlayerUnJamEvent", params)
EndFunction

; opens the weapon quickmenu
Function Hotkey_ShowWeaponMenu() Global
	;Form tempForm = Game.GetFormFromFile(0x002FDA, "AmmoTweaks.esm")
	;ScriptObject hudQuest = tempForm.CastAs("AT:HUD:HUDWidget_WeaponMenu")
	;var[] params = new var[0]
	;hudQuest.CallFunction("ShowWeaponMenu", params)
EndFunction

; opens the weapon quickmenu
Function Hotkey_WpnMenuNav(int iKey = -1) Global
	;Form tempForm = Game.GetFormFromFile(0x002FDA, "AmmoTweaks.esm")
	;ScriptObject hudQuest = tempForm.CastAs("AT:HUD:HUDWidget_WeaponMenu")
	;var[] params = new var[1]
	;params[0] = iKey
	;hudQuest.CallFunction("MenuNav", params)
EndFunction

; separate hotkey to throw grenades
Function Hotkey_ThrowGrenade() Global
	
EndFunction

; separate power attack hotkey
Function Hotkey_WeaponBash() Global
	
EndFunction



; **************** F4SE native functions:

; initializes variables/avs on weapon equip
bool Function InitEquippedWeapon(Actor ownerActor) native global

; resets variables/avs on weapon unequip
bool Function ResetEquippedWeaponVars(Actor ownerActor) native global


; fills the weaponData struct with things like droppable magazine items, caliker and crit failure keywords
MagazineData Function GetEquippedWeaponMagazineData(Actor ownerActor) native global

WeaponHolsteredData Function GetEquippedWeaponHolsterData(Actor ownerActor) native global


; updates stat requirement difference, critical percentages + roll mods, saves ammo count and condition
bool Function UpdateEquippedWeaponVars(Actor ownerActor) native global

; subtracts CND, performs a critical failure roll, saves ammo count
Spell Function WeaponFired(Actor ownerActor) native global


; -------- Swappable Mods:

WeaponModData Function GetNextWeaponModData(Actor ownerActor, int iModSlot) native global
bool Function GetWeaponModDataAtIndex(Actor ownerActor, WeaponModData modData, int iModSlot = -1, int iModIndex = -1) native global
WeaponModData Function GetCurrentWeaponModData(Actor ownerActor, int iModSlot = -1) native global

; returns the index of the equipped weapon attachment in the mod slot's list
int Function GetWeaponModIndex(int iModSlot, Actor ownerActor) native global


; -------- Ammo:

Ammo function GetInstanceAmmo(Actor ownerActor) native global
Ammo Function GetEquippedAmmo(Actor ownerActor) native global
bool Function SetEquippedAmmo(Ammo newAmmo, Actor ownerActor) native global

; adds iAmount to the equipped weapon's ammo count, returns the difference
; - standard ammo is added to/removed from ownerActor's inventory, f-core/health-based ammo disappears
; - used for Lose Ammo and Jam crit failures, Ammo per Shot, lever-action fix
int Function LoadEquippedAmmo(Actor ownerActor, int iAmount = 0) native global

int Function GetEquippedAmmoCount(Actor ownerActor) native global

bool Function SetEquippedAmmoCount(Actor ownerActor, int iAmount = 0) native global


; -------- Projectiles:

; gets/sets an equipped weapon's projectile, returns true if the projectile was found+updated
Projectile Function GetEquippedProjectile(Actor ownerActor) native global
bool Function SetEquippedProjectile(Projectile newProjectile, Actor ownerActor) native global
bool Function ResetEquippedProjectile(Actor ownerActor) native global


; -------- Crit failures:

; Math
; - Critical Chance:
;		- CriticalChance = Luck * WeaponCNDPercentage * WeaponCritMult
;		- CriticalRollMod = (6 per Better Criticals Perk or Calibrated weapon mod)
; - Critical Failure Chance:
; 		- CritFailChance = (((21 - (Luck * WeaponCNDPercentage)) / 21) * 5) * (CritFailMult * 0.01))
; 		- CritFailRollMod = (7 * ((Luck * WeaponCNDPercentage) - 7))

; returns a random damaged mod from damaged slot iModSlot or a random slot if iModSlot < 0
ObjectMod Function GetRandomDamagedMod(Actor ownerActor, int iModSlot = -1) native global


; ---- CND
bool Function SetEquippedWeaponSavedCND(float fAmount, Actor ownerActor) native global
float Function GetEquippedWeaponSavedCND(Actor ownerActor) native global
bool Function AddEquippedWeaponSavedCND(float fAmount, Actor ownerActor) native global

bool Function CheckUnjam(Actor ownerActor) native global


; -------- Global settings

; syncs global settings to changes made to the MCM config file
Function UpdateGlobalSettings() native global


bool Function SendHUDOverlayMessage(String newMessage = "") native global



; -------- debug/dev:


; dump unknown weapon data
bool Function LogEquippedWeaponUnknowns(Actor ownerActor, int iEquipSlot = 41) native global

bool Function SetWeaponAutomatic(bool bAutomatic, Actor ownerActor, int iEquipSlot = 41) native global


Function TestWeaponLog() global
	Actor tempActor = Game.GetPlayer()
	int iSlotNum = tempActor.GetEquippedItemType(0) + 32
	AT:Globals.LogEquippedWeaponUnknowns(tempActor, iSlotNum)
EndFunction


Function TestFiremode() global
	Actor tempActor = Game.GetPlayer()
	int iSlotNum = tempActor.GetEquippedItemType(0) + 32
	SetWeaponAutomatic(true, tempActor, iSlotNum)
EndFunction


Scriptname AT:EventManager extends Quest
{custom events}



; update settings when MCM closes
Event OnQuestInit()
	RegisterForExternalEvent("OnMCMMenuClose", "OnMCMMenuClose")
EndEvent

Function OnMCMMenuClose(string modName)
	debug.notification("MCM menu closed: " + modName)
	if (modName == "AmmoTweaks")
		AT:Globals.UpdateGlobalSettings()
	endIf
EndFunction


; -------- Custom Events

; ---- Mod swapping:

; attaches newMod to weaponOwner's equipped weapon (if it's a WeaponRef)
CustomEvent WeaponAttachModEvent
Function SendWeaponAttachModEvent(Actor weaponOwner, ObjectMod newMod, bool bSaveAmmoCount=false)
	; attaching weapon mods in VATS mode can crash the game
	if (Game.GetCameraState() != 2)
		var[] args = New Var[3]
		args[0] = weaponOwner
		args[1] = newMod
		args[2] = bSaveAmmoCount
		self.SendCustomEvent("WeaponAttachModEvent", args)
	endIf
EndFunction


; changes the quick swap hotkey's focused mod slot
CustomEvent PlayerChangeModQuickSlot
Function SendPlayerChangeModQuickSlot(int iModType=-1)
	var[] args = New Var[1]
	args[0] = iModType
	self.SendCustomEvent("PlayerChangeModQuickSlot", args)
EndFunction


; initializes a mod swap event
CustomEvent PlayerSwapModEvent
Function SendPlayerSwapModEvent(int iModType=-1, int iModIndex=-1)
	if (Game.GetCameraState() != 2)
		var[] args = New Var[2]
		args[0] = iModType
		args[1] = iModIndex
		self.SendCustomEvent("PlayerSwapModEvent", args)
	endIf
EndFunction


CustomEvent PlayerUnJamEvent
Function SendPlayerUnJamEvent()
	if (Game.GetCameraState() != 2)
		var[] args = New Var[0]
		self.SendCustomEvent("PlayerUnJamEvent", args)
	endIf
EndFunction


; -------- HUD/Menu Events

; used to start HUD widgets
CustomEvent ATHUDWidgetsUpdateEvent
Function SendATHUDWidgetsUpdateEvent()
	var[] args = New Var[0]
	self.SendCustomEvent("ATHUDWidgetsUpdateEvent", args)
EndFunction


; used to update HUD overlay on equip, game load
CustomEvent UpdateHUDOverlayEvent
Function SendUpdateHUDOverlayEvent(bool bEquipped = true)
	var[] args = New Var[1]
	args[0] = bEquipped
	self.SendCustomEvent("UpdateHUDOverlayEvent", args)
EndFunction


; sends a message to be displayed in the HUD message box
CustomEvent CustomHUDMessageEvent
Function SendCustomHUDMessageEvent(string sMessage="")
	var[] args = New Var[1]
	args[0] = sMessage
	self.SendCustomEvent("CustomHUDMessageEvent", args)
EndFunction


; -------- Crafting:

; used to attach workbench upgrades to ammo workbenches
CustomEvent UpgradeWorkbenchEvent
Function SendUpgradeWorkbenchEvent(ObjectMod newMod, Keyword checkKW)
	var[] args = New Var[2]
	args[0] = newMod
	args[1] = checkKW
	self.SendCustomEvent("UpgradeWorkbenchEvent", args)
EndFunction



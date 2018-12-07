scriptname AmmoTweaks:Weapon:WeaponReference extends AmmoTweaks:Weapon:WeaponRefBase
{saves current condition and instance mods, handles mod attachment}


Group WeaponConfig
	int Property iWeapType = 0 auto const
	{0=gun, 1=melee, 2=thrown}
	
	WeaponModInfo[] Property pMuzzles = none auto const
	{swappable muzzles}
	WeaponModInfo[] Property pScopes = none auto const
	{swappable scopes}
	WeaponModInfo[] Property pBarrelAttachments = none auto const
	{swappable custom bayonets/grenade launchers, etc.}
	
	
	WeaponModInfo[] Property pDamagedMods1 = none auto const
	{damaged mods - group 1 (default: receiver/capacitor, blade)}
	WeaponModInfo[] Property pDamagedMods2 = none auto const
	{damaged mods - group 2 (default: barrel, melee grip)}
	WeaponModInfo[] Property pDamagedMods3 = none auto const
	{damaged mods - group 3 (default: gun grip/stock)}
	WeaponModInfo[] Property pDamagedMods4 = none auto const
	{damaged mods - group 4 (default: magazine)}
	
	
	GlobalVariable Property	at_glob_Setting_CND_MinStartingPercentage auto const mandatory
	{global for the minimum CND percentage any weapon can start with}
	
	
	Idle Property 		pAmmoSwitchIdle_1P 							auto const
	{first person animation to play when switching between ammo types}
	Idle Property 		pAmmoSwitchIdle_3P 							auto const
	{third person animation to play when switching between ammo types}
	String Property 	sAmmoSwitchAnimEvent = 	"reloadStateExit" 	auto Const
	{animation event to listen for before attaching the ammo type mod - reload by default}
	
	AmmoTweaks:ATEventManager Property at_AmmoTweaksEventManager auto const mandatory
	{link to the event manager}
	
EndGroup


; current CND values
float 		fCurCND = 				-1.0
float 		fMaxCND = 				1.0
float 		fCNDPerShot = 			1.0

Ammo[] 		CurAmmoItems = none

ObjectMod ResetStatsMod = none

; actor this weapon is equipped on
Actor CurrentHolder = none


Keyword CurCaliberKW = none
int iAmmoTypeIndex = -1

bool bInitialized = false



; initializes the weapon ref's InstanceData modifiers
Function InitInstance()
EndFunction

 ; re-initializes instance edits when the game is reloaded
Event Actor.OnPlayerLoadGame(Actor akSender)
	UpdateWeaponInstance()
EndEvent

; attaches a mod to this weapon reference
Function WeaponAttachMod(ObjectMod tempMod)
EndFunction



; instance + variable manipulation

float Function GetSavedWeaponCondition()
	return fCurCND
EndFunction

Function SetSavedWeaponCondition(float fNewCND)
	fCurCND = fNewCND
EndFunction


float Function GetSavedMaxCondition()
	return fMaxCND
EndFunction

float Function GetSavedCNDPerShot()
	return fCNDPerShot
EndFunction



; checks for new mods/keywords since the last time this weapon was equipped
;  + updates as needed
; slow, but required for the weapon to update properly
bool Function CheckWeaponInstance(bool bIsNew=false)
	bool bForceUpdate = true
	if !bIsNew
		bForceUpdate = !HasInstanceMods()
	endIf
	
	if bForceUpdate
		UpdateWeaponInstance()
		
		fCNDPerShot = CalcWearPerShot()
		
		fMaxCND = GetMaxCND() as float
		if bIsNew
			fCurCND = ( \
				fMaxCND * Utility.RandomFloat(at_glob_Setting_CND_MinStartingPercentage.GetValue(), 1.0) \
			)
		endIf
	endIf
	
	return bForceUpdate
EndFunction



Function ResetWeaponInstance()
	if UpdateWeaponStats(CurCaliberKW, iAmmoTypeIndex)
		debug.notification("Update succeeded")
	else
		debug.notification("Update failed")
	endIf
	
	fCNDPerShot = CalcWearPerShot()
	fMaxCND = GetMaxCND() as float
	
	if iAmmoTypeIndex > -1
		CurAmmoItems = GetAmmoList()
		debug.notification("Ammo " + iAmmoTypeIndex + " = " + CurAmmoItems[iAmmoTypeIndex].GetFormID())
	else
		CurAmmoItems = new Ammo[0]
		debug.notification("Ammo " + iAmmoTypeIndex + " = none")
	endIf
	LogWeaponStats(iWeapType)
EndFunction


Function UpdateWeaponInstance()
	CurCaliberKW = GetCurCaliber()
	if (iAmmoTypeIndex < 0)
		iAmmoTypeIndex = GetCurAmmoType(CurCaliberKW)
	endIf
	ResetStatsMod = GetCurCaliberMod() as ObjectMod
	if ResetStatsMod != none
		WeaponAttachMod(ResetStatsMod)
	endIf
EndFunction


Function UpdateInstance_Ammo(int iAmmoIndex)
	iAmmoTypeIndex = iAmmoIndex
EndFunction



; ammo swapping event
Event AmmoTweaks:ATEventManager.PlayerSwapAmmoEvent(AmmoTweaks:ATEventManager akSender, Var[] args)
EndEvent



; uninitialized state
auto state WeapInitState
	Event OnBeginState(string asNewState)
		if !bInitialized
			debug.notification("WeapInitState: " + GetFormID())
			;utility.WaitMenuMode(3.0)
			;ObjectReference curContainer = GetContainer()
			;if (curContainer != none)
			;	Drop(true)
			;	curContainer.AddItem(self)
			;endIf
			;WeaponAttachMod(ResetStatsMod)
		endIf
	EndEvent
	
	Event OnLoad()
		if !bInitialized
			InitInstance()
		endIf
		GoToState("WeapReadyState")
	EndEvent
	
	; initializes the weapon ref's InstanceData modifiers
	Function InitInstance()
		CurAmmoItems = new Ammo[0]
		RegisterForRemoteEvent(Game.GetPlayer(), "OnPlayerLoadGame")
		
		CheckWeaponInstance(true)
		
	EndFunction
	
	
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			AttachMod(tempMod)
			utility.waitmenumode(0.0167)
			ResetWeaponInstance()
		endIf
	EndFunction
	
	Event OnEquipped(Actor akActor)
		Actor curContainer = akActor
		if (curContainer != none)
			curContainer.UnequipItem(self.GetBaseObject(), false, true)
			Drop(true)
			utility.waitmenumode(0.0167)
			curContainer.AddItem(self)
		endIf
	EndEvent

endState

; idle state
state WeapReadyState
	Event OnBeginState(string asNewState)
		debug.notification("WeapReadyState: " + GetFormID())
	EndEvent
	
	; updates the instance if needed, sets up swapping events
	Event OnEquipped(Actor akActor)
		CurrentHolder = akActor
		CheckWeaponInstance(false)
		
		if (akActor == Game.GetPlayer())
			GoToState("PlayerEquippedState")
		else
			GoToState("NPCEquippedState")
		endIf
	EndEvent
	
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			AttachMod(tempMod)
			utility.waitmenumode(0.0167)
			ResetWeaponInstance()
		endIf	
	EndFunction
	
endState

; equipped by the player
state PlayerEquippedState
	Event OnBeginState(string asNewState)
		debug.notification("PlayerEquippedState: " + GetFormID())
		RegisterForCustomEvent(at_AmmoTweaksEventManager, "PlayerSwapAmmoEvent")
	EndEvent
	
	
	Event OnUnequipped(Actor akActor)
		GoToState("WeapReadyState")
	EndEvent
	
	Event OnEndState(string asNewState)
		CurrentHolder = none
		UnregisterForCustomEvent(at_AmmoTweaksEventManager, "PlayerSwapAmmoEvent")
	EndEvent
	
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			Actor tempActor = CurrentHolder
			
			if CurrentHolder != none
				CurrentHolder.UnequipItem(GetBaseObject(), false, true)
			endIf
			AttachMod(tempMod)
			; min 1 frame delay
			utility.waitmenumode(0.0167)
			ResetWeaponInstance()
			
			if tempActor != none
				tempActor.EquipItem(GetBaseObject(), false, true)
			endIf
		endIf	
	EndFunction
	
	
	Event OnAnimationEvent(ObjectReference akSource, string asEventName)
		if asEventName == sAmmoSwitchAnimEvent
			UnregisterForAnimationEvent(akSource, sAmmoSwitchAnimEvent)
			UpdateWeaponInstance()
		endIf
	EndEvent
	
	
	; ammo swapping event
	Event AmmoTweaks:ATEventManager.PlayerSwapAmmoEvent(AmmoTweaks:ATEventManager akSender, Var[] args)
		int tempAmmoIndex = args[0] as int
		debug.notification("weapon SwapAmmoEvent called : " + tempAmmoIndex)
		
		if tempAmmoIndex < 0
			if (iAmmoTypeIndex + 1 < CurAmmoItems.length)
				tempAmmoIndex = iAmmoTypeIndex + 1
			else
				tempAmmoIndex = 0
			endIf
		endIf
		
		iAmmoTypeIndex = tempAmmoIndex
		
		if (CurrentHolder != none)
			RegisterForAnimationEvent(CurrentHolder as ObjectReference, sAmmoSwitchAnimEvent)
			if (CurrentHolder.GetAnimationVariableBool("IsFirstPerson"))
				CurrentHolder.PlayIdle(pAmmoSwitchIdle_1P)
			else
				CurrentHolder.PlayIdle(pAmmoSwitchIdle_3P)
			endIf
		endIf
		
	EndEvent
	
endState

; equipped by an NPC
state NPCEquippedState
	Event OnBeginState(string asNewState)
		debug.notification("NPCEquippedState: " + GetFormID())
	EndEvent
	
	Event OnUnequipped(Actor akActor)
		GoToState("WeapReadyState")
	EndEvent
	
	Event OnEndState(string asNewState)
		CurrentHolder = none
	EndEvent
	
endState

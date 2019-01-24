scriptname AmmoTweaks:Weapon:WeaponReference extends AmmoTweaks:Weapon:WeaponRefBase
{saves current condition and instance mods, handles mod attachment}


Group WeaponConfig
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

Ammo[] 		CurAmmoItems = none

ObjectMod ResetStatsMod = none

; actor this weapon is equipped on
Actor CurrentHolder = none


Keyword CurCaliberKW = none
int iAmmoTypeIndex = -1

bool bInitialized = false

bool bReEquipping = false



Event OnInit()
	if !bInitialized
		ObjectReference curContainer = GetContainer()
		if (curContainer != none)
			Drop(true)
			curContainer.AddItem(self)
		endIf
	endIf
EndEvent

Event OnLoad()
	if !bInitialized
		bInitialized = true
		InitInstance()
	endIf
EndEvent

; initializes the weapon ref's InstanceData modifiers
Function InitInstance()
	CurAmmoItems = new Ammo[0]
	RegisterForRemoteEvent(Game.GetPlayer(), "OnPlayerLoadGame")
	
	CheckWeaponInstance(true)
	GoToState("WeapReadyState")
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
	return (GetMaxCND() as float)
EndFunction

float Function GetSavedCNDPerShot()
	return CalcWearPerShot()
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
		
		if bIsNew
			float fMaxCND = GetMaxCND() as float
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
	
	if iAmmoTypeIndex > -1
		CurAmmoItems = GetAmmoList()
		debug.notification("Ammo " + iAmmoTypeIndex + " = " + CurAmmoItems[iAmmoTypeIndex].GetFormID())
	else
		CurAmmoItems = new Ammo[0]
		debug.notification("Ammo " + iAmmoTypeIndex + " = none")
	endIf
	LogWeaponStats(0)
EndFunction


Function UpdateWeaponInstance()
	CurCaliberKW = GetCurCaliber()
	if (iAmmoTypeIndex < 0)
		iAmmoTypeIndex = GetCurAmmoType(CurCaliberKW)
	endIf
	ResetStatsMod = GetCurCaliberMod()
	if ResetStatsMod != none
		WeaponAttachMod(ResetStatsMod)
	endIf
EndFunction


Function UpdateInstance_Ammo(int iAmmoIndex)
	iAmmoTypeIndex = iAmmoIndex
EndFunction



Function UpdateHUDInfo()
	
	if (CurAmmoItems.length > 0)
		
	endIf
EndFunction


; ammo swapping event
Event AmmoTweaks:ATEventManager.PlayerSwapAmmoEvent(AmmoTweaks:ATEventManager akSender, Var[] args)
EndEvent



; uninitialized state
auto state WeapInitState
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			AttachMod(tempMod)
			;utility.waitmenumode(0.0167)
			ResetWeaponInstance()
		endIf
	EndFunction
	
	Event OnEquipped(Actor akActor)
		Actor curContainer = akActor
		if (curContainer != none)
			curContainer.UnequipItem(self.GetBaseObject(), false, true)
			Drop(true)
			;utility.waitmenumode(0.0167)
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
			;utility.waitmenumode(0.0167)
			ResetWeaponInstance()
		endIf	
	EndFunction
	
endState

; equipped by the player
state PlayerEquippedState
	Event OnBeginState(string asNewState)
		debug.notification("PlayerEquippedState: " + GetFormID())
		if (!bReEquipping)
			RegisterForCustomEvent(at_AmmoTweaksEventManager, "PlayerSwapAmmoEvent")
		endIf
		bReEquipping = false
		UpdateHUDInfo()
	EndEvent
	
	
	Event OnUnequipped(Actor akActor)
		if (!bReEquipping)
			GoToState("WeapReadyState")
		endIf
	EndEvent
	
	Event OnEndState(string asNewState)
		CurrentHolder = none
		UnregisterForCustomEvent(at_AmmoTweaksEventManager, "PlayerSwapAmmoEvent")
	EndEvent
	
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			Actor tempActor = CurrentHolder
			bReEquipping = true
			
			if CurrentHolder != none
				CurrentHolder.UnequipItem(GetBaseObject(), false, true)
			endIf
			AttachMod(tempMod)
			; min 1 frame delay
			;utility.waitmenumode(0.0167)
			ResetWeaponInstance()
			
			if tempActor != none
				tempActor.EquipItem(GetBaseObject(), false, true)
			endIf
			
			bReEquipping = false
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
		if (!bReEquipping)
			GoToState("WeapReadyState")
		endIf
	EndEvent
	
	; attaches a mod to this weapon reference
	Function WeaponAttachMod(ObjectMod tempMod)
		if tempMod != none
			Actor tempActor = CurrentHolder
			bReEquipping = true
			
			if CurrentHolder != none
				CurrentHolder.UnequipItem(GetBaseObject(), false, true)
			endIf
			AttachMod(tempMod)
			; min 1 frame delay
			;utility.waitmenumode(0.0167)
			ResetWeaponInstance()
			
			if tempActor != none
				tempActor.EquipItem(GetBaseObject(), false, true)
			endIf
			
			bReEquipping = false
		endIf	
	EndFunction
	
	Event OnEndState(string asNewState)
		CurrentHolder = none
	EndEvent
	
endState

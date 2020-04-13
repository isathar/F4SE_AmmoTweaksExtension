scriptname AT:Weapon:WeaponManager_Guns extends ActiveMagicEffect
{handles weapon updates on fire and reload, critical failures, unjamming}


AT:EventManager Property 	pManagerQuest 		auto const mandatory
Action Property 			pActionReload 		auto const mandatory

String Property sFireAnimEvent = 		"weaponFire" 		auto const
String Property sReloadExitAnimEvent = 	"ReloadComplete" 	auto const
String Property sAnimVar_bIsThrowing = 	"bIsThrowing" 		auto const

; cached variables
Actor 	OwnerActor = 			none
Spell 	CritFailSpell = 		none



Event OnEffectStart(Actor akTarget, Actor akCaster)
	if (akCaster != none)
		OwnerActor = akCaster
		; register events
		RegisterForAnimationEvent(akCaster, sFireAnimEvent)
		RegisterForAnimationEvent(akCaster, sReloadExitAnimEvent)
		if (akCaster == Game.GetPlayer())
			RegisterForCustomEvent(pManagerQuest, "PlayerUnJamEvent")
		endIf
	endIf
EndEvent


; Animation Event Listeners
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if (asEventName == sFireAnimEvent)
		; ignore weaponFire events from thrown weapons
		if (!OwnerActor.GetAnimationVariableBool(sAnimVar_bIsThrowing))
			; crit failure check, damage CND if enabled
			CritFailSpell = AT:Globals.WeaponFired(OwnerActor)
			if (CritFailSpell != none)
				OwnerActor.DoCombatSpellApply(CritFailSpell, OwnerActor)
			endIf
		endIf
		return
	endIf
	
	if (asEventName == sReloadExitAnimEvent)
		AT:Globals.UpdateEquippedWeaponVars(OwnerActor)
	endIf
EndEvent


; clears the jammed state
Event AT:EventManager.PlayerUnJamEvent(AT:EventManager akSender, Var[] args)
	if (OwnerActor != none)
		if (AT:Globals.CheckUnjam(OwnerActor))
			OwnerActor.PlayIdleAction(pActionReload)
			pManagerQuest.SendUpdateHUDOverlayEvent(true)
		endIf
	endIf
EndEvent

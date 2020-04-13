scriptname AT:Effects:AmmoRegeneration extends ActiveMagicEffect
{handles recharging ammo types}


String Property 	sReloadAnimEvent = 	"reloadStateEnter" 	AutoReadOnly


Action pActionRightInterrupt = none


Actor OwnerActor = none
Ammo pBreederAmmo = none


int iLastCount = 0


Event OnEffectStart(Actor akTarget, Actor akCaster)
	OwnerActor = akTarget
	pBreederAmmo = AT:Globals.GetEquippedAmmo(akTarget)
	if (pBreederAmmo != none)
		pActionRightInterrupt = Game.GetFormFromFile(0x013456, "Fallout4.esm") as Action
		RegisterForAnimationEvent(OwnerActor, sReloadAnimEvent)
		StartTimer(5.0, 1)
	endIf
EndEvent


; add a charge every 5 seconds, add dummy ammo if needed
Event OnTimer(int aiTimerID)
	if aiTimerID == 1
		if (OwnerActor != none && !(OwnerActor.IsDead()))
			if (OwnerActor.GetItemCount(pBreederAmmo) < 1)
				OwnerActor.AddItem(pBreederAmmo, 1, true)
			endIf
			AT:Globals.LoadEquippedAmmo(OwnerActor, 1)
			iLastCount = AT:Globals.GetEquippedAmmoCount(OwnerActor)
			StartTimer(5.0, 1)
		endIf
	endIf
EndEvent


; prevent standard reloading
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if asEventName == sReloadAnimEvent
		debug.notification("MF Breeder reload event")
		OwnerActor.PlayIdleAction(pActionRightInterrupt)
		AT:Globals.SetEquippedAmmoCount(OwnerActor, iLastCount)
	endIf
EndEvent


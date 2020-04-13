scriptname AT:Effects:ExtraAmmoConsumption extends ActiveMagicEffect
{handles recharging ammo types}


ActorValue Property 		pAmmoConsumption 			auto const mandatory
Keyword Property			pIsPowerArmorBattery		auto const mandatory

; called when the weapon is fired
String Property sFireAnimEvent = 			"weaponFire" 		AutoReadOnly


Action 	pActionRightInterrupt = none

Actor OwnerActor = none
Ammo CurrentAmmo = none

int iLastCount = 0
int iAmmoConsumption = 0

bool bIsBattery = false


Event OnEffectStart(Actor akTarget, Actor akCaster)
	OwnerActor = akTarget
	iAmmoConsumption = akTarget.GetValue(pAmmoConsumption) as int
	
	if (iAmmoConsumption > 0)
		pActionRightInterrupt = Game.GetFormFromFile(0x013456, "Fallout4.esm") as Action
		bIsBattery = CurrentAmmo.HasKeyword(pIsPowerArmorBattery)
		CurrentAmmo = AT:Globals.GetEquippedAmmo(akTarget)
		iLastCount = AT:Globals.GetEquippedAmmoCount(OwnerActor)
		RegisterForAnimationEvent(akTarget, sFireAnimEvent)
	endIf
EndEvent


Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if (asEventName == sFireAnimEvent)
		if (AT:Globals.GetEquippedAmmoCount(OwnerActor) >= iAmmoConsumption)
			iLastCount = AT:Globals.LoadEquippedAmmo(OwnerActor, -iAmmoConsumption)
			OwnerActor.RemoveItem(CurrentAmmo, iAmmoConsumption, true)
		else
			OwnerActor.PlayIdleAction(pActionRightInterrupt)
		endIf
	endIf
EndEvent


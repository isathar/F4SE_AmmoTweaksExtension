scriptname AT:Effects:CritFail_Dud extends ActiveMagicEffect


Event OnEffectStart(Actor akTarget, Actor akCaster)
	Ammo AmmoItem = AT:Globals.GetEquippedAmmo(akTarget)
	if (AmmoItem != none)
		int removeCount = AT:Globals.LoadEquippedAmmo(akTarget, -1)
		if (removeCount < 0)
			removeCount = removeCount * -1
			akTarget.RemoveItem(AmmoItem, removeCount, false)
			if (akTarget == Game.GetPlayer())
				AT:Globals.SendHUDOverlayMessage("$ATCritFail_Misfire")
			endIf
		endIf
	endIf
EndEvent


scriptname AT:Effects:CritFail_LoseAmmo extends ActiveMagicEffect


Event OnEffectStart(Actor akTarget, Actor akCaster)
	Ammo AmmoItem = AT:Globals.GetEquippedAmmo(akTarget)
	if (AmmoItem != none)
		int iAmountToRemove = AT:Globals.LoadEquippedAmmo(akTarget, -999)
		if (iAmountToRemove < 0)
			iAmountToRemove = iAmountToRemove * -1
			akTarget.RemoveItem(AmmoItem, iAmountToRemove, true)
			if (akTarget == Game.GetPlayer())
				AT:Globals.SendHUDOverlayMessage("Lost " + iAmountToRemove + " rounds!")
			endIf
		endIf
	endIf
EndEvent


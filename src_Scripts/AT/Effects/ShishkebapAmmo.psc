scriptname AT:Effects:ShishkebapUseAmmo extends ActiveMagicEffect
{subtracts flamer fuel when using a shishkebap}


Ammo Property pFlamerFuel auto Const mandatory


Event OnEffectStart(Actor akTarget, Actor akCaster)
	if akTarget == Game.GetPlayer()
		if akTarget.GetItemCount(pFlamerFuel)  > 0
			akTarget.RemoveItem(pFlamerFuel, 1, false)
		endIf
	endIf
EndEvent


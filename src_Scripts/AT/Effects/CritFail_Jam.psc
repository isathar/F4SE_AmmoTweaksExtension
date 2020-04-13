scriptname AT:Effects:CritFail_Jam extends ActiveMagicEffect
{jams the target's weapon}


Ammo Property 				pAmmoJammed 		auto const mandatory
Ammo Property 				pAmmoJammedAlt 		auto const mandatory
Ammo Property 				pAmmoBroken 		auto const mandatory

Sound Property 				pJammedSound		auto const mandatory



Event OnEffectStart(Actor akTarget, Actor akCaster)
	Ammo tempAmmo = AT:Globals.GetEquippedAmmo(akTarget)
	if (tempAmmo != none)
		if ((tempAmmo != pAmmoJammed) && (tempAmmo != pAmmoJammedAlt) && (tempAmmo != pAmmoBroken))
			if (AT:Globals.GetEquippedAmmoCount(akTarget) > 1)
				AT:Globals.SetEquippedAmmoCount(akTarget, 0)
				
				pJammedSound.Play(akTarget)
				AT:Globals.SetEquippedAmmo(pAmmoJammed, akTarget)
				
				if (akTarget == Game.GetPlayer())
					AT:Globals.SendHUDOverlayMessage("$ATCritFail_Jammed")
				endIf
			endIf
		endIf
	endIf
EndEvent


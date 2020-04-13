scriptname AT:Effects:CritFail_WeaponExplode extends ActiveMagicEffect


Sound Property 				pWpnBreakSound 		auto const mandatory


Event OnEffectStart(Actor akTarget, Actor akCaster)
	Weapon tempWeap = akTarget.GetEquippedWeapon(0)
	if (tempWeap != none)
		AT:Globals.SetEquippedWeaponSavedCND(0.0, akTarget)
		; give the weapon a couple of frames to update
		utility.waitmenumode(0.0334)
		
		akTarget.UnequipItem(tempWeap)
		pWpnBreakSound.Play(akTarget)
		if (akTarget == Game.GetPlayer())
			AT:Globals.SendHUDOverlayMessage("$ATCritFail_WeapExplode")
		endIf
	endIf
EndEvent


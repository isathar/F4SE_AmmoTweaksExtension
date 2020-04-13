scriptname AT:Effects:CritFail_WeaponDamage extends ActiveMagicEffect


Sound Property 				pWpnBreakSound 		auto const mandatory


Event OnEffectStart(Actor akTarget, Actor akCaster)
	Weapon tempWeap = akTarget.GetEquippedWeapon(0)
	if (tempWeap != none)
		AT:Globals.AddEquippedWeaponSavedCND(-0.1, akTarget)
		pWpnBreakSound.Play(akTarget)
		if (akTarget == Game.GetPlayer())
			AT:Globals.SendHUDOverlayMessage("Your weapon took additional damage.")
		endIf
	endIf
EndEvent


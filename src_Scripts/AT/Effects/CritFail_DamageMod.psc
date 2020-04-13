scriptname AT:Effects:CritFail_DamageMod extends ActiveMagicEffect
{attaches a random damaged mod to the target's weapon}


Sound Property 				pWpnBreakSound 		auto const mandatory
AT:EventManager Property 	pEventManager 		auto const mandatory


Event OnEffectStart(Actor akTarget, Actor akCaster)
	ObjectMod tempMod = AT:Globals.GetRandomDamagedMod(akTarget, -1)
	
	if (tempMod != none)
		pWpnBreakSound.Play(akTarget)
		if (akTarget == Game.GetPlayer())
			AT:Globals.SendHUDOverlayMessage("$ATCritFail_ModDamage")
			pEventManager.SendWeaponAttachModEvent(akTarget, tempMod, true)
		else
			Weapon tempWeap = akTarget.GetEquippedweapon(0)
			if (tempWeap != none)
				akTarget.AttachModToInventoryItem(tempWeap, tempMod)
			endIf
		endIf
	endIf
EndEvent


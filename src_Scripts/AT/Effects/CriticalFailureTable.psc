Scriptname AT:Effects:CriticalFailureTable extends ActiveMagicEffect 
{handles picking and applying crit effects}


Event OnEffectStart(Actor akTarget, Actor akCaster)
	Spell CritFailSpell = AT:Globals.WeaponFired(akCaster)
	if (CritFailSpell != none)
		akCaster.DoCombatSpellApply(CritFailSpell, akCaster)
		if (akCaster == Game.GetPlayer())
			debug.notification("Melee Crit Failure: " + CritFailSpell.GetName())
		else
			debug.notification("NPC Melee Crit Failure: " + CritFailSpell.GetName())
		endIf
	endIf
EndEvent

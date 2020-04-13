scriptname AT:Items:WeaponRepairKit extends ActiveMagicEffect


Sound Property 		pRepairSound = none auto const

float Property 		fBaseRepairAmount = 0.25 auto const
float Property 		fAmountAddedPerPerk = 0.05 auto const

Perk[] Property 	pRepairPerks = none auto const

Keyword Property 	pRepairKeyword = none auto const
Potion Property 	pRepairKitItem = none auto const



Event OnEffectStart(Actor akTarget, Actor akCaster)
	if (akCaster != none)
		if (akCaster.WornHasKeyword(pRepairKeyword))
			float fPerkCount = 0.0
			if (pRepairPerks.length > 0)
				int i = 0
				while (i < pRepairPerks.length)
					if (akCaster.HasPerk(pRepairPerks[i]))
						fPerkCount += 1.0
					endIf
					i += 1
				endWhile
			endIf
			float fFinalAmount = fBaseRepairAmount + (fAmountAddedPerPerk * fPerkCount)
			AT:Globals.AddEquippedWeaponSavedCND(fFinalAmount, akCaster)
			
			if (pRepairSound != none)
				pRepairSound.Play(akCaster)
			endIf
			
			if (akCaster == Game.GetPlayer())
				AT:Globals.SendHUDOverlayMessage("You restored your weapon's condition by " + (fFinalAmount * 100.0) + " percent.")
			endIf
		else
			if (pRepairKitItem != none)
				akCaster.AddItem(pRepairKitItem, 1, true)
				AT:Globals.SendHUDOverlayMessage("This item can't be used to repair your weapon.")
			endIf
		endIf
	endIf
EndEvent


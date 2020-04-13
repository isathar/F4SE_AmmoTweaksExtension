scriptname AT:Weapon:MagazineManager extends ActiveMagicEffect
{handles dropping magazines/casings/drained cells on reload}


String Property sFireAnimEvent = 		"weaponFire" 		auto const
String Property sReloadStartAnimEvent = "reloadStateEnter" 	auto const

String sAnimVar_bIsThrowing = 			"bIsThrowing" 		const
String sAnimVar_IsFirstPerson = 		"IsFirstPerson" 	const

AT:Globals:MagazineData CurrentMagazine = none

bool bIsPlayerEffect = false
bool bRevPreventReloadEvent = false



Event OnEffectStart(Actor akTarget, Actor akCaster)
	CurrentMagazine = AT:Globals.GetEquippedWeaponMagazineData(akCaster)
	bIsPlayerEffect = (akCaster == Game.GetPlayer())
	if ((CurrentMagazine.MagItem != none) && (CurrentMagazine.iReloadType != -1))
		RegisterForAnimationEvent(akCaster, sReloadStartAnimEvent)
		if (bIsPlayerEffect && (CurrentMagazine.iReloadType == 2))
			RegisterForAnimationEvent(akCaster, sFireAnimEvent)
			gotoState("RevolverAnimsState")
		endIf
	endIf
EndEvent


; default animation event
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
EndEvent

Function DropMagazine(ObjectReference sourceRef, int iCount = 1)
EndFunction


auto state DefaultAnimsState
	; listen for reloadStateEnter to drop magazines or drained cells
	Event OnAnimationEvent(ObjectReference akSource, string asEventName)
		if (asEventName == sReloadStartAnimEvent)
			DropMagazine(akSource)
		endIf
	EndEvent
	
	; drops a magazine or drained cell on reload depending on weaponData.iReloadType
	Function DropMagazine(ObjectReference sourceRef, int iCount = 1)
		debug.notification("DropMagazine")
		if (sourceRef != none)
			ObjectReference tempRef = none
			if (bIsPlayerEffect && sourceRef.GetAnimationVariableBool(sAnimVar_IsFirstPerson))
				if ((CurrentMagazine.iReloadType == 0) && (CurrentMagazine.MagItem != none))
					tempRef = sourceRef.PlaceAtMe(CurrentMagazine.MagItem, 1, abInitiallyDisabled=true)
				elseIf ((CurrentMagazine.iReloadType == 1) && (CurrentMagazine.AmmoCasing != none))
					tempRef = sourceRef.PlaceAtMe(CurrentMagazine.AmmoCasing, 1, abInitiallyDisabled=true)
				endIf
				
				if (tempRef != none)
					tempRef.MoveTo(tempRef, 0.0, 0.0, sourceRef.GetHeight() * 0.4, false)
					tempRef.Enable()
					if (CurrentMagazine.iReloadType == 1)
						if (utility.randomfloat() > CurrentMagazine.fCasingChance)
							tempRef.BlockActivation(true, true)
						endIf
					endIf
				endIf
			else
				if (sourceRef.HasNode("Weapon"))
					if ((CurrentMagazine.iReloadType == 0) && (CurrentMagazine.MagItem != none))
						tempRef = sourceRef.PlaceAtNode("Weapon", CurrentMagazine.MagItem, 1)
					elseIf ((CurrentMagazine.iReloadType == 1) && (CurrentMagazine.AmmoCasing != none))
						tempRef = sourceRef.PlaceAtNode("Weapon", CurrentMagazine.AmmoCasing, 1)
						if (tempRef != none)
							if (utility.randomfloat() > CurrentMagazine.fCasingChance)
								tempRef.BlockActivation(true, true)
							endIf
						endIf
					endIf
				endIf
			endIf
		endIf
	EndFunction
endState


state RevolverAnimsState
	; revolver reload events are called after every shot in first person
	Event OnAnimationEvent(ObjectReference akSource, string asEventName)
		if asEventName == sFireAnimEvent
			; ignore weaponFire events from thrown weapons
			if (!akSource.GetAnimationVariableBool(sAnimVar_bIsThrowing))
				bRevPreventReloadEvent = akSource.GetAnimationVariableBool(sAnimVar_IsFirstPerson)
			endIf
			return
		endIf
		
		if (asEventName == sReloadStartAnimEvent)
			if (!bRevPreventReloadEvent)
				int iCurNumShots = CurrentMagazine.iAmmoCapacity - AT:Globals.GetEquippedAmmoCount(akSource as Actor)
				DropMagazine(akSource, iCurNumShots)
			endIf
			bRevPreventReloadEvent = false
		endIf
	EndEvent
	
	; drops casings on reload for revolvers
	Function DropMagazine(ObjectReference sourceRef, int iCount = 1)
		debug.notification("DropMagazine")
		if ((sourceRef != none) && (iCount > 0))
			; revolver - drop casings
			if (CurrentMagazine.AmmoCasing != none)
				ObjectReference tempRef = none
				if (bIsPlayerEffect && sourceRef.GetAnimationVariableBool(sAnimVar_IsFirstPerson))
					int i = 0
					float dropHeight = sourceRef.GetHeight() * 0.4
					while (i < iCount)
						tempRef = sourceRef.PlaceAtMe(CurrentMagazine.AmmoCasing, 1, abInitiallyDisabled=true)
						if (tempRef != none)
							tempRef.MoveTo(tempRef, 0.0, 0.0, dropHeight, false)
							tempRef.Enable()
							if (utility.randomfloat() > CurrentMagazine.fCasingChance)
								tempRef.BlockActivation(true, true)
							endIf
						endIf
						i += 1
					endWhile
				else
					if (sourceRef.HasNode("Weapon"))
						int i = 0
						while (i < iCount)
							tempRef = sourceRef.PlaceAtNode("Weapon", CurrentMagazine.AmmoCasing, 1)
							if (tempRef != none)
								if (utility.randomfloat() > CurrentMagazine.fCasingChance)
									tempRef.BlockActivation(true, true)
								endIf
							endIf
							i += 1
						endWhile
					endIf
				endIf
			endIf
		endIf
	EndFunction
endState



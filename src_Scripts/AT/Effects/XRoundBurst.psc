scriptname AT:Effects:XRoundBurst extends ActiveMagicEffect
{handles shot count for burst firemodes}


int Property 		iBurstShots = 3					auto Const
{shots per burst}

Action pActionRightInterrupt = none

int iBurstCount = 0


; init
Event OnEffectStart(Actor akTarget, Actor akCaster)
	utility.WaitMenuMode(0.2)
	pActionRightInterrupt = Game.GetFormFromFile(0x013456, "Fallout4.esm") as Action
	RegisterForAnimationEvent(akCaster, "weaponFire")
	RegisterForAnimationEvent(akCaster, "attackStateExit")
EndEvent


; listens for weapon animation events
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if (asEventName == "weaponFire")
		iBurstCount += 1
		if (iBurstCount >= iBurstShots)
			(akSource as Actor).PlayIdleAction(pActionRightInterrupt)
		endIf
	elseif (asEventName == "attackStateExit")
		iBurstCount = 0
	endIf
EndEvent


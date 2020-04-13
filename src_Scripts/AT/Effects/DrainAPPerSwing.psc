scriptname AT:Effects:DrainAPPerSwing extends ActiveMagicEffect
{handles damaging AP per melee swing}

string Property 				sFireEventName = "weaponSwing" auto Const
string Property 				sBlockEventName = "blockStart" auto Const

Idle Property 					pWeapIdle_1stP			auto Const mandatory
{first person interrupt idle}
Idle Property 					pWeapIdle_3rdP			auto Const mandatory
{third person interrupt idle}

ActorValue Property 			at_av_MeleeSwingAPCost = none		auto Const mandatory
ActorValue Property 			ActionPoints = none		auto Const mandatory

ActorValue Property			at_av_MeleeAPDmgMult			auto Const mandatory

float property fDrainAmount = 0.0 auto hidden
float property fActionPointsCount = 0.0 auto hidden

; init
Event OnEffectStart(Actor akTarget, Actor akCaster)
	Utility.WaitMenuMode(0.2)
	fDrainAmount = akTarget.GetValue(at_av_MeleeSwingAPCost)
	akTarget.SetValue(at_av_MeleeAPDmgMult, (1.0 - akTarget.GetValuePercentage(ActionPoints)))
	RegisterForAnimationEvent(akTarget as ObjectReference, sFireEventName)
	RegisterForAnimationEvent(akTarget as ObjectReference, sBlockEventName)
EndEvent


; listens for weapon animation events
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if (asEventName == sFireEventName) || (asEventName == sBlockEventName)
		if akSource.GetValue(ActionPoints) < fDrainAmount
			PlayStopFireAnimation(akSource as Actor)
			DrainAP(akSource, 1.0)
		else
			DrainAP(akSource, 1.0)
		endIf
	endIf
EndEvent


Function DrainAP(ObjectReference akSource, float fAPMult=1.0)
	float fAPAmount = akSource.GetValue(ActionPoints)
	if (fDrainAmount * fAPMult) <= fAPAmount
		akSource.DamageValue(ActionPoints, (fDrainAmount * fAPMult))
	else
		if fAPAmount > 0.0
			akSource.DamageValue(ActionPoints, fAPAmount)
		endIf
	endIf
	akSource.SetValue(at_av_MeleeAPDmgMult, (1.0 - akSource.GetValuePercentage(ActionPoints)))
EndFunction


; plays the interrupt idle
Function PlayStopFireAnimation(Actor akSource)
	if akSource == Game.GetPlayer()
		if akSource.GetAnimationVariableBool("IsFirstPerson")
			akSource.PlayIdle(pWeapIdle_1stP)
		else
			akSource.PlayIdle(pWeapIdle_3rdP)
		endIf
	else
		akSource.PlayIdle(pWeapIdle_3rdP)
	endIf
EndFunction

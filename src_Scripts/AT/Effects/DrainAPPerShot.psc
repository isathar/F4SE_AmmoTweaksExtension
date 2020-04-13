scriptname AT:Effects:DrainAPPerShot extends ActiveMagicEffect
{handles damaging AP per shot}

string Property 				sFireEventName = "weaponFire" auto Const
{firing animation event to listen for - WeaponFire for Ranged, 0 for Melee}
string Property 				sBashEventName = "meleeAttackGun" auto Const
{firing animation event to listen for - WeaponFire for Ranged, 0 for Melee}

ActorValue Property 			at_av_MeleeSwingAPCost = none		auto Const mandatory
ActorValue Property 			ActionPoints = none		auto Const mandatory



; init
Event OnEffectStart(Actor akTarget, Actor akCaster)
	RegisterForAnimationEvent(akTarget, sFireEventName)
EndEvent

; listens for weapon animation events
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if asEventName == sFireEventName
		DrainAP(akSource, 1.0)
	elseif asEventName == sBashEventName
		DrainAP(akSource, 2.0)
	endIf
EndEvent


Function DrainAP(ObjectReference akSource, float fAPMult=1.0)
	float fDrainAmount = akSource.GetValue(at_av_MeleeSwingAPCost) * fAPMult
	float fAPAmount = akSource.GetValue(ActionPoints)
	if fDrainAmount <= fAPAmount
		akSource.DamageValue(ActionPoints, fDrainAmount)
	else
		if fAPAmount > 0.0
			akSource.DamageValue(ActionPoints, fAPAmount)
		endIf
	endIf
EndFunction

scriptname AT:Effects:RipperUseAmmo extends ActiveMagicEffect
{subtracts ammo when using a ripper}


Group RipperAmmo
	Ammo Property 	pRipperAmmo 			auto Const mandatory
	bool Property 	bNPCsUseAmmo = 	false 	auto const
	int Property 	iChargesPerCell = 1		auto const
	int Property 	iCapacity = 30			auto const

	Idle Property 	pInterruptIdle_1P 		auto const mandatory
	Idle Property 	pInterruptIdle_3P 		auto const mandatory
EndGroup

String Property		sStartFireAnimEvent = 	"attackStateEnter" 	AutoReadOnly
String Property 	sStopFireAnimEvent = 	"attackStateExit" 	AutoReadOnly


Actor 	OwnerActor = 	none
int 	iLoadedAmmo = 	0


Event OnEffectStart(Actor akTarget, Actor akCaster)
	if (bNPCsUseAmmo || (akTarget == Game.GetPlayer()))
		OwnerActor = akTarget
		RegisterForAnimationEvent(OwnerActor, sStartFireAnimEvent)
		RegisterForAnimationEvent(OwnerActor, sStopFireAnimEvent)
	endIf
EndEvent


; add ammo item when timer fires
Event OnTimer(int aiTimerID)
	if (aiTimerID == 1)
		if (OwnerActor.GetItemCount(pRipperAmmo) > 1)
			OwnerActor.RemoveItem(pRipperAmmo, 1, false)
			StartTimer(1.0, 1)
		else
			if (OwnerActor.GetAnimationVariableBool("IsFirstPerson"))
				OwnerActor.PlayIdle(pInterruptIdle_1P)
			else
				OwnerActor.PlayIdle(pInterruptIdle_3P)
			endIf
		endIf
	endIf
EndEvent


Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if asEventName == sStartFireAnimEvent
		StartTimer(1.0, 1)
	elseIf asEventName == sStopFireAnimEvent
		CancelTimer(1)
	endIf
EndEvent


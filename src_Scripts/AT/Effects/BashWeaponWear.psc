scriptname AT:Effects:BashWeaponWear extends ActiveMagicEffect
{subtracts from weapon condition on melee hit/weapon bash}


ActorValue Property pWeaponCondition auto Const


Event OnEffectStart(Actor akTarget, Actor akCaster)
	akCaster.DamageValue(pWeaponCondition, 4.0)
EndEvent

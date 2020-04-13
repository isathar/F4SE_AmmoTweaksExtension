scriptname AT:Items:MeleePoisonItem extends ActiveMagicEffect


AT:EventManager Property pManagerQuest auto const mandatory

Spell Property pCurrentPoison auto hidden
int Property iNumCharges auto hidden



Event OnEffectStart(Actor akTarget, Actor akCaster)
	
EndEvent

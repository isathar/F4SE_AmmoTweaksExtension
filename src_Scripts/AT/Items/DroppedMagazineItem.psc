scriptname AT:Items:DroppedMagazineItem extends ObjectReference
{magazine test}

ActorValue Property Luck auto const mandatory
MiscObject Property MagItem auto const mandatory
Form Property ScrapOutput auto const mandatory


Event OnContainerChanged(ObjectReference akNewContainer, ObjectReference akOldContainer)
	if akNewContainer == Game.GetPlayer() as ObjectReference
		if (utility.RandomInt(0,100) < (akNewContainer.GetValue(Luck) as int))
			akNewContainer.AddItem(MagItem, 1, true)
		else
			if (utility.RandomInt(0,100) < 50)
				akNewContainer.AddItem(ScrapOutput, 1, true)
			endIf
		endIf
		akNewContainer.RemoveItem(self, 1, true)
	endIf
EndEvent


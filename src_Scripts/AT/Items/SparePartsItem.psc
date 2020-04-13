scriptname AT:Items:SparePartsItem extends ObjectReference

Potion Property pSparePartsPotion auto const mandatory

Event OnContainerChanged(ObjectReference akNewContainer, ObjectReference akOldContainer)
	if akNewContainer == Game.GetPlayer() as ObjectReference
		akNewContainer.AddItem(pSparePartsPotion, 1, true)
		akNewContainer.RemoveItem(self, 1, true)
	endIf
EndEvent

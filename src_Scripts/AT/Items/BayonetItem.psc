scriptname AT:Items:BayonetItem extends ObjectReference


Weapon Property BayonetWeapon auto const mandatory


Event OnContainerChanged(ObjectReference akNewContainer, ObjectReference akOldContainer)
	if akNewContainer == Game.GetPlayer() as ObjectReference
		akNewContainer.AddItem(BayonetWeapon, 1, true)
		akNewContainer.RemoveItem(self, 1, true)
	endIf
EndEvent

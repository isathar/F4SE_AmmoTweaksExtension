scriptname AT:Weapon:HolsterManager extends ActiveMagicEffect
{handles unequipped weapon/holster armors}


String Property 	sDrawAnimEvent = 		"weaponDraw" 		auto const
String Property 	sSheatheAnimEvent = 	"weaponSheathe" 	auto const

AT:Globals:WeaponHolsteredData PrimaryWeaponArmor = 	none
;AT:Globals:WeaponHolsteredData SecondaryWeaponArmor = 	none
;AT:Globals:WeaponHolsteredData ThrownWeaponArmor = 	none

Actor OwnerActor = none


Event OnEffectStart(Actor akTarget, Actor akCaster)
	if (akCaster != none)
		OwnerActor = akCaster
		PrimaryWeaponArmor = AT:Globals.GetEquippedWeaponHolsterData(akCaster)
		
		RegisterForAnimationEvent(akCaster, sDrawAnimEvent)
		RegisterForAnimationEvent(akCaster, sSheatheAnimEvent)
	endIf
EndEvent


Event OnItemEquipped(Form akBaseObject, ObjectReference akReference)
	if (akBaseObject as Weapon)
		PrimaryWeaponArmor = AT:Globals.GetEquippedWeaponHolsterData(OwnerActor)
		if (PrimaryWeaponArmor.WeaponHolstered != none)
			if (!OwnerActor.IsEquipped(PrimaryWeaponArmor.WeaponHolstered))
				OwnerActor.EquipItem(PrimaryWeaponArmor.WeaponHolstered, false, true)
			endIf
		endIf
	endIf
EndEvent

Event OnItemUnequipped(Form akBaseObject, ObjectReference akReference)
	if (akBaseObject as Weapon)
		if (PrimaryWeaponArmor.WeaponHolstered != none)
			if (OwnerActor.IsEquipped(PrimaryWeaponArmor.WeaponHolstered))
				OwnerActor.UnequipItem(PrimaryWeaponArmor.WeaponHolstered, false, true)
			endIf
		endIf
		PrimaryWeaponArmor = new AT:Globals:WeaponHolsteredData
	endIf
EndEvent


; draw/sheathe event listeners
; - these events should only apply to main weapons, no need to check throwables
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	Weapon curWeapon = OwnerActor.GetEquippedWeapon(0)
	if (asEventName == sDrawAnimEvent)
		if (PrimaryWeaponArmor.WeaponHolstered != none)
			if (!OwnerActor.IsEquipped(PrimaryWeaponArmor.WeaponHolstered))
				OwnerActor.EquipItem(PrimaryWeaponArmor.WeaponHolstered, false, true)
			endIf
		endIf
		return
	endIf
	
	if (asEventName == sSheatheAnimEvent)
		if (PrimaryWeaponArmor.WeaponHolstered != none)
			if (OwnerActor.IsEquipped(PrimaryWeaponArmor.WeaponHolstered))
				OwnerActor.UnequipItem(PrimaryWeaponArmor.WeaponHolstered, false, true)
			endIf
		endIf
	endIf
EndEvent


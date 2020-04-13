scriptname AT:Weapon:WeaponRef extends ObjectReference
{Weapon reference}


AT:EventManager Property pEventManager auto const mandatory

int iAmmoCount = 0
int iAmmoLauncherCount = 0


Event OnEquipped(Actor akActor)
	if (akActor == Game.GetPlayer())
		RegisterForCustomEvent(pEventManager, "WeaponAttachModEvent")
		debug.notification("WeaponRef CND: " + GetItemHealthPercent())
	endIf
EndEvent


Event OnUnEquipped(Actor akActor)
	if (akActor == Game.GetPlayer())
		UnregisterForCustomEvent(pEventManager, "WeaponAttachModEvent")
	endIf
EndEvent


; external access to attaching mods
Event AT:EventManager.WeaponAttachModEvent(AT:EventManager akSender, Var[] args)
	Actor weaponOwner = args[0] as Actor
	ObjectMod newMod = args[1] as ObjectMod
	if ((weaponOwner != none))
		if (weaponOwner == Game.GetPlayer())
			Weapon curWeap = GetBaseObject()
			weaponOwner.UnequipItem(curWeap, false, true)
			utility.waitmenumode(0.0167)
			AttachMod(newMod)
			weaponOwner.EquipItem(curWeap, false, true)
		else
			UnregisterForCustomEvent(pEventManager, "WeaponAttachModEvent")
			debug.notification("WeaponRef:AttachMod called on NPC")
		endIf
	else
		AttachMod(newMod)
		debug.notification("WeaponRef:AttachMod called on unequipped weapon")
	endIf
EndEvent


scriptname AT:Weapon:WeaponManagerBase extends ActiveMagicEffect


; 
ActorValue Property 		pDoAnimOnReEquip	auto const mandatory

AT:EventManager Property 	pEventManager 		auto const mandatory
Quest Property 				MQ102 				auto const mandatory

String Property 			sDrawAnimEvent = "weaponDraw" 			auto const
String Property 			sSheatheAnimEvent = "weaponSheathe" 	auto const


Actor OwnerActor = none
Weapon CurWeapon = none
Weapon CurGrenade = none

Armor WeaponHolstered = none
Armor HolsterArmor = none
Armor GrenadeDisp = none

AT:Globals:WeaponModData nextWeaponMod = none



; queued mod to attach during the next anim event
ObjectMod pNextModToSwapTo = none

bool bSaveAmmoCount = false

bool bIsPlayer = false

; used to ignore the unequip event when attaching a mod
bool bIgnoreUnequip = false



Event OnEffectStart(Actor akTarget, Actor akCaster)
	nextWeaponMod = new AT:Globals:WeaponModData
	
	OwnerActor = akTarget
	bIsPlayer = (OwnerActor == Game.GetPlayer())
	
	if (bIsPlayer)
		if (!MQ102.IsStageDone(10))
			RegisterForRemoteEvent(MQ102, "OnStageSet")
		endIf
	endIf
	
	RegisterForExternalEvent("ATModSwapEvent", "ATModSwapEvent")
	
	RegisterForAnimationEvent(OwnerActor, sDrawAnimEvent)
	RegisterForAnimationEvent(OwnerActor, sSheatheAnimEvent)
	
	ResetEffects()
EndEvent


; resets event listeners when the tutorial quest stage is finished
Event Quest.OnStageSet(Quest akSender, int auiStageID, int auiItemID)
	if (auiStageID >= 10)
		OwnerActor = GetTargetActor()
		UnregisterForRemoteEvent(MQ102, "OnStageSet")
		RegisterForAnimationEvent(OwnerActor, sDrawAnimEvent)
		RegisterForAnimationEvent(OwnerActor, sSheatheAnimEvent)
		ResetEffects()
	endIf
EndEvent


; reset on game load to update recoil
Event OnPlayerLoadGame()
	utility.waitmenumode(1.0)
	UpdateMainWeapon(CurWeapon)
EndEvent



Function ResetEffects()
	if (bIsPlayer)
		RegisterForCustomEvent(pEventManager, "PlayerSwapModEvent")
	endIf
	UpdateMainWeapon(CurWeapon)
EndFunction


Event OnItemEquipped(Form akBaseObject, ObjectReference akReference)
	Weapon newWeap = akBaseObject as Weapon
	if (newWeap != none)
		UpdateMainWeapon(newWeap)
	endIf
EndEvent


Event OnItemUnequipped(Form akBaseObject, ObjectReference akReference)
	Weapon oldWeap = akBaseObject as Weapon
	if (oldWeap != none)
		if (oldWeap == CurWeapon)
			CurWeapon = none
			UpdateMainWeapon(none)
			
		elseif (oldWeap == CurGrenade)
			CurGrenade = none
		endIf
	endIf
EndEvent


; animation event listener
Event OnAnimationEvent(ObjectReference akSource, string asEventName)
	if (asEventName == sDrawAnimEvent)
		if (WeaponHolstered != none)
			if (OwnerActor.IsEquipped(WeaponHolstered))
				OwnerActor.UnequipItem(WeaponHolstered, false, true)
			endIf
		endIf
		
	elseif (asEventName == sSheatheAnimEvent)
		if (WeaponHolstered != none)
			if (!OwnerActor.IsEquipped(WeaponHolstered))
				OwnerActor.EquipItem(WeaponHolstered, false, true)
			endIf
		endIf
	endIf
EndEvent


Function UpdateMainWeapon(Weapon newWeapon)
	if (newWeapon != none)
		utility.waitmenumode(0.2)
		if (newWeapon == OwnerActor.GetEquippedWeapon(0))
			; ---- right/both hands weapon slot
			
			; update weapon variables
			if (AT:Globals.InitEquippedWeapon(OwnerActor))
				; recoil needs to be updated, reset the weapon ref
				AT:Globals.SendHUDOverlayMessage("Debug: Updating recoil")
				pEventManager.SendWeaponAttachModEvent(OwnerActor, none, true)
			else
				; no recoil update needed, initialize + play anim
				CurWeapon = newWeapon
				AT:Globals.UpdateEquippedWeaponVars(OwnerActor)
				UpdateMainWeaponHolster()
				
				if (bIsPlayer)
					AT:Globals.SendHUDOverlayMessage("Equipped " + CurWeapon.GetName())
				endIf
				
				if (OwnerActor.GetValue(pDoAnimOnReEquip) > 0.0)
					utility.waitmenumode(0.35)
					PlaySwapModAnim()
				endIf
			endIf
			
			return
		endIf
		
		if (newWeapon == OwnerActor.GetEquippedWeapon(1))
			; ---- left weapon slot
			if (bIsPlayer)
				int iEquipSlot = OwnerActor.GetEquippedItemType(1) + 32
				debug.notification("left weapon equip slot: " + iEquipSlot)
				AT:Globals.SendHUDOverlayMessage("Equipped " + newWeapon.GetName())
			endIf
			return
		endIf
		
		if (newWeapon == OwnerActor.GetEquippedWeapon(2))
			; ---- thrown weapon slot
			CurGrenade = newWeapon
			; - TBD: set thrown weapon icon, penetration, crit chance
			if (bIsPlayer)
				;int iEquipSlot = OwnerActor.GetEquippedItemType(2) + 32
				;debug.notification("thrown weapon equip slot: " + iEquipSlot)
				AT:Globals.SendHUDOverlayMessage("Equipped " + CurGrenade.GetName())
			endIf
			return
		endIf
		
		; ---- something went wrong and/or is using a weapon slot that doesn't exist
		if (bIsPlayer)
			AT:Globals.SendHUDOverlayMessage("Equipped weapon in unknown slot: " + newWeapon.GetName())
		endIf
		
	else
		AT:Globals.ResetEquippedWeaponVars(OwnerActor)
	endIf
EndFunction


Function UpdateMainWeaponHolster()
	AT:Globals:WeaponHolsteredData WeaponHolsterData = AT:Globals.GetEquippedWeaponHolsterData(OwnerActor)
	
	if (WeaponHolstered != WeaponHolsterData.WeaponHolstered)
		if (WeaponHolstered != none)
			if (OwnerActor.IsEquipped(WeaponHolstered))
				OwnerActor.UnequipItem(WeaponHolstered, false, true)
			endIf
		endIf
		WeaponHolstered = WeaponHolsterData.WeaponHolstered
		
		if (!OwnerActor.IsWeaponDrawn())
			if (WeaponHolstered != none)
				if (!OwnerActor.IsEquipped(WeaponHolstered))
					OwnerActor.EquipItem(WeaponHolstered, false, true)
				endIf
			endIf
		endIf
	endIf
EndFunction


; ammo/weapon mod swap initiated by weapon menu
Function ATModSwapEvent(int iType, int iIndex)
	if (CurWeapon != none)
		debug.notification(("ATModSwapEvent: Type " + iType + ", Index " + iIndex) as String)
		SwapMod(iType, iIndex)
	else
		debug.notification("ATModSwapEvent: no weapon")
	endIf
EndFunction


; ammo/weapon mod swap initiated by script
Event AT:EventManager.PlayerSwapModEvent(AT:EventManager akSender, Var[] args)
	int iType = args[0] as int
	int iIndex = args[1] as int
	if (CurWeapon != none)
		debug.notification(("PlayerSwapModEvent: Type " + iType + ", Index " + iIndex) as String)
		SwapMod(iType, iIndex)
	else
		debug.notification("PlayerSwapModEvent: no weapon")
	endIf
EndEvent


; switches to the mod at iIndex in the weapon's ModSlot at iModSlot
; iModslot:
; 	-2: Firemodes
; 	-1: Ammo subtypes
; 	0+: Mod slots
; iIndex:
;	TBD: -2: switch to the previous mod
; 	-1: switch to the next available mod
; 	0+: specific index
Function SwapMod(int iModSlot = -1, int iIndex = -1)
	if (OwnerActor != none)
		bool bDoModSwap = false
		AT:Globals:WeaponModData oldWeaponMod = AT:Globals.GetCurrentWeaponModData(OwnerActor, iModSlot)
		
		if (iIndex == -1)
			; get the next available mod
			;GetNextWeaponModData(Actor ownerActor, int iModSlot) native global
			nextWeaponMod = AT:Globals.GetNextWeaponModData(OwnerActor, iModSlot)
			if ((nextWeaponMod.swapMod != none) && (nextWeaponMod.swapMod != oldWeaponMod.swapMod))
				bDoModSwap = true
			endIf
		else
			; get the mod at iIndex
			if (AT:Globals.GetWeaponModDataAtIndex(OwnerActor, nextWeaponMod, iModSlot, iIndex))
				if ((nextWeaponMod.swapMod != none) && (nextWeaponMod.swapMod != oldWeaponMod.swapMod))
					bDoModSwap = true
				endIf
			endIf
		endIf
		
		; add/remove modmisc items if this is a regular mod slot
		if (bDoModSwap && (iModSlot > -1))
			if (oldWeaponMod.swapModMisc != none)
				OwnerActor.AddItem(oldWeaponMod.swapModMisc, 1, true)
			endIf
			if (nextWeaponMod.swapModMisc != none)
				if (OwnerActor.GetItemCount(nextWeaponMod.swapModMisc) > 0)
					OwnerActor.RemoveItem(nextWeaponMod.swapModMisc, 1, true)
				else
					bDoModSwap = false
					AT:Globals.SendHUDOverlayMessage("Missing mod misc item for " + nextWeaponMod.swapMod.GetName())
				endIf
			endIf
		endIf
		
		if (bDoModSwap)
			; Mod swap idle animation/action
			bool bHasAnims = ((nextWeaponMod.actionSwapMod != none) || ((nextWeaponMod.idleSwapMod_1P != none) && (nextWeaponMod.idleSwapMod_3P != none)))
			
			if (bIsPlayer)
				AT:Globals.SendHUDOverlayMessage("Equipping " + nextWeaponMod.swapMod.GetName())
			endIf
			
			if (bHasAnims)
				; play the animation on re-equip
				OwnerActor.SetValue(pDoAnimOnReEquip, 1.0)
				pEventManager.SendWeaponAttachModEvent(OwnerActor, nextWeaponMod.swapMod, (iModSlot != -1))
			else
				pEventManager.SendWeaponAttachModEvent(OwnerActor, nextWeaponMod.swapMod, (iModSlot != -1))
			endIf
		endIf
	endIf
EndFunction


Function PlaySwapModAnim()
	OwnerActor.SetValue(pDoAnimOnReEquip, 0.0)
	if (nextWeaponMod.actionSwapMod != none)
		OwnerActor.PlayIdleAction(nextWeaponMod.actionSwapMod)
		
	elseif (nextWeaponMod.idleSwapMod_3P != none)
		if (OwnerActor.GetAnimationVariableBool("IsFirstPerson"))
			if (nextWeaponMod.idleSwapMod_1P != none)
				OwnerActor.PlayIdle(nextWeaponMod.idleSwapMod_1P)
			endIf
		else
			if (nextWeaponMod.idleSwapMod_3P != none)
				OwnerActor.PlayIdle(nextWeaponMod.idleSwapMod_3P)
			endIf
		endIf
	endIf
EndFunction



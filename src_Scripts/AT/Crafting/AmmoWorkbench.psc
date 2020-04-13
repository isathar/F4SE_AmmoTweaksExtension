scriptname AT:Crafting:AmmoWorkbench extends ObjectReference
; crafting workbench script


AT:EventManager Property 	pEventManager 		auto const mandatory
; FurnitureExitDefault [IDLE:00160C45]

String Property sMenuName = "AmmoTweaks/CraftingMenu" auto const

Idle Property pExitFurnitureIdle auto const



Event OnActivate(ObjectReference akActionRef)
	If (akActionRef == (Game.GetPlayer() as ObjectReference))
		RegisterForCustomEvent(pEventManager, "UpgradeWorkbenchEvent")
		RegisterForMenuOpenCloseEvent(sMenuName)
	endIf
EndEvent


Event OnMenuOpenCloseEvent(string asMenuName, bool abOpening)
	if ((asMenuName == "AmmoTweaks/CraftingMenu") && !abOpening)
		UnregisterForCustomEvent(pEventManager, "UpgradeWorkbenchEvent")
		UnregisterForMenuOpenCloseEvent(sMenuName)
		if (pExitFurnitureIdle != none)
			Actor PlayerActor = Game.GetPlayer()
			PlayerActor.PlayIdle(pExitFurnitureIdle)
		endIf
	endIf
EndEvent


Event AT:EventManager.UpgradeWorkbenchEvent(AT:EventManager akSender, Var[] args)
	ObjectMod newMod = args[0] as ObjectMod
	Keyword newKeyword = args[1] as Keyword
	if ((newMod != none) && (newKeyword != none))
		if !HasKeyword(newKeyword)
			AddKeyword(newKeyword)
			AttachMod(newMod)
		endIf
	endIf
EndEvent


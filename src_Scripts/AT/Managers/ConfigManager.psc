scriptname AT:Managers:ConfigManager extends Quest
{handles mod configuration}



; holds two InstanceNamingRules to be merged
Struct NamingRulesUpdate
	string 		sName = 				" "
	{editor-only name}
	InstanceNamingRules	pNamesBase = 	none
	{base naming rule}
	InstanceNamingRules pNamesAdd = 	none
	{addition to naming rules}
EndStruct

; holds a LeveledItem and a form to add to it
Struct LListUpdate
	string 		sName = 				" "
	{editor-only name}
	LeveledItem	liItemBase = 			none
	{base leveled list}
	Form liItemAdd = 					none
	{base leveled list}
	int iItemAdd_Level = 				1
	{item level}
	int iItemAdd_Count = 				1
	{item count}
EndStruct


Group NamingRules
{Instance Naming Rules modifications}
	GlobalVariable Property 		at_glob_ConfigNaming_AddCaliber 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdates_Caliber		auto Const mandatory
	{caliber naming rules additions}
	NamingRulesUpdate Property pNamingRulesUpdates_CaliberVIS 	auto Const mandatory
	{caliber naming rules additions}
	
	GlobalVariable Property 		at_glob_ConfigNaming_FollowerNames 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_FollowerGuns	auto Const mandatory
	{Add follower names to their unique weapons}
	NamingRulesUpdate Property pNamingRulesUpdate_FollowerMelee auto Const mandatory
	{Add follower names to their unique weapons}
	
	GlobalVariable Property 		at_glob_ConfigNaming_Add10mmtoN99 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_N99Pistol 	auto Const mandatory
	{10mm->N99 naming rules addition}
	
	GlobalVariable Property 		at_glob_ConfigNaming_AddARtoMG 					auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_ARtoMG		auto Const mandatory
	{AssaultRifle->MachineGun naming rules addition}
	
	GlobalVariable Property 		at_glob_ConfigNaming_AddHMRtoAR 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_HMRtoAR		auto Const mandatory
	{HandmadeRifle->AssaultRifle naming rules addition}
	
	GlobalVariable Property 		at_glob_ConfigNaming_AddRevolver 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_Revolver		auto Const mandatory
	{Revolver Pistol->Revolver}
	
	GlobalVariable Property 		at_glob_ConfigNaming_AddRepeater 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_Repeater		auto Const mandatory
	{Revolver Rifle -> Repeater naming rules addition}
	
	GlobalVariable Property 		at_glob_ConfigNaming_AddLegendaryNames 				auto Const mandatory
	NamingRulesUpdate Property pNamingRulesUpdate_LegendaryGuns	auto Const mandatory
	{Adds names for new legendary items}
	NamingRulesUpdate Property pNamingRulesUpdate_LegendaryMelee	auto Const mandatory
	{Adds names for new legendary items}
	NamingRulesUpdate Property pNamingRulesUpdate_LegendaryArmor	auto Const mandatory
	{Adds names for new legendary items}
	
	GlobalVariable Property 		at_glob_ConfigNaming_UseVIS 					auto Const mandatory
	
	Message Property at_mesg_Config_UpdatedNamingRules auto Const mandatory
EndGroup

Group LegendaryItems
{legendary item types to add}
	LegendaryItemQuestScript Property 			LegendaryItemQuest 					auto Const mandatory
	{link to the legendary item quest}
	
	GlobalVariable Property 		at_glob_ConfigLegendary_AddDurable 				auto Const mandatory
	LegendaryItemQuestScript:LegendaryModRule Property 	LegendaryModRule_Durable 	auto Const mandatory
	
	GlobalVariable Property 		at_glob_ConfigLegendary_AddElemental 			auto Const mandatory
	LegendaryItemQuestScript:LegendaryModRule Property 	LegendaryModRule_Elemental 	auto Const mandatory
	
	GlobalVariable Property 		at_glob_ConfigLegendary_AddUnyielding 			auto Const mandatory
	LegendaryItemQuestScript:LegendaryModRule Property 	LegendaryModRule_Unyielding auto Const mandatory
	
	GlobalVariable Property 		at_glob_ConfigLegendary_AddShocking 			auto Const mandatory
	LegendaryItemQuestScript:LegendaryModRule Property 	LegendaryModRule_Shocking 	auto Const mandatory
	
	GlobalVariable Property 		at_glob_ConfigLegendary_AddLucky 				auto Const mandatory
	LegendaryItemQuestScript:LegendaryModRule Property 	LegendaryModRule_Lucky 		auto Const mandatory
	
	Message Property at_mesg_Config_UpdatedLegendaryItems auto Const mandatory
EndGroup

Group LeveledLists
{stuff to add to leveled lists}
	GlobalVariable Property 	at_glob_ConfigItems_AddMoreSupermutantWpns 		auto Const mandatory
	LListUpdate[] Property pLLUpdate_SupermutantWeaps		auto Const
	{extra supermutant weapon types}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddRepairKits 				auto Const mandatory
	LListUpdate[] Property pLLUpdate_WeapRepairKits		auto Const
	{adds repair kits to vendors + loot}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddVendorShipments 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_VendorShipments		auto Const
	{adds ammo crafting shipments to vendors}
	
	
	GlobalVariable Property 	at_glob_ConfigItems_AddLootComponents 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_LootCompos			auto Const
	{adds ammo crafting components to loot}
	
	
	
	GlobalVariable Property 	at_glob_ConfigItems_AddDLCLeverAction 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_DLCLeverAction	auto Const
	{adds Far Harbor's Lever-Action rifle to the commonwealth}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddDLCRadiumRifle 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_DLCRadiumRifle	auto Const
	{adds Far Harbor's Radium rifle to the commonwealth}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddDLCHarpoonGun 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_DLCHarpoonGun	auto Const
	{adds Far Harbor's Harpoon gun to the commonwealth}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddDLCWesternRevolver 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_DLCWesternRevolver	auto Const
	{adds Nuka World's Western revolver to the commonwealth}
	
	GlobalVariable Property 	at_glob_ConfigItems_AddDLCHandmadeRifle 			auto Const mandatory
	LListUpdate[] Property pLLUpdate_DLCHandmadeRifle	auto Const
	{adds Nuka World's Handmade rifle to the commonwealth}
	
	Message Property at_mesg_Config_UpdatedLeveledLists auto Const mandatory
EndGroup

Group ScroungerLists
	
	GlobalVariable Property 	at_glob_ConfigScrounger_223Cal			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_223Cal auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_22LR			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_22LR 	auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_3006			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_3006 	auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_300WM			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_300WM 	auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_30Carbine		auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_30Carbine auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_32ACP			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_32ACP auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_380ACP			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_380ACP auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_338Mag			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_338Mag auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_357Mag			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_357Mag auto Const
	
	GlobalVariable Property 	at_glob_ConfigScrounger_38Spc			auto Const mandatory
	LListUpdate[] Property pLLUpdate_Scrounger_38Spc auto Const
	
EndGroup



; cache what's been modified to prevent modifying it again

; loot/vendor lists
bool bItemsAdded_Supermutants = false
bool bItemsAdded_RepairKits = false
bool bItemsAdded_Shipments = false
bool bItemsAdded_Components = false

bool bItemsAdded_DLCLeverAction = false
bool bItemsAdded_DLCRadiumRifle = false
bool bItemsAdded_DLCHarpoonGun = false
bool bItemsAdded_DLCWesternRevolver = false
bool bItemsAdded_DLCHandmadeRifle = false

; scrounger lists
bool bItemsAdded_Scrounger223Cal = false
bool bItemsAdded_Scrounger22LR = false
bool bItemsAdded_Scrounger3006 = false
bool bItemsAdded_Scrounger300WM = false
bool bItemsAdded_Scrounger30Carbine = false
bool bItemsAdded_Scrounger32ACP = false
bool bItemsAdded_Scrounger338Mag = false
bool bItemsAdded_Scrounger357Mag = false
bool bItemsAdded_Scrounger38Spc = false

; naming rules
bool bINRsAdded_Caliber = false
bool bINRsAdded_Followers = false
bool bINRsAdded_10mmN99 = false
bool bINRsAdded_ARtoMG = false
bool bINRsAdded_HMRtoAR = false
bool bINRsAdded_Revolver = false
bool bINRsAdded_Repeater = false
bool bINRsAdded_Legendary = false

; legendary items
bool bLegItemAdded_Durable = false
bool bLegItemAdded_Shocking = false
bool bLegItemAdded_Unyielding = false
bool bLegItemAdded_Elemental = false
bool bLegItemAdded_Lucky = false


; Instance Naming Rules

Function MergeNamingRules(NamingRulesUpdate tempStruct)
	InstanceNamingRules BaseRules = tempStruct.pNamesBase
	InstanceNamingRules AddRules = tempStruct.pNamesAdd
	BaseRules.MergeWith(AddRules)
EndFunction


Function UpdateNamingRules()
	bool bNamingRulesVIS = at_glob_ConfigNaming_UseVIS.GetValue() > 0.0
	int iCount = 0
	
	if at_glob_ConfigNaming_AddCaliber.GetValue() > 0.0
		if !bINRsAdded_Caliber
			iCount += 1
			bINRsAdded_Caliber = true
			if bNamingRulesVIS
				MergeNamingRules(pNamingRulesUpdates_CaliberVIS)
			else
				MergeNamingRules(pNamingRulesUpdates_Caliber)
			endIf
		endIf
	endIf
	
	if at_glob_ConfigNaming_FollowerNames.GetValue() > 0.0
		if !bINRsAdded_Followers
			iCount += 1
			bINRsAdded_Followers = true
			MergeNamingRules(pNamingRulesUpdate_FollowerGuns)
			MergeNamingRules(pNamingRulesUpdate_FollowerMelee)
		endIf
	endIf
	
	if at_glob_ConfigNaming_Add10mmtoN99.GetValue() > 0.0
		if !bINRsAdded_10mmN99
			iCount += 1
			bINRsAdded_10mmN99 = true
			MergeNamingRules(pNamingRulesUpdate_N99Pistol)
		endIf
	endIf
	
	if at_glob_ConfigNaming_AddARtoMG.GetValue() > 0.0
		if !bINRsAdded_ARtoMG
			iCount += 1
			bINRsAdded_ARtoMG = true
			MergeNamingRules(pNamingRulesUpdate_ARtoMG)
		endIf
	endIf
	
	if at_glob_ConfigNaming_AddHMRtoAR.GetValue() > 0.0
		if !bINRsAdded_HMRtoAR
			iCount += 1
			bINRsAdded_HMRtoAR = true
			MergeNamingRules(pNamingRulesUpdate_HMRtoAR)
		endIf
	endIf
	
	if at_glob_ConfigNaming_AddRevolver.GetValue() > 0.0
		if !bINRsAdded_Revolver
			iCount += 1
			bINRsAdded_Revolver = true
			MergeNamingRules(pNamingRulesUpdate_Revolver)
		endIf
	endIf
	
	if at_glob_ConfigNaming_AddRepeater.GetValue() > 0.0
		if !bINRsAdded_Repeater
			iCount += 1
			bINRsAdded_Repeater = true
			MergeNamingRules(pNamingRulesUpdate_Repeater)
		endIf
	endIf
	
	if at_glob_ConfigNaming_AddLegendaryNames.GetValue() > 0.0
		if !bINRsAdded_Legendary
			iCount += 1
			bINRsAdded_Legendary = true
			MergeNamingRules(pNamingRulesUpdate_LegendaryGuns)
			MergeNamingRules(pNamingRulesUpdate_LegendaryMelee)
			MergeNamingRules(pNamingRulesUpdate_LegendaryArmor)
		endIf
	endIf
	
	at_mesg_Config_UpdatedNamingRules.Show(iCount)
EndFunction



; Leveled Lists

Function AddLeveledLists(LListUpdate[] tempList)
	LeveledItem tempLI = none
	int i = 0
	if tempList.length > 0
		while i < tempList.length
			tempLI = tempList[i].liItemBase
			if tempLI as bool
				tempLI.AddForm(tempList[i].liItemAdd, tempList[i].iItemAdd_Level, tempList[i].iItemAdd_Count)
			endIf
			i += 1
		endWhile
	endIf
EndFunction


; basic ammo/item leveled lists
Function UpdateLeveledLists()
	int iCount = 0
	
	if at_glob_ConfigItems_AddMoreSupermutantWpns.GetValue() > 0.0
		if !bItemsAdded_Supermutants
			iCount += 1
			bItemsAdded_Supermutants = true
			AddLeveledLists(pLLUpdate_SupermutantWeaps)
		endIf
	endIf
	
	if at_glob_ConfigItems_AddRepairKits.GetValue() > 0.0
		if !bItemsAdded_RepairKits
			iCount += 1
			bItemsAdded_RepairKits = true
			AddLeveledLists(pLLUpdate_WeapRepairKits)
		endIf
	endIf
	
	if at_glob_ConfigItems_AddVendorShipments.GetValue() > 0.0
		if !bItemsAdded_Shipments
			iCount += 1
			bItemsAdded_Shipments = true
			AddLeveledLists(pLLUpdate_VendorShipments)
		endIf
	endIf
	
	if at_glob_ConfigItems_AddLootComponents.GetValue() > 0.0
		if !bItemsAdded_Components
			iCount += 1
			bItemsAdded_Components = true
			AddLeveledLists(pLLUpdate_LootCompos)
		endIf
	endIf
	
	at_mesg_Config_UpdatedLeveledLists.Show(iCount)
EndFunction


; Scrounger perk ammo lists
Function UpdateScroungerLists()
	int iCount = 0
	LeveledItem tempLI = none
	
	if at_glob_ConfigScrounger_223Cal.GetValue() > 0.0
		if !bItemsAdded_Scrounger223Cal
			iCount += 1
			bItemsAdded_Scrounger223Cal = true
			AddLeveledLists(pLLUpdate_Scrounger_223Cal)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_22LR.GetValue() > 0.0
		if !bItemsAdded_Scrounger22LR
			iCount += 1
			bItemsAdded_Scrounger22LR = true
			AddLeveledLists(pLLUpdate_Scrounger_22LR)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_3006.GetValue() > 0.0
		if !bItemsAdded_Scrounger3006
			iCount += 1
			bItemsAdded_Scrounger3006 = true
			AddLeveledLists(pLLUpdate_Scrounger_3006)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_300WM.GetValue() > 0.0
		if !bItemsAdded_Scrounger300WM
			iCount += 1
			bItemsAdded_Scrounger300WM = true
			AddLeveledLists(pLLUpdate_Scrounger_300WM)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_30Carbine.GetValue() > 0.0
		if !bItemsAdded_Scrounger30Carbine
			iCount += 1
			bItemsAdded_Scrounger30Carbine = true
			AddLeveledLists(pLLUpdate_Scrounger_30Carbine)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_32ACP.GetValue() > 0.0
		if !bItemsAdded_Scrounger32ACP
			iCount += 1
			bItemsAdded_Scrounger32ACP = true
			AddLeveledLists(pLLUpdate_Scrounger_32ACP)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_338Mag.GetValue() > 0.0
		if !bItemsAdded_Scrounger338Mag
			iCount += 1
			bItemsAdded_Scrounger338Mag = true
			AddLeveledLists(pLLUpdate_Scrounger_338Mag)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_357Mag.GetValue() > 0.0
		if !bItemsAdded_Scrounger357Mag
			iCount += 1
			bItemsAdded_Scrounger357Mag = true
			AddLeveledLists(pLLUpdate_Scrounger_357Mag)
		endIf
	endIf
	
	if at_glob_ConfigScrounger_38Spc.GetValue() > 0.0
		if !bItemsAdded_Scrounger38Spc
			iCount += 1
			bItemsAdded_Scrounger38Spc = true
			AddLeveledLists(pLLUpdate_Scrounger_38Spc)
		endIf
	endIf
	
	at_mesg_Config_UpdatedLeveledLists.Show(iCount)
EndFunction


; Legendary Items

Function UpdateLegendaryItems()
	int iCount = 0
	
	if at_glob_ConfigLegendary_AddDurable.GetValue() > 0.0
		if !bLegItemAdded_Durable
			iCount += 1
			bLegItemAdded_Durable = true
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRule_Durable)
		endIf
	endIf
	
	if at_glob_ConfigLegendary_AddUnyielding.GetValue() > 0.0
		if !bLegItemAdded_Unyielding
			iCount += 1
			bLegItemAdded_Unyielding = true
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRule_Unyielding)
		endIf
	endIf
	
	if at_glob_ConfigLegendary_AddLucky.GetValue() > 0.0
		if !bLegItemAdded_Lucky
			iCount += 1
			bLegItemAdded_Lucky = true
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRule_Lucky)
		endIf
	endIf
	
	if at_glob_ConfigLegendary_AddElemental.GetValue() > 0.0
		if !bLegItemAdded_Elemental
			iCount += 1
			bLegItemAdded_Elemental = true
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRule_Elemental)
		endIf
	endIf
	
	if at_glob_ConfigLegendary_AddShocking.GetValue() > 0.0
		if !bLegItemAdded_Shocking
			iCount += 1
			bLegItemAdded_Shocking = true
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRule_Shocking)
		endIf
	endIf
	
	at_mesg_Config_UpdatedLegendaryItems.Show(iCount)
EndFunction


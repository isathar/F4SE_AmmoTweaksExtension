Scriptname AT:Managers:UpdateManager extends Quest
{handles any needed updates to FormLists, LeveledItems, legendary items, and/or globals}


Struct FormListUpdate
	string 		sName = 				" "
	{editor-only name}
	FormList	pListBase = 			none
	{base LI's formID}
	FormList 	pListAdd = 				none
	{leveled item to add to the list}
EndStruct

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

; holds globals to update and their new values
Struct UpdateGlobal
	GlobalVariable 	pGlobal = 	none
	{the global to update}
	float 			fNewValue = 0.0
	{the global's new value}
EndStruct


Group UpdateCheck
	float Property 					fNewVersion = 2.0					auto Const
	{the new version to update to if required}
	float Property 					fUpdateDelaySeconds = 1.0			auto Const
	{time to wait before running}
	
	GlobalVariable Property 		at_glob_CurModVersion 				auto Const mandatory
	{the currently saved version number}
	Message Property 				pUpdateMessage						auto Const
	{message to show on completion}
	bool Property 					bSkipVersionCheck = false			auto Const
	{whether to skip the version number check/set}
	
	
EndGroup

Group UpdateData
{required FormList modifications for this update}
	FormListUpdate[] Property 			pFormListUpdates	auto Const
	{formlists to add (to)}
	
	LListUpdate[] Property 			pLLUpdates			auto Const
	{reachable leveledlists to add reachable forms to}
	
	UpdateGlobal[] Property 			pGlobalsUpdate 		auto Const
	{GlobalVariables that need to be set}
	
	LegendaryItemQuestScript Property 						LegendaryItemQuest 	auto const mandatory
	{link to the legendary item quest}
	LegendaryItemQuestScript:LegendaryModRule[] Property 	LegendaryModRules 	auto const
	{legendary mod rules to add}
	LegendaryItemQuestScript:AmmoDatum[] Property 			AmmoData 			auto const
	{ammo types to add to legendary items}
	
	NamingRulesUpdate[] Property 		pNamingRulesUpdates	auto Const
	{naming rules additions}
EndGroup




Event OnQuestInit()
	utility.WaitMenuMode(fUpdateDelaySeconds)
	; mod version check
	if !bSkipVersionCheck
		float fOldVersion = at_glob_CurModVersion.GetValue()
		if fOldVersion < fNewVersion
			at_glob_CurModVersion.SetValue(fNewVersion)
			RunUpdate()
		endIf
	else
		RunUpdate()
	endIf
EndEvent


Function RunUpdate()
	UpdateGlobals()
	UpdateFormLists()
	UpdateLeveledItems()
	UpdateLegendaryItems()
	MergeNamingRules()
	utility.WaitMenuMode(0.1)
	if pUpdateMessage != none
		pUpdateMessage.Show(fNewVersion)
	endIf
	Stop()
EndFunction


;/ ******** FormList Updates ********** /;

; GlobalVariables
Function UpdateGlobals()
	int i = 0
	int iCount = pGlobalsUpdate.length
	GlobalVariable tempGlobal
	float fNewVal
	if iCount > 0
		while (i < iCount)
			tempGlobal = pGlobalsUpdate[i].pGlobal
			fNewVal = pGlobalsUpdate[i].fNewValue
			tempGlobal.SetValue(fNewVal)
			i += 1
		endwhile
	endIf
EndFunction


; FormLists
Function UpdateFormLists()
	int i = 0
	int j = 0
	int iNumBase = pFormListUpdates.length
	int iNumAdd = 0
	FormList flAdd = none
	FormList flBase = none
	Form formAdd = none
	
	if iNumBase > 0
		while i < iNumBase
			j = 0
			flBase = pFormListUpdates[i].pListBase
			flAdd = pFormListUpdates[i].pListAdd
			iNumAdd = flAdd.GetSize()
			while j < iNumAdd
				formAdd = flAdd.GetAt(j)
				if flBase.Find(formAdd) < 0
					flBase.AddForm(formAdd)
				endIf
				j += 1
			endWhile
			i += 1
		endWhile
	endIf
EndFunction


; LeveledItems
Function UpdateLeveledItems()
	
EndFunction


; Legendary items
; - from DLCXX:DLCAddLegendaryItemQuestScript
Function UpdateLegendaryItems()
	int i = 0
	int len = LegendaryModRules.length
	if len > 0
		while (i < len)
			;add our current item to the main game quest's array
			LegendaryItemQuest.LegendaryModRules.add(LegendaryModRules[i])
			i += 1
		endwhile
	endIf
	
	len = AmmoData.length
	if len > 0
		i = 0
		while (i < len)
			;add our current item to the main game quest's array
			LegendaryItemQuest.AmmoData.add(AmmoData[i])
			i += 1
		endwhile
	endIf
EndFunction


;/ ******** InstanceNamingRules Updates ********** /;

;/ updates instance naming rules to include weapon name additions /;
Function MergeNamingRules()
	InstanceNamingRules BaseRules = none
	InstanceNamingRules AddRules = none
	int counter = 0
	int i = 0
	NamingRulesUpdate tempStruct = new NamingRulesUpdate
	
	while i < pNamingRulesUpdates.length
		tempStruct = pNamingRulesUpdates[i]
		BaseRules = tempStruct.pNamesBase
		AddRules = tempStruct.pNamesAdd
		BaseRules.MergeWith(AddRules)
		i += 1
	endWhile
	
EndFunction

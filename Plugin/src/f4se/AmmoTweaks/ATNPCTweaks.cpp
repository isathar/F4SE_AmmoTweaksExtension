#include "f4se/PapyrusNativeFunctions.h"

#include "ATNPCTweaks.h"



#define SCRIPTNAME "AmmoTweaks:ATGlobals"


namespace NPCTweaks
{

	void LogSpellList(StaticFunctionTag*, bool bAddSpells)
	{
		PlayerCharacter *tempplayer = *g_player;

		TESRace *tempRace = tempplayer->GetActorRace();
		
		if (&tempRace->spellList) {
			ATTESSpellList* tempSpellList = (ATTESSpellList*)&tempRace->spellList;
			ATSpellListEntries *spellData = tempSpellList->unk08;
			if (spellData) {
				UInt32 iNumSpells = spellData->numSpells;

				// add some test spells
				if (bAddSpells) {
					
					_MESSAGE("\n...adding test spells...");
					_MESSAGE("\n        TESRace size: %i, tempRace size: %i", sizeof(TESRace), sizeof(*tempRace));

					SpellItem ** newSpells = new SpellItem*[iNumSpells + 4];
					for (UInt32 i = 0; i < iNumSpells; i++)
						newSpells[i] = (spellData->spells[i]);

					newSpells[iNumSpells] = (SpellItem*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|0x002F10");
					newSpells[iNumSpells + 1] = (SpellItem*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|0x002F12");
					newSpells[iNumSpells + 2] = (SpellItem*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|0x002F16");
					newSpells[iNumSpells + 3] = (SpellItem*)ATShared::GetFormFromIdentifier("AmmoTweaks.esm|0x002F14");

					iNumSpells = iNumSpells + 4;

					spellData->numSpells = iNumSpells;
					spellData->spells = new SpellItem*[iNumSpells];

					for (UInt32 i = 0; i < iNumSpells; i++)
						spellData->spells[i] = (newSpells[i]);

					delete[] newSpells;

					_MESSAGE("\n        TESRace size: %i, tempRace size: %i", sizeof(TESRace), sizeof(*tempRace));

					ATConfig::SaveGameDataToINI();
					
				}
				else
					ATConfig::LoadGameDataFromINI();


				_MESSAGE("\nSpellData: 0x%X", *spellData);
				_MESSAGE("      count  (0C)=  0x%08X", iNumSpells);
				_MESSAGE("      spells (00)=  0x%08X", spellData->spells);
				SpellItem *tempSpellX = nullptr;
				for (UInt32 i = 0; i < iNumSpells; i++) {
					tempSpellX = (spellData->spells[i]);
					_MESSAGE("          spell %i = %s", i, ATShared::GetIdentifierFromFormID(tempSpellX->formID));
				}
			}
		}
	}



}

bool ATNPCTweaks::RegisterPapyrus(VirtualMachine * vm)
{
	NPCTweaks::RegisterFuncs(vm);
	_MESSAGE("Registered ATNPCTweaks native functions.");
	return true;
}

void NPCTweaks::RegisterFuncs(VirtualMachine* vm)
{
	vm->RegisterFunction(
		new NativeFunction1 <StaticFunctionTag, void, bool>("LogSpellList", SCRIPTNAME, NPCTweaks::LogSpellList, vm));

}

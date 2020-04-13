#include <shlobj.h>
#include "f4se/PluginAPI.h"
#include "f4se_common/f4se_version.h"
#include "Config.h"
#include "ATShared.h"



IDebugLog				gLog;
PluginHandle			g_pluginHandle =	kPluginHandle_Invalid;
F4SEPapyrusInterface	* g_papyrus   =		NULL;
F4SEMessagingInterface	* g_messaging =		NULL;
F4SEScaleformInterface	* g_scaleform =		NULL;


class ATCombatEventSink : public BSTEventSink<TESCombatEvent>
{
public:
	virtual	EventResult	ReceiveEvent(TESCombatEvent * evn, void * dispatcher)
	{
		if (!evn || !evn->source) {
			return kEvent_Continue;
		}
		if (evn->target) {
			switch (evn->state) {
				case 0:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) stopped combat with %s (0x%08X)", evn->source->baseForm->GetFullName(), evn->source->formID, evn->target->baseForm->GetFullName(), evn->target->formID);
					break;
				case 1:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) started combat with %s (0x%08X)", evn->source->baseForm->GetFullName(), evn->source->formID, evn->target->baseForm->GetFullName(), evn->target->formID);
					break;
				case 2:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) is searching for %s (0x%08X)", evn->source->baseForm->GetFullName(), evn->source->formID, evn->target->baseForm->GetFullName(), evn->target->formID);
					break;
				default:
					_MESSAGE("ATCombatEventSink: undefined event - source: %s, target: %s, state: 0x%08X", evn->source->baseForm->GetFullName(), evn->target->baseForm->GetFullName(), evn->state);
			}
		}
		else {
			switch (evn->state) {
				case 0:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) is stopping combat", evn->source->baseForm->GetFullName(), evn->source->formID);
					break;
				case 1:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) is starting combat", evn->source->baseForm->GetFullName(), evn->source->formID);
					break;
				case 2:
					_MESSAGE("ATCombatEventSink: %s (0x%08X) is searching", evn->source->baseForm->GetFullName(), evn->source->formID);
					break;
				default:
					_MESSAGE("ATCombatEventSink: undefined event - source: %s, state: 0x%08X", evn->source->baseForm->GetFullName(), evn->state);
			}
		}
		return kEvent_Continue;
	};
};

ATCombatEventSink g_combatSink;


void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady) {
		if (msg->data) {
			// load data from config files
			if (ATConfigReader::LoadGameData()) {
				// register menus
				ATHUD::RegisterMenus();
				GetEventDispatcher<TESCombatEvent>()->AddEventSink(&g_combatSink);
			}
		}
	}
}



extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
	gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\AmmoTweaks.log");

	info->infoVersion = PluginInfo::kInfoVersion;
	info->name =		PLUGIN_NAME_SHORT;
	info->version =		PLUGIN_VERSION;
	
	// version check
	if (f4se->runtimeVersion != SUPPORTED_RUNTIME_VERSION) {
		char buf[512];
		sprintf_s(buf, "AmmoTweaks does not work with the installed game version!\n\nExpected: %d.%d.%d.%d\nCurrent:  %d.%d.%d.%d",
			GET_EXE_VERSION_MAJOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_MINOR(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_BUILD(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_SUB(SUPPORTED_RUNTIME_VERSION),
			GET_EXE_VERSION_MAJOR(f4se->runtimeVersion),
			GET_EXE_VERSION_MINOR(f4se->runtimeVersion),
			GET_EXE_VERSION_BUILD(f4se->runtimeVersion),
			GET_EXE_VERSION_SUB(f4se->runtimeVersion));
		MessageBox(NULL, buf, "Game Version Error", MB_OK | MB_ICONEXCLAMATION);
		_FATALERROR("ERROR: Game version mismatch");
		return false;
	}
	
	g_pluginHandle = f4se->GetPluginHandle();
	g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
	if (!g_messaging) {
		_FATALERROR("ERROR: Messaging query failed");
		return false;
	}
	g_scaleform = (F4SEScaleformInterface *)f4se->QueryInterface(kInterface_Scaleform);
	if (!g_scaleform) {
		_FATALERROR("ERROR: Scaleform query failed");
		return false;
	}
	g_papyrus = (F4SEPapyrusInterface *)f4se->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus) {
		_FATALERROR("ERROR: Papyrus query failed");
		return false;
	}
	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
	if (g_messaging) {
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);
	}
	if (g_scaleform) {
		g_scaleform->Register("AmmoTweaks", ATHUD::RegisterScaleform);
	}
	if (g_papyrus) {
		g_papyrus->Register(ATGlobals::RegisterPapyrus);
	}
	_MESSAGE("%s v%s initialized...\n", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);
    return true;
}

}

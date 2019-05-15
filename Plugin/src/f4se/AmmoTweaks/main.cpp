#include <shlobj.h>
#include "f4se/PluginAPI.h"
#include "Config.h"
#include "ATShared.h"


IDebugLog				gLog;
PluginHandle			g_pluginHandle =	kPluginHandle_Invalid;
F4SEPapyrusInterface	*g_papyrus   =		NULL;
F4SEMessagingInterface	*g_messaging =		NULL;


void F4SEMessageHandler(F4SEMessagingInterface::Message* msg)
{
	if (msg->type == F4SEMessagingInterface::kMessage_GameDataReady) {
		if (msg->data == (void*)true) {
			// load extra data from config files
			ATConfig::LoadGameData();
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
	
	if (f4se->runtimeVersion != RUNTIME_VERSION_1_10_130) {
		_MESSAGE("Aborting - Game version mismatch");
		return false;
	}
	
	g_pluginHandle =	f4se->GetPluginHandle();

	g_messaging = (F4SEMessagingInterface *)f4se->QueryInterface(kInterface_Messaging);
	if (!g_messaging) {
		_MESSAGE("Aborting - Messaging query failed");
		return false;
	}

	g_papyrus = (F4SEPapyrusInterface *)f4se->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus) {
		_MESSAGE("Aborting - Papyrus query failed");
		return false;
	}

	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
	if (g_messaging)
		g_messaging->RegisterListener(g_pluginHandle, "F4SE", F4SEMessageHandler);
	
	if (g_papyrus)
		g_papyrus->Register(ATShared::RegisterPapyrus);
	
	_MESSAGE("%s v%s is loading...", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);
	
    return true;
}

}

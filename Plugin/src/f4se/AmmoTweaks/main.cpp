#include <shlobj.h>
#include "../f4se/PluginAPI.h"

#include "../f4se/PapyrusVM.h"
#include "../f4se/PapyrusNativeFunctions.h"

#include "Config.h"
#include "PapyrusAT.h"


IDebugLog gLog;
PluginHandle g_pluginHandle = kPluginHandle_Invalid;
F4SEPapyrusInterface   *g_papyrus   = NULL;


extern "C"
{

bool F4SEPlugin_Query(const F4SEInterface * f4se, PluginInfo * info)
{
    gLog.OpenRelative(CSIDL_MYDOCUMENTS, "\\My Games\\Fallout4\\F4SE\\AmmoTweaks.log");
    _MESSAGE("initializing %s v%s...", PLUGIN_NAME_SHORT, PLUGIN_VERSION_STRING);
    
    // populate the info structure
    info->infoVersion = PluginInfo::kInfoVersion;
    info->name =		PLUGIN_NAME_SHORT;
    info->version =		PLUGIN_VERSION;

	g_pluginHandle =	f4se->GetPluginHandle();

	g_papyrus = (F4SEPapyrusInterface *)f4se->QueryInterface(kInterface_Papyrus);
	if (!g_papyrus) {
		_MESSAGE("Aborting - Papyrus query failed");
		return false;
	}

	return true;
}

bool F4SEPlugin_Load(const F4SEInterface *f4se)
{
    _MESSAGE("%s loaded", PLUGIN_NAME_SHORT);

	g_papyrus->Register(PapyrusAmmoTweaks::RegisterPapyrus);

    return true;
}

}
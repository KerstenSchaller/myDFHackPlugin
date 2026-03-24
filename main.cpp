
#include "PluginManager.h"
#include "DataDefs.h"

#include "modules/EventManager.h"
#include "DFDataLogger.hpp"

#include <vector>
#include <set>

using namespace DFHack;
using namespace std;

DFHACK_PLUGIN("FortressChronicle");

//bool interposed = false;

/* Command Definition for DFHack Console */
DFhackCExport command_result plugin_init(color_ostream &out, std::vector<PluginCommand> &commands) 
{
    setPluginSelf(plugin_self);
    commands.push_back(PluginCommand("startChronicle", "Sets up a few event triggers.", setupLogging));
    return CR_OK;
}



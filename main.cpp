
#include "PluginManager.h"
#include "DataDefs.h"

#include "modules/EventManager.h"
#include "DFDataLogger.hpp"

#include "df/plotinfost.h"
#include "df/world_site.h"
#include "df/world_data.h"
#include "df/world.h"
#include "modules/Translation.h"

using namespace DFHack;
using namespace std;

DFHACK_PLUGIN("FortressChronicle");

// get some meta info
//auto fortressName = DF2UTF(DFHack::Translation::translateName(&df::global::plotinfo->main.fortress_site->name, true));
//auto worldName = DF2UTF(DFHack::Translation::translateName(&df::global::world->world_data->name, true));



/* Command Definition for DFHack Console */
DFhackCExport command_result plugin_init(color_ostream &out, std::vector<PluginCommand> &commands) 
{
    DataLogger::Parameters params;
    params.plugin = plugin_self;
    //params.dbName = "dfhack-config/df_chronicle/" + worldName + "_" + fortressName + ".db";
    params.dbName = "myDatabase.db";
    DataLogger::setParams(params);

    commands.push_back(PluginCommand("startChronicle", "Sets up logging to database", DataLogger::setupLogging));
    return CR_OK;
}


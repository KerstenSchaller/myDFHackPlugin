#include <type_traits>

//#include "Debug.h"
#include "LuaTools.h"
//#include "PluginManager.h"
#include "PluginLua.h"
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

DFHACK_PLUGIN("fortress-chronicle");

// get some meta info
//auto fortressName = DF2UTF(DFHack::Translation::translateName(&df::global::plotinfo->main.fortress_site->name, true));
//auto worldName = DF2UTF(DFHack::Translation::translateName(&df::global::world->world_data->name, true));




std::vector<UnitRecord> getNewCitizens(int32_t year)
{
    return DataLogger::getNewCitizens(year);
}

std::vector<JobRecord> getJobsDone(int32_t year)
{
    return DataLogger::getJobsDone(year);
}

command_result testGetNewCitizens(color_ostream& out, std::vector<std::string>& parameters) {
    auto newCitizens = getNewCitizens(110);
    out.print("New citizens in year {}: {}\n", 110, newCitizens.size());
    for (const auto& citizen : newCitizens) 
    {
        out.print(" - Unit ID: {}, Name: {}, Age: {}, Profession: {}\n",
            citizen.unit_id, citizen.name, citizen.age, citizen.profession);
     }
    return CR_OK;
}

command_result testGetJobsDone(color_ostream& out, std::vector<std::string>& parameters) {
    auto jobsDone = getJobsDone(110);
    out.print("Jobs done in year {}: {}\n", 110, jobsDone.size());
    for (const auto& job : jobsDone) 
    {
        out.print(" - Event ID: {}, Unit ID: {}, Job: {}, Material: {}\n",
            job.event_id, job.unit_tid, job.job_name, job.material);
    }
    return CR_OK;
}


 

/* Command Definition for DFHack Console */
DFhackCExport command_result plugin_init(color_ostream &out, std::vector<PluginCommand> &commands) 
{
    DataLogger::Parameters params;
    params.plugin = plugin_self;
    //params.dbName = "dfhack-config/df_chronicle/" + worldName + "_" + fortressName + ".db";
    params.dbName = "myDatabase.db";
    DataLogger::setParams(params);

    commands.push_back(PluginCommand("startChronicle", "Sets up logging to database", DataLogger::setupLogging));
    commands.push_back(PluginCommand("getNewCitizens", "Gets new citizens for a given year", testGetNewCitizens));
    commands.push_back(PluginCommand("getJobsDone", "Gets completed jobs for a given year", testGetJobsDone));
    return CR_OK;
}

int pluginTest()
{
    return 15;
}

struct pluginTestStruct {
    int value;
    string name;
};

static int pluginTest2(lua_State *L)
{
    pluginTestStruct s{15, "Test"};

    lua_createtable(L, 0, 2);                 // result table
    DFHack::Lua::SetField(L, s.value, -1, "value");
    DFHack::Lua::SetField(L, s.name,  -1, "name");
    return 1;
}


DFHACK_PLUGIN_LUA_FUNCTIONS {
    DFHACK_LUA_FUNCTION(pluginTest),
    DFHACK_LUA_END
};

DFHACK_PLUGIN_LUA_COMMANDS {
    DFHACK_LUA_COMMAND(pluginTest2),
    DFHACK_LUA_END
};



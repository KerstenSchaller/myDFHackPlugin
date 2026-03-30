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

command_result testGetNewCitizens(color_ostream& out, std::vector<std::string>& parameters) 
{
    auto newCitizens = getNewCitizens(110);
    out.print("New citizens in year {}: {}\n", 110, newCitizens.size());
    for (const auto& citizen : newCitizens) 
    {
        out.print(" - Unit ID: {}, Name: {}, Age: {}, Profession: {}\n",
            citizen.unit_id, citizen.name, citizen.age, citizen.profession);
     }
    return CR_OK;
}

command_result testGetJobsDone(color_ostream& out, std::vector<std::string>& parameters) 
{
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
    return 11;
}

struct pluginTestStruct
{
    int value;
    std::string name;
};

static int pluginTest2(lua_State *L)
{
    pluginTestStruct s{15, "Test"};

    lua_createtable(L, 0, 2); // result table
    DFHack::Lua::SetField(L, s.value, -1, "value");
    DFHack::Lua::SetField(L, s.name,  -1, "name");
    return 1;
}



// generic function to get data from Lua
// returns a table of tables, where each inner table represents a token
// tokens are used by the dfhack gui lua to encode fg color, bg color, text, gap, hasLinebreak
static int getTokenizedData(lua_State *L, std::vector<string> textData, int32_t fgColor = 7, int32_t bgColor = 0, int32_t gap = 0, bool hasLinebreak = false)
{
    // convert to Lua table
    lua_createtable(L, (int)textData.size(), 0);
    int i = 1;
    for (const auto &_text : textData) {
        lua_createtable(L, 0, 4);
        DFHack::Lua::SetField(L, fgColor,    -1, "fgColor");
        DFHack::Lua::SetField(L, bgColor,       -1, "bgColor");
        DFHack::Lua::SetField(L, _text,        -1, "text");
        DFHack::Lua::SetField(L, gap, -1, "gap");
        DFHack::Lua::SetField(L, hasLinebreak, -1, "hasLinebreak");
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

// get new citizens as tokens
    static int getNewCitizens(lua_State *L)
{
    int32_t year = (int32_t)luaL_checkinteger(L, 1);
    auto newCitizens = getNewCitizens(year);
    vector<string> textData;
    for (const auto& citizen : newCitizens) 
    {
        textData.push_back("Unit ID: " + to_string(citizen.unit_id) + ", Name: " + citizen.name );
    }
    

    // convert to Lua table
    getTokenizedData(L, textData); // pass textData to getTokenizedData

    return 1;
}

// get years as a plain Lua array of integers
static int getUniqueYears(lua_State *L)
{
    auto years = DataLogger::getUniqueYears();
    // Return a plain Lua array of integers.
    lua_createtable(L, (int)years.size(), 0);
    int i = 1;
    for (const auto &year : years) {
        lua_pushinteger(L, year);
        lua_rawseti(L, -2, i++);
    }
    return 1;
}

static int getCitizensDied(lua_State *L)
{
    int32_t year = (int32_t)luaL_checkinteger(L, 1);
    auto citizenDeaths = DataLogger::getCitizenDeaths(year);
    vector<string> textData;
    for (const auto& death : citizenDeaths) 
    {
        std::string formatStr = fmt::format("{}, a {} year old {} {}, died of {}.", death.victim.name, death.victim.age, death.victim.race, death.victim.profession, death.death_cause);
        textData.push_back(formatStr);
    }

    // convert to Lua table
    getTokenizedData(L, textData); // pass textData to getTokenizedData

    return 1;
}

static int getPopulationData(lua_State *L)
{
    int32_t year = (int32_t)luaL_checkinteger(L, 1);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    // based on the index different types of population data can be returned, for example:
    switch(index) {
        // get new citizens
        case 0: 
        {
            getNewCitizens(L);
            break;
        }
        case 1:
        {
            getCitizensDied(L);
            break;
        }
    }
    return 1;
}

DFHACK_PLUGIN_LUA_FUNCTIONS {
    DFHACK_LUA_FUNCTION(pluginTest),
    DFHACK_LUA_END
};

DFHACK_PLUGIN_LUA_COMMANDS {
    DFHACK_LUA_COMMAND(pluginTest2),
    DFHACK_LUA_COMMAND(getPopulationData),
    DFHACK_LUA_COMMAND(getUniqueYears),
    DFHACK_LUA_END
};



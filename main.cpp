#include <type_traits>

//#include "Debug.h"
#include "LuaTools.h"
//#include "PluginManager.h"
#include "PluginLua.h"
#include "DataDefs.h"

#include "modules/EventManager.h"

#include "df/plotinfost.h"
#include "df/world_site.h"
#include "df/world_data.h"
#include "df/world.h"
#include "modules/Translation.h"

#include "DFDataLogger.hpp"
#include "test.hpp"

using namespace DFHack;
using namespace std;

DFHACK_PLUGIN("fortress-chronicle");


// is called by DFHack when the plugin is loaded, used to set up commands and initialize logging
DFhackCExport command_result plugin_onstatechange(color_ostream &out, state_change_event event) 
{
    switch (event) 
    {
        case DFHack::SC_MAP_LOADED:
            DataLogger::setupLogging();
            break;
        case DFHack::SC_MAP_UNLOADED:
            EventManager::unregisterAll(plugin_self);
            break;
        default:
             break;
    }
    return CR_OK;
}

std::vector<UnitRecord> getNewCitizens(int32_t year)
{
    return DataLogger::getNewCitizens(year);
}

std::vector<JobRecord> getJobsDone(int32_t year)
{
    return DataLogger::getJobsDone(year);
}




 

/* Command Definition for DFHack Console */
DFhackCExport command_result plugin_init(color_ostream &out, std::vector<PluginCommand> &commands) 
{
    DataLogger::Parameters params;
    params.plugin = plugin_self;
    //params.dbName = "dfhack-config/df_chronicle/" + worldName + "_" + fortressName + ".db";
    params.dbName = "myDatabase.db";
    DataLogger::setParams(params);

    //commands.push_back(PluginCommand("startChronicle", "Sets up logging to database", DataLogger::setupLogging));

    return CR_OK;
}





// generic function to get data from Lua
// returns a table of tables, where each inner table represents a token
// tokens are used by the dfhack gui lua to encode fg color, bg color, text, gap, hasLinebreak
static int getTokenizedData(lua_State *L, std::vector<string> textData, int32_t fgColor = 7, int32_t bgColor = 0, int32_t gap = 0, bool hasLinebreak = false)
{
    // convert to Lua table
    lua_createtable(L, (int)textData.size(), 0);
    int i = 1;
    for (const auto &_text : textData) 
    {
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
static int getUnits(lua_State *L, const std::vector<UnitRecord>& newCitizens)
{
    int32_t year = (int32_t)luaL_checkinteger(L, 1);
    lua_createtable(L, (int)newCitizens.size(), 0);
    vector<string> textData;
    int index = 1;
    for (const auto& citizen : newCitizens) 
    {
        textData.push_back("Unit ID: " + to_string(citizen.unit_id) + ", Name: " + citizen.name );
        // add to lua table
        lua_createtable(L, 0, 5);
        DFHack::Lua::SetField(L, citizen.name, -1, "name");
        DFHack::Lua::SetField(L, citizen.name_english, -1, "english_name");
        DFHack::Lua::SetField(L, citizen.age, -1, "age");
        DFHack::Lua::SetField(L, citizen.sex, -1, "sex");
        DFHack::Lua::SetField(L, citizen.race, -1, "race");
        DFHack::Lua::SetField(L, citizen.profession, -1, "profession");
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

static int getDeathData(lua_State *L, const std::vector<DataLogger::unitDeathInfo>& deathInfos)
{
    lua_createtable(L, (int)deathInfos.size(), 0);
    int index = 1;
    for (const auto& deathInfo : deathInfos) 
    {
        // add to lua table
        lua_createtable(L, 0, 10);
        DFHack::Lua::SetField(L, deathInfo.victim.name, -1, "victim_name");
        DFHack::Lua::SetField(L, deathInfo.victim.age, -1, "victim_age");
        DFHack::Lua::SetField(L, deathInfo.victim.sex, -1, "victim_sex");
        DFHack::Lua::SetField(L, deathInfo.victim.race, -1, "victim_race");
        DFHack::Lua::SetField(L, deathInfo.victim.profession, -1, "victim_profession");
        DFHack::Lua::SetField(L, deathInfo.death_cause, -1, "cause_of_death");
        DFHack::Lua::SetField(L, deathInfo.killer.name, -1, "killer_name");
        DFHack::Lua::SetField(L, deathInfo.killer.age, -1, "killer_age");
        DFHack::Lua::SetField(L, deathInfo.killer.sex, -1, "killer_sex");
        DFHack::Lua::SetField(L, deathInfo.killer.race, -1, "killer_race");
        DFHack::Lua::SetField(L, deathInfo.killer.profession, -1, "killer_profession");
        lua_rawseti(L, -2, index++);
    }
    return 1;
}

static int getBirthData(lua_State *L, const std::vector<DataLogger::unitBirthInfo>& birthInfos)
{
    lua_createtable(L, (int)birthInfos.size(), 0);
    int index = 1;
    for (const auto& birthInfo : birthInfos) 
    {
        // add to lua table
        lua_createtable(L, 0, 15);
        DFHack::Lua::SetField(L, birthInfo.newborn.name, -1, "newborn_name");
        DFHack::Lua::SetField(L, birthInfo.newborn.age, -1, "newborn_age");
        DFHack::Lua::SetField(L, birthInfo.newborn.sex, -1, "newborn_sex");
        DFHack::Lua::SetField(L, birthInfo.newborn.race, -1, "newborn_race");
        DFHack::Lua::SetField(L, birthInfo.mother.name, -1, "mother_name");
        DFHack::Lua::SetField(L, birthInfo.mother.age, -1, "mother_age");
        DFHack::Lua::SetField(L, birthInfo.mother.sex, -1, "mother_sex");
        DFHack::Lua::SetField(L, birthInfo.mother.race, -1, "mother_race");
        DFHack::Lua::SetField(L, birthInfo.mother.profession, -1, "mother_profession");
        DFHack::Lua::SetField(L, birthInfo.father.name, -1, "father_name");
        DFHack::Lua::SetField(L, birthInfo.father.age, -1, "father_age");
        DFHack::Lua::SetField(L, birthInfo.father.sex, -1, "father_sex");
        DFHack::Lua::SetField(L, birthInfo.father.race, -1, "father_race");
        DFHack::Lua::SetField(L, birthInfo.father.profession, -1, "father_profession");
        lua_rawseti(L, -2, index++);
    }
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



static int getPopulationData(lua_State *L)
{
    int32_t year = (int32_t)luaL_checkinteger(L, 1);
    int32_t index = (int32_t)luaL_checkinteger(L, 2);

    // based on the index different types of population data can be returned, for example:
    switch(index) {
        // get new citizens
        case 0: 
        {
            Logger::log("Getting new citizens for year: " + to_string(year));
            getUnits(L, DataLogger::getNewCitizens(year));
            break;
        }
        case 1:
        {
            Logger::log("Getting citizen deaths for year: " + to_string(year));
            getDeathData(L, DataLogger::getCitizenDeaths(year));
            break;
        }
        case 2:
        {
            Logger::log("Getting citizen births for year: " + to_string(year));
            getBirthData(L, DataLogger::getCitizenBirths(year));
            break;
        }
    }
    return 1;
}

DFHACK_PLUGIN_LUA_FUNCTIONS 
{
    DFHACK_LUA_END
};

DFHACK_PLUGIN_LUA_COMMANDS {
    DFHACK_LUA_COMMAND(getPopulationData),
    DFHACK_LUA_COMMAND(getUniqueYears),
    DFHACK_LUA_END
};



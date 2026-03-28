#pragma once
#include <vector>

#include "DataDefs.h"
#include "PluginManager.h"
#include "modules/EventManager.h"

#include "modules/World.h"
#include "modules/Job.h"




#include "SQLiteWrapper.hpp"
#include "Events.hpp"
#include "Jobs.hpp"
#include "Units.hpp"
#include "Items.hpp"
#include "Deaths.hpp"

#include "DateAndTime.hpp"

namespace DataLogger{

struct Parameters
{
    DFHack::Plugin* plugin;
    std::string dbName;
};




/* Register events */
command_result setupLogging(color_ostream& out, std::vector<std::string>& parameters);



// event handlers
void jobCompleted(color_ostream& out, void* _job);

void timePassed(color_ostream& out, void* ptr);

void unitDeath(color_ostream& out, void* ptr);

void itemCreate(color_ostream& out, void* ptr);

void syndrome(color_ostream& out, void* ptr);

void invasion(color_ostream& out, void* ptr);

void setParams(Parameters params);

// query structs
struct unitDeathInfo
{
    UnitRecord victim;
    UnitRecord killer;
    std::string death_cause;
};

// query functions
std::vector<int32_t> getUniqueYears();
std::vector<UnitRecord> getNewCitizens(int32_t year);
std::vector<unitDeathInfo> getCitizenDeaths(int32_t year);
std::vector<JobRecord> getJobsDone(int32_t year);


}
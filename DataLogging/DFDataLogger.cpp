
#include "DFDataLogger.hpp"

#include <memory>

#include "modules/World.h"
#include "modules/Job.h"

#include "df/body_part_raw.h"
#include "df/caste_body_info.h"
#include "df/construction.h"
#include "df/coord.h"
#include "df/item.h"
#include "df/item_actual.h"
#include "df/job.h"
#include "df/unit.h"
#include "df/unit_wound.h"
#include "df/unit_wound_layerst.h"
#include "df/world.h"
#include "df/death_type.h"


#include "RepeatedLoggers.hpp"
#include "SQLiteWrapper.hpp"
#include "Events.hpp"
#include "Jobs.hpp"
#include "Units.hpp"
#include "Items.hpp"
#include "Deaths.hpp"
#include "Petitions.hpp"
#include "Sieges.hpp"
#include "DateAndTime.hpp"

/// set plugin self pointer for use in event handlers (to be set from main.cpp)
DFHack::Plugin *plugin_self = nullptr;
DataLogger::Parameters params;

DB::Database myDb("myDatabase.db");

DB::Table<EventRecord> eventsTable = myDb.getTable<EventRecord>();
DB::Table<JobRecord> jobsTable = myDb.getTable<JobRecord>();
DB::Table<UnitRecord> unitsTable = myDb.getTable<UnitRecord>();
DB::Table<ItemRecord> itemsTable = myDb.getTable<ItemRecord>();
DB::Table<DeathRecord> deathsTable = myDb.getTable<DeathRecord>();
DB::Table<PetitionRecord> petitionsTable = myDb.getTable<PetitionRecord>();
DB::Table<SiegeRecord> siegesTable = myDb.getTable<SiegeRecord>();

std::unique_ptr<EventManager::EventHandler> timeHandler = nullptr;


date lastLoggedDateMonth;
date lastLoggedDateDay;





//forward declaration for this file
void logUnitsAndOthers();


void DataLogger::setParams(Parameters _params)
{
    params = _params;
    plugin_self = params.plugin;
}

/* Register events */
command_result DataLogger::setupLogging(color_ostream& out, std::vector<std::string>& parameters) 
{
    EventManager::EventHandler completeHandler(plugin_self, jobCompleted, 0);
    timeHandler = std::make_unique<EventManager::EventHandler>(plugin_self, timePassed, 1);
    EventManager::EventHandler deathHandler(plugin_self, unitDeath, 500);
    EventManager::EventHandler itemHandler(plugin_self, itemCreate, 1);




    // Unregister any existing handlers for this plugin to avoid duplicates
    EventManager::unregisterAll(plugin_self);

    // Register handlers for specific events
    EventManager::registerListener(EventManager::EventType::JOB_COMPLETED, completeHandler);
    EventManager::registerListener(EventManager::EventType::UNIT_DEATH, deathHandler);
    EventManager::registerListener(EventManager::EventType::ITEM_CREATED, itemHandler);
    EventManager::registerTick(*timeHandler, 1200);
    out.print("Events registered.\n");

    lastLoggedDateMonth = getDate();
    lastLoggedDateDay = lastLoggedDateMonth;
    logUnitsAndOthers(); // log the initial state of units and others when logging is set up

    return CR_OK;
}





void DataLogger::jobCompleted(color_ostream& out, void* _job) 
{
    auto currentDate = getDate();
    auto day = currentDate.day;
    auto month = currentDate.month;
    auto year = currentDate.year;
    auto tick = currentDate.tick;
    df::job* job = (df::job*)_job;
    auto jobType = job->job_type;
    std::string jobTypeStr = std::string("JobCompleted: ") + ENUM_KEY_STR(job_type, jobType);

    // Log the event
    EventRecord event = EventRecord(day, month, year, tick, event_type::JOB_COMPLETED, jobTypeStr);
    auto event_id = eventsTable.insertData(event);

    
    //log the worker details if available
    auto worker = DFHack::Job::getWorker(job);
    int32_t workerId = -1;
    if (worker) 
    {
        UnitRecord unitRecord = UnitRecord(event_id, worker);
        workerId = unitsTable.insertData(unitRecord);
    }
    else
    {
        Logger::log("Job completed with no worker. Job id: " + std::to_string(job->id));
    }

    

    
    // log the corresponding job details
    JobRecord record = JobRecord(event_id, workerId, jobTypeStr, jobType, job->mat_type, job->mat_index);
    auto job_id = jobsTable.insertData(record);

}



void logUnitsAndOthers()
{
    auto allActiveUnits = df::global::world->units.active;

    // Log the event
    auto currentDate = getDate();
    auto day = currentDate.day;
    auto month = currentDate.month;
    auto year = currentDate.year;
    auto tick = currentDate.tick;

    EventRecord citizenEvent = EventRecord(day, month, year, tick, event_type::MONTHLY_CITIZEN_LOG, "Log of all active citizens");    
    auto citizenEvent_id = eventsTable.insertData(citizenEvent);

    EventRecord animalEvent = EventRecord(day, month, year, tick, event_type::MONTHLY_ANIMAL_LOG, "Log of all active animals");    
    auto animalEvent_id = eventsTable.insertData(animalEvent);

    EventRecord otherEvent = EventRecord(day, month, year, tick, event_type::MONTHLY_OTHER_LOG, "Log of other active units");    
    auto otherEvent_id = eventsTable.insertData(otherEvent);

    Logger::log("Logging units and other details for date: " + currentDate.toString() + ". Total active units: " + std::to_string(allActiveUnits.size()));

    // log the units
    for (auto& unit : allActiveUnits) 
    {
        if(DFHack::Units::isVisible(unit) == false)continue;

        if (DFHack::Units::isCitizen(unit))
        {   
            Logger::log("Logging citizen unit: ");
             UnitRecord unitRecord = UnitRecord(citizenEvent_id, unit);
             auto unitId = unitsTable.insertData(unitRecord);
        }
        else if (DFHack::Units::isAnimal(unit))
        {
            Logger::log("Logging animal unit: " );
             UnitRecord unitRecord = UnitRecord(animalEvent_id, unit);
             auto unitId = unitsTable.insertData(unitRecord);
        }
        else
        {
            Logger::log("Logging other unit: " );
             UnitRecord unitRecord = UnitRecord(otherEvent_id, unit);
             auto unitId = unitsTable.insertData(unitRecord);
        }

    }


}

void DataLogger::timePassed(color_ostream& out, void* ptr) 
{
    // Register the tick handler again to continue receiving time updates
    EventManager::registerTick(*timeHandler, 1200);
    auto currentDate = getDate();
    Logger::log("Time passed event. Current date: " + currentDate.toString());
    
    // log citzens, animals and others every month
    if (currentDate - lastLoggedDateMonth == timePassedData{0,1,0}) // one month has passed since last log
    {
       logUnitsAndOthers();
       lastLoggedDateMonth = currentDate;
    }
    // check for new citizens, books,sieges and petitions every day(logs them)
    if (currentDate - lastLoggedDateDay == timePassedData{0,0,1}) // one day has passed since last log
    {
       CitizenLogger::checkForNewCitizens(eventsTable, unitsTable);
       BookLogger::checkForNewBooks(eventsTable, itemsTable);
       PetitionLogger::checkForNewPetitions(eventsTable, petitionsTable);
       SiegeLogger::checkForNewSieges(eventsTable, siegesTable);
       lastLoggedDateDay = currentDate;
    }
}

void DataLogger::unitDeath(color_ostream& out, void* ptr) {
    out.print("Death: {}\n", (intptr_t)(ptr));
    int32_t unitId = (intptr_t)ptr;
    auto unit = df::unit::find(unitId);

    // Log the event
    auto currentDate = getDate();
    auto day = currentDate.day;
    auto month = currentDate.month;
    auto year = currentDate.year;
    auto tick = currentDate.tick;
    EventRecord event = EventRecord(day, month, year, tick, event_type::UNIT_DEATH, "Unit death");
    auto event_id = eventsTable.insertData(event);

    // log victim unit details if available
    int32_t victimId = -1;
    if (unit) 
    {
        UnitRecord unitRecord = UnitRecord(event_id, unit);
        victimId = unitsTable.insertData(unitRecord);
    }

    // log killer unit details if available
    auto incidentInfo = getIncidentInfo(unit);
    int32_t killerId = -1;
    if (incidentInfo.killer)
    {
        UnitRecord unitRecord = UnitRecord(event_id, incidentInfo.killer);
        killerId = unitsTable.insertData(unitRecord);
    }

    // log the corresponding death details if available
    if (unit) 
    {
        df::death_type death_cause = incidentInfo.death_cause;
        DeathRecord deathRecord = DeathRecord(event_id, ENUM_KEY_STR(death_type,death_cause), victimId, killerId);
        auto death_id = deathsTable.insertData(deathRecord);
    }
    else
    {
        Logger::log("Unit death with no unit. Event id: " + std::to_string(event_id));
    }
}

void DataLogger::itemCreate(color_ostream& out, void* ptr) {
    int32_t item_index = df::item::binsearch_index(df::global::world->items.all, (intptr_t)ptr);
    if ( item_index == -1 ) {
        out.print("{}: Error.\n", __FILE__, __LINE__);
    }
    df::item* item = df::global::world->items.all[item_index];

    auto currentDate = getDate();
    auto day = currentDate.day;
    auto month = currentDate.month;
    auto year = currentDate.year;
    auto tick = currentDate.tick;

    std::string itemDescrs = "Item created: " + DFHack::Items::getReadableDescription(item);

    // Log the event
    EventRecord event = EventRecord(day, month, year, tick, event_type::ITEM_CREATED, itemDescrs);
    auto event_id = eventsTable.insertData(event);


    // log the corresponding item details if available
    int32_t itemId = -1;
    if (item) 
    {
        ItemRecord itemRecord = ItemRecord(event_id, item);
        itemId = itemsTable.insertData(itemRecord);
    }
    else
    {
        Logger::log("Item created with no item. Item id: " + std::to_string(itemId));
    }

}

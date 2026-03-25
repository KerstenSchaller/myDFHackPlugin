
#include "DFDataLogger.hpp"

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


#include "SQLiteWrapper.hpp"
#include "Events.hpp"
#include "Jobs.hpp"
#include "Units.hpp"
#include "Items.hpp"
#include "Deaths.hpp"

DB::Database myDb("FortressChronicle.db");

auto eventsTable = myDb.create_table<EventRecord>();
auto jobsTable = myDb.create_table<JobRecord>();
auto unitsTable = myDb.create_table<UnitRecord>();
auto itemsTable = myDb.create_table<ItemRecord>();
auto deathsTable = myDb.create_table<DeathRecord>();

DFHack::Plugin *plugin_self = nullptr;

void setPluginSelf(DFHack::Plugin* plugin)
{
    plugin_self = plugin;
}

/* Register events */
command_result setupLogging(color_ostream& out, std::vector<std::string>& parameters) 
{
    EventManager::EventHandler completeHandler(plugin_self, jobCompleted, 0);
    EventManager::EventHandler timeHandler(plugin_self, timePassed, 1);
    EventManager::EventHandler deathHandler(plugin_self, unitDeath, 500);
    EventManager::EventHandler itemHandler(plugin_self, itemCreate, 1);
    EventManager::EventHandler invasionHandler(plugin_self, invasion, 1000);
    EventManager::unregisterAll(plugin_self);

    EventManager::registerListener(EventManager::EventType::JOB_COMPLETED, completeHandler);
    EventManager::registerListener(EventManager::EventType::UNIT_DEATH, deathHandler);
    EventManager::registerListener(EventManager::EventType::ITEM_CREATED, itemHandler);
    EventManager::registerListener(EventManager::EventType::INVASION, invasionHandler);
    EventManager::registerTick(timeHandler, 1);
    EventManager::registerTick(timeHandler, 2);
    EventManager::registerTick(timeHandler, 4);
    EventManager::registerTick(timeHandler, 8);
    int32_t t = EventManager::registerTick(timeHandler, 16);
    timeHandler.freq = t;
    EventManager::unregister(EventManager::EventType::TICK, timeHandler);
    t = EventManager::registerTick(timeHandler, 32);
    t = EventManager::registerTick(timeHandler, 32);
    t = EventManager::registerTick(timeHandler, 32);
    timeHandler.freq = t;
    EventManager::unregister(EventManager::EventType::TICK, timeHandler);
    EventManager::unregister(EventManager::EventType::TICK, timeHandler);

    out.print("Events registered.\n");
    return CR_OK;
}



date getDate()
{
    auto day = DFHack::World::ReadCurrentDay();
    auto month = DFHack::World::ReadCurrentMonth();
    auto year = DFHack::World::ReadCurrentYear();
    auto tick = DFHack::World::ReadCurrentTick();
    return date(day, month, year, tick);
}

void jobCompleted(color_ostream& out, void* _job) 
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

void timePassed(color_ostream& out, void* ptr) {
    out.print("Time: {}\n", (intptr_t)(ptr));
}

void unitDeath(color_ostream& out, void* ptr) {
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

void itemCreate(color_ostream& out, void* ptr) {
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

void syndrome(color_ostream& out, void* ptr) {
    EventManager::SyndromeData* data = (EventManager::SyndromeData*)ptr;
    out.print("Syndrome started: unit {}, syndrome {}.\n", data->unitId, data->syndromeIndex);
}

void invasion(color_ostream& out, void* ptr) {
    out.print("New invasion! {}\n", (intptr_t)ptr);
}


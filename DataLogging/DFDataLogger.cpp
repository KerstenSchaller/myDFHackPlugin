
#include "DFDataLogger.hpp"

#include <memory>
#include <filesystem>

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
#include "df/world_data.h"
#include "df/world_site.h"
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
#include "Announcements.hpp"

#include "models/Primitives.hpp"
#include "Sieges.hpp"
#include "DateAndTime.hpp"


/// set plugin self pointer for use in event handlers (to be set from main.cpp)
DFHack::Plugin *plugin_self = nullptr;
DataLogger::Parameters params;



//dfhack-config/df_chronicle/myDatabase.db
//std::string dbPath = "dfhack-config/df_chronicle/" + worldName + "_" + fortressName + ".db";


    std::shared_ptr<DB::Database> myDb = nullptr;

    std::shared_ptr<DB::Table<EventRecord>> eventsTable = nullptr;
    std::shared_ptr<DB::Table<JobRecord>> jobsTable = nullptr;
    std::shared_ptr<DB::Table<UnitRecord>> unitsTable = nullptr;
    std::shared_ptr<DB::Table<ItemRecord>> itemsTable = nullptr;
    std::shared_ptr<DB::Table<DeathRecord>> deathsTable = nullptr;
    std::shared_ptr<DB::Table<PetitionRecord>> petitionsTable = nullptr;
    std::shared_ptr<DB::Table<AnnouncementRecord>> announcementsTable = nullptr;
    std::shared_ptr<DB::Table<SiegeRecord>> siegesTable = nullptr;

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
command_result DataLogger::setupLogging() 
{
    // get some meta info
    auto fortressName = DF2UTF(DFHack::Translation::translateName(&df::global::plotinfo->main.fortress_site->name, true));
    auto worldName = DF2UTF(DFHack::Translation::translateName(&df::global::world->world_data->name, true));
    //replace spaces with underscores for file naming
    std::replace(fortressName.begin(), fortressName.end(), ' ', '_');
    std::replace(worldName.begin(), worldName.end(), ' ', '_');

    myDb = std::make_shared<DB::Database>(std::string("dfhack-config/df_chronicle/" + worldName  + "_" + fortressName + ".db"));

    eventsTable = std::make_shared<DB::Table<EventRecord>>(myDb->create_table<EventRecord>());
    jobsTable = std::make_shared<DB::Table<JobRecord>>(myDb->create_table<JobRecord>());
    unitsTable = std::make_shared<DB::Table<UnitRecord>>(myDb->create_table<UnitRecord>());
    itemsTable = std::make_shared<DB::Table<ItemRecord>>(myDb->create_table<ItemRecord>());
    deathsTable = std::make_shared<DB::Table<DeathRecord>>(myDb->create_table<DeathRecord>());
    petitionsTable = std::make_shared<DB::Table<PetitionRecord>>(myDb->create_table<PetitionRecord>());
    announcementsTable = std::make_shared<DB::Table<AnnouncementRecord>>(myDb->create_table<AnnouncementRecord>());
    siegesTable = std::make_shared<DB::Table<SiegeRecord>>(myDb->create_table<SiegeRecord>());

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
    auto event_id = eventsTable->insertData(event);

    
    //log the worker details if available
    auto worker = DFHack::Job::getWorker(job);
    int32_t workerId = -1;
    if (worker) 
    {
        UnitRecord unitRecord = UnitRecord(event_id, worker);
        workerId = unitsTable->insertData(unitRecord);
    }
    else
    {
        Logger::log("Job completed with no worker. Job id: " + std::to_string(job->id));
    }

    

    
    // log the corresponding job details
    JobRecord record = JobRecord(event_id, workerId, jobTypeStr, jobType, job->mat_type, job->mat_index);
    auto job_id = jobsTable->insertData(record);

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
    auto citizenEvent_id = eventsTable->insertData(citizenEvent);

    EventRecord animalEvent = EventRecord(day, month, year, tick, event_type::MONTHLY_ANIMAL_LOG, "Log of all active animals");    
    auto animalEvent_id = eventsTable->insertData(animalEvent);

    EventRecord otherEvent = EventRecord(day, month, year, tick, event_type::MONTHLY_OTHER_LOG, "Log of other active units");    
    auto otherEvent_id = eventsTable->insertData(otherEvent);


    // log the units
    for (auto& unit : allActiveUnits) 
    {
        if(DFHack::Units::isVisible(unit) == false)continue;

        if (DFHack::Units::isCitizen(unit))
        {   
             UnitRecord unitRecord = UnitRecord(citizenEvent_id, unit);
             auto unitId = unitsTable->insertData(unitRecord);
        }
        else if (DFHack::Units::isAnimal(unit))
        {
             UnitRecord unitRecord = UnitRecord(animalEvent_id, unit);
             auto unitId = unitsTable->insertData(unitRecord);
        }
        else
        {
             UnitRecord unitRecord = UnitRecord(otherEvent_id, unit);
             auto unitId = unitsTable->insertData(unitRecord);
        }

    }


}

void DataLogger::timePassed(color_ostream& out, void* ptr) 
{
    // Register the tick handler again to continue receiving time updates
    EventManager::registerTick(*timeHandler, 1200);
    auto currentDate = getDate();
    
    // log citzens, animals and others every month
    if (currentDate - lastLoggedDateMonth == timePassedData{0,1,0}) // one month has passed since last log
    {
       logUnitsAndOthers();
       lastLoggedDateMonth = currentDate;
    }
    // check for new citizens, books,sieges and petitions every day(logs them)
    if (currentDate - lastLoggedDateDay == timePassedData{1,0,0}) // one day has passed since last log
    {
       CitizenLogger::checkForNewCitizens(eventsTable, unitsTable);
       BookLogger::checkForNewBooks(eventsTable, itemsTable);
       PetitionLogger::checkForNewPetitions(eventsTable, petitionsTable);
       SiegeLogger::checkForNewSieges(eventsTable, siegesTable, unitsTable);
       AnnouncementLogger::checkForNewAnnouncements(eventsTable, announcementsTable);
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
    auto event_id = eventsTable->insertData(event);

    // log victim unit details if available
    int32_t victimId = -1;
    if (unit) 
    {
        UnitRecord unitRecord = UnitRecord(event_id, unit);
        victimId = unitsTable->insertData(unitRecord);
    }

    // log killer unit details if available
    auto incidentInfo = getIncidentInfo(unit);
    int32_t killerId = -1;
    if (incidentInfo.killer)
    {
        UnitRecord unitRecord = UnitRecord(event_id, incidentInfo.killer);
        killerId = unitsTable->insertData(unitRecord);
    }

    // log the corresponding death details if available
    if (unit) 
    {
        df::death_type death_cause = incidentInfo.death_cause;
        DeathRecord deathRecord = DeathRecord(event_id, ENUM_KEY_STR(death_type,death_cause), victimId, killerId);
        auto death_id = deathsTable->insertData(deathRecord);
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

    std::string itemDescrs = "Item created: " + DF2UTF(DFHack::Items::getReadableDescription(item));

    // Log the event
    EventRecord event = EventRecord(day, month, year, tick, event_type::ITEM_CREATED, itemDescrs);
    auto event_id = eventsTable->insertData(event);


    // log the corresponding item details if available
    int32_t itemId = -1;
    if (item) 
    {
        ItemRecord itemRecord = ItemRecord(event_id, item);
        itemId = itemsTable->insertData(itemRecord);
    }
    else
    {
        Logger::log("Item created with no item. Item id: " + std::to_string(itemId));
    }

}

// Query functions

std::vector<int32_t> DataLogger::getUniqueYears()
{
    std::vector<int32_t> years;
    std::string sqlString = "SELECT DISTINCT year FROM event_records;";
    

    return myDb->query<int32_t>(sqlString);
}

std::vector<UnitRecord> DataLogger::getNewCitizens(int32_t year)
{
    std::string sqlString = "IN (SELECT id FROM event_records WHERE event_type = "
                + std::to_string(static_cast<int>(event_type::NEW_CITIZEN));

    if (year != -1) // if year 
    {
        sqlString += " AND year = " + std::to_string(year) + ")";
    }
    else
    {
        sqlString += ")";
    }

    std::vector<DB::WhereClause> clauses = {
        DB::WhereClause("event_id", sqlString)
    };
    
    
    return unitsTable->queryWhere(clauses);
}

std::vector<DataLogger::unitDeathInfo> DataLogger::getCitizenDeaths(int32_t year)
{
    std::vector<DataLogger::unitDeathInfo> deathInfos;

    std::string sqlString = "SELECT u.name, u.age, u.sex, u.race, u.profession, d.death_cause, k.name, k.age, k.sex, k.race, k.profession FROM " + UnitRecord().tableName() + " u "
    "JOIN " + DeathRecord().tableName() + " d ON u.id = d.victim_id "
    "LEFT JOIN " + UnitRecord().tableName() + " k ON d.killer_id = k.id "
    "WHERE u.event_id IN (SELECT id FROM event_records WHERE event_type = "
     + std::to_string(static_cast<int>(event_type::UNIT_DEATH))
     + "AND u.isCitizen = 1";   
    
    //if year is -1 get all deaths, otherwise filter by year
    if (year != -1)
    {
        sqlString += " AND year = " + std::to_string(year) + ")";
    }
    else
    {
        sqlString += ")";
    }

        myDb->query(sqlString, [&deathInfos](const std::string& victimName, double victimAge, const std::string& victimSex, const std::string& victimRace, const std::string& victimProfession, const std::string& causeOfDeath, const std::string& killerName, double killerAge, const std::string& killerSex, const std::string& killerRace, const std::string& killerProfession)
        {
            DataLogger::unitDeathInfo info;
            info.victim.name = victimName;
            info.victim.age = victimAge;
            info.victim.sex = victimSex;
            info.victim.race = victimRace;
            info.victim.profession = victimProfession;
            info.death_cause = causeOfDeath;
            info.killer.name = killerName;
            info.killer.age = killerAge;
            info.killer.sex = killerSex;
            info.killer.race = killerRace;
            info.killer.profession = killerProfession;
            deathInfos.push_back(info);
        });

    return deathInfos;

}

std::vector<JobRecord> DataLogger::getJobsDone(int32_t year)
{
    // from event_records, get the events of type JOB_COMPLETED for the given year, then get the corresponding job records
    std::vector<DB::WhereClause> clauses = {
        DB::WhereClause("event_id", "IN (SELECT id FROM event_records WHERE event_type = "
            + std::to_string(static_cast<int>(event_type::JOB_COMPLETED))
            + " AND year = " + std::to_string(year) + " AND day = " + std::to_string(23) + ")")
    };
    return jobsTable->queryWhere(clauses);
}
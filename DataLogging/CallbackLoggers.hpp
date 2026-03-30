#pragma once

#include "df/item.h"
#include "df/job.h"


#include "DateAndTime.hpp"
#include "models/Events.hpp"
#include "models/Deaths.hpp"
#include "models/Items.hpp"
#include "models/Jobs.hpp"
#include "models/Units.hpp"

#include "Helper.hpp"

namespace EventCallbacks
{

    static void logItems(df::item *item, std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<ItemRecord>> itemsTable, std::shared_ptr<DB::Table<UnitRecord>> unitsTable)
    {
        auto currentDate = getDate();
        auto day = currentDate.day;
        auto month = currentDate.month;
        auto year = currentDate.year;
        auto tick = currentDate.tick;

        std::string itemDescrs = "Item created: " + DF2UTF(DFHack::Items::getReadableDescription(item));

        // Log the event
        EventRecord event = EventRecord(day, month, year, tick, event_type::ITEM_CREATED, itemDescrs);
        auto event_id = eventsTable->insertData(event);
        {
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
                // log the maker of the item if available
                auto makerUnit = getMakerFromItem(item);
                if (makerUnit)
                {
                    UnitRecord unitRecord = UnitRecord(event_id, makerUnit);
                    auto unitId = unitsTable->insertData(unitRecord);
                }
                else
                {
                    Logger::log("Item created with no maker. Item id: " + std::to_string(itemId));
                }
            }
            else
            {
                Logger::log("Item created with no item. Item id: " + std::to_string(itemId));
            }
        }
    }

    static void logUnitDeath(df::unit *unit, std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<UnitRecord>> unitsTable, std::shared_ptr<DB::Table<DeathRecord>> deathsTable)
    {
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
            DeathRecord deathRecord = DeathRecord(event_id, ENUM_KEY_STR(death_type, death_cause), victimId, killerId);
            auto death_id = deathsTable->insertData(deathRecord);
        }
        else
        {
            Logger::log("Unit death with no unit. Event id: " + std::to_string(event_id));
        }
    }

    static void logJob(df::job *job, std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<JobRecord>> jobsTable, std::shared_ptr<DB::Table<UnitRecord>> unitsTable)
    {
        auto jobType = job->job_type;
        std::string jobTypeStr = std::string("JobCompleted: ") + ENUM_KEY_STR(job_type, jobType);

        // Log the event
        auto currentDate = getDate();
        auto day = currentDate.day;
        auto month = currentDate.month;
        auto year = currentDate.year;
        auto tick = currentDate.tick;
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
}
#pragma once

#include <unordered_set>

#include "df/item.h"
#include "df/agreement_details_type.h"
#include "models/Events.hpp"
#include "models/Units.hpp"
#include "models/Items.hpp"
#include "models/Petitions.hpp"
#include "df/world.h"
#include "df/plotinfost.h"

#include "SQLITEWrapper.hpp"
#include "DateAndTime.hpp"

#include "models/Sieges.hpp"

class BookLogger
{
    static int32_t lastItemIndexChecked;

public:
    static void checkForNewBooks(DB::Table<EventRecord>& eventsTable, DB::Table<ItemRecord>& itemsTable)
    {
        auto allItems = df::global::world->items.all;
        for (size_t i = lastItemIndexChecked + 1; i < allItems.size(); ++i)
        {
            df::item* item = allItems[i];
            auto bookTitle = DFHack::Items::getBookTitle(item);
            if (item && bookTitle != "")
            {
                // Log the event
                auto currentDate = getDate();
                auto day = currentDate.day;
                auto month = currentDate.month;
                auto year = currentDate.year;
                auto tick = currentDate.tick;
                EventRecord event = EventRecord(day, month, year, tick, event_type::ITEM_CREATED, "Book created: " + bookTitle);
                auto event_id = eventsTable.insertData(event);

                // log the book details
                ItemRecord record = ItemRecord(event_id, item);
                record.bookTitle = DF2UTF(bookTitle);
                itemsTable.insertData(record);
            }
        }
        if (!allItems.empty())
        {
            lastItemIndexChecked = allItems.size() - 1;
        }
    }
};

class CitizenLogger
{
    static int32_t lastUnitIndexChecked;
    public:
    static void checkForNewCitizens(DB::Table<EventRecord>& eventsTable, DB::Table<UnitRecord>& unitsTable)
    {
        // Log the event
        auto currentDate = getDate();
        auto day = currentDate.day;
        auto month = currentDate.month;
        auto year = currentDate.year;
        auto tick = currentDate.tick;
        EventRecord event = EventRecord(day, month, year, tick, event_type::NEW_CITIZEN, "New Citizens detected");
        auto event_id = eventsTable.insertData(event);

        auto allUnits = df::global::world->units.active;
        for (size_t i = lastUnitIndexChecked + 1; i < allUnits.size(); ++i)
        {
            df::unit* unit = allUnits[i];
            if (unit && (DFHack::Units::isCitizen(unit) || DFHack::Units::isResident(unit)))
            {
                // log the citizen details
                UnitRecord record = UnitRecord(event_id, unit);
                unitsTable.insertData(record);
            }
        }
        if (!allUnits.empty())
        {
            lastUnitIndexChecked = allUnits.size() - 1;
        }
    }
};


class PetitionLogger
{
    static std::unordered_set<uint64_t> seenPetitionDetails;

    static bool isTrackedType(df::agreement_details_type type)
    {
        return type == df::agreement_details_type::Citizenship ||
               type == df::agreement_details_type::Location ||
               type == df::agreement_details_type::OfferService ||
               type == df::agreement_details_type::Parley ||
               type == df::agreement_details_type::Residency;
    }

    static uint64_t makeDetailKey(int32_t agreement_id, int32_t details_id)
    {
        return (static_cast<uint64_t>(static_cast<uint32_t>(agreement_id)) << 32) |
               static_cast<uint32_t>(details_id);
    }

public:
    static void checkForNewPetitions(DB::Table<EventRecord>& eventsTable, DB::Table<PetitionRecord>& petitionsTable)
    {
        auto agreements = df::global::world->agreements.all;
        for (auto *agreement : agreements)
        {
            if (!agreement)
            {
                continue;
            }

            for (auto *details : agreement->details)
            {
                if (!details || !isTrackedType(details->type))
                {
                    continue;
                }

                uint64_t detailKey = makeDetailKey(agreement->id, details->id);
                if (!seenPetitionDetails.insert(detailKey).second)
                {
                    continue;
                }

                auto currentDate = getDate();
                EventRecord event(
                    currentDate.day,
                    currentDate.month,
                    currentDate.year,
                    currentDate.tick,
                    event_type::PETITION,
                    std::string("New petition: ") + ENUM_KEY_STR(agreement_details_type, details->type));
                auto event_id = eventsTable.insertData(event);

                petitionsTable.insertData(PetitionRecord(event_id, agreement, details));
            }
        }
    }


};

class SiegeLogger
{
    static int32_t lastSiegeIndexChecked;
    static bool siegeActive;
    static int32_t currentSiegeIndex;

    public:
    static void checkForNewSieges(DB::Table<EventRecord>& eventsTable, DB::Table<SiegeRecord>& siegesTable)
    {
        auto sieges = df::global::plotinfo->invasions.list;

        auto currentBiggestIndex = sieges.size() - 1;
        if(siegeActive == false)
        {
            if (currentBiggestIndex > lastSiegeIndexChecked)
            {
                auto siege = sieges[currentBiggestIndex];
                if (siege && siege->flags.bits.active)
                {
                    // Log the event
                    auto currentDate = getDate();
                    auto day = currentDate.day;
                    auto month = currentDate.month;
                    auto year = currentDate.year;
                    auto tick = currentDate.tick;
                    EventRecord event = EventRecord(day, month, year, tick, event_type::SIEGE_START, "New Siege detected");
                    auto event_id = eventsTable.insertData(event);

                    // log the siege details
                    SiegeRecord record = SiegeRecord(event_id, siege);
                    siegesTable.insertData(record);
                }
            }
            lastSiegeIndexChecked = currentBiggestIndex;
        }
        else
        {
            // Check if the current siege has ended
            auto siege = sieges[currentSiegeIndex];
            if (siege && !siege->flags.bits.active)
            {
                // Log the event
                auto currentDate = getDate();
                auto day = currentDate.day;
                auto month = currentDate.month;
                auto year = currentDate.year;
                auto tick = currentDate.tick;
                EventRecord event = EventRecord(day, month, year, tick, event_type::SIEGE_END, "Siege ended");
                auto event_id = eventsTable.insertData(event);

                // log the siege details
                SiegeRecord record = SiegeRecord(event_id, siege);
                siegesTable.insertData(record);

                siegeActive = false;
            }

        }
    }
};

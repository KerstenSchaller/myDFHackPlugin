#pragma once

#include <unordered_set>

#include "df/item.h"
#include "df/agreement_details_type.h"
#include "df/army_controller.h"
#include "df/historical_figure.h"
#include "models/Events.hpp"
#include "models/Units.hpp"
#include "models/Items.hpp"
#include "models/Petitions.hpp"
#include "models/Announcements.hpp"
#include "df/world.h"
#include "df/plotinfost.h"

#include "SQLITEWrapper.hpp"
#include "DateAndTime.hpp"

#include "models/Sieges.hpp"

template <typename Container, typename Callback>
inline void forEachNewEntrySince(const Container &entries, size_t &lastLoggedIndex, Callback &&callback)
{
    if (entries.empty())
    {
        lastLoggedIndex = 0;
        return;
    }

    if (lastLoggedIndex < entries.size())
    {
        for (size_t index = lastLoggedIndex; index < entries.size(); ++index)
        {
            callback(entries[index]);
        }
    }
    else
    {
        // Ringbuffer-safe fallback: if the size did not advance (or reset), re-check all entries.
        for (size_t index = 0; index < entries.size(); ++index)
        {
            callback(entries[index]);
        }
    }

    lastLoggedIndex = entries.size();
}


class BookLogger
{
    static std::unordered_set<uint64_t> seenBookIds;
    static bool firstCheckDone;
    static size_t lastLoggedIndex;

public:
    static void checkForNewBooks(std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<ItemRecord>> itemsTable)
    {
        if (firstCheckDone == false)
        {
            auto allItems = df::global::world->items.all;
            // Mark all existing books as seen
            for (auto *item : allItems)
            {
                if (item)
                {
                    seenBookIds.insert(reinterpret_cast<uint64_t>(item));
                }
            }
            lastLoggedIndex = allItems.size();
            firstCheckDone = true;
            return;
        }
        auto allItems = df::global::world->items.all;
        forEachNewEntrySince(allItems, lastLoggedIndex, [&](auto *item) {
            if (!item)
            {
                return;
            }

            uint64_t bookId = reinterpret_cast<uint64_t>(item);
            // Check if we've already logged this book
            if (seenBookIds.count(bookId))
            {
                return;
            }

            // New book found
            seenBookIds.insert(bookId);
            
            auto bookTitle = DFHack::Items::getBookTitle(item);
            if (bookTitle != "")
            {
                // Log the event
                auto currentDate = getDate();
                auto day = currentDate.day;
                auto month = currentDate.month;
                auto year = currentDate.year;
                auto tick = currentDate.tick;
                EventRecord event = EventRecord(day, month, year, tick, event_type::ITEM_CREATED, "Book created: " + DF2UTF(bookTitle));
                auto event_id = eventsTable->insertData(event);

                // log the book details
                ItemRecord record = ItemRecord(event_id, item);
                record.bookTitle = DF2UTF(bookTitle);
                itemsTable->insertData(record);
            }
        });
    }
};

class CitizenLogger
{
    static std::unordered_set<uint64_t> seenUnitIds;
    static bool firstCheckDone;
    static size_t lastLoggedIndex;

    public:
    static void checkForNewCitizens(std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<UnitRecord>> unitsTable)
    {
        if (firstCheckDone == false)
        {
            auto allUnits = df::global::world->units.active;
            // Mark all existing units as seen
            for (auto *unit : allUnits)
            {
                if (unit)
                {
                    seenUnitIds.insert(reinterpret_cast<uint64_t>(unit));
                }
            }
            lastLoggedIndex = allUnits.size();
            firstCheckDone = true;
            return;
        }

        auto allUnits = df::global::world->units.active;
        forEachNewEntrySince(allUnits, lastLoggedIndex, [&](auto *unit) {
            if (!unit)
            {
                return;
            }

            uint64_t unitId = reinterpret_cast<uint64_t>(unit);
            // Check if we've already logged this unit
            if (seenUnitIds.count(unitId))
            {
                return;
            }

            // New unit found
            if (DFHack::Units::isCitizen(unit) || DFHack::Units::isResident(unit))
            {
                seenUnitIds.insert(unitId);
                
                // Log the event
                auto currentDate = getDate();
                auto day = currentDate.day;
                auto month = currentDate.month;
                auto year = currentDate.year;
                auto tick = currentDate.tick;
                EventRecord event = EventRecord(day, month, year, tick, event_type::NEW_CITIZEN, "New Citizens detected");
                auto event_id = eventsTable->insertData(event);

                // log the citizen details
                UnitRecord record = UnitRecord(event_id, unit);
                unitsTable->insertData(record);
            }
        });
    }
};


class PetitionLogger
{
    static std::unordered_set<uint64_t> seenPetitionDetails;
    static bool firstCheckDone;
    static size_t lastLoggedIndex;

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
    static void checkForNewPetitions(std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<PetitionRecord>> petitionsTable)
    {
        if (firstCheckDone == false)
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

                    seenPetitionDetails.insert(makeDetailKey(agreement->id, details->id));
                }
            }
            lastLoggedIndex = agreements.size();
            firstCheckDone = true;
            return;
        }
        auto agreements = df::global::world->agreements.all;
        forEachNewEntrySince(agreements, lastLoggedIndex, [&](auto *agreement) {
            if (!agreement)
            {
                return;
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
                auto event_id = eventsTable->insertData(event);

                petitionsTable->insertData(PetitionRecord(event_id, agreement, details));
            }
        });
    }


};

class SiegeLogger
{
    static std::unordered_set<uint64_t> seenSiegeStartIds;
    static std::unordered_set<uint64_t> seenSiegeEndIds;
    static bool firstCheckDone;
    static size_t lastLoggedIndex;

    static df::army_controller* findArmyController(int32_t controller_id)
    {
        for (auto* ac : df::global::world->army_controllers.all)
        {
            if (ac && ac->id == controller_id)
                return ac;
        }
        return nullptr;
    }

    public:
    // Returns the commander unit for a siege (nullptr if not found or not on map)
    static df::unit* getCommanderFromSiege(df::invasion_info* siege)
    {
        if (!siege) return nullptr;
        auto* ac = findArmyController(siege->origin_master_army_controller_id);
        if (!ac) return nullptr;
        int32_t hfId = (ac->commander_hf != -1) ? ac->commander_hf : ac->master_hf;
        if (hfId == -1) return nullptr;
        auto* histFig = df::historical_figure::find(hfId);
        if (!histFig || histFig->unit_id == -1) return nullptr;
        return df::unit::find(histFig->unit_id);
    }


    static void checkForNewSieges(std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<SiegeRecord>> siegesTable, std::shared_ptr<DB::Table<UnitRecord>> unitsTable)
    {
        if (firstCheckDone == false)
        {
            auto sieges = df::global::plotinfo->invasions.list;
            // Mark all existing sieges as seen for both start and end if not active
            for (auto *siege : sieges)
            {
                if (siege)
                {
                    uint64_t siegeId = reinterpret_cast<uint64_t>(siege);
                    if (siege->flags.bits.active)
                    {
                        seenSiegeStartIds.insert(siegeId);
                    }
                    else
                    {
                        // Mark both to avoid logging old inactive sieges
                        seenSiegeStartIds.insert(siegeId);
                        seenSiegeEndIds.insert(siegeId);
                    }
                }
            }
            lastLoggedIndex = sieges.size();
            firstCheckDone = true;
            return;
        }
        auto sieges = df::global::plotinfo->invasions.list;

        forEachNewEntrySince(sieges, lastLoggedIndex, [&](auto *siege) {
            if (!siege)
            {
                return;
            }

            uint64_t siegeId = reinterpret_cast<uint64_t>(siege);

            // Check for new active siege (SIEGE_START)
            if (siege->flags.bits.active && !seenSiegeStartIds.count(siegeId))
            {
                seenSiegeStartIds.insert(siegeId);
                
                // Log the event
                auto currentDate = getDate();
                EventRecord event = EventRecord(currentDate.day, currentDate.month, currentDate.year, currentDate.tick, event_type::SIEGE_START, "New Siege detected");
                auto event_id = eventsTable->insertData(event);

                // log the siege details
                SiegeRecord record = SiegeRecord(event_id, siege);
                siegesTable->insertData(record);

                auto commander = getCommanderFromSiege(siege);
                if (commander)
                {
                    UnitRecord unitRecord(event_id, commander);
                    unitsTable->insertData(unitRecord);
                }
            }
            // Check for siege that ended (SIEGE_END)
            else if (!siege->flags.bits.active && seenSiegeStartIds.count(siegeId) && !seenSiegeEndIds.count(siegeId))
            {
                seenSiegeEndIds.insert(siegeId);
                
                // Log the event
                auto currentDate = getDate();
                EventRecord event = EventRecord(currentDate.day, currentDate.month, currentDate.year, currentDate.tick, event_type::SIEGE_END, "Siege ended");
                auto event_id = eventsTable->insertData(event);

                // log the siege details
                SiegeRecord record = SiegeRecord(event_id, siege);
                siegesTable->insertData(record);
            }
        });
    }
};

class AnnouncementLogger
{
    static std::unordered_set<uint64_t> seenAnnouncementIds;
    static bool firstCheckDone;
    static size_t lastLoggedIndex;

    public:
    static void checkForNewAnnouncements(std::shared_ptr<DB::Table<EventRecord>> eventsTable, std::shared_ptr<DB::Table<AnnouncementRecord>> announcementsTable)
    {
        if (firstCheckDone == false)
        {
            auto announcements = df::global::world->status.reports;
            // Mark all existing announcements as seen
            for (auto *announcement : announcements)
            {
                if (announcement)
                {
                    seenAnnouncementIds.insert(reinterpret_cast<uint64_t>(announcement));
                }
            }
            lastLoggedIndex = announcements.size();
            firstCheckDone = true;
            return;
        }
        auto announcements = df::global::world->status.reports;
        forEachNewEntrySince(announcements, lastLoggedIndex, [&](auto *announcement) {
            if (!announcement)
            {
                return;
            }

            uint64_t announcementId = reinterpret_cast<uint64_t>(announcement);
            // Check if we've already logged this announcement
            if (seenAnnouncementIds.count(announcementId))
            {
                return;
            }

            // New announcement found
            seenAnnouncementIds.insert(announcementId);
            
            // Log the event
            auto currentDate = getDate();
            EventRecord event = EventRecord(currentDate.day, currentDate.month, currentDate.year, currentDate.tick, event_type::ANNOUNCEMENT, "New Announcement detected: ");
            auto event_id = eventsTable->insertData(event);

            // log the announcement details
            AnnouncementRecord record = AnnouncementRecord(event_id, announcement);
            announcementsTable->insertData(record);
        });
    }
};


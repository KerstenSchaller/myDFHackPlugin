#pragma once

#include <type_traits>

#include "SQLITEWrapper.hpp"

    enum event_type {
        JOB_COMPLETED = 0,
        UNIT_DEATH = 1,
        ITEM_CREATED = 2,
        INVASION = 3
    };

class EventRecord : public DB::BaseModel
{
public:
    int16_t day;
    int16_t month;
    int16_t year;
    int32_t tick;
    event_type eventType;
    std::string text;



    EventRecord() = default;
    EventRecord(int16_t day, int16_t month, int16_t year, int32_t tick, event_type eventType, const std::string &text) : day(day), month(month), year(year), tick(tick), eventType(eventType), text(text) {}

    std::string tableName() const override
    {
        return "event_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"day INTEGER,month INTEGER,year INTEGER,tick INTEGER,event_type INTEGER,text TEXT"};
    }
};

template <>
struct DB::ModelTraits<EventRecord>
{
    static std::string insertColumns()
    {
        return "day,month,year,tick,event_type,text";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?";
    }

    template <typename Statement>
    static void bindInsert(Statement &statement, const EventRecord &event)
    {
        statement << event.day << event.month << event.year << event.tick << static_cast<int>(event.eventType) << event.text;
    }

    static void select(sqlite::database &db, const std::string &sql, std::vector<EventRecord> &results)
    {
        db << sql >> [&results](int16_t id, int16_t day, int16_t month, int16_t year, int32_t tick, int eventType, const std::string &text)
        {
            results.emplace_back(day, month, year, tick, static_cast<event_type>(eventType), text);
        };
    }
};
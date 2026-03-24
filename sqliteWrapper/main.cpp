#include <iostream>
#include "SQLITEWrapper.hpp"


class Event : public DB::BaseModel
{
public:
    int16_t day;
    int16_t month;
    int16_t year;
    int32_t tick;
    std::string text;

    Event() = default;
    Event(int16_t day, int16_t month, int16_t year, int32_t tick, const std::string &text) : day(day), month(month), year(year), tick(tick), text(text) {}

    std::string tableName() const override
    {
        return "events";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"day INTEGER,month INTEGER,year INTEGER,tick INTEGER,text TEXT"};
    }
};

template <>
struct DB::ModelTraits<Event>
{
    static std::string insertColumns()
    {
        return "day,month,year,tick,text";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?";
    }

    template <typename Statement>
    static void bindInsert(Statement &statement, const Event &event)
    {
        statement << event.day << event.month << event.year << event.tick << event.text;
    }

    static void select(sqlite::database &db, const std::string &sql, std::vector<Event> &results)
    {
        db << sql >> [&results](int16_t id, int16_t day, int16_t month, int16_t year, int32_t tick, const std::string &text)
        {
            results.emplace_back(day, month, year, tick, text);
        };
    }
};


int main(int argc, char* argv[]) 
{
    //parse command line arguments for database name, default to "test.db"
    std::string db_name = "test.db";

    DB::Database myDb(db_name);
    auto eventsTable = myDb.getTable<Event>();

    Event event1(1, 7, 2024, 12345, "Event 1");
    Event event2(2, 7, 2024, 54321, "Event 2");
    Event event3(4, 8, 2024, 67890, "Event 3");
    Event event4(5, 7, 2023, 11111, "Event 4");

    eventsTable.insertData(event1);
    eventsTable.insertData(event2);
    eventsTable.insertData(event3);
    eventsTable.insertData(event4);


    
    auto events = eventsTable.queryWhere({DB::WhereClause("day", "> 3"), DB::WhereClause("month", "= 7", "AND")});
    std::cout << "Events after 2024-07-01:\n";
    for (const auto& event : events) {
        std::cout << "Text: " << event.text << ", Date: " << event.year << "-" << (event.month < 10 ? "0" : "") << event.month << "-" << (event.day < 10 ? "0" : "") << event.day << "\n";
    }

    return 0;
}
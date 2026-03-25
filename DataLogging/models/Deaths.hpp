#pragma once

#include "df/incident.h"
#include "df/unit.h"
#include "df/world.h"
#include "df/death_type.h"

#include "SQLITEWrapper.hpp"

namespace models
{
namespace deaths
{

}
}

struct incident_info
{
    df::unit* killer;
    df::death_type death_cause;
};

incident_info getIncidentInfo(df::unit* unit);

class DeathRecord : public DB::BaseModel
{
    public:
    int64_t event_id;
    std::string death_cause;
    int64_t victim_id;
    int64_t killer_id;

    DeathRecord() = default;
    DeathRecord(int64_t event_id, const std::string& death_cause, int64_t victim_id, int64_t killer_id) : event_id(event_id), death_cause(death_cause), victim_id(victim_id), killer_id(killer_id) {}

    std::string tableName() const override
    {
        return "death_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"event_id INTEGER","death_cause TEXT","victim_id INTEGER","killer_id INTEGER"};
    }

};

template<>
struct DB::ModelTraits<DeathRecord>
{
    static std::string insertColumns()
    {
        return "event_id,death_cause,victim_id,killer_id";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?";
    }

    template <typename Statement>
    static void bindInsert(Statement &statement, const DeathRecord &record)
    {
        statement << record.event_id << record.death_cause << record.victim_id << record.killer_id;
    }

    static void select(sqlite::database &db, const std::string &sql, std::vector<DeathRecord> &results)
    {
        db << sql >> [&results](int64_t event_id, std::string death_cause, int64_t victim_id, int64_t killer_id) {
            results.emplace_back(event_id, death_cause, victim_id, killer_id);
        };
    }

};
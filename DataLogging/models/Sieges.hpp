#pragma once

#include <string>

#include "df/invasion_info.h"

class SiegeRecord : public DB::BaseModel
{
    public:
    int32_t event_id;
    int32_t siegeId;
    int32_t year;
    int32_t mission;
    std::string missionStr;
    int32_t civID;
    bool wantsParley;
    bool undead;
    bool planless;
    bool handed_over_artifact;

    SiegeRecord() = default;
    SiegeRecord(int32_t event_id, int32_t siegeId, int32_t year, int32_t mission, const std::string& missionStr, int32_t civID, bool wantsParley, bool undead, bool planless, bool handed_over_artifact)
        : event_id(event_id),
          siegeId(siegeId),
          year(year),
          mission(mission),
          missionStr(missionStr),
          civID(civID),
          wantsParley(wantsParley),
          undead(undead),
          planless(planless),
          handed_over_artifact(handed_over_artifact)
    {}
    SiegeRecord(int32_t event_id, df::invasion_info* siegeInfo)
        : event_id(event_id),
          siegeId(siegeInfo->id),
          year(siegeInfo->created_year),
          mission(siegeInfo->mission),
          missionStr(ENUM_KEY_STR(mission_type, siegeInfo->mission)),
          civID(siegeInfo->civ_id),
          wantsParley(siegeInfo->flags.bits.want_parley),
          undead(siegeInfo->flags.bits.undead_source),
          planless(siegeInfo->flags.bits.planless),
          handed_over_artifact(siegeInfo->flags.bits.handed_over_artifact)
    {}

    std::string tableName() const override
    {
        return "siege_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"event_id INTEGER", "siegeId INTEGER", "year INTEGER", "mission INTEGER", "missionStr TEXT", "civID INTEGER", "wantsParley BOOLEAN", "undead BOOLEAN", "planless BOOLEAN", "handed_over_artifact BOOLEAN"};
    }

};

template<>
struct DB::ModelTraits<SiegeRecord>
{
    static std::string insertColumns()
    {
        return "event_id,siegeId,year,mission,missionStr,civID,wantsParley,undead,planless,handed_over_artifact";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?,?,?,?,?";
    }

    template<typename Statement>
    static void bindInsert(Statement& statement, const SiegeRecord& record)
    {
        statement << record.event_id << record.siegeId << record.year << record.mission << record.missionStr << record.civID << record.wantsParley << record.undead << record.planless << record.handed_over_artifact;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<SiegeRecord>& results)
    {
        db << sql >> [&results](int32_t event_id, int32_t siegeId, int32_t year, int32_t mission, const std::string& missionStr, int32_t civID, bool wantsParley, bool undead, bool planless, bool handed_over_artifact)
        {
            results.emplace_back(event_id, siegeId, year, mission, missionStr, civID, wantsParley, undead, planless, handed_over_artifact);
        };
    };

};
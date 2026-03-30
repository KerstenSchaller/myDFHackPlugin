
// Query functions
#include "SQLiteWrapper.hpp"

// forward declarations for tables and db
extern std::shared_ptr<DB::Database> myDb;
extern std::shared_ptr<DB::Table<UnitRecord>> unitsTable;
extern std::shared_ptr<DB::Table<JobRecord>> jobsTable;

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

    if (year != -1) // if year  is -1, get all citizens, otherwise filter by year
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
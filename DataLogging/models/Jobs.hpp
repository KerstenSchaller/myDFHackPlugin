#pragma once

#include <type_traits>

#include "SQLITEWrapper.hpp"

#include "modules/Materials.h"
#include "df/material.h"
#include "df/job.h"


class JobRecord : public DB::BaseModel
{
    public:
    int64_t event_id;
    int64_t unit_tid; // lookUp in unit_table
    std::string job_name;
    uint8_t job_type; // replace with enum value
    uint8_t material_type;
    uint8_t material_index;
    std::string material;

    JobRecord() = default;
    JobRecord(int64_t event_id, int64_t unit_tid, const std::string& job_name, uint8_t job_type, 
              uint8_t material_type, uint8_t material_index) 
        : event_id(event_id),
          unit_tid(unit_tid),
          job_name(job_name), 
          job_type(job_type), 
          material_type(material_type), 
          material_index(material_index) 
    {
        DFHack::MaterialInfo mat;
        mat.decode(material_type, material_index);
        material = mat.toString();
    }

    JobRecord(int64_t event_id, int64_t unit_tid, df::job* job)
        : event_id(event_id),
          unit_tid(unit_tid), 
          job_name(ENUM_KEY_STR(job_type, job->job_type)), 
          job_type(job->job_type), 
          material_type(job->mat_type), 
          material_index(job->mat_index) 
    {
        DFHack::MaterialInfo mat;
        mat.decode(material_type, material_index);
        material = mat.toString();
    }

    std::string tableName() const override
    {
        return "job_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"event_id INTEGER","unit_tid INTEGER","job_name TEXT", "job_type INTEGER", "material_type INTEGER", "material_index INTEGER", "material TEXT"};
    }
};

template<>
struct DB::ModelTraits<JobRecord>
{
    static std::string insertColumns()
    {
        return "event_id,unit_tid,job_name,job_type,material_type,material_index,material";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?,?";
    }

    template<typename Statement>
    static void bindInsert(Statement& statement, const JobRecord& record)
    {
        statement << record.event_id << record.unit_tid << record.job_name << record.job_type << record.material_type << record.material_index << record.material;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<JobRecord>& results)
    {
        db << sql >> [&results](int64_t event_id, int64_t unit_tid, const std::string& job_name, uint8_t job_type, uint8_t material_type, uint8_t material_index, const std::string& material)
        {
            results.emplace_back(event_id, unit_tid, job_name, job_type, material_type, material_index);
        };
    }
};


    
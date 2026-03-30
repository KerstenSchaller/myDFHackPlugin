#pragma once

#include "df/world.h"
#include "df/report.h"

#include "SQLITEWrapper.hpp"


class AnnouncementRecord : DB::BaseModel
{
    public:
    int64_t event_id;
    int32_t announcement_id;
    std::string text;
    //"data":{"type":256,"year":110,"pos2":{"x":-30000,"z":-30000,"y":-30000,"zoom_type":0},"unconscious":false,"speaker_id":-1,"announcement":true,"text":"Zan Kikrostborlon has created a masterpiece ☼Riddlepartnered's giant cave spider silk cloth☼!","activity_event_id":-1,"activity_id":-1,"pos1":{"x":92,"z":29,"y":120,"zoom_type":0},"id":23404,"time":21542,"continuation":false}}
    int32_t type;
    int32_t year;
    int32_t pos_x;
    int32_t pos_y;
    int32_t pos_z;
    int32_t zoom_type;
    bool unconscious;
    int32_t speaker_id;
    bool announcement;
    int32_t activity_event_id;
    int32_t activity_id;
    int32_t pos1_x;
    int32_t pos1_y;
    int32_t pos1_z;
    int32_t pos1_zoom_type;
    int32_t time;
    bool continuation;

    AnnouncementRecord() = default;

    AnnouncementRecord(
        int64_t event_id,
        int32_t announcement_id,
        const std::string& text,
        int32_t type,
        int32_t year,
        int32_t pos_x,
        int32_t pos_y,
        int32_t pos_z,
        int32_t zoom_type,
        bool unconscious,
        int32_t speaker_id,
        bool announcement,
        int32_t activity_event_id,
        int32_t activity_id,
        int32_t pos1_x,
        int32_t pos1_y,
        int32_t pos1_z,
        int32_t pos1_zoom_type,
        int32_t time,
        bool continuation
    ) : event_id(event_id),
        announcement_id(announcement_id),
        text(text),
        type(type),
        year(year),
        pos_x(pos_x),
        pos_y(pos_y),
        pos_z(pos_z),
        zoom_type(zoom_type),
        unconscious(unconscious),
        speaker_id(speaker_id),
        announcement(announcement),
        activity_event_id(activity_event_id),
        activity_id(activity_id),
        pos1_x(pos1_x),
        pos1_y(pos1_y),
        pos1_z(pos1_z),
        pos1_zoom_type(pos1_zoom_type),
        time(time),
        continuation(continuation) {}

        AnnouncementRecord(int64_t event_id, df::report* report) : event_id(event_id)
        {
            if (report)
            {
                announcement_id = report->id;
                text = report->text;
                type = report->type;
                year = report->year;
                pos_x = report->pos.x;
                pos_y = report->pos.y;
                pos_z = report->pos.z;
                zoom_type = report->zoom_type;
                unconscious = report->flags.bits.unconscious;
                speaker_id = report->speaker_id;
                announcement = report->flags.bits.announcement;
                activity_event_id = report->activity_event_id;
                activity_id = report->activity_id;
                pos1_x = report->pos2.x;
                pos1_y = report->pos2.y;
                pos1_z = report->pos2.z;
                pos1_zoom_type = report->zoom_type2;
                time = report->time;
                continuation = report->flags.bits.continuation;
            }
        }

    std::string tableName() const override
    {
        return "announcement_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {
            "event_id INTEGER",
            "announcement_id INTEGER",
            "text TEXT",
            "type INTEGER",
            "year INTEGER",
            "pos_x INTEGER",
            "pos_y INTEGER",
            "pos_z INTEGER",
            "zoom_type INTEGER",
            "unconscious INTEGER",
            "speaker_id INTEGER",
            "announcement INTEGER",
            "activity_event_id INTEGER",
            "activity_id INTEGER",
            "pos1_x INTEGER",
            "pos1_y INTEGER",
            "pos1_z INTEGER",
            "pos1_zoom_type INTEGER",
            "time INTEGER",
            "continuation INTEGER"
        };
    }

};

template<>
struct DB::ModelTraits<AnnouncementRecord>
{
    static std::string insertColumns()
    {
        return "event_id,announcement_id,text,type,year,pos_x,pos_y,pos_z,zoom_type,unconscious,speaker_id,announcement,activity_event_id,activity_id,pos1_x,pos1_y,pos1_z,pos1_zoom_type,time,continuation";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?";
    }

    template<typename Statement>
    static void bindInsert(Statement& statement, const AnnouncementRecord& record)
    {
        statement << record.event_id
                  << record.announcement_id
                  << DF2UTF(record.text)
                  << record.type
                  << record.year
                  << record.pos_x
                  << record.pos_y
                  << record.pos_z
                  << record.zoom_type
                  << static_cast<int>(record.unconscious)
                  << record.speaker_id
                  << static_cast<int>(record.announcement)
                  << record.activity_event_id
                  << record.activity_id
                  << record.pos1_x
                  << record.pos1_y
                  << record.pos1_z
                  << record.pos1_zoom_type
                  << record.time
                  << static_cast<int>(record.continuation);
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<AnnouncementRecord>& results)
    {
        db << sql >> [&results](int64_t event_id, int32_t announcement_id, const std::string& text, int32_t type, int32_t year, int32_t pos_x, int32_t pos_y, int32_t pos_z, int32_t zoom_type, int unconsciousInt, int32_t speaker_id, int announcementInt, int32_t activity_event_id, int32_t activity_id, int32_t pos1_x, int32_t pos1_y, int32_t pos1_z, int32_t pos1_zoom_type, int32_t time, int continuationInt)
        {
            bool unconscious = static_cast<bool>(unconsciousInt);
            bool announcement = static_cast<bool>(announcementInt);
            bool continuation = static_cast<bool>(continuationInt);

            results.emplace_back(event_id, announcement_id, text, type, year, pos_x, pos_y, pos_z, zoom_type, unconscious, speaker_id, announcement, activity_event_id, activity_id, pos1_x, pos1_y, pos1_z, pos1_zoom_type, time, continuation);
        };
    }

};
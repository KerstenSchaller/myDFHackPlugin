#pragma once
#include "SQLITEWrapper.hpp"
#include "df/item.h"

class ItemRecord : public DB::BaseModel
{
    public:
        int64_t event_id;
        int64_t item_id;
        std::string item_name;
        int64_t item_type;
        std::string item_type_str;
        std::string item_descr;
        bool is_artifact;
        int64_t mat_index;
        int64_t mat_type;
        int64_t quality;
        int64_t value;
        std::string bookTitle;

    ItemRecord() = default;
    ItemRecord(int64_t event_id, int64_t item_id, const std::string& item_name, int64_t item_type, const std::string& item_type_str, const std::string& item_descr, bool is_artifact, int64_t mat_index, int64_t mat_type, int64_t quality, int64_t value)
        : event_id(event_id), item_id(item_id), item_name(item_name), item_type(item_type), item_type_str(item_type_str), item_descr(item_descr), is_artifact(is_artifact), mat_index(mat_index), mat_type(mat_type), quality(quality), value(value) {}

    ItemRecord(int64_t event_id, df::item* item)
        : event_id(event_id),
          item_id(item->id),
          item_name(DF2UTF(DFHack::Items::getDescription(item))),
          item_type(item->getType()),
          item_type_str(ENUM_KEY_STR(item_type, item->getType())),
          item_descr(DF2UTF(DFHack::Items::getReadableDescription(item))),
          is_artifact(item->flags.bits.artifact),
          mat_index(item->getMaterialIndex()),
          mat_type(item->getMaterial()),
          quality(item->getQuality()),
          value(DFHack::Items::getValue(item)) {}

    std::string tableName() const override
    {
        return "item_records";
    }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"event_id INTEGER","item_id INTEGER","item_name TEXT","item_type INTEGER","item_type_str TEXT","item_descr TEXT","is_artifact BOOLEAN","mat_index INTEGER","mat_type INTEGER","quality INTEGER","value INTEGER","book_title TEXT"};
    }

};

template<>
struct DB::ModelTraits<ItemRecord>
{
    static std::string insertColumns()
    {
        return "event_id,item_id,item_name,item_type,item_type_str,item_descr,is_artifact,mat_index,mat_type,quality,value,book_title";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?,?,?,?,?,?,?";
    }

    template <typename Statement>
    static void bindInsert(Statement &statement, const ItemRecord &record)
    {
        statement << record.event_id << record.item_id << record.item_name << record.item_type << record.item_type_str << record.item_descr << record.is_artifact << record.mat_index << record.mat_type << record.quality << record.value << record.bookTitle;
    }

    static void select(sqlite::database &db, const std::string &sql, std::vector<ItemRecord> &results)
    {
        db << sql >> [&results](int64_t event_id, int64_t item_id, const std::string& item_name, int64_t item_type, const std::string& item_type_str, const std::string& item_descr, bool is_artifact, int64_t mat_index, int64_t mat_type, int64_t quality, int64_t value, const std::string& book_title)
        {
            ItemRecord r(event_id, item_id, item_name, item_type, item_type_str, item_descr, is_artifact, mat_index, mat_type, quality, value);
            r.bookTitle = book_title;
            results.push_back(std::move(r));
        };
    }
};
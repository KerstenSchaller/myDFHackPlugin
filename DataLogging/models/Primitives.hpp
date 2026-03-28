#pragma once

#include <string>
#include <vector>

#include "SQLITEWrapper.hpp"

// Single-value wrapper models for use with DB::Table<T> (insert/queryWhere).
// Direct ModelTraits specializations for int32_t, bool, std::string also provided
// so that DB::Database::query<T>() works with raw primitive types (e.g. getUniqueYears).

// ---------------------------------------------------------------------------
// IntModel
// ---------------------------------------------------------------------------

class IntModel : public DB::BaseModel {
public:
    int32_t value;

    IntModel() = default;
    IntModel(int32_t value) : value(value) {}

    std::string tableName() const override { return ""; }
    std::vector<std::string> columnDefinitions() const override { return {"value INTEGER"}; }
};

template<>
struct DB::ModelTraits<IntModel> {
    static std::string insertColumns() { return "value"; }
    static std::string insertPlaceholders() { return "?"; }

    template<typename Statement>
    static void bindInsert(Statement& statement, const IntModel& m) {
        statement << m.value;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<IntModel>& results) {
        db << sql >> [&results](int32_t v) {
            results.emplace_back(v);
        };
    }
};

// Direct specialization for query<int32_t>() (e.g. getUniqueYears)
template<>
struct DB::ModelTraits<int32_t> {
    static void select(sqlite::database& db, const std::string& sql, std::vector<int32_t>& results) {
        db << sql >> [&results](int32_t v) {
            results.push_back(v);
        };
    }
};

// ---------------------------------------------------------------------------
// BoolModel
// ---------------------------------------------------------------------------

class BoolModel : public DB::BaseModel {
public:
    bool value;

    BoolModel() = default;
    BoolModel(bool value) : value(value) {}

    std::string tableName() const override { return ""; }
    std::vector<std::string> columnDefinitions() const override { return {"value INTEGER"}; }
};

template<>
struct DB::ModelTraits<BoolModel> {
    static std::string insertColumns() { return "value"; }
    static std::string insertPlaceholders() { return "?"; }

    template<typename Statement>
    static void bindInsert(Statement& statement, const BoolModel& m) {
        statement << static_cast<int>(m.value);
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<BoolModel>& results) {
        db << sql >> [&results](int v) {
            results.emplace_back(static_cast<bool>(v));
        };
    }
};

// Direct specialization for query<bool>()
template<>
struct DB::ModelTraits<bool> {
    static void select(sqlite::database& db, const std::string& sql, std::vector<bool>& results) {
        db << sql >> [&results](int v) {
            results.push_back(static_cast<bool>(v));
        };
    }
};

// ---------------------------------------------------------------------------
// StringModel
// ---------------------------------------------------------------------------

class StringModel : public DB::BaseModel {
public:
    std::string value;

    StringModel() = default;
    StringModel(const std::string& value) : value(value) {}

    std::string tableName() const override { return ""; }
    std::vector<std::string> columnDefinitions() const override { return {"value TEXT"}; }
};

template<>
struct DB::ModelTraits<StringModel> {
    static std::string insertColumns() { return "value"; }
    static std::string insertPlaceholders() { return "?"; }

    template<typename Statement>
    static void bindInsert(Statement& statement, const StringModel& m) {
        statement << m.value;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<StringModel>& results) {
        db << sql >> [&results](std::string v) {
            results.emplace_back(v);
        };
    }
};

// Direct specialization for query<std::string>()
template<>
struct DB::ModelTraits<std::string> {
    static void select(sqlite::database& db, const std::string& sql, std::vector<std::string>& results) {
        db << sql >> [&results](std::string v) {
            results.push_back(std::move(v));
        };
    }
};

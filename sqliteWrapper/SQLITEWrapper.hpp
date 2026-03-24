#ifndef SQLITEWRAPPER_HPP
#define SQLITEWRAPPER_HPP

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>

#include "sqlite_modern_cpp.h"

#include "Logger.hpp"

namespace DB{


template<typename>
struct dependent_false : std::false_type {};

template<typename T>
struct ModelTraits {
    static_assert(dependent_false<T>::value, "ModelTraits specialization is required for this model type.");
};

class BaseModel {
public:
    virtual ~BaseModel() = default;
    virtual std::string tableName() const = 0;
    virtual std::vector<std::string> columnDefinitions() const = 0;
};

/* Example model and traits specialization:
class User : public BaseModel {
public:
    std::string name;
    int age;
    
    User() = default;
    User(int id, const std::string& name, int age) :  name(name), age(age) {}

    std::string tableName() const override {
        return "users";
    }

    std::vector<std::string> columnDefinitions() const override {
        return {"name TEXT", "age INTEGER"};
    }
};

template<>
struct ModelTraits<User> {
    static std::string insertColumns() {
        return "name,age";
    }

    static std::string insertPlaceholders() {
        return "?,?";
    }

    template<typename Statement>
    static void bindInsert(Statement& statement, const User& user) {
        statement << user.name << user.age;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<User>& results) {
        db << sql >> [&results](int id, const std::string& name, int age) {
            results.emplace_back(id, name, age);
        };
    }
};

*/


class WhereClause {
public:
    std::string key;
    std::string conditionP2;
    std::string connector = "";

    WhereClause(const std::string& key, const std::string& conditionP2, const std::string& connector = "") : key(key), conditionP2(conditionP2), connector(connector) {}
};


template<typename T>
class Table {
    sqlite::database& db;
public:
    Table(sqlite::database& db) : db(db) {}

    std::vector<T> queryWhere(const std::vector<WhereClause>& whereClauses) {
        std::vector<T> results;
        std::string sqlString = "SELECT * FROM " + T().tableName() + " WHERE ";

        for (size_t i = 0; i < whereClauses.size(); ++i) {
            std::string connector = (i == 0) ? "" : whereClauses[i].connector;
            sqlString += connector + " " + whereClauses[i].key + " " + whereClauses[i].conditionP2 + " ";
        }
        sqlString += ";";

        try {
            ModelTraits<T>::select(db, sqlString, results);
        } catch (std::exception& e) {
            std::cerr << "Error querying data: " << e.what() << "\n";
            Logger::log("Error querying data: " + std::string(e.what()) + "\nSQL: " + sqlString);
        }
        return results;
    }

    uint64_t insertData(const T& data)
    {
        std::string sqlString = "INSERT INTO " + data.tableName() + " ("
            + ModelTraits<T>::insertColumns() + ") VALUES ("
            + ModelTraits<T>::insertPlaceholders() + ");";

        try {
            auto statement = db << sqlString;
            ModelTraits<T>::bindInsert(statement, data);
            statement++;
        } catch (std::exception& e) {
            std::cerr << "Error inserting data: " << e.what() << "\n";
            std::cout << "SQL: " << sqlString << "\n";
            Logger::log("Error inserting data: " + std::string(e.what()) + "\nSQL: " + sqlString);
        }
        return db.last_insert_rowid();
    }
};

class Database {
    sqlite::database db;
    public:
        Database(const std::string& db_name) : db(db_name){ }

        template<typename T>
        Table<T> getTable() {
            return Table<T>(db);
        }

        template<typename T>
        Table<T> create_table() {
            create_table(T().tableName(), T().columnDefinitions());
            return Table<T>(db);
        }

        void create_table(const std::string& tableName, const std::vector<std::string>& columns)
        {
            std::string sqlString = "CREATE TABLE IF NOT EXISTS " + tableName + " (id INTEGER PRIMARY KEY AUTOINCREMENT,";
            for (size_t i = 0; i < columns.size(); ++i) {
            sqlString += columns[i];
            if (i < columns.size() - 1) {
                sqlString += ",";
            }
            }
            sqlString += ");";

            try {
            db << sqlString;
            } catch (std::exception& e) {
            std::cerr << "Error creating table: " << e.what() << "\n";
            std::cout << "SQL: " << sqlString << "\n";
            Logger::log("Error creating table: " + std::string(e.what()) + "\nSQL: " + sqlString);
            }
        }
    };

}


#endif // SQLITEWRAPPER_HPP
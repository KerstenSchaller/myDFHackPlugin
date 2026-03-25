#pragma once

#include "df/unit.h"
#include "modules/Units.h"

#include <type_traits>
#include "SQLITEWrapper.hpp"
#include "df/history_event_hf_relationship_deniedst.h"

using namespace DFHack;

class UnitRecord : public DB::BaseModel
{
    private:
        std::string getSex(int16_t sex)
        {
            if (sex == 0)return "Female";
            if (sex == 1)return "Male";
            return "Unknown";
        }
    public:
      int64_t event_id;
      std::string name;
      std::string name_english;
      double age;
      std::string sex;
      std::string race;
      std::string profession;
      int64_t fatherId;
      int64_t motherId;
      int64_t spouseId;
      bool hostile;
      int64_t unit_id;
      bool isAnimal;
      bool isCitizen;
      bool isGuest;
      bool isMerchant;
      bool isPet;
      bool isResident;
      bool caged;
      bool butchered;
      bool isTame;
      int64_t petOwner;
      int64_t invasion_role;

      UnitRecord() = default;
            UnitRecord(int64_t event_id, const std::string &name, const std::string &name_english,
                                 double age, const std::string &sex, const std::string &race,
                                 const std::string &profession, int64_t fatherId, int64_t motherId,
                                 int64_t spouseId, bool hostile, int64_t unit_id, bool isAnimal,
                                 bool isCitizen, bool isGuest, bool isMerchant, bool isPet,
                                 bool isResident, bool caged, bool butchered, bool isTame,
                                 int64_t petOwner, int64_t invasion_role)
                    : event_id(event_id),
                      name(name),
                      name_english(name_english),
                      age(age),
                      sex(sex),
                      race(race),
                      profession(profession),
                      fatherId(fatherId),
                      motherId(motherId),
                      spouseId(spouseId),
                      hostile(hostile),
                      unit_id(unit_id),
                      isAnimal(isAnimal),
                      isCitizen(isCitizen),
                      isGuest(isGuest),
                      isMerchant(isMerchant),
                      isPet(isPet),
                      isResident(isResident),
                      caged(caged),
                      butchered(butchered),
                      isTame(isTame),
                      petOwner(petOwner),
                      invasion_role(invasion_role) {}
      UnitRecord(int64_t event_id, df::unit *unit)
                    : event_id(event_id),
                      name(DF2UTF(Units::getReadableName(unit, false))),
                      name_english(DF2UTF(Units::getReadableName(unit, true))),
                      age(Units::getAge(unit)), sex(getSex(unit->sex)),
                      race(Units::getRaceName(unit)),
                      profession(Units::getProfessionName(unit)),
                      fatherId(unit->relationship_ids[df::unit_relationship_type::Father]),
                      motherId(unit->relationship_ids[df::unit_relationship_type::Mother]),
                      spouseId(unit->relationship_ids[df::unit_relationship_type::Spouse]),
                      hostile(Units::isDanger(unit)),
                      unit_id(unit->id),
                      isAnimal(Units::isAnimal(unit)),
                      isCitizen(Units::isCitizen(unit)),
                      isGuest(Units::isVisitor(unit)),
                      isMerchant(Units::isMerchant(unit)),
                      isPet(Units::isPet(unit)),
                      isResident(Units::isResident(unit)),
                      caged(unit->flags1.bits.caged),
                      butchered(unit->flags2.bits.slaughter),
                      isTame(Units::isTame(unit)),
                      petOwner(unit->relationship_ids[df::unit_relationship_type::PetOwner]),
                      invasion_role(unit->invasion_role) {}

      std::string tableName() const override
      {
          return "unit_records";
      }

    std::vector<std::string> columnDefinitions() const override
    {
        return {"event_id INTEGER", "name TEXT", "name_english TEXT", "age INTEGER", "sex TEXT", "race TEXT", "profession TEXT", "fatherId INTEGER", "motherId INTEGER", "spouseId INTEGER", "hostile BOOLEAN", "unit_id INTEGER", "isAnimal BOOLEAN", "isCitizen BOOLEAN", "isGuest BOOLEAN", "isMerchant BOOLEAN", "isPet BOOLEAN", "isResident BOOLEAN", "caged BOOLEAN", "butchered BOOLEAN", "isTame BOOLEAN", "petOwner INTEGER", "invasion_role INTEGER"};
    }


};

template<>
struct DB::ModelTraits<UnitRecord>
{
    static std::string insertColumns()
    {
        return "event_id,name,name_english,age,sex,race,profession,fatherId,motherId,spouseId,hostile,unit_id,isAnimal,isCitizen,isGuest,isMerchant,isPet,isResident,caged,butchered,isTame,petOwner,invasion_role";
    }

    static std::string insertPlaceholders()
    {
        return "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?";
    }

    template<typename Statement>
    static void bindInsert(Statement& statement, const UnitRecord& record)
    {
        statement << record.event_id << record.name << record.name_english << record.age << record.sex << record.race << record.profession << record.fatherId << record.motherId << record.spouseId << record.hostile << record.unit_id << record.isAnimal << record.isCitizen << record.isGuest << record.isMerchant << record.isPet << record.isResident << record.caged << record.butchered << record.isTame << record.petOwner << record.invasion_role;
    }

    static void select(sqlite::database& db, const std::string& sql, std::vector<UnitRecord>& results)
    {
        db << sql >> [&results](int64_t event_id, const std::string &name, const std::string &name_english, double age, const std::string &sex, const std::string &race, const std::string &profession, int64_t fatherId, int64_t motherId, int64_t spouseId, bool hostile, int64_t unit_id, bool isAnimal, bool isCitizen, bool isGuest, bool isMerchant, bool isPet, bool isResident, bool caged, bool butchered, bool isTame, int64_t petOwner, int64_t invasion_role)
        {
            results.emplace_back(event_id, name, name_english, age, sex, race, profession, fatherId, motherId, spouseId, hostile, unit_id, isAnimal, isCitizen, isGuest, isMerchant, isPet, isResident, caged, butchered, isTame, petOwner, invasion_role);
        };
    };
};
#pragma once

#include <type_traits>
#include "df/agreement.h"
#include "df/agreement_details.h"
#include "df/agreement_details_data_citizenship.h"
#include "df/agreement_details_data_demonic_binding.h"
#include "df/agreement_details_data_join_party.h"
#include "df/agreement_details_data_location.h"
#include "df/agreement_details_data_offer_service.h"
#include "df/agreement_details_data_parley.h"
#include "df/agreement_details_data_plot_abduct.h"
#include "df/agreement_details_data_plot_assassination.h"
#include "df/agreement_details_data_plot_conviction.h"
#include "df/agreement_details_data_plot_frame_treason.h"
#include "df/agreement_details_data_plot_induce_war.h"
#include "df/agreement_details_data_plot_infiltration_coup.h"
#include "df/agreement_details_data_plot_sabotage.h"
#include "df/agreement_details_data_plot_steal_artifact.h"
#include "df/agreement_details_data_position_corruption.h"
#include "df/agreement_details_data_promise_position.h"
#include "df/agreement_details_data_residency.h"
#include "df/agreement_details_data_retrieve_artifact.h"

#include "SQLITEWrapper.hpp"




class PetitionRecord : public DB::BaseModel
{
public:
  int64_t event_id;
  int32_t agreement_id;
  int32_t year;
  int32_t ticks;
  std::string details_type_str;

  // Fields for Parley and Residency types
  int32_t reason;
  std::string reason_str;
  int32_t applicant_party;
  int32_t government_party;
  int32_t site_id;
  int32_t end_year;
  int32_t end_season_tick;

    // Fields for Location type
  int32_t location_type;
  std::string location_type_str;
  int32_t location_tier;
  int32_t location_profession;
  std::string location_profession_str;
  int32_t location_deity_type;
  std::string location_deity_type_str;
  int32_t location_deity_data;
  bool location_warned_is_ready;

    // Fields for OfferService type
  int32_t service_requesting_party;
  int32_t service_serving_party;
  int32_t service_served_entity;

  PetitionRecord()
    : event_id(-1),
      agreement_id(-1),
      year(-1),
      ticks(-1),
      reason(-1),
      applicant_party(-1),
      government_party(-1),
      site_id(-1),
      end_year(-1),
      end_season_tick(-1),
      location_type(-1),
      location_tier(-1),
      location_profession(-1),
      location_deity_type(-1),
      location_deity_data(-1),
      location_warned_is_ready(false),
      service_requesting_party(-1),
      service_serving_party(-1),
      service_served_entity(-1) {}

  PetitionRecord(int64_t eventId, df::agreement *agreement, df::agreement_details *details)
    : PetitionRecord()
  {
    event_id = eventId;
    if (!agreement || !details)
    {
      return;
    }

    agreement_id = agreement->id;
  year = details->year;
  ticks = details->year_tick;
    details_type_str = ENUM_KEY_STR(agreement_details_type, details->type);

    switch (details->type)
    {
    case df::agreement_details_type::Citizenship:
      if (auto *data = details->data.Citizenship)
      {
        applicant_party = data->applicant;
        government_party = data->government;
        site_id = data->site;
        end_year = data->end_year;
        end_season_tick = data->end_season_tick;
      }
      break;

    case df::agreement_details_type::Location:
      if (auto *data = details->data.Location)
      {
        applicant_party = data->applicant;
        government_party = data->government;
        site_id = data->site;

        location_type = static_cast<int32_t>(data->type);
        location_type_str = ENUM_KEY_STR(abstract_building_type, data->type);

        location_tier = data->tier;
        location_profession = static_cast<int32_t>(data->profession);
        location_profession_str = ENUM_KEY_STR(profession, data->profession);

        location_deity_type = static_cast<int32_t>(data->deity_type);
        location_deity_type_str = ENUM_KEY_STR(religious_practice_type, data->deity_type);
        location_deity_data = data->deity_data.practice_id;

        location_warned_is_ready = data->flags.bits.warned_is_ready;
      }
      break;

    case df::agreement_details_type::OfferService:
      if (auto *data = details->data.OfferService)
      {
        service_requesting_party = data->requesting_party;
        service_serving_party = data->serving_party;
        service_served_entity = data->served_entity;
      }
      break;

    case df::agreement_details_type::Parley:
      if (auto *data = details->data.Parley)
      {
        reason = static_cast<int32_t>(data->reason);
        reason_str = ENUM_KEY_STR(history_event_reason, data->reason);

        applicant_party = data->asker;
        government_party = data->target;
        site_id = data->site;
        end_year = data->end_year;
        end_season_tick = data->end_season_tick;
      }
      break;

    case df::agreement_details_type::Residency:
      if (auto *data = details->data.Residency)
      {
        reason = static_cast<int32_t>(data->reason);
        reason_str = ENUM_KEY_STR(history_event_reason, data->reason);

        applicant_party = data->applicant;
        government_party = data->government;
        site_id = data->site;
        end_year = data->end_year;
        end_season_tick = data->end_season_tick;
      }
      break;

    default:
      break;
    }
  }

  std::string tableName() const override
  {
    return "petition_records";
  }

  std::vector<std::string> columnDefinitions() const override
  {
    return {
      "event_id INTEGER",
      "agreement_id INTEGER",
      "year INTEGER",
      "ticks INTEGER",
      "details_type_str TEXT",
      "reason INTEGER",
      "reason_str TEXT",
      "applicant_party INTEGER",
      "government_party INTEGER",
      "site_id INTEGER",
      "end_year INTEGER",
      "end_season_tick INTEGER",
      "location_type INTEGER",
      "location_type_str TEXT",
      "location_tier INTEGER",
      "location_profession INTEGER",
      "location_profession_str TEXT",
      "location_deity_type INTEGER",
      "location_deity_type_str TEXT",
      "location_deity_data INTEGER",
      "location_warned_is_ready BOOLEAN",
      "service_requesting_party INTEGER",
      "service_serving_party INTEGER",
      "service_served_entity INTEGER"};
  }
};

template<>
struct DB::ModelTraits<PetitionRecord>
{
  static std::string insertColumns()
  {
    return "event_id,agreement_id,year,ticks,details_type_str,reason,reason_str,applicant_party,government_party,site_id,end_year,end_season_tick,location_type,location_type_str,location_tier,location_profession,location_profession_str,location_deity_type,location_deity_type_str,location_deity_data,location_warned_is_ready,service_requesting_party,service_serving_party,service_served_entity";
  }

  static std::string insertPlaceholders()
  {
    return "?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?";
  }

  template<typename Statement>
  static void bindInsert(Statement &statement, const PetitionRecord &record)
  {
    statement << record.event_id
          << record.agreement_id
          << record.year
          << record.ticks
          << record.details_type_str
          << record.reason
          << record.reason_str
          << record.applicant_party
          << record.government_party
          << record.site_id
          << record.end_year
          << record.end_season_tick
          << record.location_type
          << record.location_type_str
          << record.location_tier
          << record.location_profession
          << record.location_profession_str
          << record.location_deity_type
          << record.location_deity_type_str
          << record.location_deity_data
          << record.location_warned_is_ready
          << record.service_requesting_party
          << record.service_serving_party
          << record.service_served_entity;
  }

  static void select(sqlite::database &db, const std::string &sql, std::vector<PetitionRecord> &results)
  {
    db << sql >> [&results](
      int64_t event_id,
      int32_t agreement_id,
      int32_t year,
      int32_t ticks,
      const std::string &details_type_str,
      int32_t reason,
      const std::string &reason_str,
      int32_t applicant_party,
      int32_t government_party,
      int32_t site_id,
      int32_t end_year,
      int32_t end_season_tick,
      int32_t location_type,
      const std::string &location_type_str,
      int32_t location_tier,
      int32_t location_profession,
      const std::string &location_profession_str,
      int32_t location_deity_type,
      const std::string &location_deity_type_str,
      int32_t location_deity_data,
      bool location_warned_is_ready,
      int32_t service_requesting_party,
      int32_t service_serving_party,
      int32_t service_served_entity)
    {
      PetitionRecord record;
      record.event_id = event_id;
      record.agreement_id = agreement_id;
      record.year = year;
      record.ticks = ticks;
      record.details_type_str = details_type_str;
      record.reason = reason;
      record.reason_str = reason_str;
      record.applicant_party = applicant_party;
      record.government_party = government_party;
      record.site_id = site_id;
      record.end_year = end_year;
      record.end_season_tick = end_season_tick;
      record.location_type = location_type;
      record.location_type_str = location_type_str;
      record.location_tier = location_tier;
      record.location_profession = location_profession;
      record.location_profession_str = location_profession_str;
      record.location_deity_type = location_deity_type;
      record.location_deity_type_str = location_deity_type_str;
      record.location_deity_data = location_deity_data;
      record.location_warned_is_ready = location_warned_is_ready;
      record.service_requesting_party = service_requesting_party;
      record.service_serving_party = service_serving_party;
      record.service_served_entity = service_served_entity;
      results.push_back(std::move(record));
    };
  }
};
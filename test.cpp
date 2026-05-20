#include "test.hpp"

#include "ColorText.h"

#include "df/world.h"
#include "df/occupation.h"
#include "df/unit.h"
#include "df/world_site.h"
#include "df/historical_entity.h"
#include "modules/World.h"
#include <map>
#include <vector>

// Map from guild occupation pointer to vector of member unit pointers
std::map<df::occupation*, std::vector<df::unit*>> guilds_and_members;

command_result test_guilds_and_members(color_ostream &out, state_change_event event) 
{
    int32_t current_site_id = DFHack::World::GetCurrentSiteId(); // or use World::GetCurrentSiteId()

    for (auto occ : df::global::world->occupations.all) 
    {
        // Check if occupation is a guild and belongs to the current fortress site
        if ((occ->type == df::occupation_type::CraftGuild ||
             occ->type == df::occupation_type::MerchantGuild ||
             occ->type == df::occupation_type::FarmerGuild) // add other guild types as needed
            && occ->site_id == current_site_id)
        {
            df::unit* member = df::unit::find(occ->unit_id);
            if (member) 
            {
                guilds_and_members[occ].push_back(member);
            }
        }
    }
}

// Now guilds_and_members contains all guilds and their members for the current fortress
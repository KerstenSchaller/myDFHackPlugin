
#include "Deaths.hpp"

incident_info getIncidentInfo(df::unit* unit)
{
    auto incidents = df::global::world->incidents.all;
    for (auto incident : incidents)
    {
        if (incident->type == df::incident_type::Death)
        {
            if (incident->victim == unit->id)
            {
                incident_info info;
                info.killer = df::unit::find(incident->criminal);
                info.death_cause = incident->death_cause;
                return info;
            }
        }
    }
    return {nullptr, df::death_type::NONE};
}
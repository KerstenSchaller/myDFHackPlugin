#include "modules/World.h"

#include "DateAndTime.hpp"


date getDate()
{
    auto day = DFHack::World::ReadCurrentDay();
    auto month = DFHack::World::ReadCurrentMonth();
    auto year = DFHack::World::ReadCurrentYear();
    auto tick = DFHack::World::ReadCurrentTick();
    return date(day, month, year, tick);
}
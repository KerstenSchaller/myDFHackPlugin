#pragma once

#include "df/army_controller.h"
#include "df/historical_figure.h"
#include "df/invasion_info.h"
#include "df/item.h"
#include "df/unit.h"

static df::unit *getUnitFromHistFigId(int32_t histFigId)
{
    for (auto &unit : df::global::world->units.active)
    {
        if (unit && unit->hist_figure_id == histFigId)
            return unit;
    }
    return nullptr;
}

static df::unit *getMakerFromItem(df::item *item)
{
    auto maker = item->getMaker();
    if (!maker)
    {
        return nullptr;
    }
    return getUnitFromHistFigId(maker);
}

static df::army_controller *findArmyController(int32_t controller_id)
{
    for (auto *ac : df::global::world->army_controllers.all)
    {
        if (ac && ac->id == controller_id)
            return ac;
    }
    return nullptr;
}

static df::unit *getCommanderFromSiege(df::invasion_info *siege)
{
    if (!siege)
        return nullptr;
    auto *ac = findArmyController(siege->origin_master_army_controller_id);
    if (!ac)
        return nullptr;
    int32_t hfId = (ac->commander_hf != -1) ? ac->commander_hf : ac->master_hf;
    if (hfId == -1)
        return nullptr;
    auto *histFig = df::historical_figure::find(hfId);
    if (!histFig || histFig->unit_id == -1)
        return nullptr;
    return df::unit::find(histFig->unit_id);
}
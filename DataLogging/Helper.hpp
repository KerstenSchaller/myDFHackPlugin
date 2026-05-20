#pragma once
#include <iostream>
#include <chrono>
#include <vector>
#include <memory>
#include <numeric>
#include <fstream>
#include <unordered_map>

#include "df/army_controller.h"
#include "df/historical_figure.h"
#include "df/invasion_info.h"
#include "df/item.h"
#include "df/unit.h"

#include "Logger.hpp"



class Profiler
{
public:
    Profiler(const std::string& name) : name(name), averageDuration(0) {}

    void start()
    {
        start_time = std::chrono::high_resolution_clock::now();
    }
    void stop()
    {
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start_time).count();
        averageDuration = (averageDuration * currentNumberOfSamples + duration) / (currentNumberOfSamples + 1);
        if (currentNumberOfSamples < maxNumberOfSamples)
        {
            ++currentNumberOfSamples;
        }

    }

    std::chrono::high_resolution_clock::duration getAverageDuration() const
    {
        return std::chrono::milliseconds(static_cast<long long>(averageDuration));
    }

    std::string getName() const
    {
        return name;
    }

private:
    std::string name;
    std::chrono::high_resolution_clock::time_point start_time;
    double averageDuration = 0.0;
    int currentNumberOfSamples = 0;
    int maxNumberOfSamples = 100;

};

class ProfilerContainer
{
    // holds multiple profilers and has a function to log their
    // average durations to a file
public:
    void logAverages()
    {
        std::ofstream logFile("profiler_log.txt", std::ios_base::trunc);
        if (logFile.is_open())
        {
            for (const auto& profiler : profilers)
            {
                logFile << profiler->getName() << ": " << profiler->getAverageDuration().count() << " ms" << std::endl;
            }
            logFile.close();
        }
        else
        {
            std::cerr << "Unable to open profiler log file." << std::endl;
        }
    }
    void addProfiler(std::shared_ptr<Profiler> profiler)
    {
        profilers.push_back(profiler);
    }
    private:

    std::vector<std::shared_ptr<Profiler>> profilers;
};


static df::unit *getUnitFromHistFigId(int32_t histFigId)
{
    // add a map to store and return previously found units by histFigId to avoid repeated searches
    static std::unordered_map<int32_t, df::unit*> histFigIdToUnitMap;
    auto it = histFigIdToUnitMap.find(histFigId);
    if (it != histFigIdToUnitMap.end())
    {
        return it->second;
    }
    for (auto &unit : df::global::world->units.all)
    {
        if (unit && unit->hist_figure_id == histFigId)
        {
            histFigIdToUnitMap[histFigId] = unit;
            return unit;
        }
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
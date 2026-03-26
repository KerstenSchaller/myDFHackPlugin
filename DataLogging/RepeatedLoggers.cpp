#include "RepeatedLoggers.hpp"

int32_t BookLogger::lastItemIndexChecked = -1;
int32_t CitizenLogger::lastUnitIndexChecked = -1;
std::unordered_set<uint64_t> PetitionLogger::seenPetitionDetails;
int32_t SiegeLogger::lastSiegeIndexChecked = -1;
int32_t SiegeLogger::currentSiegeIndex = -1;
bool SiegeLogger::siegeActive = false;
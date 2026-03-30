#include "RepeatedLoggers.hpp"

std::unordered_set<uint64_t> BookLogger::seenBookIds;
std::unordered_set<uint64_t> CitizenLogger::seenUnitIds;
std::unordered_set<uint64_t> PetitionLogger::seenPetitionDetails;
std::unordered_set<uint64_t> SiegeLogger::seenSiegeStartIds;
std::unordered_set<uint64_t> SiegeLogger::seenSiegeEndIds;
std::unordered_set<uint64_t> AnnouncementLogger::seenAnnouncementIds;

size_t BookLogger::lastLoggedIndex = 0;
size_t CitizenLogger::lastLoggedIndex = 0;
size_t PetitionLogger::lastLoggedIndex = 0;
size_t SiegeLogger::lastLoggedIndex = 0;
size_t AnnouncementLogger::lastLoggedIndex = 0;

bool BookLogger::firstCheckDone = false;
bool CitizenLogger::firstCheckDone = false;
bool PetitionLogger::firstCheckDone = false;
bool SiegeLogger::firstCheckDone = false;
bool AnnouncementLogger::firstCheckDone = false;

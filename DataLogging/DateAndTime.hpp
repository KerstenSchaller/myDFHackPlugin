#pragma once

#include <iostream>
#include <string>
#include <cmath>



struct timePassedData {
    int16_t day;
    int16_t month;
    int16_t year;

    public:
    timePassedData(): day(-1), month(-1), year(-1) {}
    timePassedData(int16_t day, int16_t month, int16_t year): day(day), month(month), year(year) {}


    //overwrite == operator to compare timePassedData, return true if the time passed is greater than or equal to the other timePassedData
    bool operator==(const timePassedData& other) const 
    {

        return (year == other.year && month == other.month && day == other.day);
    }

    bool operator!=(const timePassedData& other) const 
    {
        return !(*this == other);
    }




    std::string toString() const 
    {
        return std::to_string(year) + " years, " + std::to_string(month) + " months, " + std::to_string(day) + " days";
    }
};

struct date {
    int16_t day;
    int16_t month;
    int16_t year;
    int32_t tick;

    public:
    date(): day(-1), month(-1), year(-1), tick(-1) {}
    date(int16_t day, int16_t month, int16_t year, int32_t tick) : day(day), month(month), year(year), tick(tick) {}

    // Subtract two dates using only calendar fields.
    // A month is 28 days and a year is 12 months. `tick` is intentionally ignored.
    timePassedData operator-(const date& other) const 
    {
        constexpr int32_t DAYS_PER_MONTH = 28;
        constexpr int32_t DAYS_PER_YEAR = DAYS_PER_MONTH * 12;

        const int32_t thisTotalDays =
            static_cast<int32_t>(year) * DAYS_PER_YEAR +
            static_cast<int32_t>(month) * DAYS_PER_MONTH +
            static_cast<int32_t>(day);
        const int32_t otherTotalDays =
            static_cast<int32_t>(other.year) * DAYS_PER_YEAR +
            static_cast<int32_t>(other.month) * DAYS_PER_MONTH +
            static_cast<int32_t>(other.day);

        int32_t deltaDays = thisTotalDays - otherTotalDays;
        const int32_t sign = (deltaDays < 0) ? -1 : 1;
        deltaDays = std::abs(deltaDays);

        const int16_t yearsPassed = static_cast<int16_t>(deltaDays / DAYS_PER_YEAR);
        deltaDays %= DAYS_PER_YEAR;
        const int16_t monthsPassed = static_cast<int16_t>(deltaDays / DAYS_PER_MONTH);
        const int16_t daysPassed = static_cast<int16_t>(deltaDays % DAYS_PER_MONTH);

        return timePassedData{
            static_cast<int16_t>(daysPassed * sign),
            static_cast<int16_t>(monthsPassed * sign),
            static_cast<int16_t>(yearsPassed * sign)
        };
    }

    std::string toString() const 
    {
        return std::to_string(year) + " years, " + std::to_string(month) + " months, " + std::to_string(day) + " days, " + std::to_string(tick) + " ticks";
    }


};

date getDate();
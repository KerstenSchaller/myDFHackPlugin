#include <iostream>
#include <string>
#include "../DateAndTime.hpp"

bool assertEqual(bool a, bool b, const std::string& testName) 
{
    if (a != b) {
        std::cerr << "Test failed: " << testName << " - Expected: " << a << ", Got: " << b << std::endl;
        return false;
    }
    return true;
}

bool assertTrue(bool condition, const std::string& testName) 
{
    if (!condition) {
        std::cerr << "Test failed: " << testName << " - Condition is false." << std::endl;
        return false;
    }
    return true;
}

bool assertFalse(bool condition, const std::string& testName) 
{
    if (condition) {
        std::cerr << "Test failed: " << testName << " - Condition is true." << std::endl;
        return false;
    }
    return true;
}


int main() 
{
    // Test timePassedData equality operator
    assertTrue(timePassedData{0,0,0} == timePassedData{0,0,0}, "timePassedDataEquality operator - equal times");
    assertFalse(timePassedData{0,0,0} == timePassedData{0,1,0}, "timePassedDataEquality operator - different months");
    assertFalse(timePassedData{0,0,0} == timePassedData{0,0,1}, "timePassedDataEquality operator - different days");
    assertFalse(timePassedData{0,0,0} == timePassedData{1,0,0}, "timePassedDataEquality operator - different years");
    
    // Test date subtraction operator (only based on ticks and years, since months and days are derived from ticks)

    date date1(1, 1, 1, 0);
    date date2(2, 1, 1, 0); // 1 day later
    if(!assertEqual(date2 - date1 == timePassedData{1,0,0}, true, "Date subtraction operator - 1 day later"))
    {
        timePassedData result = date2 - date1;
        std::cerr << "Result: " << result.toString() << std::endl;
    }

    date2 = date(1, 2, 1, 0); // 1 month later
    if(!assertEqual(date2 - date1 == timePassedData{0,1,0}, true, "Date subtraction operator - 1 month later"))
    {
        timePassedData result = date2 - date1;
        std::cerr << "Result: " << result.toString() << std::endl;
    }

    date2 = date(1, 1, 2, 0); // 1 year later
    if(!assertEqual(date2 - date1 == timePassedData{0,0,1}, true, "Date subtraction operator - 1 year later"))
    {
        timePassedData result = date2 - date1;
        std::cerr << "Result: " << result.toString() << std::endl;
    }
    
    date2 = date(2, 2, 2, 0); // 1 year, 1 month and 1 day later
    if(!assertEqual(date2 - date1 == timePassedData{1,1,1}, true, "Date subtraction operator - 1 year, 1 month and 1 day later"))
    {
        timePassedData result = date2 - date1;
        std::cerr << "Result: " << result.toString() << std::endl;
    }

    timePassedData result = date2 - date1;
}
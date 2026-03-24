#pragma once

#include <fstream>
#include <iostream>
#include <string>

class Logger
{
    public:
    static void log(const std::string& message)
    {
        static std::string filepath = "fortress_chronicle.log";

        //log to file
        std::ofstream logFile(filepath, std::ios_base::app);
        if (logFile.is_open())
        {
            logFile << message << std::endl;
            logFile.close();
        }
        else
        {
            std::cerr << "Unable to open log file." << std::endl;
        }

    }

};
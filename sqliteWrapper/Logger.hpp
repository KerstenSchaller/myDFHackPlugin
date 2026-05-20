#pragma once

#include <fstream>
#include <iostream>
#include <string>
#include <chrono>
#include <iomanip>

class Logger
{
    inline static std::string name = "FortressChronicle";
    public:
    
    static void setName(const std::string& loggerName)
    {
        Logger::name = loggerName;
    }
    static void log(const std::string& message)
    {
        static std::string filepath = "dfhack-config\\df_chronicle\\" + Logger::name + ".log";

        //log to file
        std::ofstream logFile(filepath, std::ios_base::app);
        if (logFile.is_open())
        {
            // get current date and time
            auto now = std::chrono::system_clock::now();
            auto now_c = std::chrono::system_clock::to_time_t(now);
            logFile << std::put_time(std::localtime(&now_c), "%Y-%m-%d %H:%M:%S") << " - " << message << std::endl;
            logFile.close();
        }
        else
        {
            std::cerr << "Unable to open log file." << std::endl;
        }

    }

};
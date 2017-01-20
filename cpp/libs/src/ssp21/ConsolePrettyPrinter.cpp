
#include "ssp21/ConsolePrettyPrinter.h"

#include "ssp21/LogLevels.h"

#include <iostream>
#include <chrono>
#include <sstream>

using namespace std;
using namespace std::chrono;

namespace ssp21
{

    void ConsolePrettyPrinter::log(openpal::ModuleId module, const char* id, openpal::LogLevel level, char const* location, char const* message)
    {
        auto now = high_resolution_clock::now();
        auto millis = duration_cast<milliseconds>(now.time_since_epoch()).count();

        ostringstream oss;

        oss << "ms(" << millis << ") ";

        if (settings.printId)
        {
            oss << id << " ";
        }

        oss << get_prefix(level.value) << message;


        std::cout << oss.str() << std::endl;
    }

    const char* ConsolePrettyPrinter::get_prefix(int level)
    {
        switch (level)
        {
        case(1 << 0):
            return "event   ";
        case(1 << 1):
            return "error   ";
        case(1 << 2):
            return "warn    ";
        case(1 << 3):
            return "info    ";
        case(1 << 4):
            return "debug   ";
        case(1 << 5):
            return "<--     ";
        case(1 << 6):
            return "-->     ";
        default:
            return "            ";
        }

    }

}



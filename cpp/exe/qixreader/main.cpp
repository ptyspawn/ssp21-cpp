
#include "qix/QIXFrameReader.h"

#include "log4cpp/ConsolePrettyPrinter.h"
#include "ssp21/stack/LogLevels.h"

#include "log4cpp/LogMacros.h"

#include <iostream>

using namespace ssp21;

class QIXPrinter : public IQIXFrameHandler
{

    virtual void handle(const QIXFrame& frame) override
    {
        std::cout << "Received frame w/ id: " << frame.key_id << std::endl;
    }
};

int main(int argc, char*  argv[])
{
    if (argc != 2)
    {
        std::cerr << "Usage: " << std::endl;
        std::cerr << "qixreader <serial device>" << std::endl;
        return -1;
    }

    try
    {
        QIXFrameReader reader(
            std::make_shared<QIXPrinter>(),
            log4cpp::Logger(std::make_shared<log4cpp::ConsolePrettyPrinter>(), Module::id, "qix-reader", log4cpp::LogLevels(~0)),
            argv[1]
        );

        std::cout << "waiting for QIX frames" << std::endl;
        std::cout << "press <enter> to terminate" << std::endl;
        std::cin.ignore();
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
    }

    return 0;
}
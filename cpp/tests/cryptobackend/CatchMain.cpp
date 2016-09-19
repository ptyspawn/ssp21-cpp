
#define CATCH_CONFIG_RUNNER
#include "catch.hpp"

#include "sodium/Initialize.h"

int main(int argc, char*  argv[])
{
    // global setup...
    if (argc < 2)
    {
        std::cerr << "You did not specify a backend to test" << std::endl;
        return 1;
    }
    else
    {
        if (strcmp(argv[1], "--sodium") == 0)
        {
	   if(!ssp21::sodium::initialize())
	   {
             std::cerr << "Unable to initialize sodium backend" << std::endl;
             return 1;
	   }
        }
        else
        {
            std::cerr << "Unknown backend flag: " << argv[1] << std::endl;
            return 1;
        }
    }

    // massage the arguments to catch so that the backend flag is ignored
    argv[1] = argv[0];
    int result = Catch::Session().run(argc - 1, argv + 1);

    // global clean-up...

    return result;
}

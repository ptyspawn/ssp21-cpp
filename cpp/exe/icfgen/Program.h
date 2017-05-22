
#ifndef ICFGEN_PROGRAM_H
#define ICFGEN_PROGRAM_H

#include <argagg/argagg.hpp>

namespace flags {
	static const char* const help = "help";
	static const char* const x25519 = "x25519";
}

class Program
{	
	
public:

	Program();

	void run(int argc, char*  argv[]);

private:
	
	void print_help();	

	argagg::parser parser;
};

#endif
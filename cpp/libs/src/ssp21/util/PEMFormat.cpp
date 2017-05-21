#include "ssp21/util/PEMFormat.h"

#include <cstdio>


namespace ssp21
{

    PEMDecodeError PEMFormat::decode(IReader& reader, IHandler& handler, std::error_code& ec)
    {		

        //
        return PEMDecodeError::ok;
    }

    bool PEMFormat::get_section(const std::string& line, std::string& id)
    {
        if (line.length() < (total_begin_line_delim_length + 1))
        {
            return false;
        }

        if (!strncmp(PEM_BEGIN_DELIM_START, line.c_str(), begin_delim_length))
        {
            return false;
        }

        const auto id_length = line.length() - total_begin_line_delim_length;
        const auto trailer_index = begin_delim_length + id_length;

        if (!strncmp(PEM_DELIM_END, line.c_str() + trailer_index, trailer_delim_length))
        {
            return false;
        }

        id = line.substr(begin_delim_length, id_length);

        // TODO - test id for alpha/numeric/spaces?

        return true;
    }

}



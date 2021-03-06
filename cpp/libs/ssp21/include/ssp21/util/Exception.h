
#ifndef SSP21_EXCEPTION_H
#define SSP21_EXCEPTION_H

#include "ssp21/util/StringUtil.h"
#include <stdexcept>

namespace ssp21 {
/**
    * Super class of std::runtime_error that allows
    * message to be built dynamically
    */
class Exception : public std::runtime_error {

public:
    explicit Exception(const char* message)
        : std::runtime_error(message)
    {
    }

    template <class T, class... Args>
    Exception(T t, Args... args)
        : std::runtime_error(ssp21::strings::join(t, args...))
    {
    }
};
}

#endif
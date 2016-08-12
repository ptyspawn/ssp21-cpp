
#include "ssp21/ErrorCodes.h"

namespace ssp21
{					

ErrorCategory ErrorCategory::instance;

std::string ErrorCategory::message(int ev) const
{
	switch (static_cast<Error>(ev))
	{
	case(Error::BAD_KEY_TYPE) :
		return "bad key type";
	case(Error::DH_X25519_FAIL) :
		return "X25519 DH operation failed. Bad public key?";
	default:
		return "unknown error";
	}
}

std::error_code make_error_code(Error err)
{
	return std::error_code(static_cast<int>(err), ErrorCategory::Instance());
}
	
}



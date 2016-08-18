//
//  _   _         ______    _ _ _   _             _ _ _
// | \ | |       |  ____|  | (_) | (_)           | | | |
// |  \| | ___   | |__   __| |_| |_ _ _ __   __ _| | | |
// | . ` |/ _ \  |  __| / _` | | __| | '_ \ / _` | | | |
// | |\  | (_) | | |___| (_| | | |_| | | | | (_| |_|_|_|
// |_| \_|\___/  |______\__,_|_|\__|_|_| |_|\__, (_|_|_)
//                                           __/ |
//                                          |___/
// 
// This file is auto-generated. Do not edit manually
// 
// License TBD
//

#include "ssp21/gen/FormatError.h"

namespace ssp21 {

const char* FormatErrorSpec::to_string(FormatError arg)
{
  switch(arg)
  {
    case(FormatError::ok):
      return "ok";
    case(FormatError::insufficient_space):
      return "insufficient_space";
    case(FormatError::bad_sequence_length):
      return "bad_sequence_length";
    default:
      return "undefined";
  }
}

}
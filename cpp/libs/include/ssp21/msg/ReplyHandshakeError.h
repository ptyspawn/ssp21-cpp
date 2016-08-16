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

#ifndef SSP21_REPLYHANDSHAKEERROR_H
#define SSP21_REPLYHANDSHAKEERROR_H

#include "ssp21/gen/Function.h"
#include "ssp21/gen/HandshakeError.h"
#include "openpal/util/Uncopyable.h"

namespace ssp21 {

struct ReplyHandshakeError : openpal::Uncopyable
{
  ReplyHandshakeError();

  HandshakeError handshake_error;
};

}

#endif
package com.automatak.render.ssp21.messages

import com.automatak.render.ssp21.enums.CryptoFunction


object ReplyHandshakeAuth extends Message {

  override def name : String = "ReplyHandshakeAuth"

  override def fields: List[Field] = List (
    FixedEnum(CryptoFunction(), CryptoFunction.requestHandshakeAuth),
    Seq8("mac")
  )

}

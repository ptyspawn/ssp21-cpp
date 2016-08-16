package com.automatak.render.ssp21.messages

import com.automatak.render.ssp21.enums._
import com.automatak.render.ssp21.enums.ssp21.CryptoFunction


object ReplyHandshakeBegin extends Message {

  override def name : String = "ReplyHandshakeBegin"

  override def fields: List[Field] = List(
      FixedEnum(CryptoFunction(), CryptoFunction.replyHandshakeBegin),
      U16("version"),
      Seq8("ephemeral_public_key"),
      Seq8Seq16("certificates")
  )

}
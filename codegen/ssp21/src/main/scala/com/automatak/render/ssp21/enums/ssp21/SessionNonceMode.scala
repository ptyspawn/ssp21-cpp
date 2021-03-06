/**
  * License TBD
  */
package com.automatak.render.ssp21.enums.ssp21

import com.automatak.render._

object SessionNonceMode extends EnumModel {

  override def name: String = "SessionNonceMode"

  override def underscoredName: String = "session_nonce_mode"

  override def comments: List[String] = List("Determines how nonces are verified during the session")

  override def nonDefaultValues: List[EnumValue] = codes

  override def defaultValue: Option[EnumValue] = Some(EnumValue.undefined(255))

  private val codes = List(
    EnumValue("strict_increment", 0, "new nonce must strictly be equal to last nonce plus one"),
    EnumValue("greater_than_last_rx", 1, "new nonce must be greater than last nonce")
  )

}




/**
 * License TBD
 */
/**
  * License TBD
  */
package com.automatak.render.ssp21.enums.ssp21

import com.automatak.render._

object SessionCryptoMode extends EnumModel {

  override def name: String = "SessionCryptoMode"

  override def underscoredName: String = "session_crypto_mode"

  override def comments: List[String] = List("Specifies the complete set of algorithms used to secure the session")

  override def nonDefaultValues: List[EnumValue] = codes

  override def defaultValue: Option[EnumValue] = Some(EnumValue.undefined(255))

  private val codes = List(
    EnumValue("hmac_sha256_16", 0, "HMAC-SHA256 truncated to 16 bytes"),
    EnumValue("aes_256_gcm", 1, "AES 256 in GCM mode"),
  )

}




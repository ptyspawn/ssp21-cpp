/**
 * License TBD
 */
package com.automatak.render.ssp21.messages.generators

import com.automatak.render._
import com.automatak.render.ssp21.messages.Message
import com.automatak.render.ssp21.{Include, Includes}


final case class MessageGenerator(msg: Message) extends StructGenerator(msg, false) {

  override def prefixSize: Int = 1 // the function

  override def interfaces: String = ": public IMessage"

  override def headerIncludes: List[Include] = List(Includes.imessage, Includes.function)

  override def outputReadWritePrint: Boolean = false

  override def extraHeaderConstants: Iterator[String] = Iterator(
    "static const Function function = Function::%s;".format(msg.function.name)
  )

  override def extraHeaderSignatures: Iterator[String] = Iterator(
    "virtual ParseError read(seq32_t input) override;",
    "virtual FormatResult write(wseq32_t& output) const override;",
    "virtual void print(IMessagePrinter& printer) const override;",
    "virtual Function get_function() const override { return Function::%s; }".format(msg.function.name)
  ) ++ space

  override def extraImplFunctions(implicit indent: Indentation): Iterator[String] = {

    def read = {
      "ParseError %s::read(seq32_t input)".format(msg.name).iter ++ bracket {
        "auto read_fields = [this](seq32_t& input) -> ParseError ".iter ++ bracketSemiColon {
          readSomeInternals
        } ++ space ++
          "return MessageParser::read_message(input, Function::%s, read_fields);".format(msg.function.name).iter
      }
    }

    def write = {
      "FormatResult %s::write(wseq32_t& output) const".format(msg.name).iter ++ bracket {
        "auto write_fields = [this](wseq32_t& output) -> FormatError ".iter ++ bracketSemiColon {
          writeInternals
        } ++ space ++
          "return MessageFormatter::write_message(output, Function::%s, write_fields);".format(msg.function.name).iter
      }
    }

    def print = {
      "void %s::print(IMessagePrinter& printer) const".format(msg.name).iter ++ bracket {
        printInternals
      }
    }

    space ++ read ++ space ++ write ++ print ++ space
  }


}

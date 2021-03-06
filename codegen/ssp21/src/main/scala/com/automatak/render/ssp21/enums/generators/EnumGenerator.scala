/**
 * License TBD
 */
/**
  * License TBD
  */
package com.automatak.render.ssp21.enums.generators

import com.automatak.render._
import com.automatak.render.cpp._
import com.automatak.render.ssp21._

case class EnumGenerator(cfg: EnumConfig) extends WriteCppFiles {

  def cppNamespace = "ssp21"

  override def hasImpl: Boolean = cfg.anyOptionalFunctions

  private val renderers: List[HeaderImplModelRender[EnumModel]] = {

    def conversions = if (cfg.conversions) List(EnumToType, EnumFromType) else Nil
    def stringify = if (cfg.stringConv) List(EnumToString) else Nil

    conversions ::: stringify
  }

  override def mainClassName: String = cfg.model.name

  override def header(implicit i: Indentation): Iterator[String] = {

    def license = commented(LicenseHeader.lines)
    def enum = EnumModelRenderer.render(cfg.model)
    def signatures = renderers.map(c => c.header.render(cfg.model)).flatten.toIterator
    def nameLine: Iterator[String] = if (cfg.model.errorCategory.isDefined) {
      space ++ "static const char* name;".iter
    } else Iterator.empty

    def spec = "struct %s : private ser4cpp::StaticOnly".format(cfg.model.specName).iter ++ bracketSemiColon {
      "using enum_type_t = %s;".format(cfg.model.name).iter ++
        nameLine ++
        space ++
        signatures
    }

    def castFunc: Iterator[String] = {
      cfg.model.boolCastValue match {
        case Some(value) => {
          space ++
            "inline bool any(%s value)".format(cfg.model.name).iter ++ bracket {
            "return value != %s::%s;".format(cfg.model.name, value.name).iter
          }
        }
        case None => Iterator.empty
      }
    }

    def errorCategory: Iterator[String] = cfg.model.errorCategory match {
      case Some(cat) => {
        space ++
          "using %s = ErrorCategory<%s>;".format(cat.className, cfg.model.specName).iter ++
          space ++
          "inline std::error_code make_error_code(%s err)".format(cfg.model.name).iter ++ bracket {
          "return std::error_code(static_cast<int>(err), %s::get());".format(cat.className).iter
        }
      }
      case None => Iterator.empty
    }

    def includes: List[Include] = {
      val baseIncludes = List(Includes.uncopyable, Includes.cstdint)
      if (cfg.model.errorCategory.isDefined) Includes.errorCategory :: baseIncludes else baseIncludes
    }

    def isErrorCodeEnum: Iterator[String] = cfg.model.errorCategory match {
      case Some(cat) => space ++ namespace("std") {
        "template <>".iter ++
          "struct is_error_code_enum<ssp21::%s> : public true_type {};".format(cfg.model.name).iter
      }
      case None => Iterator.empty
    }

    license ++ space ++ includeGuards(cfg.model.name)(
      Includes.lines(includes) ++ space ++ namespace(cppNamespace) {
        enum ++ castFunc ++ space ++ spec ++ errorCategory
      } ++ isErrorCodeEnum
    )
  }

  override def impl(implicit i: Indentation): Iterator[String] = {

    def license = commented(LicenseHeader.lines)
    def funcs = renderers.map(r => r.impl.render(cfg.model)).flatten.toIterator
    def constants: Iterator[String] = cfg.model.errorCategory match {
      case Some(cat) => "const char* %s::name = %s;".format(cfg.model.specName, quoted(cat.stringName)).iter ++ space
      case None => Iterator.empty
    }

    def includes = cfg.test match {
      case true => Includes.test(cfg.model.name).line
      case false => Includes.enum(cfg.model.name, cfg.public).line
    }

    license ++ space ++ includes ++ space ++ namespace(cppNamespace)(constants ++ funcs)
  }
}

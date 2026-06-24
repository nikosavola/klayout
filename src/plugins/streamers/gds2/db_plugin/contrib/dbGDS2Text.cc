
/*

  KLayout Layout Viewer
  Copyright (C) 2006-2026 Matthias Koefferlein

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA

*/


#include <string>
#include <map>

#include "dbGDS2Converter.h"
#include "dbGDS2TextReader.h"
#include "dbGDS2TextWriter.h"
#include "dbGDS2.h"
#include "dbStream.h"

#include "tlClassRegistry.h"

namespace db
{

// ---------------------------------------------------------------
//  GDS2 Text format implementation

class GDS2TextFormatDeclaration
  : public db::StreamFormatDeclaration
{
  std::string format_name () const override { return "GDS2Text"; }
  std::string format_desc () const override { return "GDS2 Text"; }
  std::string format_title () const override { return "GDS2 (ASCII text representation)"; }
  std::string file_format () const override { return "GDS2 Text files (*.txt *.TXT )"; }

  bool detect (tl::InputStream &s) const override 
  {
    try {

      tl::TextInputStream stream (s);

      while (! stream.at_end ()) {

        std::string line = stream.get_line ();
        tl::Extractor ex (line.c_str ());
        if (ex.test ("#") || ex.at_end ()) {
          //  ignore comment or empty lines
        } else {
          return (ex.test ("HEADER") || ex.test ("BGNLIB") || ex.test ("UNITS"));
        } 

      }

    } catch (...) { // NOLINT(bugprone-empty-catch)
    }

    return false;
  }

  ReaderBase *create_reader (tl::InputStream &s) const override 
  {
    return new db::GDS2ReaderText(s);
  }

  WriterBase *create_writer () const override 
  {
    return new db::GDS2WriterText();
  }

  bool can_read () const override
  {
    return true;
  }

  bool can_write () const override
  {
    return true;
  }

  bool supports_context () const override
  {
    return true;
  }
};

static tl::RegisteredClass<db::StreamFormatDeclaration> format_txt_decl (new GDS2TextFormatDeclaration(), 1, "GDS2Text");

}


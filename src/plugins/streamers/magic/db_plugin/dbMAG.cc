
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


#include "dbMAG.h"
#include "dbMAGReader.h"
#include "dbMAGWriter.h"
#include "dbStream.h"

#include "tlClassRegistry.h"

namespace db
{

// ---------------------------------------------------------------
//  MAGDiagnostics implementation

MAGDiagnostics::~MAGDiagnostics ()
{
  //  .. nothing yet ..
}

// ---------------------------------------------------------------
//  MAG format declaration

class MAGFormatDeclaration
  : public db::StreamFormatDeclaration
{
public:
  MAGFormatDeclaration ()
  {
    //  .. nothing yet ..
  }

  std::string format_name () const override { return "MAG"; }
  std::string format_desc () const override { return "Magic"; }
  std::string format_title () const override { return "MAG (Magic layout format)"; }
  std::string file_format () const override { return "Magic files (*.mag *.MAG *.mag.gz *.MAG.gz)"; }

  bool detect (tl::InputStream &s) const override 
  {
    return s.read_all (5) == "magic";
  }

  ReaderBase *create_reader (tl::InputStream &s) const override 
  {
    return new db::MAGReader (s);
  }

  WriterBase *create_writer () const override
  {
    return new db::MAGWriter ();
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
    return false;
  }

  tl::XMLElementBase *xml_reader_options_element () const override
  {
    return new db::ReaderOptionsXMLElement<db::MAGReaderOptions> ("mag",
      tl::make_member (&db::MAGReaderOptions::lambda, "lambda") +
      tl::make_member (&db::MAGReaderOptions::dbu, "dbu") +
      tl::make_member (&db::MAGReaderOptions::layer_map, "layer-map") +
      tl::make_member (&db::MAGReaderOptions::create_other_layers, "create-other-layers") +
      tl::make_member (&db::MAGReaderOptions::keep_layer_names, "keep-layer-names") +
      tl::make_member (&db::MAGReaderOptions::merge, "merge") +
      tl::make_element<std::vector<std::string>, db::MAGReaderOptions> (&db::MAGReaderOptions::lib_paths, "lib-paths",
        tl::make_member<std::string, std::vector<std::string>::const_iterator, std::vector<std::string> > (&std::vector<std::string>::begin, &std::vector<std::string>::end, &std::vector<std::string>::push_back, "lib-path")
      )
    );
  }

  tl::XMLElementBase *xml_writer_options_element () const override
  {
    return new db::WriterOptionsXMLElement<db::MAGWriterOptions> ("mag",
      tl::make_member (&db::MAGWriterOptions::lambda, "lambda") +
      tl::make_member (&db::MAGWriterOptions::tech, "tech") +
      tl::make_member (&db::MAGWriterOptions::write_timestamp, "write-timestamp")
    );
  }
};

//  NOTE: Because MAG has such a high degree of syntactic freedom, the detection is somewhat
//  fuzzy: do MAG at the very end of the detection chain
static tl::RegisteredClass<db::StreamFormatDeclaration> reader_decl (new MAGFormatDeclaration (), 2200, "MAG");

//  provide a symbol to force linking against
int force_link_MAG = 0;

}



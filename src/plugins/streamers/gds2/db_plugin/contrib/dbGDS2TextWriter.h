
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


#ifndef HDR_dbGDS2WriterText
#define HDR_dbGDS2WriterText

#include "dbPluginCommon.h"
#include "dbGDS2WriterBase.h"
#include <sstream>
#include <climits>

namespace db
{


class DB_PLUGIN_PUBLIC GDS2WriterText
  : public db::GDS2WriterBase
{

public:
  GDS2WriterText();
  ~GDS2WriterText() override;

protected:
  /**
   *  @brief Write a byte
   */
  void write_byte (unsigned char b) override;

  /**
   *  @brief Write a short
   */
  void write_short (int16_t i) override;

  /**
   *  @brief Write a long
   */
  void write_int (int32_t l) override;

  /**
   *  @brief Write a double
   */
  void write_double (double d) override;

  /**
   *  @brief Write the time
   */
  void write_time (const short *t) override;

  /**
   *  @brief Write a string
   */
  void write_string (const char *t) override;

  /**
   *  @brief Write a string
   */
  void write_string (const std::string &t) override;

  /**
   *  @brief Write the size of the record
   */
  void write_record_size (int16_t i) override;

  /**
   *  @brief Write a record identifier
   */
  void write_record (int16_t i) override;

  /**
   *  @brief Set the stream to write the data to
   */
  void set_stream (tl::OutputStream &stream) override
  {
    pStream = &stream;
  }

  /**
   *  @brief Establish a checkpoint for progress reporting
   */
  void progress_checkpoint () override;

private:
  tl::OutputStream *pStream;
  std::stringstream ssFormattingStream;
  short siCurrentRecord;
  bool  bIsXCoordinate;
  tl::AbsoluteProgress mProgress;
};


} // namespace db

#endif



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



#ifndef HDR_dbGDS2Reader
#define HDR_dbGDS2Reader

#include "dbPluginCommon.h"
#include "dbLayout.h"
#include "dbGDS2Format.h"
#include "dbGDS2ReaderBase.h"
#include "dbCommonReader.h"
#include "dbStreamLayers.h"

#include "tlException.h"
#include "tlInternational.h"
#include "tlProgress.h"
#include "tlString.h"
#include "tlStream.h"

namespace db
{

/**
 *  @brief Generic base class of GDS2 reader exceptions
 */
class DB_PLUGIN_PUBLIC GDS2ReaderException
  : public ReaderException 
{
public:
  GDS2ReaderException (const std::string &msg, size_t p, size_t n, const std::string &cell, const std::string &source)
    : ReaderException (tl::sprintf (tl::to_string (tr ("%s (position=%ld, record number=%ld, cell=%s), in file: %s")), msg, p, n, cell, source))
  { }
};

/**
 *  @brief The GDS2 format stream reader
 */
class DB_PLUGIN_PUBLIC GDS2Reader
  : public GDS2ReaderBase
{
public: 
  /**
   *  @brief Construct a stream reader object
   *
   *  @param s The stream delegate from which to read stream data from
   */
  GDS2Reader (tl::InputStream &s);

  /**  
   *  @brief Destructor
   */
  ~GDS2Reader () override;

  /**
   *  @brief Format
   */
  const char *format () const override { return "GDS2"; }

protected:
  void init (const LoadLayoutOptions &options) override;

private:
  tl::InputStream &m_stream;
  size_t m_recnum;
  size_t m_reclen;
  size_t m_recptr;
  unsigned char *mp_rec_buf;
  tl::string m_string_buf;
  short m_stored_rec;
  bool m_allow_big_records;
  tl::AbsoluteProgress m_progress;

  void error (const std::string &txt) override;
  void warn (const std::string &txt, int wl = 1) override;

  std::string path () const override;
  const char *get_string () override;
  void get_string (std::string &s) const override;
  int get_int () override;
  short get_short () override;
  unsigned short get_ushort () override;
  double get_double () override;
  short get_record () override;
  void unget_record (short rec_id) override;
  void get_time (unsigned int *mod_time, unsigned int *access_time) override;
  GDS2XY *get_xy_data (unsigned int &length) override;
  void progress_checkpoint () override;

  void record_underflow_error ();
};

}

#endif


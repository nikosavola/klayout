
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


#include "tlMPI.h"
#include "tlThreads.h"
#include "tlLog.h"

#if defined(HAVE_MPI)
#  include <mpi.h>
#  include <stdint.h>
#  include <limits>
#  include <algorithm>
#endif

namespace tl
{

namespace mpi
{

#if defined(HAVE_MPI)

//  The message tag used for all of the point-to-point transfers below. As MPI
//  preserves message ordering between a given (source, destination, tag) triple
//  we can use a single tag for the length header followed by the payload.
static const int s_tag = 0x4b4c;  // 'KL'

//  The maximum number of bytes sent in a single MPI_Send/MPI_Recv. MPI counts
//  are "int", so payloads larger than this are transferred in multiple chunks.
static const size_t s_max_chunk = size_t (1) << 30;  // 1 GiB

static tl::Mutex s_init_mutex;

namespace
{
  //  Calls MPI_Finalize at process exit if MPI was initialized by us and has
  //  not been finalized yet.
  struct Finalizer
  {
    ~Finalizer ()
    {
      int is_init = 0, is_final = 0;
      MPI_Initialized (&is_init);
      if (is_init) {
        MPI_Finalized (&is_final);
        if (! is_final) {
          MPI_Finalize ();
        }
      }
    }
  };
}

static void
send_all (const char *data, size_t n, int dest)
{
  uint64_t len = uint64_t (n);
  MPI_Send (&len, 1, MPI_UINT64_T, dest, s_tag, MPI_COMM_WORLD);

  size_t off = 0;
  while (off < n) {
    int chunk = int (std::min (s_max_chunk, n - off));
    MPI_Send (data + off, chunk, MPI_BYTE, dest, s_tag, MPI_COMM_WORLD);
    off += size_t (chunk);
  }
}

static std::string
recv_all (int source)
{
  uint64_t len = 0;
  MPI_Recv (&len, 1, MPI_UINT64_T, source, s_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

  std::string res;
  res.resize (size_t (len));

  size_t off = 0;
  while (off < size_t (len)) {
    int chunk = int (std::min (s_max_chunk, size_t (len) - off));
    MPI_Recv (&res[0] + off, chunk, MPI_BYTE, source, s_tag, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    off += size_t (chunk);
  }

  return res;
}

bool available ()
{
  return true;
}

bool initialized ()
{
  int flag = 0;
  MPI_Initialized (&flag);
  return flag != 0;
}

bool ensure_initialized ()
{
  tl::MutexLocker locker (&s_init_mutex);

  int flag = 0;
  MPI_Initialized (&flag);
  if (! flag) {
    //  We do not have access to the original argc/argv here. MPI_Init with NULL
    //  arguments is valid since MPI-2 and supported by MPICH.
    MPI_Init (0, 0);
    //  We keep the default (fatal) error handler: a transport failure aborts
    //  loudly rather than silently proceeding on partially-filled buffers. The
    //  collectives below therefore do not need to inspect return codes.
    static Finalizer s_finalizer;
    (void) s_finalizer;
  }

  return true;
}

int rank ()
{
  if (! initialized ()) {
    return 0;
  }
  int r = 0;
  MPI_Comm_rank (MPI_COMM_WORLD, &r);
  return r;
}

int size ()
{
  if (! initialized ()) {
    return 1;
  }
  int s = 1;
  MPI_Comm_size (MPI_COMM_WORLD, &s);
  return s;
}

void barrier ()
{
  if (initialized ()) {
    MPI_Barrier (MPI_COMM_WORLD);
  }
}

bool any (bool flag)
{
  if (! initialized ()) {
    return flag;
  }
  int local = flag ? 1 : 0;
  int global = 0;
  MPI_Allreduce (&local, &global, 1, MPI_INT, MPI_LOR, MPI_COMM_WORLD);
  return global != 0;
}

std::vector<std::string>
gather_to_root (const std::string &local)
{
  std::vector<std::string> res;

  if (! is_parallel ()) {
    res.push_back (local);
    return res;
  }

  int r = rank ();
  int n = size ();

  if (r == 0) {
    res.resize (size_t (n));
    res[0] = local;
    for (int src = 1; src < n; ++src) {
      res[size_t (src)] = recv_all (src);
    }
  } else {
    send_all (local.c_str (), local.size (), 0);
  }

  return res;
}

#else

bool available ()
{
  return false;
}

bool initialized ()
{
  return false;
}

bool ensure_initialized ()
{
  return false;
}

int rank ()
{
  return 0;
}

int size ()
{
  return 1;
}

void barrier ()
{
  //  .. nothing ..
}

bool any (bool flag)
{
  return flag;
}

std::vector<std::string>
gather_to_root (const std::string &local)
{
  std::vector<std::string> res;
  res.push_back (local);
  return res;
}

#endif

bool is_parallel ()
{
  return initialized () && size () > 1;
}

bool is_root ()
{
  return rank () == 0;
}

} // namespace mpi

} // namespace tl

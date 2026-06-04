
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


#ifndef HDR_tlMPI
#define HDR_tlMPI

#include "tlCommon.h"

#include <string>
#include <vector>

namespace tl
{

/**
 *  @brief A thin abstraction layer over MPI (MPICH)
 *
 *  This namespace provides a minimal, dependency-free interface to the parts
 *  of MPI that KLayout uses for distributing work across processes (ranks).
 *  It is built around the world communicator and message passing of opaque
 *  byte buffers - enough to scatter a work plan and gather serialized results.
 *
 *  When KLayout is built without MPI support (HAVE_MPI undefined), all
 *  functions degrade to a well-defined single-process behaviour: rank() is 0,
 *  size() is 1, is_parallel() is false and the collectives act as identity
 *  operations. This lets callers use the same code path regardless of whether
 *  MPI is available.
 *
 *  Threading note: KLayout is initialized at MPI_THREAD_SINGLE level. All MPI
 *  calls (the collectives below) must be issued from the main thread only.
 *  Worker threads must not call into this namespace.
 */
namespace mpi
{

/**
 *  @brief Returns true if MPI support was compiled in
 */
TL_PUBLIC bool available ();

/**
 *  @brief Lazily initializes MPI
 *
 *  This call is idempotent. On first use it calls MPI_Init and registers
 *  MPI_Finalize to run at process exit. It is a no-op if MPI is not available
 *  or already initialized.
 *
 *  All ranks must reach this call at the same logical point before issuing any
 *  collective. Returns true if MPI is active (initialized and available) after
 *  the call.
 */
TL_PUBLIC bool ensure_initialized ();

/**
 *  @brief Returns true if MPI has been initialized in this process
 */
TL_PUBLIC bool initialized ();

/**
 *  @brief Returns the rank of this process in the world communicator
 *
 *  Returns 0 if MPI is not active.
 */
TL_PUBLIC int rank ();

/**
 *  @brief Returns the number of processes in the world communicator
 *
 *  Returns 1 if MPI is not active.
 */
TL_PUBLIC int size ();

/**
 *  @brief Returns true if running under MPI with more than one rank
 */
TL_PUBLIC bool is_parallel ();

/**
 *  @brief Returns true on the root (rank 0) process
 */
TL_PUBLIC bool is_root ();

/**
 *  @brief A collective barrier across all ranks
 *
 *  No-op if MPI is not active.
 */
TL_PUBLIC void barrier ();

/**
 *  @brief Logical OR reduction across all ranks
 *
 *  Returns true on every rank if any rank passed true. This is used to reach a
 *  common decision (e.g. abort on error) so that all ranks take the same code
 *  path and no rank is left waiting in a subsequent collective.
 *
 *  Returns the input value unchanged if MPI is not active.
 */
TL_PUBLIC bool any (bool flag);

/**
 *  @brief Gathers per-rank byte buffers onto the root
 *
 *  Each rank contributes its own @p local buffer. On the root (rank 0) the
 *  returned vector holds one buffer per rank, indexed by rank (so result[0] is
 *  the root's own buffer). On non-root ranks the returned vector is empty.
 *
 *  This is a collective: every rank must call it. If MPI is not active the
 *  result is a single-element vector holding @p local.
 */
TL_PUBLIC std::vector<std::string> gather_to_root (const std::string &local);

} // namespace mpi

} // namespace tl

#endif

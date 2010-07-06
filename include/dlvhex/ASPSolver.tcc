/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
 * 
 * This file is part of dlvhex.
 *
 * dlvhex is free software; you can redistribute it and/or modify it
 * under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * dlvhex is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with dlvhex; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA
 * 02110-1301 USA.
 */


/**
 * @file   ASPSolver.tcc
 * @author Thomas Krennwallner
 * @date   Thu Feb 21 09:20:32 CET 2008
 * 
 * @brief  Definition of ASP solver class.
 * 
 */

#if !defined(_DLVHEX_ASPSOLVER_TCC)
#define _DLVHEX_ASPSOLVER_TCC

#include "dlvhex/Error.h"
#include "dlvhex/Program.h"
#include "dlvhex/AtomSet.h"
#include "dlvhex/globals.h"
#include "dlvhex/Benchmarking.h"

#include <sstream>

DLVHEX_NAMESPACE_BEGIN

// DLV specialization
template<>
void ASPSolverManager::solve<ASPSolverManager::DLVSoftware>(
    const Program& idb, const AtomSet& edb, std::vector<AtomSet>& result,
    const ASPSolverManager::DLVSoftware::Options& options) throw (FatalError);
template<>
void ASPSolverManager::solveString<ASPSolverManager::DLVSoftware>(
    const std::string& program, std::vector<AtomSet>& result,
    const ASPSolverManager::DLVSoftware::Options& options) throw (FatalError);
template<>
void ASPSolverManager::solveFile<ASPSolverManager::DLVSoftware>(
    const std::string& filename, std::vector<AtomSet>& result,
    const ASPSolverManager::DLVSoftware::Options& options) throw (FatalError);

DLVHEX_NAMESPACE_END

#endif // _DLVHEX_ASPSOLVER_TCC

// vim:se ts=8:
// Local Variables:
// mode: C++
// End:

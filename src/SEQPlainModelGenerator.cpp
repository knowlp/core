/* dlvhex -- Answer-Set Programming with external interfaces.
 * Copyright (C) 2005, 2006, 2007 Roman Schindlauer
 * Copyright (C) 2006, 2007, 2008, 2009, 2010 Thomas Krennwallner
 * Copyright (C) 2009, 2010 Peter Schüller
 * Copyright (C) 2013 Andreas Humenberger
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
 * @file   SEQPlainModelGenerator.cpp
 * @author Andreas Humenberger <e1026602@student.tuwien.ac.at>
 *
 * @brief
 */

#include "dlvhex2/SEQPlainModelGenerator.h"
#include "dlvhex2/config_values.h"
#include "dlvhex2/Benchmarking.h"

#include "dlvhex2/ClaspSolver.h"

#define BM_MODEL_COUNT		"[SEQ] classical model"
#define BM_HMINIMAL_COUNT	"[SEQ] hminimal model"

DLVHEX_NAMESPACE_BEGIN

SEQPlainModelGeneratorFactory::SEQPlainModelGeneratorFactory(ProgramCtx& ctx, const ComponentInfo& ci, ASPSolverManager::SoftwareConfigurationPtr externalEvalConfig):
	HTPlainModelGeneratorFactory(ctx, ci, externalEvalConfig)
{
}

SEQPlainModelGenerator::SEQPlainModelGenerator(Factory& factory, InterprConstPtr input):
	HTPlainModelGenerator(factory, input),
	onlyanswersets(false)
{
}

SEQPlainModelGenerator::~SEQPlainModelGenerator()
{
}

InterpretationPtr SEQPlainModelGenerator::nextAnswerSet()
{
	if (assolver == GenuineSolverPtr()) {
		const OrdinaryASPProgram& p = solver->getGroundProgram();
		assolver = GenuineGroundSolver::getInstance(ctx, p);
	}
	return assolver->getNextModel();
#if 0
		InterpretationPtr model = solver->getNextModel();
	while (model && !ctx.terminationRequest) {
		DLVHEX_BENCHMARK_REGISTER_AND_COUNT(sidmcount, BM_MODEL_COUNT, 1);
		//ufscm->initialize(model);
		std::vector<IDAddress> ufs = ufscm->getUnfoundedSet(model);
		if (ufs.size() == 0) {
			return model;
		}
		model = solver->getNextModel();
	}
	return InterpretationPtr();
#endif
}

ModelGapPtr SEQPlainModelGenerator::nextHMinimal()
{
	if (hminimal.size() == 0 || hminimalit == hminimal.end()) {
		DBGLOG(DBG, "[SEQPlain] compute hminimal HT models");
		hminimal.clear();
		InterpretationPtr model = solver->getNextModel();
		if (model) {
			DLVHEX_BENCHMARK_REGISTER_AND_COUNT(sidmcount, BM_MODEL_COUNT, 1);
			bool foundufs = false;
			bool nextufs = true;
			while (nextufs && !ctx.terminationRequest) {
				std::vector<IDAddress> ufs = ufscm->getUnfoundedSet(model);
				if (ufs.size() == 0) {
					nextufs = false;
					if (!foundufs) {
						DBGLOG(DBG, "[SEQPlain] found answer set during hminimal search");
						// model with gap 0 exists, 
						// hence from now on we only compute answer sets
						onlyanswersets = true;
						return ModelGapPtr(new ModelGap(BVec(), model));
					}
				}
				foundufs = true;
				BVec bufs;
				BOOST_FOREACH (IDAddress id, ufs) {
					if (!reg->ogatoms.getIDByAddress(id).isAuxiliary()) {
						bufs.set(id);
					}
				}
				bool insert = true;
				MVec::iterator it = hminimal.begin();
				while (it != hminimal.end()) {
					if (bm_subset((*it)->first, bufs)) {
						it = hminimal.erase(it);
					} else if (bm_subset(bufs, (*it)->first)) {
						insert = false;
						break;
					} else {
						++it;
					}
				}
				if (insert) {
					hminimal.push_back(ModelGapPtr(new ModelGap(bufs, model)));
				}
			}
			DBGLOG(DBG, "[SEQPlain] found " << hminimal.size() << " hminimal HT models");
			hminimalit = hminimal.begin();
		} else {
			return ModelGapPtr();
		}
	}
	DBGLOG(DBG, "[SEQPlain] return cached hminimal HT model");
	return (*(hminimalit++));
}

SEQPlainModelGenerator::InterprPtr SEQPlainModelGenerator::generateNextModel()
{
	onlyanswersets |= (ctx.config.getOption(CFG_SEQ_MODELS) == SEQModels_AnswerSets);
	if (onlyanswersets) {
		InterpretationPtr model = nextAnswerSet();
		if (model) {
			DBGLOG(DBG, "[SEQPlain] got the following answer set: " << *model);
			return InterprPtr(new HTInterpretation(reg, model->getStorage()));
		}
		return HTInterpretationPtr();
	} else if (seqmodels.size() == 0) {
		ModelGapPtr p;
		while ((p = nextHMinimal()) && !ctx.terminationRequest) {
			if (onlyanswersets) {
				// an HT model with gap 0 was found in nextHMinimal()
				seqmodels.clear();
				InterpretationPtr model = nextAnswerSet();
				if (model) {
					DBGLOG(DBG, "[SEQPlain] got the following answer set: " << *model);
					return InterprPtr(new HTInterpretation(reg, model->getStorage()));
				}
				return HTInterpretationPtr();
			#if 0
				DBGLOG(DBG, "[SEQPlain] got the following answer set during h-minimal search: " << *(p->second));
				return InterprPtr(new HTInterpretation(p->second->getStorage()));
			#endif
			}
			DLVHEX_BENCHMARK_REGISTER_AND_COUNT(sidhminimal, BM_HMINIMAL_COUNT, 1);
			bool insert = true;
			MVec::iterator it = seqmodels.begin();
			while (it != seqmodels.end()) {
				if (bm_subset(p->first, (*it)->first)) {
					it = seqmodels.erase(it);
				} else if (bm_subset((*it)->first, p->first)) {
					insert = false;
					break;
				} else {
					++it;
				}
			}
			if (insert) {
				seqmodels.push_back(p);
			}
		}
		seqmodelsit = seqmodels.begin();
	}
	if (seqmodelsit != seqmodels.end()) {
		ModelGapPtr p = *(seqmodelsit++);
		return InterprPtr(new HTInterpretation(reg, p->second->getStorage(), p->first));
	}
	return HTInterpretationPtr();
}

bool bm_subseteq(const bm::bvector<>& v1, const bm::bvector<>& v2)
{
	return ((v1 ^ v2) & v2) == (v1 ^ v2);
}
bool bm_subset(const bm::bvector<>& v1, const bm::bvector<>& v2)
{
	return bm_subseteq(v1, v2) && v1 != v2;
}

DLVHEX_NAMESPACE_END
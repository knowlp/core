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
 * @file   PluginInterface.cpp
 * @author Roman Schindlauer
 * @date   Mon Oct 17 14:37:07 CEST 2005
 * 
 * @brief Definition of Classes PluginAtom, PluginRewriter,
 * and PluginInterface.
 * 
 *      
 */     

#include "dlvhex/PluginInterface.h"

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include "dlvhex/Registry.hpp"
#include "dlvhex/ProgramCtx.h"
#include "dlvhex/HexParser.hpp"
#include "dlvhex/InputProvider.hpp"
#include "dlvhex/InternalGrounder.hpp"
#include "dlvhex/Term.hpp"
#include "dlvhex/ID.hpp"
#include "dlvhex/Benchmarking.h"

DLVHEX_NAMESPACE_BEGIN

#if 0
bool PluginAtom::Query::operator<(const Query& other) const
{
	/*
  return
    ( interpretation < other.interpretation ) ||
    ( interpretation == other.interpretation &&
      input < other.input ) ||
    ( interpretation == other.interpretation &&
      input == other.input &&
      pattern < other.pattern );
			*/
}
#endif
        
bool PluginAtom::Query::operator==(const Query& other) const
{
  return
      (input == other.input) &&
      (pattern == other.pattern) &&
      (
        (interpretation == other.interpretation) ||
        (interpretation != 0 && other.interpretation != 0 &&
         *interpretation == *other.interpretation)
      );
}

void PluginAtom::Query::builtInputPredicateTable(){

	int index = 0;
	BOOST_FOREACH (ID inp, input){
		inputPredicateTable[inp] = index++;
	}
}

// hash function for QueryAnswerCache
std::size_t hash_value(const PluginAtom::Query& q)
{
  std::size_t seed = 0;
  boost::hash_combine(seed, q.input);
  //LOG("hash_combine inp " << printrange(q.input) << " yields " << seed);
  boost::hash_combine(seed, q.pattern);
  //LOG("hash_combine pat " << printrange(q.pattern) << " yields " << seed);
  // TODO: can we take hash of pointer to interpretation here?
  if( q.interpretation == 0 )
  {
    boost::hash_combine(seed, 0);
  }
  else
  {
    // TODO: outsource this
    //boost::hash_combine(seed, q.interpretation->getStorage());
    const Interpretation::Storage& bits = q.interpretation->getStorage();
    for(Interpretation::Storage::enumerator en = bits.first();
        en != bits.end(); ++en)
    {
      boost::hash_combine(seed, *en);
      //LOG("hash_combine at " << *en << " yields " << seed);
    }
  }
  //LOG("hash_combine returning " << seed);
  return seed;
}
        
PluginAtom::Answer::Answer():
  output(new std::vector<Tuple>),
  used(false)
{
}

void
PluginAtom::addInputPredicate()
{
	// throw error if last input term was tuple
	if (inputType.size() > 0)
		if (inputType.back() == TUPLE)
			throw GeneralError("Tuple inputs must be specified last in input list");

    inputType.push_back(PREDICATE);
}


void
PluginAtom::addInputConstant()
{
	// throw error if last input term was tuple
	if (inputType.size() > 0)
		if (inputType.back() == TUPLE)
			throw GeneralError("Tuple inputs must be specified last in input list");

    inputType.push_back(CONSTANT);
}


void
PluginAtom::addInputTuple()
{
    inputType.push_back(TUPLE);
}


bool
PluginAtom::checkInputArity(const unsigned arity) const
{
  bool ret = (inputType.size() == arity);

  if (!inputType.empty())
    {
      return inputType.back() == TUPLE ? true : ret;
    }
  else
    {
      return ret;
    }
}


void
PluginAtom::setOutputArity(const unsigned arity)
{
    outputSize = arity;
}


bool
PluginAtom::checkOutputArity(const unsigned arity) const
{
    return arity == outputSize;
}


void PluginAtom::retrieveCached(const Query& query, Answer& answer)
{
  DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sidrc,"PluginAtom::retrieveCached");
  // Cache answer for queries which were already done once:
  //
  // The most efficient way would be:
  // * use cache for same inputSet + same *inputi + more specific pattern
  // * store new cache for new inputSet/*inputi combination or unrelated (does not unify) pattern
  // * replace cache for existing inputSet/*inputi combination and less specific (unifies in one direction) pattern
  // 
  // The currently implemented "poor (wo)man's version" is:
  // * store answers in cache with queries as keys, disregard relations between patterns
  ///@todo: efficiency could be increased for certain programs by considering pattern relationships as indicated above

#if 0
  #include "dlvhex/PrintVisitor.h"
  #include <iostream>
  std::cerr << "cache:" << std::endl;
  for( QueryAnswerCache::const_iterator i = queryAnswerCache.begin(); i != queryAnswerCache.end(); ++i)
  {
	  std::cerr << "  query: <";
	  RawPrintVisitor visitor(std::cerr);
	  i->first.getInterpretation().accept(visitor);
	  std::cerr << "|" << i->first.getInputTuple() << "|" << i->first.getPatternTuple() << ">" << std::endl;
  }


	  std::cerr << "retrieving query: <";
	  RawPrintVisitor visitor(std::cerr);
	  query.getInterpretation().accept(visitor);
	  std::cerr << "|" << query.getInputTuple() << "|" << query.getPatternTuple() << ">";
#endif

  //LOG("before queryAnswerCache");
  Answer& ans = queryAnswerCache[query];
  //LOG("after queryAnswerCache");
  if( ans.hasBeenUsed() )
  {
    // answer was not default constructed
    // -> use cache
    answer = ans;
  }
  else
  {
    // answer was default constructed
    // -> retrieve and replace in cache
    {
      DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sidr,"PluginAtom retrieve");
      #if 0
      #ifndef NDEBUG
      std::stringstream o;
      RawPrinter printer(o, query.interpretation->getRegistry());
      o << "retrieving for ";
      printer.printmany(query.input, ",");
      o << "/";
      printer.printmany(query.pattern, ",");
      o << "/" << *query.interpretation;
      LOG(o.str());
      #endif
      #endif
      retrieve(query, ans);
      // if there was no answer, perhaps it has never been used, so we use it manually
      ans.use();
      //LOG("after retrieve: answ is used = " << ans.hasBeenUsed());
    }
    answer = ans;
  }
}

void PluginAtom::retrieveCached(const Query& query, Answer& answer, ProgramCtx* ctx, NogoodContainerPtr nogoods)
{
	DBGLOG(DBG, "Retrieve with learning, pointer nogood container: " << (nogoods == NogoodContainerPtr() ? "not " : "") << "available" );

	DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sidrc,"PluginAtom::retrieveCached");
	// Cache answer for queries which were already done once:
	//
	// The most efficient way would be:
	// * use cache for same inputSet + same *inputi + more specific pattern
	// * store new cache for new inputSet/*inputi combination or unrelated (does not unify) pattern
	// * replace cache for existing inputSet/*inputi combination and less specific (unifies in one direction) pattern
	// 
	// The currently implemented "poor (wo)man's version" is:
	// * store answers in cache with queries as keys, disregard relations between patterns
	///@todo: efficiency could be increased for certain programs by considering pattern relationships as indicated above

	Answer& ans = queryAnswerCache[query];
	if( ans.hasBeenUsed() )
	{
		// answer was not default constructed
		// -> use cache
		answer = ans;
	}
	else
	{
		// answer was default constructed
		// -> retrieve and replace in cache
		{
			DLVHEX_BENCHMARK_REGISTER_AND_SCOPE(sidr,"PluginAtom retrieve");

			retrieve(query, ans, ctx, nogoods);
			// if there was no answer, perhaps it has never been used, so we use it manually
			ans.use();
		}
		answer = ans;
	}

#if 0
	retrieveCached(query, answer);

	if (solver != CDNLSolverPtr()){

		DBGLOG(DBG, "Learning from external call");

		// find relevant input
		bm::bvector<>::enumerator en = query.eatom->getPredicateInputMask()->getStorage().first();
		bm::bvector<>::enumerator en_end = query.eatom->getPredicateInputMask()->getStorage().end();

		Nogood extNgInput;

		while (en < en_end){
			extNgInput.insert(solver->createLiteral(*en, query.interpretation->getFact(*en)));
			en++;
		}

		DBGLOG(DBG, "Input nogood: " << extNgInput);

		// construct replacement atom
		OrdinaryAtom replacement(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG | ID::PROPERTY_AUX);
		replacement.tuple.resize(1);
		replacement.tuple[0] = registry->getAuxiliaryConstantSymbol('n', query.eatom->predicate);
		replacement.tuple.insert(replacement.tuple.end(), query.input.begin(), query.input.end());
		int s = replacement.tuple.size();

		const std::vector<Tuple>& otuples = answer.get();
		BOOST_FOREACH (Tuple otuple, otuples){
			replacement.tuple.resize(s);
			// add current output
			replacement.tuple.insert(replacement.tuple.end(), otuple.begin(), otuple.end());
			// get ID of this replacement atom
			ID idreplacement = registry->storeOrdinaryGAtom(replacement);

			Nogood extNg = extNgInput;
			extNg.insert(solver->createLiteral(idreplacement.address));
			DBGLOG(DBG, "Overall nogood: " << extNg);
			solver->addNogood(extNg);
		}
	}
#endif
}

void PluginAtom::retrieve(const Query& query, Answer& answer, ProgramCtx* ctx, NogoodContainerPtr nogoods){

	retrieve(query, answer);

	learnFromInputOutputBehavior(ctx, nogoods, query, answer);
	learnFromFunctionality(ctx, nogoods, query, answer);
}

void PluginAtom::learnFromInputOutputBehavior(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, const Answer& answer){

	if (ctx != 0 && nogoods != NogoodContainerPtr()){

		if (ctx->config.getOption("ExternalLearningEABehavior")){
			DBGLOG(DBG, "External Learning: EABehavior" << (ctx->config.getOption("ExternalLearningMonotonicity") ? " by exploiting monotonicity" : ""));

			Nogood extNgInput = getInputNogood(ctx, nogoods, query);

			Set<ID> out = getOutputAtoms(ctx, nogoods, query, answer, false);
			BOOST_FOREACH (ID oid, out){
				Nogood extNg = extNgInput;
				extNg.insert(oid);
				DBGLOG(DBG, "Learned nogood " << extNg << " from input-output behavior");
				nogoods->addNogood(extNg);
			}
		}
	}
}

void PluginAtom::learnFromFunctionality(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, const Answer& answer){

	if (ctx != 0 && nogoods != NogoodContainerPtr()){

		if (ctx->config.getOption("ExternalLearningFunctionality") && isFunctional(query)){
			DBGLOG(DBG, "External Learning: Functionality");

			// there is a unique output
			const std::vector<Tuple>& otuples = answer.get();

			if (otuples.size() > 0){
				ID uniqueOut = getOutputAtom(ctx, nogoods, query, otuples[0], true);

				// go through all output tuples which have been generated so far
				BOOST_FOREACH (Tuple t, this->otuples){
					ID id = getOutputAtom(ctx, nogoods, query, t, true);
					if (id != uniqueOut){
						Nogood excludeOthers;
						excludeOthers.insert(uniqueOut);
						excludeOthers.insert(id);
						DBGLOG(DBG, "Learned nogood " << excludeOthers << " from functionality");
						nogoods->addNogood(excludeOthers);
					}
				}
			}
		}
	}
}

void PluginAtom::learnFromGroundRule(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, ID groundRule){

	if (ctx != 0 && nogoods != NogoodContainerPtr()){
		DBGLOG(DBG, "External Learning: Ground Rule");

		const Rule& rule = ctx->registry()->rules.getByID(groundRule);

		Nogood ng;
		BOOST_FOREACH (ID hId, rule.head){
			const OrdinaryAtom& oat = ctx->registry()->ogatoms.getByID(hId);
			Tuple t;
			t.insert(t.end(), oat.tuple.begin() + 1, oat.tuple.end());
			ng.insert(getOutputAtom(ctx, nogoods, query, t, false));
		}
		BOOST_FOREACH (ID bId, rule.body){
			ng.insert(bId);
		}
		DBGLOG(DBG, "Learned nogood " << ng << " from rule");
		nogoods->addNogood(ng);
	}
}

void PluginAtom::learnFromRule(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, ID rid){

	if (ctx != 0 && nogoods != NogoodContainerPtr()){
		DBGLOG(DBG, "External Learning: Rule");

		DBGLOG(DBG, "Rewriting rule");
		const Rule& rule = ctx->registry()->rules.getByID(rid);

		Rule rrule(ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR);
		rrule.head = rule.head;
		BOOST_FOREACH (ID batom, rule.body){

			const OrdinaryAtom& oatom = batom.isOrdinaryGroundAtom() ? ctx->registry()->ogatoms.getByID(batom) : ctx->registry()->onatoms.getByID(batom);

			// replace predicate name by parameter from query.input
			OrdinaryAtom roatom = oatom;
			bool found = false;
			for (int inp = 0; inp < query.input.size(); inp++){
				std::stringstream inPredStr;
				inPredStr << "in" << (inp + 1);
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, inPredStr.str());
				ID inPredID = ctx->registry()->storeTerm(inPredTerm);

				if (roatom.tuple[0] == inPredID){
					roatom.tuple[0] = query.input[inp];
					found = true;
					break;
				}
			}
			assert(found);

			ID batomId = batom.isOrdinaryGroundAtom() ? ctx->registry()->storeOrdinaryGAtom(roatom) : ctx->registry()->storeOrdinaryNAtom(roatom);
			ID mask(ID::MAINKIND_LITERAL, 0);
			if (batom.isNaf()) mask = mask | ID(ID::NAF_MASK, 0);
			if (batom.isOrdinaryGroundAtom()) mask = mask | ID(ID::SUBKIND_ATOM_ORDINARYG, 0);
			if (batom.isOrdinaryNongroundAtom()) mask = mask | ID(ID::SUBKIND_ATOM_ORDINARYN, 0);
			rrule.body.push_back(ID(mask.kind, batomId.address));
		}
		ID rruleId = ctx->registry()->storeRule(rrule);

		DBGLOG(DBG, "Building ASP Program");
		InterpretationConstPtr edb = query.interpretation;
		std::vector<ID> idb;
		idb.push_back(rruleId);
		OrdinaryASPProgram program(ctx->registry(), idb, edb);

		DBGLOG(DBG, "Grounding learning rule");
		InternalGrounderPtr ig = InternalGrounderPtr(new InternalGrounder(*ctx, program, InternalGrounder::builtin));
		OrdinaryASPProgram gprogram = ig->getGroundProgram();

		DBGLOG(DBG, "Generating nogoods for all ground rules");
		BOOST_FOREACH (ID rid, gprogram.idb){
			learnGroundRuleNogood(ctx, nogoods, query, rid);
		}
	}







#if 0

	std::vector<Nogood> outnogoods;

	DBGLOG(DBG, "Constructing EDB");
	std::vector<ID> idb;
	idb.push_back(rule);
	InterpretationPtr edb = InterpretationPtr(new Interpretation(ctx->registry()));

	// go through all input atoms
	DBGLOG(DBG, "Rewriting input");
	const std::vector<ID>& params = query.input;
	for(Interpretation::Storage::enumerator it =
	    query.interpretation->getStorage().first();
	    it != query.interpretation->getStorage().end(); ++it){

		ID ogid(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *it);
		const OrdinaryAtom& ogatom = ctx->registry()->ogatoms.getByID(ogid);

		// check if the predicate matches any of the input parameters to simulator atom
		bool found = false;
		for (int inp = 0; inp < params.size(); ++inp){
			DBGLOG(DBG, "Checking predicate " << ogatom.tuple[0] << " against parameter " << inp);
			if (ogatom.tuple[0] == params[inp]){
				// replace the predicate by "in[inp]"
				std::stringstream inPredStr;
				inPredStr << "in" << (inp + 1);
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, inPredStr.str());
				ID inPredID = ctx->registry()->storeTerm(inPredTerm);
				OrdinaryAtom oareplace = ogatom;
				oareplace.tuple[0] = inPredID;

				// get ID of replaced atom
				ID oareplaceID = ctx->registry()->storeOrdinaryGAtom(oareplace);

				// set this atom in the input interpretation
				edb->getStorage().set_bit(oareplaceID.address);
				found = true;
				break;
			}
		}
		assert(found);
	}
	DBGLOG(DBG, "EDB: " << *edb);

	DBGLOG(DBG, "Grounding learning rule");
	OrdinaryASPProgram program(ctx->registry(), idb, edb);
	InternalGrounderPtr ig = InternalGrounderPtr(new InternalGrounder(*ctx, program, InternalGrounder::builtin));
	OrdinaryASPProgram gprogram = ig->getGroundProgram();

	DBGLOG(DBG, "Generating nogoods for all ground rules");
	BOOST_FOREACH (ID rid, gprogram.idb){
		DBGLOG(DBG, "Rewriting rule with ID " << rid);
		const Rule& rule = ctx->registry()->rules.getByID(rid);

		Rule rrule(ID::MAINKIND_RULE | ID::SUBKIND_RULE_REGULAR);
		rrule.head = rule.head;
		BOOST_FOREACH (ID batom, rule.body){
			const OrdinaryAtom& ogatom = ctx->registry()->ogatoms.getByID(batom);

			// replace predicate name by parameter from query.input
			OrdinaryAtom rogatom = ogatom;
			bool found = false;
			for (int inp = 0; inp < query.input.size(); inp++){
				std::stringstream inPredStr;
				inPredStr << "in" << (inp + 1);
				Term inPredTerm(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, inPredStr.str());
				ID inPredID = ctx->registry()->storeTerm(inPredTerm);

				if (rogatom.tuple[0] == inPredID){
					rogatom.tuple[0] = query.input[inp];
					found = true;
					break;
				}
			}
			assert(found);

			ID batomId = ctx->registry()->storeOrdinaryGAtom(rogatom);
			if (batom.isNaf()){
				rrule.body.push_back(batomId | ID(ID::NAF_MASK, 0));
			}else{
				rrule.body.push_back(batomId);
			}
		}
		ID rruleId = ctx->registry()->storeRule(rrule);

		Nogood ng = getGroundRuleNogood(ctx, nogoods, query, rruleId);
		outnogoods.push_back(ng);
	}
	return outnogoods;
#endif
}

Nogood PluginAtom::getInputNogood(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query){

	// find relevant input
	bm::bvector<>::enumerator en = query.eatom->getPredicateInputMask()->getStorage().first();
	bm::bvector<>::enumerator en_end = query.eatom->getPredicateInputMask()->getStorage().end();

	Nogood extNgInput;

	while (en < en_end){
		// get the predicate of the current input atom
		ID pred = query.interpretation->getRegistry()->ogatoms.getByID(ID(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG, *en)).tuple[0];

		// find the parameter index of this atom
		int index = query.inputPredicateTable.find(pred)->second;

		// for nonmonotonic parameters we need the positive and negative input, for monotonic ones the positive input suffices
		if (query.interpretation->getFact(*en) || !isMonotonic(query, index) || !ctx->config.getOption("ExternalLearningMonotonicity")){
			extNgInput.insert(nogoods->createLiteral(*en, query.interpretation->getFact(*en)));
		}
		en++;
	}

	return extNgInput;
}

Set<ID> PluginAtom::getOutputAtoms(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, const Answer& answer, bool sign){

	Set<ID> out;

	// construct replacement atom
	OrdinaryAtom replacement(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG | ID::PROPERTY_AUX);
	replacement.tuple.resize(1);
	replacement.tuple[0] = registry->getAuxiliaryConstantSymbol(sign ? 'r' : 'n', query.eatom->predicate);
	replacement.tuple.insert(replacement.tuple.end(), query.input.begin(), query.input.end());
	int s = replacement.tuple.size();

	const std::vector<Tuple>& otuples = answer.get();
	BOOST_FOREACH (Tuple otuple, otuples){
		// remember that otuple was generated
		this->otuples.push_back(otuple);

		replacement.tuple.resize(s);
		// add current output
		replacement.tuple.insert(replacement.tuple.end(), otuple.begin(), otuple.end());
		// get ID of this replacement atom
		ID idreplacement = nogoods->createLiteral(registry->storeOrdinaryGAtom(replacement));
		out.insert(idreplacement);
	}

	return out;
}

ID PluginAtom::getOutputAtom(ProgramCtx* ctx, NogoodContainerPtr nogoods, const Query& query, Tuple otuple, bool sign){

	// construct replacement atom with input from query
	OrdinaryAtom replacement(ID::MAINKIND_ATOM | ID::SUBKIND_ATOM_ORDINARYG | ID::PROPERTY_AUX);
	replacement.tuple.resize(1);
	replacement.tuple[0] = registry->getAuxiliaryConstantSymbol(sign ? 'r' : 'n', query.eatom->predicate);
	replacement.tuple.insert(replacement.tuple.end(), query.input.begin(), query.input.end());
	int s = replacement.tuple.size();

	// add output tuple
	replacement.tuple.insert(replacement.tuple.end(), otuple.begin(), otuple.end());

	ID idreplacement = nogoods->createLiteral(registry->storeOrdinaryGAtom(replacement));
	return idreplacement;
}

ID PluginAtom::getIDOfRule(ProgramCtx* ctx, std::string rule){

	// parse rule
	DBGLOG(DBG, "Parsing rule " << rule);
	InputProviderPtr ip(new InputProvider());
	ip->addStringInput(rule, "rule");
	Logger::Levels l = Logger::Instance().getPrintLevels();	// workaround: verbose causes the parse call below to fail (registry pointer is 0)
	Logger::Instance().setPrintLevels(0);
	ProgramCtx pc;
	pc.changeRegistry(ctx->registry());
	ModuleHexParser hp;
	hp.parse(ip, pc);
	Logger::Instance().setPrintLevels(l);

	DBGLOG(DBG, "Parsed " << rule);
	if (pc.idb.size() != 1){
		DBGLOG(DBG, "Error: Got " << pc.idb.size() << " rules");
		return ID_FAIL;
	}else{
		DBGLOG(DBG, "Got 1 rule");
		return pc.idb[0];
	}
}

PluginAtom::InputType
PluginAtom::getInputType(const unsigned index) const
{
	if (index >= inputType.size())
	{
		assert(inputType.back() == TUPLE);
		return inputType.back();
	}

    return inputType[index];
}

void PluginAtom::setRegistry(RegistryPtr reg)
{
  // i think we really don't want to change registry during the lifetime,
  // it would invalidate the cache and more bad things would happen
  assert(registry == 0);
  assert(reg != 0);
  registry = reg;
  predicateID = registry->terms.getIDByString(predicate);
  if( predicateID == ID_FAIL )
  {
    Term t(ID::MAINKIND_TERM | ID::SUBKIND_TERM_CONSTANT, predicate);
    predicateID = registry->terms.storeAndGetID(t);
  }
  assert(predicateID != ID_FAIL);
}

// call printUsage for each loaded plugin
void PluginInterface::printUsage(std::ostream& o) const
{
  // don't fail if no usage message has been defined, simply do not give one
}

// call processOptions for each loaded plugin
// (this is supposed to remove "recognized" options from pluginOptions)
void PluginInterface::processOptions(std::list<const char*>& pluginOptions, ProgramCtx& ctx)
{
  // don't fail if no option processing has been defined, simply do not process
}

DLVHEX_NAMESPACE_END

// Local Variables:
// mode: C++
// End:

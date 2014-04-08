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
 * @file   Converter.h
 * @author Andreas Humenberger <e1026602@student.tuwien.ac.at>
 * 
 * @brief  
 */

#ifndef CONVERTER_H
#define CONVERTER_H

#include "dlvhex2/PlatformDefinitions.h"
#include "dlvhex2/ProgramCtx.h"

DLVHEX_NAMESPACE_BEGIN

class RuleConverter
{
private:
	ProgramCtx ctx;
public:
	RuleConverter(ProgramCtx& ctx):
		ctx(ctx)
	{
	}
	~RuleConverter()
	{
	}
	ID convertRule(ID ruleid);
	void convertBody(const Tuple& body, Tuple& convbody);
};

DLVHEX_NAMESPACE_END

#endif // CONVERTER_H
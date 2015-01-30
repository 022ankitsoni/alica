/*
 * BasicFalseExpression.cpp
 *
 *  Created on: Oct 8, 2014
 *      Author: Stefan Jakob
 */

#include "engine/expressionhandler/BasicFalseCondition.h"

namespace alica
{

	BasicFalseCondition::BasicFalseCondition()
	{
	}

	BasicFalseCondition::~BasicFalseCondition()
	{
	}

	bool BasicFalseCondition::evaluate(shared_ptr<RunningPlan> rp)
	{
		return false;
	}

} /* namespace alica */

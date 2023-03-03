#pragma once

#include <alica_tests/DomainCondition.h>
#include <alica_tests/DomainPlan.h>
#include <boost/dll/alias.hpp>
#include <engine/BasicUtilityFunction.h>
#include <engine/DefaultUtilityFunction.h>
#include <engine/UtilityFunction.h>

namespace alica
{
class Defend : public DomainPlan
{
public:
    Defend(PlanContext& context);
};

BOOST_DLL_ALIAS(alica::BasicPlan::create, Defend)
BOOST_DLL_ALIAS(alica::BasicUtilityFunction::create, DefendUtilityFunction)
} /* namespace alica */
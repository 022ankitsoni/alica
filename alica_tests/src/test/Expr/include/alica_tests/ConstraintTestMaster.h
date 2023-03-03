#pragma once

#include <alica_tests/DomainCondition.h>
#include <alica_tests/DomainPlan.h>
#include <boost/dll/alias.hpp>
#include <engine/BasicUtilityFunction.h>
#include <engine/DefaultUtilityFunction.h>
#include <engine/UtilityFunction.h>

namespace alica
{
class ConstraintTestMaster : public DomainPlan
{
public:
    ConstraintTestMaster(PlanContext& context);
};

BOOST_DLL_ALIAS(alica::BasicPlan::create, ConstraintTestMaster)
BOOST_DLL_ALIAS(alica::BasicUtilityFunction::create, ConstraintTestMasterUtilityFunction)
} /* namespace alica */
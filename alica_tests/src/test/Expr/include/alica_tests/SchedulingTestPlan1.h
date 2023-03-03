#pragma once

#include <alica_tests/DomainCondition.h>
#include <alica_tests/DomainPlan.h>
#include <boost/dll/alias.hpp>
#include <engine/BasicUtilityFunction.h>
#include <engine/DefaultUtilityFunction.h>
#include <engine/UtilityFunction.h>
#include <libalica-tests/util/AlicaTestsPlan.h>

namespace alica
{
class SchedulingTestPlan1 : public AlicaTestsPlan<SchedulingTestPlan1>
{
public:
    SchedulingTestPlan1(PlanContext& context);

protected:
    virtual void onInit();
    virtual void onTerminate();
};

BOOST_DLL_ALIAS(alica::SchedulingTestPlan1::create, SchedulingTestPlan1)
BOOST_DLL_ALIAS(alica::BasicUtilityFunction::create, SchedulingTestPlan1UtilityFunction)
} /* namespace alica */
#include <alica_tests/SimpleTestPlan1412252439925.h>
/*PROTECTED REGION ID(eph1412252439925) ENABLED START*/
// Add additional using directives here
#include <alica/test/CounterClass.h>
/*PROTECTED REGION END*/

namespace alica
{
// Plan:  SimpleTestPlan (1412252439925)
//
// Tasks:
//   - DefaultTask (1225112227903) (Entrypoint: 1412252439927)
//
// States:
//   - TestState1 (1412252439926)
//   - TestState2 (1412761855746)
SimpleTestPlan1412252439925::SimpleTestPlan1412252439925(PlanContext& context)
        : DomainPlan(context)
{
    /*PROTECTED REGION ID(con1412252439925) ENABLED START*/
    // Add additional options here
    /*PROTECTED REGION END*/
}
SimpleTestPlan1412252439925::~SimpleTestPlan1412252439925()
{
    /*PROTECTED REGION ID(dcon1412252439925) ENABLED START*/
    // Add additional options here
    /*PROTECTED REGION END*/
}

// Check of PreCondition - (Name): NewPreCondition, (ConditionString):  , (Comment) :

/**
 * Available Vars:
 */
bool PreCondition1412781707952::evaluate(std::shared_ptr<RunningPlan> rp, const IAlicaWorldModel* wm)
{
    /*PROTECTED REGION ID(1412781707952) ENABLED START*/
    //--> "PreCondition:1412781707952  not implemented";
    return true;
    /*PROTECTED REGION END*/
}
// Check of RuntimeCondition - (Name): NewRuntimeCondition, (ConditionString): , (Comment) :

/**
 * Available Vars:
 */
bool RunTimeCondition1412781693884::evaluate(std::shared_ptr<RunningPlan> rp, const IAlicaWorldModel* wm)
{
    /*PROTECTED REGION ID(1412781693884) ENABLED START*/
    CounterClass::called++;
    return true;
    /*PROTECTED REGION END*/
}
/**
 * Task: DefaultTask  -> EntryPoint-ID: 1412252439927
 */
std::shared_ptr<UtilityFunction> UtilityFunction1412252439925::getUtilityFunction(Plan* plan)
{
    /*PROTECTED REGION ID(1412252439925) ENABLED START*/

    shared_ptr<UtilityFunction> defaultFunction = make_shared<DefaultUtilityFunction>(plan);
    return defaultFunction;

    /*PROTECTED REGION END*/
}

/*PROTECTED REGION ID(methods1412252439925) ENABLED START*/
// Add additional options here
void SimpleTestPlan1412252439925::onInit()
{
    LockedBlackboardRW bb(*(getBlackboard()));
    bb.set("targetChildStatus", PlanStatus::Success);
}
/*PROTECTED REGION END*/
} // namespace alica
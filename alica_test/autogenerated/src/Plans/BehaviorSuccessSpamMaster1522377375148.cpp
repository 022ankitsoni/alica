#include "Plans/BehaviorSuccessSpamMaster1522377375148.h"
using namespace alica;
/*PROTECTED REGION ID(eph1522377375148) ENABLED START*/  // Add additional using directives here
/*PROTECTED REGION END*/
namespace alicaAutogenerated {
// Plan:BehaviorSuccessSpamMaster

/* generated comment

 Task: DefaultTask  -> EntryPoint-ID: 1522377375150

 */
shared_ptr<UtilityFunction> UtilityFunction1522377375148::getUtilityFunction(Plan* plan) {
    /*PROTECTED REGION ID(1522377375148) ENABLED START*/

    shared_ptr<UtilityFunction> defaultFunction = make_shared<DefaultUtilityFunction>(plan);
    return defaultFunction;

    /*PROTECTED REGION END*/
}

// State: Normal in Plan: BehaviorSuccessSpamMaster

/*
 *
 * Transition:
 *   - Name: MISSING_NAME, ConditionString: , Comment :
 *
 * Plans in State:
 *   - Plan - (Name): SuccessSpamDefault, (PlanID): 1522377419087
 *
 * Tasks:
 *   - DefaultTask (1225112227903) (Entrypoint: 1522377375150)
 *
 * States:
 *   - Normal (1522377375149)
 *   - Dummy (1522377929290)
 *
 * Vars:
 */
bool TransitionCondition1522377944921::evaluate(shared_ptr<RunningPlan> rp) {
    /*PROTECTED REGION ID(1522377944058) ENABLED START*/
    return rp->anyChildrenStatus(alica::PlanStatus::Success);
    /*PROTECTED REGION END*/
}

// State: Dummy in Plan: BehaviorSuccessSpamMaster

/*
 *
 * Transition:
 *   - Name: MISSING_NAME, ConditionString: , Comment :
 *
 * Plans in State:
 *
 * Tasks:
 *   - DefaultTask (1225112227903) (Entrypoint: 1522377375150)
 *
 * States:
 *   - Normal (1522377375149)
 *   - Dummy (1522377929290)
 *
 * Vars:
 */
bool TransitionCondition1522377946607::evaluate(shared_ptr<RunningPlan> rp) {
    /*PROTECTED REGION ID(1522377945069) ENABLED START*/
    bool ok = rp->anyChildrenStatus(alica::PlanStatus::Success);
    // std::cout << "B "<<ok <<std::endl;
    return ok;
    /*PROTECTED REGION END*/
}

}  // namespace alicaAutogenerated

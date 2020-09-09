#include "BehaviourCreator.h"
#include "ConditionCreator.h"
#include "ConstraintCreator.h"
#include "TestWorldModel.h"
#include "UtilityFunctionCreator.h"
#include "engine/Assignment.h"
#include "engine/BasicBehaviour.h"
#include "engine/BehaviourPool.h"
#include "engine/DefaultUtilityFunction.h"
#include "engine/IAlicaCommunication.h"
#include "engine/PlanBase.h"
#include "engine/PlanRepository.h"
#include "engine/TeamObserver.h"
#include "engine/model/Behaviour.h"
#include "engine/model/Plan.h"
#include "engine/model/State.h"
#include <Behaviour/Attack.h>
#include <communication/AlicaDummyCommunication.h>
#include <engine/AlicaClock.h>
#include <engine/AlicaEngine.h>
#include <essentials/IDManager.h>
#include <gtest/gtest.h>
#include <test_alica.h>

namespace alica
{
namespace
{

class AlicaMultiAgent : public AlicaTestMultiAgentFixture
{
protected:
    const int agentCount = 2;
    const char* getRoleSetName() const override { return "RolesetTA"; }
    const char* getMasterPlanName() const override { return "MultiAgentTestMaster"; }
    int getAgentCount() const override { return agentCount; }
    const char* getHostName(int agentNumber) const override
    {
        if (agentNumber) {
            return "hairy";
        } else {
            return "nase";
        }
    }
};
/**
 * Tests whether it is possible to use multiple agents.
 */
TEST_F(AlicaMultiAgent, runMultiAgentPlan)
{
    ASSERT_NO_SIGNAL
    tcs[0]->startEngine();
    tcs[1]->startEngine();
    tcs[0]->getAlicaClock().sleep(getDiscoveryTimeout());
    tcs[0]->stepEngine();
    tcs[1]->stepEngine();

    for (int i = 0; i < 20; i++) {
        // I think that is done by stepEngine already
//        ASSERT_TRUE(tcs[0]->getPlanBase().isWaiting());
//        ASSERT_TRUE(tcs[1]->getPlanBase().isWaiting());
        std::cout << "AE1 step " << i << "(" << tcs[0]->getLocalAgentId() << ")" << std::endl;
        tcs[0]->stepEngine();

        std::cout << "AE2 step " << i << "(" << tcs[1]->getLocalAgentId() << ")" << std::endl;
        tcs[1]->stepEngine();

        //        if (i > 24)
        //        {
        //            if (aes[0]->getPlanBase().getDeepestNode() != nullptr)
        //                cout << "AE: " << aes[0]->getPlanBase().getDeepestNode()->toString() << endl;
        //            if (aes[1]->getPlanBase().getDeepestNode() != nullptr)
        //                cout << "AE2: " << aes[1]->getPlanBase().getDeepestNode()->toString() << endl;
        //            cout << "-------------------------" << endl;
        //        }

        if (i < 10) {
            ASSERT_EQ(tcs[0]->getRootNode()->getActiveState()->getId(), 1413200842974);
            ASSERT_EQ(tcs[1]->getRootNode()->getActiveState()->getId(), 1413200842974);
        }
        if (i == 10) {
            std::cout << "1--------- Initial State passed ---------" << std::endl;
            alicaTests::TestWorldModel::getOne()->setTransitionCondition1413201227586(true);
            alicaTests::TestWorldModel::getTwo()->setTransitionCondition1413201227586(true);
        }
        if (i > 11 && i < 15) {
            ASSERT_EQ(tcs[0]->getRootNode()->getActiveState()->getId(), 1413201213955);
            ASSERT_EQ(tcs[1]->getRootNode()->getActiveState()->getId(), 1413201213955);
            ASSERT_EQ(tcs[0]->getRootNode()->getChildren()[0]->getActivePlan()->getName(), std::string("MultiAgentTestPlan"));
            ASSERT_EQ(tcs[1]->getRootNode()->getChildren()[0]->getActivePlan()->getName(), std::string("MultiAgentTestPlan"));
        }
        if (i == 15) {
            for (const auto& iter : tcs[0]->getBehaviourPool().getAvailableBehaviours()) {
                if (iter.second->getName() == "Attack") {
                    ASSERT_GT(static_cast<alica::Attack*>(&*iter.second)->callCounter, 5);
                    if (((alica::Attack*) &*iter.second)->callCounter > 3) {
                        alicaTests::TestWorldModel::getOne()->setTransitionCondition1413201052549(true);
                        alicaTests::TestWorldModel::getTwo()->setTransitionCondition1413201052549(true);
                        alicaTests::TestWorldModel::getOne()->setTransitionCondition1413201370590(true);
                        alicaTests::TestWorldModel::getTwo()->setTransitionCondition1413201370590(true);
                    }
                }
            }
            std::cout << "2--------- Engagement to cooperative plan passed ---------" << std::endl;
        }
        if (i == 16) {
            ASSERT_TRUE(tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413201030936 ||
                                tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413201030936)
                    << std::endl
                    << tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() << " "
                    << tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() << std::endl;

            ASSERT_TRUE(tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413807264574 ||
                                tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413807264574)
                    << std::endl
                    << tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() << " "
                    << tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() << std::endl;
            alicaTests::TestWorldModel::getOne()->setTransitionCondition1413201227586(false);
            alicaTests::TestWorldModel::getTwo()->setTransitionCondition1413201227586(false);
            std::cout << "3--------- Passed transitions in subplan passed ---------" << std::endl;
        }
        if (i >= 17 && i <= 18) {
            ASSERT_TRUE(tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413201030936 ||
                                tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413201030936)
                    << "AE State: " << tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId()
                    << " AE2 State: " << tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() << std::endl;
            ASSERT_TRUE(tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413807264574 ||
                                tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() == 1413807264574)
                    << "AE State: " << tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->getId() << " "
                    << tcs[0]->getRootNode()->getChildren()[0]->getActiveState()->toString() << std::endl
                    << " AE2 State: " << tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->getId() << " "
                    << tcs[1]->getRootNode()->getChildren()[0]->getActiveState()->toString() << std::endl;
            if (i == 18) {
                std::cout << "4--------- Stayed in these state although previous transitions are not true anymore ---------" << std::endl;
                alicaTests::TestWorldModel::getOne()->setTransitionCondition1413201389955(true);
                alicaTests::TestWorldModel::getTwo()->setTransitionCondition1413201389955(true);
            }
        }
        if (i == 19) {
            ASSERT_TRUE(tcs[1]->getRootNode()->getActiveState()->getId() == 1413201380359 &&
                                tcs[0]->getRootNode()->getActiveState()->getId() == 1413201380359)
                    << " AE State: " << tcs[0]->getRootNode()->getActiveState()->getId()
                    << " AE2 State: " << tcs[1]->getRootNode()->getActiveState()->getId() << std::endl;
        }
    }
}
}
}
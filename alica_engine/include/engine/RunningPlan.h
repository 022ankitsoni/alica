/*
 * RunningPlan.h
 *
 *  Created on: Jun 10, 2014
 *      Author: Stefan Jakob
 */

#ifndef RUNNINGPLAN_H_
#define RUNNINGPLAN_H_

using namespace std;

#include <map>
#include <list>
#include <memory>
#include <iostream>
#include <string>
#include <sstream>
#include <unordered_set>
#include <algorithm>
#include <SystemConfig.h>

#include "engine/PlanStatus.h"
#include "engine/PlanChange.h"

using namespace std;

namespace alica
{

	class BasicBehaviour;
	class AbstractPlan;
	class Assignment;
	class State;
	class EntryPoint;
	class PlanType;
	class IBehaviourPool;
	class ITeamObserver;
	class Plan;
	class RuleBook;
	class ConstraintStore;
	class CycleManager;
	class BehaviourConfiguration;
	class IPlanTreeVisitor;
	class SimplePlanTree;

	/**
	 * A RunningPlan represents a plan or a behaviour in execution, holding all information relevant at runtime.
	 */
	class RunningPlan : public enable_shared_from_this<RunningPlan>
	{
	public:
		RunningPlan();
		RunningPlan(Plan* plan);
		RunningPlan(PlanType* pt);
		RunningPlan(BehaviourConfiguration* bc);
		virtual ~RunningPlan();
		bool isBehaviour();
		void setBehaviour(bool behaviour);
		bool isAllocationNeeded();
		list<shared_ptr<RunningPlan>>& getChildren();
		void setChildren(list<shared_ptr<RunningPlan>> children);
		AbstractPlan* getPlan();
		void setPlan(AbstractPlan* plan);
		shared_ptr<BasicBehaviour> getBasicBehaviour();
		void setBasicBehaviour(shared_ptr<BasicBehaviour> basicBehaviour);
		Assignment* getAssignment();
		void setAssignment(Assignment* assignment);
		void printRecursive();
		unsigned long getPlanStartTime();
		unsigned long getStateStartTime();
		bool isActive();
		void setActive(bool active);
		void setRobotsAvail(unique_ptr<list<int> > robots);
		void setAllocationNeeded(bool allocationNeeded);
		void setFailHandlingNeeded(bool failHandlingNeeded);
		void setOwnEntryPoint(EntryPoint* value);
		PlanChange tick(RuleBook* rules);
		ConstraintStore* getConstraintStore() const;
		EntryPoint* getOwnEntryPoint() const;
		void setParent(weak_ptr<RunningPlan> s);
		weak_ptr<RunningPlan> getParent() const;
		bool getFailHandlingNeeded() const;
		PlanStatus getStatus() const;
		PlanType* getPlanType();
		bool evalPreCondition();
		bool evalRuntimeCondition();
		State* getActiveState();
		void setActiveState(State* activeState);
		void addChildren(shared_ptr<list<shared_ptr<RunningPlan>>> runningPlans);
		void moveState(State* nextState);
		void clearFailures();
		void clearFailedChildren();
		void addFailure();
		void addChildren(list<shared_ptr<RunningPlan>>& children);
		int getFailure();
		void deactivateChildren();
		void clearChildren();
		void adaptAssignment(shared_ptr<RunningPlan> r);
		void setFailedChild(AbstractPlan* child);
		unique_ptr<list<int> > getRobotsAvail();
		void setRobotAvail(int robot);
		void setRobotUnAvail(int robot);
		void accept(IPlanTreeVisitor* vis);
		void deactivate();
		bool anyChildrenStatus(PlanStatus ps);
		bool allChildrenStatus(PlanStatus ps);
		bool anyChildrenTaskSuccess();
		bool anyChildrenTaskFailure();
		bool anyChildrenTaskTerminated();
		void activate();
		EntryPoint* getActiveEntryPoint();
		void setActiveEntryPoint(EntryPoint* activeEntryPoint);
		void limitToRobots(unordered_set<int> robots);
		CycleManager* getCycleManagement();
		void setCycleManagement(CycleManager* cycleManagement);
		void revokeAllConstraints();
		void attachPlanConstraints();
		bool recursiveUpdateAssignment(list<shared_ptr<SimplePlanTree> > spts, vector<int> availableAgents,list<int> noUpdates, unsigned long now);
		void toMessage(list<long>& message, shared_ptr<RunningPlan>& deepestNode, int& depth, int curDepth);
		string toString();

	private:
		void setConstraintStore(ConstraintStore* constraintStore);

	protected:
		weak_ptr<RunningPlan> parent;
		bool behaviour;
		AbstractPlan* plan;
		shared_ptr<BasicBehaviour> basicBehaviour;
		list<shared_ptr<RunningPlan> > children;
		Assignment* assignment;
		State* activeState;
		EntryPoint* activeEntryPoint;
		PlanStatus status;
		/**
		 * The (ROS-)timestamp referring to when the local robot entered the ActiveState.
		 */
		unsigned long stateStartTime;
		/**
		 * The timestamp referring to when this plan was started by the local robot
		 */
		unsigned long planStartTime;
		int ownId;
		unique_ptr<list<int> > robotsAvail;
		map<AbstractPlan*, int> failedSubPlans;
		PlanType* planType;
		int failCount;
		bool failHandlingNeeded;
		/**
		 * Whether or not this running plan is active or has been removed from the plan tree
		 */
		bool active;
		IBehaviourPool* bp;
		ITeamObserver* to;bool allocationNeeded;
		unsigned long assignmentProtectionTime = (((*supplementary::SystemConfig::getInstance())["Alica"]->get<
				unsigned long>("Alica.AssignmentProtectionTime", NULL)) * 1000000);
		CycleManager* cycleManagement;
		ConstraintStore* constraintStore;
	};

}
/* namespace alica */

#endif /* RUNNINGPLAN_H_ */

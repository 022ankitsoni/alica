/*
 * CycleManager.cpp
 *
 *  Created on: Jul 10, 2014
 *      Author:  Paul Panin
 */

#include "engine/allocationauthority/CycleManager.h"

#include <SystemConfig.h>
#include "engine/AlicaEngine.h"
#include "engine/allocationauthority/AllocationDifference.h"
#include "engine/ITeamObserver.h"
#include "engine/IAlicaClock.h"
#include "engine/model/EntryPoint.h"
#include "engine/Assignment.h"
#include "engine/allocationauthority/EntryPointRobotPair.h"
#include "engine/RunningPlan.h"
#include "engine/model/Plan.h"
#include "engine/model/AbstractPlan.h"
#include "engine/containers/AllocationAuthorityInfo.h"
#include "engine/model/PlanType.h"
#include "engine/PlanRepository.h"
#include "engine/containers/EntryPointRobots.h"
#include "engine/collections/StateCollection.h"

namespace alica
{
	/**
	 * Construct a CycleManager for a RunningPlan
	 * @param p A RunningPlan
	 */
	CycleManager::CycleManager(AlicaEngine* ae, RunningPlan* p)
	{
		sc = supplementary::SystemConfig::getInstance();
		maxAllocationCycles = (*sc)["Alica"]->get<int>("Alica", "CycleDetection", "CycleCount");
		enabled = (*sc)["Alica"]->get<bool>("Alica", "CycleDetection", "Enabled");
		minimalOverrideTimeInterval = (*sc)["Alica"]->get<unsigned long>(
				"Alica", "CycleDetection", "MinimalAuthorityTimeInterval") * 1000000;
		maximalOverrideTimeInterval = (*sc)["Alica"]->get<unsigned long>(
				"Alica", "CycleDetection", "MaximalAuthorityTimeInterval") * 1000000;
		overrideShoutInterval = (*sc)["Alica"]->get<unsigned long>("Alica", "CycleDetection",
																							"MessageTimeInterval")
				* 1000000;
		overrideWaitInterval = (*sc)["Alica"]->get<unsigned long>("Alica", "CycleDetection",
																							"MessageWaitTimeInterval")
				* 1000000;
		historySize = (*sc)["Alica"]->get<int>("Alica", "CycleDetection", "HistorySize");

		this->ae = ae;
		this->intervalIncFactor = (*sc)["Alica"]->get<double>("Alica", "CycleDetection", "IntervalIncreaseFactor");
		this->intervalDecFactor = (*sc)["Alica"]->get<double>("Alica", "CycleDetection", "IntervalDecreaseFactor");
		this->allocationHistory = vector<AllocationDifference*>(this->historySize);
		for (int i = 0; i < this->allocationHistory.size(); i++)
		{
			this->allocationHistory[i] = new AllocationDifference();
		}
		this->newestAllocationDifference = 0;
		this->state = CycleState::observing;
		this->rp = p;
		this->myID = ae->getTeamObserver()->getOwnId();
		this->pr = ae->getPlanRepository();
		this->overrideTimestamp = 0;
		this->overrideShoutTime = 0;
		this->fixedAllocation = nullptr;
	}

	CycleManager::~CycleManager()
	{
		for (int i = 0; i < this->allocationHistory.size(); i++)
		{
			delete this->allocationHistory[i];
		}
	}

	/**
	 * Called once per engine iteration by the corresponding RunningPlan.
	 * Checks whether a state change is needed.
	 */
	void CycleManager::update()
	{
		if (!this->enabled)
		{
			return;
		}
		if (this->rp->isBehaviour())
		{
			return;
		}
		if (this->state == CycleState::observing)
		{
			if (detectAllocationCycle())
			{
				cout << "Cycle Detected!" << endl;

				this->state = CycleState::overriding;
				this->rp->getPlan()->setAuthorityTimeInterval(
						min(maximalOverrideTimeInterval,
							(alicaTime)(this->rp->getPlan()->getAuthorityTimeInterval() * intervalIncFactor)));
				this->overrideShoutTime = 0;
				cout << "Assuming Authority for " << this->rp->getPlan()->getAuthorityTimeInterval() / 1000000000.0
						<< "sec!" << endl;
				this->overrideTimestamp = ae->getIAlicaClock()->now();
			}
			else
			{
				this->rp->getPlan()->setAuthorityTimeInterval(
						max(minimalOverrideTimeInterval,
							(alicaTime)(this->rp->getPlan()->getAuthorityTimeInterval() * intervalDecFactor)));
			}
		}
		else
		{
			if (this->state == CycleState::overriding
					&& this->overrideTimestamp + this->rp->getPlan()->getAuthorityTimeInterval()
							< ae->getIAlicaClock()->now())
			{
#ifdef CM_DEBUG
				cout << "Resume Observing!" << endl;
#endif
				this->state = CycleState::observing;
				this->fixedAllocation = nullptr;
			}
			else if (this->state == CycleState::overridden
					&& this->overrideShoutTime + this->rp->getPlan()->getAuthorityTimeInterval()
							< ae->getIAlicaClock()->now())
			{
#ifdef CM_DEBUG
				cout << "Resume Observing!" << endl;
#endif
				this->state = CycleState::observing;
				this->fixedAllocation = nullptr;

			}
		}
	}

	/**
	 * Indicates whether an authorative allocation is present
	 * @return bool
	 */
	bool CycleManager::isOverridden()
	{
		return this->state == CycleState::overridden && this->fixedAllocation != nullptr;
	}

	/**
	 * Notify the CycleManager of a new AllocationDifference
	 * @param curP The RunningPlan of this CycleManager, in case it has changed.
	 * @param aldif The new AllocationDifference
	 */
	void alica::CycleManager::setNewAllocDiff(AllocationDifference* aldif)
	{
		if (!enabled)
		{
			return;
		}
		lock_guard<mutex> lock(this->allocationHistoryMutex);
		try
		{
			this->newestAllocationDifference = (this->newestAllocationDifference + 1) % this->allocationHistory.size();
			delete this->allocationHistory[this->newestAllocationDifference];
			this->allocationHistory[this->newestAllocationDifference] = aldif;
		}
		catch (exception &e)
		{
			cerr << "Exception in Alloc Difference Calculation:" << endl;
			cerr << e.what();
		}
	}

	/**
	 * Notify the CycleManager of a change in the assignment
	 * @param curP The RunningPlan of this CycleManager, in case it has changed.
	 * @param oldAss The former Assignment
	 * @param newAss The new Assignment
	 * @param reas The AllocationDifference.Reason for this change.
	 */
	void alica::CycleManager::setNewAllocDiff(shared_ptr<Assignment> oldAss, shared_ptr<Assignment> newAss,
												AllocationDifference::Reason reas)
	{
		if (!enabled)
		{
			return;
		}
		if (oldAss == nullptr)
		{
			return;
		}
		lock_guard<mutex> lock(this->allocationHistoryMutex);
		try
		{
			this->newestAllocationDifference = (this->newestAllocationDifference + 1) % this->allocationHistory.size();
			this->allocationHistory[this->newestAllocationDifference]->reset();

			for (EntryPoint* ep : (*oldAss->getEntryPoints()))
			{

				auto newRobots = newAss->getRobotsWorking(ep);
				auto oldRobots = oldAss->getRobotsWorking(ep);
				for (int oldId : (*oldRobots))
				{
					// C# newRobots->size() == nullptr
					if (newRobots->size() == 0 || find(newRobots->begin(), newRobots->end(), oldId) == newRobots->end())
					{
						this->allocationHistory[this->newestAllocationDifference]->getSubtractions().push_back(
								new EntryPointRobotPair(ep, oldId));
					}
				}
				if (newRobots != nullptr)
				{
					for (int newId : (*newRobots))
					{
						if (find(oldRobots->begin(), oldRobots->end(), newId) == oldRobots->end())
						{
							this->allocationHistory[this->newestAllocationDifference]->getAdditions().push_back(
									new EntryPointRobotPair(ep, newId));
						}
					}
				}

			}
			this->allocationHistory[this->newestAllocationDifference]->setReason(reas);
		}
		catch (exception &e)
		{
			cerr << "Exception in Alloc Difference Calculation:" << endl;
			cerr << e.what();

		}
	}

	/**
	 * Message Handler
	 * @param aai A shared_ptr<AllocationAuthorityInfo>
	 */
	void alica::CycleManager::handleAuthorityInfo(shared_ptr<AllocationAuthorityInfo> aai)
	{
		if (!enabled)
		{
			return;
		}
		long rid = aai->authority;
		if (rid == myID)
		{
			return;
		}
		if (rid > myID)
		{
			this->state = CycleState::overridden;
			this->overrideShoutTime = ae->getIAlicaClock()->now();
			this->fixedAllocation = aai;
		}
		else
		{
			cout << "Rcv: Rejecting Authority!" << endl;
			if (this->state != CycleState::overriding)
			{
				this->state = CycleState::overriding;
				this->rp->getPlan()->setAuthorityTimeInterval(
						min(maximalOverrideTimeInterval,
							(alicaTime)(this->rp->getPlan()->getAuthorityTimeInterval() * intervalIncFactor)));
				this->overrideTimestamp = ae->getIAlicaClock()->now();
				this->overrideShoutTime = 0;
			}
		}
	}

	bool alica::CycleManager::needsSending()
	{
		return this->state == CycleState::overriding
				&& (this->overrideShoutTime + overrideShoutInterval < ae->getIAlicaClock()->now());
	}

	/**
	 * Indicate to the manager that a corresponding message has been sent.
	 */
	void alica::CycleManager::sent()
	{
		this->overrideShoutTime = ae->getIAlicaClock()->now();
	}

	/**
	 * Indicates whether the local agent currently holds authority over the plan.
	 * @return A bool
	 */
	bool alica::CycleManager::haveAuthority()
	{
		return this->state == CycleState::overriding;
	}

	/**
	 * Apply the authorative assignment to the RunningPlan
	 * @param r A shared_ptr<RunningPlan>
	 * @return A bool
	 */
	bool CycleManager::setAssignment(shared_ptr<RunningPlan> r)
	{
#ifdef CM_DEBUG
		cout << "Setting new Assignment " << rp->getPlan()->getName() << "!" << endl;
#endif
		EntryPoint* myEntryPoint = nullptr;
		if (this->fixedAllocation == nullptr)
		{
			return false;
		}
		bool modifiedSelf = false;
		bool modified = false;
		if (this->fixedAllocation->planId != rp->getPlan()->getId())
		{
			if (rp->getPlanType()->getId() != this->fixedAllocation->planType)
			{
				return false;
			}
			Plan* newPlan = nullptr;
			for (Plan* p : rp->getPlanType()->getPlans())
			{
				if (p->getId() == this->fixedAllocation->planId)
				{
					newPlan = p;
					rp->setPlan(p);
					break;
				}
			}
			rp->setAssignment(make_shared<Assignment>(newPlan, this->fixedAllocation));
			for (EntryPointRobots epr : this->fixedAllocation->entryPointRobots)
			{
				if (find(epr.robots.begin(), epr.robots.end(), myID) != epr.robots.end())
				{
					myEntryPoint = pr->getEntryPoints().at(epr.entrypoint);
				}
			}

			modifiedSelf = true;

		}
		else
		{
			for (EntryPointRobots epr : this->fixedAllocation->entryPointRobots)
			{
				for (int robot : epr.robots)
				{
					EntryPoint* e = pr->getEntryPoints().at(epr.entrypoint);
					bool changed = rp->getAssignment()->updateRobot(robot, e);
					if (changed)
					{
						if (robot == myID)
						{
							modifiedSelf = true;
							myEntryPoint = e;
						}
						else
						{
							modified = true;
						}
					}
				}
			}
		}
		if (modifiedSelf)
		{
			rp->setOwnEntryPoint(myEntryPoint);
			rp->deactivateChildren();
			rp->clearChildren();
			rp->clearFailedChildren();
			rp->setAllocationNeeded(true);

		}
		else
		{
			if (rp->getActiveState() != nullptr)
			{
				auto robotsJoined = rp->getAssignment()->getRobotStateMapping()->getRobotsInState(rp->getActiveState());
				for (shared_ptr<RunningPlan> c : rp->getChildren())
				{
					c->limitToRobots(robotsJoined);
				}
			}
		}
#ifdef CM_DEBUG
		if (!modifiedSelf)
		{
			cout << "But no change!" << endl;
		}
#endif
		return modifiedSelf || modified;
	}

	/**
	 * Indicates wether authority allows the assignment of this plan to be changed.
	 * @return A bool
	 */
	bool CycleManager::mayDoUtilityCheck()
	{
		return this->state != CycleState::overridden;
	}

	bool CycleManager::detectAllocationCycle()
	{
		//A Cycle occurs n-times,
		//Consists of 1 UtilityChange, m message update
		//after uc, allocation is same again (delta = 0)
		int cyclesFound = 0;
		int count = 0;
		AllocationDifference* utChange = nullptr;
		shared_ptr<AllocationDifference> temp = make_shared<AllocationDifference>();
		lock_guard<mutex> lock(this->allocationHistoryMutex);
		{
			for (int i = this->newestAllocationDifference; count < this->allocationHistory.size(); i--)
			{
				count++;
				if (i < 0)
				{
					i = this->allocationHistory.size() - 1;
				}

				if (this->allocationHistory[i]->getReason() == AllocationDifference::Reason::utility)
				{
					if (utChange != nullptr)
					{
						return false;
					}
					utChange = this->allocationHistory[i];
					temp->reset();
					temp->applyDifference(utChange);
				}
				else
				{
					if (this->allocationHistory[i]->getReason() == AllocationDifference::Reason::empty)
					{
						return false;
					}
					if (utChange == nullptr)
					{
						continue;
					}
					temp->applyDifference(this->allocationHistory[i]);
					if (temp->isEmpty())
					{
						cyclesFound++;
						if (cyclesFound > maxAllocationCycles)
						{
							for (int k = 0; k < this->allocationHistory.size(); k++)
							{
								this->allocationHistory[k]->reset();
							}
							return true;
						}
						utChange = nullptr;
					}

				}
			}

		}
		return false;
	}

} /* namespace supplementary */

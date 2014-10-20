/*
 * ConstraintQuery.cpp
 *
 *  Created on: Oct 17, 2014
 *      Author: Philipp
 */
//#define CQ_DEBUG
#include "engine/constraintmodul/ConstraintQuery.h"

#include "engine/AlicaEngine.h"
#include "engine/ITeamObserver.h"
#include "engine/RunningPlan.h"
#include "engine/collections/RobotEngineData.h"
#include "engine/constraintmodul/ConstraintCall.h"
#include "engine/constraintmodul/ConstraintDescriptor.h"
#include "engine/constraintmodul/ConstraintHelper.h"
#include "engine/constraintmodul/ConstraintStore.h"
#include "engine/constraintmodul/IConstraintSolver.h"
#include "engine/constraintmodul/SolverTerm.h"
#include "engine/constraintmodul/SolverVariable.h"
#include "engine/model/Condition.h"
#include "engine/model/Parametrisation.h"
#include "engine/model/PlanType.h"
#include "engine/model/Variable.h"

#include <iostream>

namespace alica
{

	ConstraintQuery::ConstraintQuery()
	{
		store = make_shared<UniqueVarStore>();
		queriedStaticVariables = vector<Variable*>();
		queriedDomainVariables = vector<Variable*>();
		relevantStaticVariables = vector<Variable*>();
		relevantDomainVariables = vector<Variable*>();
		calls = vector<shared_ptr<ConstraintCall>>();
		to = (new AlicaEngine())->getTeamObserver();
	}

	void ConstraintQuery::addVariable(Variable* v)
	{
		queriedStaticVariables.push_back(v);
	}

	void ConstraintQuery::addVariable(int robot, string ident)
	{
		queriedDomainVariables.push_back(to->getRobotById(robot)->getSortedVariable(ident));
	}

	void ConstraintQuery::clearDomainVariables()
	{
		queriedDomainVariables.clear();
	}

	void ConstraintQuery::clearStaticVariables()
	{
		queriedStaticVariables.clear();
	}

	bool ConstraintQuery::existsSolution(shared_ptr<RunningPlan> rp)
	{
		store->clear();
		relevantStaticVariables.clear();
		relevantDomainVariables.clear();
		calls.clear();
		relevantStaticVariables.insert(relevantStaticVariables.end(), queriedStaticVariables.begin(),
										queriedStaticVariables.end());
		relevantDomainVariables.insert(relevantDomainVariables.end(), queriedDomainVariables.begin(),
										queriedDomainVariables.end());
		for (Variable* v : relevantStaticVariables)
		{
			store->add(v);
		}
#ifdef CQ_DEBUG
		cout << "CQ: Starting Existential Query with static Vars:";
		for (Variable* v : relevantStaticVariables)
		{
			cout << " " << v->getName();
		}
		cout << endl;
#endif
		while (rp != nullptr && (relevantStaticVariables.size() > 0 || relevantDomainVariables.size() > 0))
		{
#ifdef CQ_DEBUG
			cout << "CQ: Query at " << rp->getPlan()->getName() << endl;
#endif
			rp->getConstraintStore()->acceptQuery(shared_from_this());
			if (rp->getPlanType() != nullptr)
			{ //process bindings for plantype
				vector<Variable*> tmpVector = vector<Variable*>();
				for (Parametrisation* p : rp->getPlanType()->getParametrisation())
				{
					if (p->getSubPlan() == rp->getPlan()
							&& find(relevantStaticVariables.begin(), relevantStaticVariables.end(), p->getSubVar())
									!= relevantStaticVariables.end())
					{
						tmpVector.push_back(p->getVar());
						store->addVarTo(p->getSubVar(), p->getVar());
					}
				}
				relevantStaticVariables = tmpVector;
			}

			shared_ptr<RunningPlan> parent;
			if (rp->getParent().use_count() > 0 && (parent = rp->getParent().lock())->getActiveState() != nullptr)
			{
				vector<Variable*> tmpVector = vector<Variable*>();
				for (Parametrisation* p : rp->getPlanType()->getParametrisation())
				{
					if ((p->getSubPlan() == rp->getPlan() || p->getSubPlan() == rp->getPlanType())
							&& find(relevantStaticVariables.begin(), relevantStaticVariables.end(), p->getSubVar())
									!= relevantStaticVariables.end())
					{
						tmpVector.push_back(p->getVar());
						store->addVarTo(p->getSubVar(), p->getVar());
					}
				}
				relevantStaticVariables = tmpVector;
			}
			rp = parent;
		}

		if (calls.size() == 0)
		{
#ifdef CQ_DEBUG
			cout << "CQ: Empty Query!" << endl;
#endif
			return false;
		}

		vector<shared_ptr<ConstraintDescriptor>> cds = vector<shared_ptr<ConstraintDescriptor>>();
		for (shared_ptr<ConstraintCall> c : calls)
		{
			vector<Variable*> conditionVariables = vector<Variable*>(c->getCondition()->getVariables());

			vector<shared_ptr<SolverVariable>> varr = vector<shared_ptr<SolverVariable>>(conditionVariables.size());
			for (int j = 0; j < conditionVariables.size(); ++j)
			{
				varr[j] = store->getRep(conditionVariables[j])->getSolverVar();
			}
			vector<vector<vector<shared_ptr<SolverTerm>>> > sortedVars = vector<vector<vector<shared_ptr<SolverTerm>>>>();
			vector<vector<int>> agentsInScope = vector<vector<int>>();
			for (int j = 0; j < c->getSortedVariables().size(); ++j)
			{
				vector<vector<shared_ptr<SolverTerm>>> ll = vector<vector<shared_ptr<SolverTerm>>>();
				agentsInScope.push_back(c->getAgentsInScope()[j]);
				sortedVars.push_back(ll);
				for (vector<Variable*> dvarr : c->getSortedVariables()[j])
				{
					vector<shared_ptr<SolverTerm>> dtvarr = vector<shared_ptr<SolverTerm>>(dvarr.size());
					for (int i = 0; i < dtvarr.size(); ++i)
					{
						dtvarr[i] = dvarr[i]->getSolverVar();
					}
					ll.push_back(dtvarr);
				}
			}
			shared_ptr<ConstraintDescriptor> cd = make_shared<ConstraintDescriptor>(varr, sortedVars);
			cd->setAgentsInScope(agentsInScope);
//			c->getCondition()->getConstraint(cd, c->getRunningPlan());	TODO: not implemented yet
			cds.push_back(cd);
		}
		vector<Variable*> qVars = store->getAllRep();
		qVars.insert(qVars.end(), relevantDomainVariables.begin(), relevantDomainVariables.end());
		return ConstraintHelper::getSolver()->existsSolution(qVars, cds);
	}

	bool ConstraintQuery::getSolution(shared_ptr<RunningPlan> rp, vector<double>& result)
	{
		// TODO:
	}

	vector<Variable*> ConstraintQuery::getRelevantStaticVariables()
	{
		// TODO:
	}

	void ConstraintQuery::setRelevantStaticVariables(vector<Variable*> relevantStaticVariables)
	{
		// TODO:
	}

	vector<Variable*> ConstraintQuery::getRelevantDomainVariables()
	{
		// TODO:
	}

	void ConstraintQuery::setRelevantDomainVariables(vector<Variable*> relevantDomainVariables)
	{
		// TODO:
	}

	void ConstraintQuery::addConstraintCalls(vector<shared_ptr<ConstraintCall>> l)
	{
		// TODO:
	}

	ConstraintQuery::UniqueVarStore::UniqueVarStore()
	{
		store = vector<vector<Variable*>>();
	}

	void ConstraintQuery::UniqueVarStore::clear()
	{
		store.clear();
	}

	void ConstraintQuery::UniqueVarStore::add(Variable* v)
	{
		vector<Variable*> l = vector<Variable*>();
		l.push_back(v);
		store.push_back(l);
	}

	Variable* ConstraintQuery::UniqueVarStore::getRep(Variable* v)
	{
		for (vector<Variable*> l : store)
		{
			for (Variable* s : l)
			{
				if (s == v)
				{
					return l.front();
				}
			}
		}
		add(v);
		return v;
	}

	void ConstraintQuery::UniqueVarStore::addVarTo(Variable* representing, Variable* toAdd)
	{
		for (vector<Variable*> l : store)
		{
			for (Variable* cv : l)
			{
				if (representing == cv)
				{
					l.insert(l.begin(), toAdd);
					return;
				}
			}
		}
		vector<Variable*> nl = vector<Variable*>();
		nl.insert(nl.begin(), representing);
		nl.insert(nl.begin(), toAdd);
		store.push_back(nl);
	}

	vector<Variable*> ConstraintQuery::UniqueVarStore::getAllRep()
	{
		vector<Variable*> ret = vector<Variable*>();
		for (vector<Variable*> l : store)
		{
			ret.push_back(l.front());
		}
		return ret;
	}

	int ConstraintQuery::UniqueVarStore::getIndexOf(Variable* v)
	{
		for (int i = 0; i < store.size(); ++i)
		{
			for (Variable* c : store[i])
			{
				if (c == v)
				{
					return i;
				}
			}
		}
		return -1;
	}
} /* namespace alica */

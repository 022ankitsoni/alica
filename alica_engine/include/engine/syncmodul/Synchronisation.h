#pragma once
//#define SM_MISC
//#define SM_SUCCESS
//#define SM_FAILURE
//#define SM_MESSAGES

#include "engine/IRobotID.h"
#include <list>
#include <mutex>
#include <memory>

namespace alica
{
	using std::shared_ptr;
	using std::mutex;
	using std::list;

	class SyncModul;
	class SyncTransition;
	class SyncRow;
	class RunningPlan;
	struct SyncData;
	struct SyncReady;
	struct SyncTalk;
	class AlicaEngine;

	class Synchronisation
	{
	public:
		Synchronisation(AlicaEngine* ae);
		Synchronisation(AlicaEngine* ae, const alica::IRobotID* myID, SyncTransition* st, SyncModul* sm);
		virtual ~Synchronisation();
		void setTick(unsigned long now);
		void changeOwnData (long transitionID, bool conditionHolds);
		bool isValid(unsigned long curTick);
		bool integrateSyncTalk(shared_ptr<SyncTalk> talk, unsigned long curTick);
		void integrateSyncReady(shared_ptr<SyncReady> ready);
		SyncTransition* getSyncTransition();
		void setSyncTransition(SyncTransition* syncTransition);

	private:
		bool allSyncReady();
		void printMatrix();

	protected:
		AlicaEngine* ae;
		mutex syncMutex;
		mutex rowOkMutex;
		SyncModul* syncModul;
		SyncTransition* syncTransition;
		const alica::IRobotID* myID;
		unsigned long lastTalkTime;
		SyncData* lastTalkData;
		unsigned long syncStartTime;
		bool readyForSync;
		unsigned long lastTick;
		list<shared_ptr<SyncReady>> receivedSyncReadys;
		list<long> connectedTransitions;
		RunningPlan* runningPlan;
		list<SyncRow*> rowsOK;
		SyncRow* myRow;
		list<SyncRow*> syncMatrix;
		void sendTalk(SyncData* sd);
		void sendSyncReady();
		bool isSyncComplete();

	};

} /* namespace alica */

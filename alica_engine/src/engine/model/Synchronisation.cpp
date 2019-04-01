#include "engine/model/Synchronisation.h"
#include "engine/model/Plan.h"
#include "engine/model/Transition.h"

namespace alica
{

Synchronisation::Synchronisation()
        : _failOnSyncTimeout(false)
        , _syncTimeout(AlicaTime::milliseconds(3000))
        , _talkTimeout(AlicaTime::milliseconds(30))
        , _plan(nullptr)
{
}

Synchronisation::~Synchronisation() {}

std::string Synchronisation::toString() const
{
    std::stringstream ss;
    ss << "#Synchronisation: " << getName() << " " << getId() << std::endl;
    if (_plan != nullptr) {
        ss << "\t Plan: " << _plan->getId() << " " << _plan->getName() << std::endl;
    }
    ss << std::endl;
    ss << "\t TalkTimeOut: " << _talkTimeout.inMilliseconds() << std::endl;
    ss << "\t SyncTimeOut: " << _syncTimeout.inMilliseconds() << std::endl;
    ss << "\t FailOnSyncTimeOut: " << _failOnSyncTimeout << std::endl;
    ss << "\t InSync: " << _inSync.size() << std::endl;
    for (const Transition* t : _inSync) {
        ss << "\t" << t->getId() << " " << t->getName() << std::endl;
    }
    ss << std::endl;
    ss << "#EndSynchronisation" << std::endl;
    return ss.str();
}

void Synchronisation::setFailOnSyncTimeOut(bool failOnSyncTimeOut)
{
    _failOnSyncTimeout = failOnSyncTimeOut;
}

void Synchronisation::setSyncTimeOut(AlicaTime syncTimeOut)
{
    _syncTimeout = syncTimeOut;
}

void Synchronisation::setTalkTimeOut(AlicaTime talkTimeOut)
{
    _talkTimeout = talkTimeOut;
}

void Synchronisation::setPlan(const Plan* plan)
{
    _plan = plan;
}

void Synchronisation::setInSync(const TransitionGrp& inSync)
{
    _inSync = inSync;
}

} // namespace alica

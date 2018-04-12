/*
 * ResultEntry.cpp
 *
 *  Created on: Nov 24, 2014
 *      Author: Philipp
 */
#include "engine/constraintmodul/ResultEntry.h"

#include <engine/model/Variable.h>
#include <engine/AlicaEngine.h>
#include <engine/containers/SolverVar.h>
#include <engine/containers/SolverResult.h>
#include <utility>

namespace alica {

ResultEntry::ResultEntry(const supplementary::AgentID* robotId) 
    : _id(robotId)
    {}


ResultEntry::ResultEntry(ResultEntry&& o)
    : _id(o._id)
    , _values(std::move(o._values))
    , _valueLock()
    {}

ResultEntry& ResultEntry::operator=(ResultEntry&& o) {
    std::swap(_id, o._id);
    _values = std::move(o._values);
    return *this;
}


void ResultEntry::addValue(int64_t vid, Variant val, AlicaTime time) {
    lock_guard<std::mutex> lock(_valueLock);
    auto it = _values.find(vid);
    if (it != _values.end()) {
        VarValue& vv = it->second;
        vv._val = val;
        vv._lastUpdate = time;
    } else {
        _values.emplace(vid, VarValue(val, time));
    }
}

void ResultEntry::clear() {
    lock_guard<std::mutex> lock(_valueLock);
    _values.clear();
}

void ResultEntry::getCommunicatableResults(AlicaTime earliest, std::vector<SolverVar>& o_result) const {
    lock_guard<std::mutex> lock(_valueLock);
    for (const std::pair<int64_t, VarValue>& p : _values) {
        if (p.second._lastUpdate > earliest) {
            SolverVar sv;
            sv.id = p.first;
            p.second._val.serializeTo(sv.value);
            o_result.push_back(std::move(sv));
        }
    }
}

Variant ResultEntry::getValue(int64_t vid, AlicaTime earliest) const {
    lock_guard<std::mutex> lock(_valueLock);
    auto it = _values.find(vid);
    if (it != _values.end()) {
        if (it->second._lastUpdate > earliest) {
            return it->second._val;
        }
    }
    return Variant();
}

bool ResultEntry::getValues(const VariableSet& query, AlicaTime earliest, std::vector<Variant>& o_values) const {
    o_values.resize(query.size());
    int i = 0;
    int nans = 0;
    for (const Variable* v : query) {
        o_values[i] = getValue(v->getId(), earliest);
        if (!o_values[i].isSet()) {
            ++nans;
        }
        ++i;
    }
    return nans != i;
}

} /* namespace alica */

#include "Timer.hpp"

#include <inc/natives.h>
#include <chrono>

CTimer::CTimer(int64_t timeout) :
    mPeriod(timeout),
    mPreviousTime(now()) {
}

void CTimer::Reset() {
    mPreviousTime = now();
}

void CTimer::Reset(int64_t newTimeout) {
    mPeriod = newTimeout;
    mPreviousTime = now();
}

bool CTimer::Expired() const {
    return now() > mPreviousTime + mPeriod;
}

int64_t CTimer::Elapsed() const {
    return now() - mPreviousTime;
}

int64_t CTimer::Period() const {
    return mPeriod;
}

CSysTimer::CSysTimer(int64_t timeout) :
    CTimer(timeout) {
}

int64_t CSysTimer::now() const {
    using namespace std::chrono;
    auto tEpoch = steady_clock::now().time_since_epoch();
    return duration_cast<milliseconds>(tEpoch).count();
}

int64_t CGameTimer::now() const {
    return MISC::GET_GAME_TIMER();
}

CGameTimer::CGameTimer(int64_t timeout) :
    CTimer(timeout) {
}

#pragma once
#include <cstdint>

class CTimer {
public:
    explicit CTimer(int64_t timeout);
    virtual ~CTimer() = default;
    virtual void Reset();
    virtual void Reset(int64_t newTimeout);
    virtual bool Expired() const;
    virtual int64_t Elapsed() const;
    virtual int64_t Period() const;
protected:
    virtual int64_t now() const { return 0; };
    int64_t mPeriod;
    int64_t mPreviousTime;
};

class CSysTimer : public CTimer {
public:
    explicit CSysTimer(int64_t timeout);
protected:
    int64_t now() const override;
};

class CGameTimer : public CTimer {
public:
    explicit CGameTimer(int64_t timeout);
protected:
    int64_t now() const override;
};

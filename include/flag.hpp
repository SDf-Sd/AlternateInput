#pragma once

// taken from Dolphin Core/Common

#include <atomic>

class Flag final
{
public:
    explicit Flag(bool initial_value = false) : m_val(initial_value) {}
    void Set(bool val = true) {m_val.store(val); }
    void Clear() {Set(false); }
    bool IsSet() const {return m_val.load(); }
    bool TestAndSet(bool val = true)
    {
        bool expected = !val;
        return m_val.compare_exchange_strong(expected, val);
    }
    bool TestAndClear() { return TestAndSet(false); }
private:
    std::atomic_bool m_val;
};
#pragma once

class CtorDtorCounter {
public:
    inline static int ctored = 0;
    inline static int dtored = 0;

    CtorDtorCounter() {
        ctored++;
    }

    CtorDtorCounter(const CtorDtorCounter& _) {
        ctored++;
    }

    CtorDtorCounter& operator=(const CtorDtorCounter& _) {
        return *this;
    }

    ~CtorDtorCounter() {
        dtored++;
    }

    static void reset() {
        ctored = 0;
        dtored = 0;
    }
};

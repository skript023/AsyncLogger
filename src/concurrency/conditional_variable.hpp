#pragma once
#include "mutex.hpp"

#if defined(_WIN32)
#include <windows.h>
#else
#include <pthread.h>
#include <errno.h>
#endif

namespace al
{
	class ConditionVariable {
public:
    ConditionVariable();
    ~ConditionVariable();

    // Tunggu hingga predicate benar (atau dibangunkan)
    template <typename Predicate>
    void wait(Mutex& mutex, Predicate pred) {
        while (!pred())
            wait(mutex);
    }

    // Tunggu tanpa predicate
    void wait(Mutex& mutex);

    void notify_one();
    void notify_all();

private:
#if defined(_WIN32)
    CONDITION_VARIABLE m_cond;
#else
    pthread_cond_t m_cond;
#endif
};

}
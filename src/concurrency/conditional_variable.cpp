#include "conditional_variable.hpp"

#if defined(_WIN32)

ConditionVariable::ConditionVariable() {
    ::InitializeConditionVariable(&m_cond);
}

ConditionVariable::~ConditionVariable() {
    // Tidak perlu delete, CONDITION_VARIABLE tidak dialokasikan
}

void ConditionVariable::wait(Mutex& mutex) {
    ::SleepConditionVariableCS(&m_cond, mutex.native_handle(), INFINITE);
}

void ConditionVariable::notify_one() {
    ::WakeConditionVariable(&m_cond);
}

void ConditionVariable::notify_all() {
    ::WakeAllConditionVariable(&m_cond);
}

#else // POSIX

ConditionVariable::ConditionVariable() {
    pthread_cond_init(&m_cond, nullptr);
}

ConditionVariable::~ConditionVariable() {
    pthread_cond_destroy(&m_cond);
}

void ConditionVariable::wait(Mutex& mutex) {
    pthread_cond_wait(&m_cond, mutex.native_handle());
}

void ConditionVariable::notify_one() {
    pthread_cond_signal(&m_cond);
}

void ConditionVariable::notify_all() {
    pthread_cond_broadcast(&m_cond);
}

#endif

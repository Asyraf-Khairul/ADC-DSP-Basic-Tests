#pragma once
#include <cassert>
struct semaphore_t { int count=0, maximum=0; };
inline void sem_init(semaphore_t *s, int n, int max) { *s={n,max}; }
inline bool sem_try_acquire(semaphore_t *s) { if (!s->count) return false; --s->count; return true; }
inline void sem_acquire_blocking(semaphore_t *s) { const bool ok=sem_try_acquire(s); assert(ok); }
inline void sem_release(semaphore_t *s) { assert(s->count<s->maximum); ++s->count; }

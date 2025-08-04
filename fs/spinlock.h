#ifndef _UNOS_SPINLOCK_H_
#define _UNOS_SPINLOCK_H_

#include <unostype.h> // Pastikan ada tipe BOOL, USINT32, dll

typedef struct {
    volatile USINT32 locked;
} SPINLOCK;

STATIC INLINE VOID spinlock_init(IN SPINLOCK *lock) {
    lock->locked = 0;
}

STATIC INLINE VOID acquire_lock(IN SPINLOCK *lock) {
    while (__sync_lock_test_and_set(&lock->locked, 1)) {
        // Tunggu sampai lock dilepas
        while (lock->locked) {
            __asm__ volatile("pause"); // hint untuk CPU
        }
    }
}

STATIC INLINE VOID release_lock(IN SPINLOCK *lock) {
    __sync_lock_release(&lock->locked);
}

GLOBAL SPINLOCK fat_lock;

#endif

/* futex_event.h
 *
 * Simple wrappers for signaling, broadcasting, and waiting with the
 * futex() system call.
 */

#include <unistd.h>
#include <stdint.h>
#include <linux/futex.h>
#include <sys/syscall.h>	

inline int futex(int *uaddr, int op, int val, const struct timespec *timeout,
		 int *uaddr2, int val3) {
    return syscall(SYS_futex, uaddr, op, val, timeout, uaddr2, val3);
}

typedef int futex_t;

inline void futex_signal(futex_t key) {
    futex(&key, FUTEX_WAKE, 1, NULL, NULL, 0);
}

inline void futex_broadcast(futex_t key) {
    futex(&key, FUTEX_WAKE, INT_MAX, NULL, NULL, 0);
}

inline void futex_wait(futex_t key) {
    futex(&key, FUTEX_WAIT, key, NULL, NULL, 0);
}

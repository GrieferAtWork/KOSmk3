/* Copyright (c) 2018 Griefer@Work                                            *
 *                                                                            *
 * This software is provided 'as-is', without any express or implied          *
 * warranty. In no event will the authors be held liable for any damages      *
 * arising from the use of this software.                                     *
 *                                                                            *
 * Permission is granted to anyone to use this software for any purpose,      *
 * including commercial applications, and to alter it and redistribute it     *
 * freely, subject to the following restrictions:                             *
 *                                                                            *
 * 1. The origin of this software must not be misrepresented; you must not    *
 *    claim that you wrote the original software. If you use this software    *
 *    in a product, an acknowledgement in the product documentation would be  *
 *    appreciated but is not required.                                        *
 * 2. Altered source versions must be plainly marked as such, and must not be *
 *    misrepresented as being the original software.                          *
 * 3. This notice may not be removed or altered from any source distribution. *
 */
#ifndef _KOS_THREAD_H
#define _KOS_THREAD_H 1

#include <__stdinc.h>
#include <hybrid/host.h>
#include <bits/types.h>

#if defined(__i386__) || defined(__x86_64__)
#include "i386-kos/thread.h"
#else
#error "Unsupported arch"
#endif

__DECL_BEGIN

/* Threading in KOS:
 *
 *    User-space KOS threading is designed to be usable, even without the need of
 *    a pthread library. As a matter of fact: pthread is actually holding back KOS
 *    threading, as it makes it impossible to properly access all of KOS's thread
 *    functionality. However to make things simple, functions are compared to their
 *    pthread equivalents in the following paragraph.
 *    Threading libraries designed for KOS should therefor not use pthread, but
 *    interface with functions described below.
 *    NOTE: All functions documented below can also be obtained using:
 *          >> #define _KOS_SOURCE 1
 *          >> #include <kos/thread.h>
 *
 *    - pthread_create()
 *        The underlying system call is `clone()':
 *        >> #define _GNU_SOURCE 1
 *        >> #include <sched.h>
 *        clone() offers a multitude of functionality that cannot be replicated
 *        using a raw call to `pthread_create()', such as seperate VMs, file
 *        descriptors, filesystem contexts, etc.
 *        To get started and create a thread with behavior you'd expect from
 *        a call to `pthread_create()', you may use `CLONE_NEW_THREAD', a
 *        collection of flags that create a new thread with naturally expected
 *        semantics (a.g. same VM, FDs, FS, etc.)
 *
 *    - pthread_join()
 *        The equivalent system call is `wait(WEXITED)', but waiting can also be
 *        implemented using xppoll(), which also allows one to wait for
 *        practically any other condition to arise simultaniously.
 *        >> #include <sys/wait.h>  // wait() and friends
 *        >> #include <kos/futex.h> // xppoll()
 *        Another way of joining a thread is to make use of `SYS_set_tid_address'
 *        and use `futex_wait()' to wait for the thread to terminate.
 *        This is the method used by libpthread, just as its linux implementation
 *        does using linux's futex() system call. However that method can only be
 *        used to wait for threads within the same VM, making it less versatile.
 *        Note that using `wait()' is much more versatile. E.g.: by specifying the
 *        `WNOREAP' flag, you can prevent the thread from getting reaped, an action
 *        which semantically mirrors pthread_join()'s behavior of closing the thread's
 *        ID, allowing that ID to be re-used at another time.
 *        Think to pthread_join() doing a close() call to destroy the thread's
 *        handle (an action also done by pthread_detach()), however using wait(),
 *        you can prevent it from doing this, if you so choose.
 *
 *    - pthread_tryjoin_np()
 *        Can either be implemented using wait() with `WNOHANG|WEXITED',
 *        or alternatively by using xppoll(). However the later which will never
 *        reap child processes, operating as though `WNOREAP' was always set,
 *        meaning if you wish to close the child process's handle (accessed
 *        through its PID), you must call `detach(2)' to close it.
 *        Alternatively, if `SYS_set_tid_address' is used, you can simply implement
 *        a small piece of code that uses `futex_wait()' to join the thread.
 *        >> #include <sys/wait.h>  // wait() and friends, detach()
 *        >> #include <kos/futex.h> // xppoll()
 *
 *    - pthread_timedjoin_np()
 *        Can be implemented using `xppoll()', as POSIX's versions of the
 *        wait() family of functions don't include one taking a timeout.
 *        Note however that when doing this, you will also have to call
 *       `detach(2)' afterwards in order to reap the thread.
 *        >> #include <sys/wait.h>  // wait() and friends, detach()
 *        >> #include <kos/futex.h> // xppoll()
 *
 *    - pthread_detach()
 *        Implemented using a dedicated system call `detach(2)', or
 *        alternatively, a thread can be constructed in a detached
 *        state by passing the `CLONE_DETACHED' flag to `clone(2)'
 *        >> #include <sys/wait.h>  // detach()
 *
 *    - pthread_self()
 *        Since the native system only operates using TIDs, talking
 *        to your own thread can be done by taking your own tid, as
 *        can be retrieved by a call to `gettid(2)'
 *        >> #include <kos/futex.h> // gettid()
 *
 *    - pthread_exit()
 *        Can be implemented using exit_thread(). Note however that
 *        instead of being able to exit with a wide variety of return
 *        values, exit_thread() can only return as many values as
 *        can already be returned by your main() method.
 *        However it should be fairly easy to have the thread save
 *        its return value in a structure passed to it as argument
 *        during the initial call to `clone()'
 *
 *    - pthread_equal()
 *        They're TIDs, just compare them!
 *        Unless of course you're using `CLONE_NEWPID', in which case
 *        you have to be careful which ID belongs to which namespace.
 *        >> #include THREAD_EQUAL(a,b) ((a) == (b))
 *
 *    - pthread_cancel()
 *        This is where it gets a bit complicated, as KOS implements
 *        a much more powerful mechanism than ~just~ being able to
 *        cancel a thread. Because in order to remotely exit a thread,
 *        all you have to do is impersonate it and do something that
 *        will cause that thread to exit, using an RPC:
 *        >> #include <kos/rpc.h>
 *        >> unsigned int terminate_me(void *arg) {
 *        >>     error_throwf(E_EXIT_THREAD,0);
 *        >> }
 *        >> void terminate_pid(pid_t pid) {
 *        >>     queue_rpc(pid,&terminate_me,NULL,RPC_FSYNCHRONOUS|RPC_FWAITFOR);
 *        >> }
 *        This way you can ensure that the thread isn't in some undefined state,
 *        but is currently waiting for some blocking lock, or alternatively
 *        you can also terminate a thread asynchronously:
 *          - PTHREAD_CANCEL_DEFERRED     -- RPC_FSYNCHRONOUS
 *          - PTHREAD_CANCEL_ASYNCHRONOUS -- RPC_FASYNCHRONOUS
 *        The RPC mechanism is even more useful for other applications which
 *        cannot even be expressed using pthread functionality, such as accessing
 *        thread-local variables of another thread, or detaching another thread
 *        by performing a call to `unshare()', or as a more complex usage case:
 *        getting a thread to release a recursive lock, as is done by the user-space
 *        implementation of recursive R/W-locks in order to implement aggressive
 *        write-locks.
 *
 *    - pthread_testcancel()
 *        Since cancellation can easily be implemented using RPCs, testing
 *        for cancellation is as easy as testing for RPCs, which can be
 *        done using the `rpc_serve()' function
 *        >> #include <kos/rpc.h> // rpc_serve()
 *
 *    - sem_t
 *        KOS implements an very small (4 bytes), exception-enabled
 *        semaphore `semaphore_t' in <kos/sched/semaphore.h>, that
 *        is still fully featured with every function you might want.
 *
 *    - pthread_mutex_t
 *        KOS implements an very small (8 bytes), exception-enabled
 *        mutex `mutex_t' in <kos/sched/mutex.h>, that is still
 *        fully featured with every function you might want.
 *        NOTE: KOS's mutex always allowed for recursion.
 *
 *    - pthread_rwlock_t
 *        KOS implements an very small (8 bytes), exception-enabled
 *        R/W-lock `rwlock_t' in <kos/sched/rwlock.h>, that is still
 *        fully featured with every function you might want.
 *        NOTE: KOS's mutex always allowed for recursion.
 *        Additionally, KOS's version supports recursive write-after-read,
 *        and provides a solution to the multiple-readers-then-upgrade
 *        problem, ontop of which it allows for aggressive lock acquisition
 *        which allows a writer to kill of readers by queuing RPCs.
 *
 *    - pthread_key_t
 *        KOS implements ELF thread-local variables, meaning you can
 *        make use of `ATTR_THREAD' to define thread-local variables.
 *
 *    - pthread_spinlock_t
 *        Use `atomic_rwlock_t' from <hybrid/sched/atomic-rwlock.h>.
 *        Not only can it easily be used as a regular mutex by only
 *        ever acquiring write-locks, but it doesn't even require any
 *        library backing it, and is entirely implemented in the header.
 *       (Meaning using it will generate the fastest possible code,
 *        thanks to inline optimizations)
 *
 * Functions not even possible using pthreads:
 *    - Enumerate the PIDs of all running threads:
 *      >> #include <dirent.h>
 *      >> d = opendir("/proc/self/task")
 *      >> while ((ent = readdir(d)) != NULL) {
 *      >>     pid_t p;
 *      >>     if (sscanf(ent->d_name,"%u",&p) != 1) continue;
 *      >>     ...
 *      >> }
 *
 *    - Accessing thread-local variables of another thread:
 *      >> #include <kos/thread.h>    // queue_rpc()
 *      >> unsigned int read_thread_local_variable(int *arg) {
 *      >>     *arg = GET_TLS_VARIABLE();
 *      >>     return RPC_RETURN_FORCE_RESTART;
 *      >> }
 *      >> ...
 *      >> int value;
 *      >> queue_rpc(&read_thread_local_variable,&value)
 *
 *    - Generating a traceback of another thread while it is running:
 *      >> #include <kos/thread.h>    // queue_rpc()
 *      >> #include <kos/context.h>   // cpu_getcontext()
 *      >> #include <kos/addr2line.h> // Xxunwind()
 *      >> #include <stdio.h>         // fprintf(), stderr
 *      >> unsigned int print_traceback(int *arg) {
 *      >>     struct cpu_context ctx;
 *      >>     cpu_getcontext(&ctx);
 *      >>     do {
 *      >>         fprintf(stderr,"%[vinfo:%f(%l,%c) : %n : %p : Execution is here]\n");
 *      >>     } while (Xxunwind(&ctx,NULL,NULL));
 *      >>     return RPC_RETURN_FORCE_RESTART;
 *      >> }
 *      >> ...
 *      >> queue_rpc(&print_traceback);
 *
 */



#if 0 /* XXX: Isn't this what /proc is for? */
/* NOTE: Thread scope enumerate is still restricted to PID namespaces:
 *       You can't enumerate threads apart of a PID
 *       namespace that is invisible to you own thread. */
#define THREAD_SCOPE_CURRENT        0x01 /* Only enumerate the named thread (but still validate that it exists). */
#define THREAD_SCOPE_PROCESS        0x02 /* Enumerate all threads in the same process as the one specified. */
#define THREAD_SCOPE_CHILDREN       0x03 /* Enumerate all threads in, and parented by the one specified. */
#define THREAD_SCOPE_GRANDCHILDREN  0x04 /* Enumerate all threads in, and parented by the one specified, and any parented by those. */
#define THREAD_SCOPE_PROCESS_GROUP  0x05 /* Enumerate all threads apart of processes within the same process group as the thread specified. */
#define THREAD_SCOPE_SESSION        0x06 /* Enumerate all threads apart of processes within the same session as the thread specified. */
#define THREAD_SCOPE_GLOBAL         0x07 /* Enumerate all threads on the entire system (The given PID is ignored). */

#ifndef __KERNEL__
/* Enumerate threads.
 * @param: SCOPE:   The scope to enumerate (One of `THREAD_SCOPE_*')
 * @param: PID:     The PID of a thread apart of some grouping related to `SCOPE'.
 *                  You may pass ZERO(0) to use your own TID here.
 * @param: BUF:     A user-provided buffer to fill with PIDs.
 * @param: BUFSIZE: The size of the provided buffer (in bytes)
 * @return: * :     The required buffer size (in bytes). */
__REDIRECT_EXCEPT(__LIBC,__WUNUSED,__ssize_t,__LIBCCALL,enumerate_threads,(unsigned int __scope, __pid_t __pid, __pid_t *__buf, __size_t __bufsize),(__scope,__pid,__buf,__bufsize))
#ifdef __USE_EXCEPT
__LIBC __WUNUSED __size_t (__LIBCCALL Xenumerate_threads)(unsigned int __scope, __pid_t __pid, __pid_t *__buf, __size_t __bufsize);
#endif /* __USE_EXCEPT */
#endif
#endif

__DECL_END

/* Pull in everything that might be useful for threading. */
#ifndef __KERNEL__
#include <kos/futex.h>    /* futex_wait(), xppoll() */
#include <kos/rpc.h>      /* queue_rpc(), rpc_serve() */
#include <sched.h>        /* clone(), exit_thread() */
#include <sys/wait.h>     /* wait() and friends, detach() */
#endif


#endif /* !_KOS_THREAD_H */

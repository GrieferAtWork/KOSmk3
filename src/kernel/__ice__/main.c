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
#ifndef GUARD_KERNEL_MAIN_C
#define GUARD_KERNEL_MAIN_C 1

#include <hybrid/compiler.h>
#include <hybrid/asm.h>
#include <hybrid/types.h>
#include <sys/io.h>

#include <sched/task.h>
#include <sched/rwlock.h>
#include <kernel/malloc.h>
#include <kernel/sections.h>
#include <kernel/heap.h>
#include <kernel/debug.h>
#include <unwind/context.h>
#include <unwind/eh_frame.h>
#include <kernel/memory.h>
#include <kernel/interrupt.h>
#include <kernel/user.h>
#include <kernel/syscall.h>
#include <sched/signal.h>
#include <fs/node.h>
#include <fs/path.h>
#include <fs/device.h>
#include <hybrid/kdev_t.h>

#include <fcntl.h>
#include <string.h>
#include <except.h>
#include <kos/context.h>
#include <asm/cfi.h>


DECL_BEGIN


void throw_error(void) {
 volatile int x = 0;
 volatile int y = 0;

 TRY x /= y;
 CATCH (E_DIVIDE_BY_ZERO) {
  debug_printf("Cannot divide by zero\n");
  error_continue(0);
 }
 debug_printf("HERE\n");
 error_throw(43);
 error_rethrow();
}



void handler(void) {
 void *ebp,*esp;
 __asm__("movl %%esp, %0" : "=g" (esp));
 __asm__("movl %%ebp, %0" : "=g" (ebp));
 debug_printf("Before throw %p, %p\n",esp,ebp);
#if 0
 throw_error();
#else
 TRY {
  //error_rethrow();
  //__asm__("int3");
  throw_error();
  debug_printf("After rethrow\n");
 } FINALLY {
  debug_printf("In finally - old %p, %p\n",esp,ebp);
  __asm__("movl %%esp, %0" : "=g" (esp));
  __asm__("movl %%ebp, %0" : "=g" (ebp));
  debug_printf("In finally - new %p, %p\n",esp,ebp);
  debug_printf("exception_code = %d\n",error_code());
 }
 debug_printf("After handler\n");
#endif
}

struct assert_descriptor {
    void        *ad_retry;
    char const  *ad_file;
    intptr_t     ad_line;
    char const   ad_expr[1];
};

#if 0 /* Assertions through interrupts? How about it? */
#define ASSERT(expr) \
 XBLOCK({ __assert_retry: \
          if unlikely(!(expr)) { \
              PRIVATE struct { \
                  void *_r; char const *_f; __INTPTR_TYPE__ _l; \
                  char _e[COMPILER_LENOF(#expr)]; \
              } const __assert_desc = { &&__assert_retry, __FILE__, __LINE__, #expr }; \
              __asm__("int $0x7f" : : "a" (&__assert_desc) : "memory"); \
          } \
          (void)0; })
#endif


INTDEF struct except_handler __except_start[];
INTDEF struct except_handler __except_end[];


PRIVATE void KCALL do_overflow(int x) {
 debug_printf("do_overflow(%d,%p)\n",x,&x);
 do_overflow(x+1);
}

INTERN void KCALL my_thread_main(void *arg) {
 debug_printf("IN THREAD::: my_thread_main(%p)\n",arg);
 __asm__("pause");
 debug_printf("IN THREAD::: my_thread_main(%p)\n",arg);
 task_yield();
 debug_printf("IN THREAD::: my_thread_main(%p)\n",arg);
 debug_printf("IN THREAD::: THIS_TASK = %p\n",THIS_TASK);
 debug_printf("THIS_CPU->cpu_id = %u\n",THIS_CPU->cpu_id);
 TRY {
  do_overflow(0);
 } FINALLY {
  debug_printf("my_thread_main -- FINALLY\n");
 }
}

void kernel_main(void) {
 PREEMPTION_ENABLE();
 debug_printf("NUM_EXCEPT = %Iu\n",
              __except_end-__except_start);

#if 0
 REF struct directory_node *dir;
 
 /* Lookup some folder. */
 {
  REF struct path *p;
  char const *pstr = "/bin";
  p = fs_path(NULL,pstr,strlen(pstr),
              O_RDONLY|O_DIRECTORY,
             (REF struct inode **)&dir);
  path_decref(p);
 }

 TRY {
read_again:
  rwlock_read(&dir->d_node.i_lock);
  TRY {
   struct directory_entry *iter;
   while ((iter = directory_readnext(dir)) != NULL) {
    debug_printf("ENTRY: %q (%u)\n",
                 iter->de_name,
                 iter->de_type);
   }
  } FINALLY {
   if (rwlock_endread(&dir->d_node.i_lock))
       goto read_again;
  }
 } FINALLY {
  inode_decref(&dir->d_node);
 }
 task_yield();
 debug_printf("STOP\n");
 debug_printf("superblock_shutdownall() -> %Iu\n",
               superblock_shutdownall());


#elif 0
 struct task *t;
 t = task_alloc();                                 debug_printf("Thread allocated\n");
 task_setup_kernel(t,&my_thread_main,(void *)42);  debug_printf("Thread setup\n");
 t->t_cpu = cpu_vector[1];
 task_start(t);                                    debug_printf("Thread started\n");
 task_connect_join(t);                             debug_printf("Thread connected\n");
 task_wait();                                      debug_printf("Thread joined\n");
 task_decref(t);
 heap_validate_all();
 task_serve();
 heap_validate_all();

#elif 0
#define HERE  debug_printf("%s(%d)\n",__FILE__,__LINE__)
 rwlock_t temp = RWLOCK_INIT;

 assert(!rwlock_reading(&temp));
 assert(!rwlock_writing(&temp));
 HERE,rwlock_read(&temp),assert(rwlock_reading(&temp));
 HERE,rwlock_read(&temp),assert(rwlock_reading(&temp));
 HERE,rwlock_endread(&temp),assert(rwlock_reading(&temp));
 HERE,rwlock_endread(&temp),assert(!rwlock_reading(&temp));
 assert(!rwlock_writing(&temp));
 HERE,rwlock_write(&temp),assert(rwlock_writing(&temp));
 assert(rwlock_reading(&temp));
 HERE,rwlock_write(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_endwrite(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_endwrite(&temp),assert(!rwlock_writing(&temp));

 HERE,rwlock_write(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_write(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_read(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_endread(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_endwrite(&temp),assert(rwlock_writing(&temp));
 HERE,rwlock_endwrite(&temp),assert(!rwlock_writing(&temp));

 HERE,rwlock_read(&temp);
 assert(rwlock_reading(&temp));
 assert(!rwlock_writing(&temp));
 HERE,rwlock_write(&temp);
 assert(rwlock_reading(&temp));
 assert(rwlock_writing(&temp));
 HERE,rwlock_endwrite(&temp);
 assert(rwlock_reading(&temp));
 assert(!rwlock_writing(&temp));
 HERE,rwlock_endread(&temp);
 assert(!rwlock_reading(&temp));
 assert(!rwlock_writing(&temp));


 debug_printf("DONE\n");
#elif 0
 struct sig s = SIG_INIT;
 assert(PREEMPTION_ENABLED());

 debug_printf("Connect to %p\n",&s);
 sig_get(&s);
 task_connect(&s);
 sig_put(&s);
 debug_printf("broadcast = %Iu\n",sig_broadcast(&s));
 debug_printf("task_disconnect() -> %p\n",task_disconnect());
 debug_printf("task_wait() -> %p\n",task_wait());


#elif 0
 u8 *p;
 debug_printf("task_temppage() = %p\n",
               VM_PAGE2ADDR(task_temppage()));
 TRY {
  p = (u8 *)kmalloc(42,GFP_SHARED);
  debug_printf("p = %p\n",p);
  //debug_printf("%p\n",kmalloc((size_t)-8024,GFP_SHARED));
  debug_printf("%p\n",kmalloc(42,GFP_SHARED));
  debug_printf("%p\n",kmalloc(42,GFP_SHARED));
  debug_printf("%p\n",kmalloc(42,GFP_SHARED));
  debug_printf("%p\n",kmalloc(42,GFP_SHARED));
  format_hexdump(&debug_printer,NULL,p,42,16,
                 FORMAT_HEXDUMP_FLAG_ADDRESS);
  debug_printf("\n");
  kfree(p);
  format_hexdump(&debug_printer,NULL,p,42,16,
                 FORMAT_HEXDUMP_FLAG_ADDRESS);
  debug_printf("\n");
  heap_validate_all();
/*
  p += 6;
  debug_printf("Before corrupt %p\n",p);
  *p = 42;
  debug_printf("After corrupt\n");
*/
  heap_validate_all();
 } FINALLY {
  debug_printf("In finally\n");
 }

#elif 0
 struct meminfo const *info;
 unsigned int i;

 MEMINFO_FOREACH(info) {
  //if (info->mi_type == MEMTYPE_NDEF)
  //    continue;

  debug_printf("MEMORY %.16I64X...%.16I64X (%s)\n",
               MEMINFO_MIN(info),MEMINFO_MAX(info),
               memtype_names[info->mi_type]);
 }

 for (i = 0; i < mzone_count; ++i) {
  debug_printf("ZONE   %.16I64X...%.16I64X (%Iu/%Iu free pages)\n",
             ((u64)mzones[i]->mz_min)*PAGESIZE,
            (((u64)mzones[i]->mz_max+1)*PAGESIZE)-1,
               mzones[i]->mz_used,mzones[i]->mz_free);
 }

#if 0
 {
  pageptr_t pageptr,pageptr2;
  pageptr  = page_malloc(42,MZONE_ANY);
  pageptr2 = page_malloc(128000000,MZONE_ANY);
  debug_printf("pageptr = %p\n",pageptr*PAGESIZE);
  debug_printf("pageptr2 = %p\n",pageptr2*PAGESIZE);
  i = mzone_count-1;
  debug_printf("ZONE   %.16I64X...%.16I64X (%Iu/%Iu free pages)\n",
             ((u64)mzones[i]->mz_min)*PAGESIZE,
            (((u64)mzones[i]->mz_max+1)*PAGESIZE)-1,
               mzones[i]->mz_used,mzones[i]->mz_free);
  page_free(pageptr,42);
  debug_printf("ZONE   %.16I64X...%.16I64X (%Iu/%Iu free pages)\n",
             ((u64)mzones[i]->mz_min)*PAGESIZE,
            (((u64)mzones[i]->mz_max+1)*PAGESIZE)-1,
               mzones[i]->mz_used,mzones[i]->mz_free);
  page_free(pageptr2,128);
  debug_printf("ZONE   %.16I64X...%.16I64X (%Iu/%Iu free pages)\n",
             ((u64)mzones[i]->mz_min)*PAGESIZE,
            (((u64)mzones[i]->mz_max+1)*PAGESIZE)-1,
               mzones[i]->mz_used,mzones[i]->mz_free);
 }
#endif

#else
 debug_printf("call handler\n");

 TRY {
  TRY {
   TRY {
#if 0
    handler();
#else
    volatile int x = 0;
    if unlikely(!x) {
     handler();
     handler();
     handler();
     handler();
     handler();
     handler();
    } else {
     debug_printf("Foobar\n");
    }
#endif
   } FINALLY {
    debug_printf("Inner finally #%u\n",123);
   }
  } CATCH(43) {
   debug_printf("CATCH(43)\n");
  }
 } CATCH(77) {
  debug_printf("CATCH(77)\n");
 }
 debug_printf("Exception handled\n");
#endif
 for (;;) __asm__("hlt");
}


DECL_END

#endif /* !GUARD_KERNEL_MAIN_C */

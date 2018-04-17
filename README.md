
### <b>KOS (Hooby) Operating System & Kernel</b> ###

The third rendition of the KOS Operating System & Kernel, this time with a lot more emphasis on integration, as well as streamlining. Additionally, many optional features have been implemented this time around, including actually working true <b>SMP</b> and <b>IPI</b>, support for the x86 <code>sysenter</code> instruction, or making use of <b>APIC</b> interrupt timers for preemption (all things that KOSmk2 couldn't do)

Much emphasis has also been put on implementing posix <code>signal()</code> behavior <i>correctly</i> without any shortcuts with a lot of work put into speeding up system calls such as <code>fork()</code>, making use of lazy copy-on-write, rather than duplicating data immediatly. Similarly, the kernel now implements much better support for <code>termios</code> flags, supporting numerous options that had no support at all before.

The most important difference between this and KOSmk2 is probably the idea that initially convinced me to start over from scratch, that idea being: <b>Exceptions</b>. But not just any exceptions. Oh no. I'm not using some C++ compiler now. It's all still 100% pure C code making use of inline assembly, as well as DWARF's CFI meta-instruction set, alongside a lot of preprocessor <i>magic</i>.

Also, a large number of the core concepts already found in KOSmk2 have been re-invisioned using new logic facilities that completely remove functions such as <code>task_crit()</code>, now instead relying on <b>RPC</b> functionality that could not be seen anywhere in KOSmk2, allowing for much easier to use scheduling semantics, as well as new mechanisms for defining per-cpu, per-task or per-vm variables, rather than having to cram everything into one humonguous and overcomplicated control structure.

KOSmk3 is a much more stable platform than its predecessors, relying less than ever on code that hasn't been written by me personally, including my very own heap implementation with support for GC-based memory leak detection that scans registers, global data, as well as stack memory for pointers in order to determine what memory is actually reachable, similar to what the linux kernel does (presumably), leading to my bold claim that the KOS kernel should currently be <b>leakless</b>.

Like everything, this is another <b>one-person project</b>, with development (of this rendition) having started on <b>10.02.2017</b>.

Chaos|KOS - You probably got here through the definition of chaoticity (<i>which totally is a real word</i>).

## State ##
 - Able to run busybox (using the same binary that was originally compiled for KOSmk2)

## Features ##
 - i486+ (Yes. Your machine can't be older than time itself...)
   - Protected-mode ring #3
 - Exceptions
   - try-catch-finally
   - Stack unwinding into user-space (Exceptions thrown by the kernel can be caught by user-applications)
   - Highly robust kernel, even in the face of faulty kernel code (of which there probably is a lot)
   - Zero-effort exceptions using guard-descriptors invoked by unwinding the stack with the help of CFI instructions. In other words, I'm not going the windows route of the uglyness that is SEH and its stack-based linked list.
   - Implemented using pure, unmodified gcc (these aren't c++ exceptions, which is why I do actually have a finally statement)
 - QEMU
 - multiboot/multiboot2
 - Paging
   - Page directory self-mapping
   - <code>%cr3</code> is only changed during preemption
   - dynamic memory
   - ALLOA (ALLocateOnAccess)
   - LOA (LoadOnAccess)
   - COW (CopyOnWrite)
   - SWAP (SWAPping memory to disk)
   - memory-mapped files for reading/writing
 - Heap
   - GC-based memory leak detection
   - <code>heap_alloc()</code>
   - <code>kmalloc()</code>
   - <code>malloc()</code>
   - New malloc functions such as <code>memalign_offset()</code>
 - syscall
   - Highly linux-compatible using <code>int $0x80</code> and same ids/registers
   - Support for the x86 <code>sysenter</code> instruction
   - Support for system calls using page faults
     - It's quite useful and allows signal handlers to <i>return</i> by executing the <code>sigreturn</code> system calls without the need of any trampoline code
 - Unix-compliant user-space interfaces/APIs
   - ANSI-compliant Terminal (needs a rewrite though. - I just copied the old one over)
   - <code>fork()</code>/<code>exec()</code>/<code>wait()</code>/<code>pipe()</code>
   - <code>mmap()</code>/<code>munmap()</code>/<code>mremap()</code>/<code>brk()</code>/<code>sbrk()</code>
     - Full support for file to memory mappings
   - <code>signal()</code>/<code>raise()</code>/<code>kill()</code>/<code>sigprocmask()</code>
     - Terminate/suspend/resume support for <code>SIGKILL</code>, <code>SIGSTOP</code>, <code>SIGCONT</code>
     - Signal-based exception handling (<code>SIGSEGV</code>, etc.)
     - Greatly improved conformance to POSIX behavior.
   - <code>open()</code>/<code>read()</code>/<code>write()</code>/<code>lseek()</code>
   - <code>fcntl()</code>/<code>ioctl()</code>/<code>openpty()</code>
   - <code>mount()</code>/<code>umount()</code>
   - <code>main()</code>/<code>argc</code>/<code>argv</code>/<code>environ</code>
   - <code>dlopen()</code>/<code>dlclose()</code>/<code>dlsym()</code>
     - Still needs an emulation library, but all the system calls are already there
   - <code>clone()</code>/<code>futex()</code>/<code>exit_group()</code>
     - Linux-compatible multi-threading support
   - Can run <b>busybox</b>
 - Custom-made Libc
   - Binary (ABI) compatibility with both MSVCrt and GLibC
     - For the most part. Compatibility will further be improved over time.
   - System headers designed with portability and clean namespaces
     - Use of <code>\<features.h\></code> emulating GLibC, allowing for control of available features using <code>\*\_SOURCE</code> macros
     - Option to warn about use of non-portable functions
     - Redundant fallbacks of (some) KOS-specific extensions allows for use on other platforms
     - You can actually use KOS's /include folder to spoof MSVCrt and get a much more portable library at the same time!
   - Support for future-proof <b>64-bit time\_t</b> types and functions
 - multitasking/scheduler
   - PID, including PID namespaces
   - <code>CLONE_*</code> flags compatibility allows for individual thread components to either be shared with other threads, or be unique to that thread alone.
   - Multi-core SMP support
   - Use of <code>hlt</code> when no threads are actively running
 - ELF binaries/libraries (<i>no extension</i> / <b>.so</b>)
 - Disk I/O
   - ATA driver supporting PIO28, PIO48 and CHS addressing
   - Filesystem
     - FAT-12/16/32
     - <code>/dev</code>
 - Drivers
   - PS/2 driver api
     - Using a miniature interpreter, <i>programs</i> can be run for comunicating with devices.
     - Support for an arbitrary number of connected data handles.
     - PS/2 keyboard driver
       - Supports all scansets (#1, #2 and #3)
       - Support for LEDs, as well as various KOS-specific keyboard ioctls
 - Monolithic kernel design with (untested & unused) support for modules.

## Fixes / Improvements when compared against KOSmk2 ##
 - Fix Unicode conversion
   - I had no idea how UTF-8 actually worked when I wrote the KOSmk2 API
 - POSIX signal support
   - KOSmk2 only half-a$$ed its implementation to the point where busybox started working, at which point I simply stopped implementing what was left, or fix what I did wrong.
 - Return errors when invoking system calls with unsupported flags
   - KOSmk2 used to just ignore invalid flags
 - Complete conceptual re-work of the core concepts driving the entire kernel using exceptions
 - Use of exceptions to deliver error-states absurdly reduces the number of check that would otherwise need to be done to deal with error cases (imagine an <code>malloc()</code> that never returns <code>NULL</code>, and what you get is <code>Xmalloc()</code>, or any of the underlying heap-functions)
 - Keep arbitrary page directory switches to an absolute minimum
   - Physical memory allocators no longer use a linked list, but rather a large bitset from which pages are allocated using atomic instruction. Aka. a lock-less <code>page_malloc()</code> function
   - Also make use of the x86 <code>GLOBAL</code> flag in page directories, meaning that memory access remains as fast as possible, even after page directories have been switched.
 - The new KOS has shown a tendency towards becoming a monolithic kernel, despite the fact that facilities for kernel modules already exists, though have yet to be tested.
 - Fixed many conceptual problems related to how the old KOS dealt with signal delivery to tasks in different situations, including numerous situations that could have resulted in dead locks caused by something as simple as pressing a button
   - Introduction of the concept of async-safety, as well as highly detailed rules to-be followed for code running in the context of interrupts and the like.
 - Actually working SMP support (not just conceptually), including IPIs and RPCs.
 - Fully redesigned filesystem with almost all emphasis put on caching
   - KOSmk2 though it was a good idea to not include any way of caching directory contents, or have some way of loading INodes using only their number
   - Implementation of a <code>pread()</code> and <code>pwrite()</code> operator as basis of any INode now negates the need of intermediate and file streams (the FAT driver simply caches disk positions of file chunks)
   - Much better support for <code>O\_\*</code> flags, such as <code>O\_NONBLOCK</code>
 - Using exceptions, hardware-generated exceptions such as divide-by-zero, or overflow is possible

## Interesting, but potentially useless functions ##
 - VIO
   - By analyzing assembly surrounding the return address of a #PF interrupt, as well as emulating a considerable portion of the X86 instruction set, the KOS kernel is able to emulate behavior that is usually associated with memory-mapped devices
   - Basically, KOS is able to create memory mappings that allow high-level read() / write() operators to be invoked whenever any piece of code tries to access that memory.
   - It's somewhat similar to what was discussed over here: https://forum.osdev.org/viewtopic.php?f=15&t=22521
   - For an example use, look at <code>include/kos/ushare.h</code> under <code>struct ushare\_procinfo</code>


## Known problems ##
 - You can't just compile the kernel with <code>-O3</code>. That'll break exception support due to GCC re-arranging code.
   - Related to this, I have yet to fully trace down which optimization flags actually cause problems.
   - As far as I can tell, the following optimization flags cannot be enabled:
     - <code>-fcrossjumping</code>, <code>-freorder-blocks</code>, <code>-freorder-blocks-and-partition</code>

## Planned ##
 - Rewrite the VGA terminal to run in kernel-space as a real TTY supporting job control (CTRL + ALT + F1-F12)
 - Support for DMA in the ATA driver
 - Support for USB
 - Support for bios-calls done through vm86 (including a fallback disk driver)
 - Support for <code>/proc</code>
 - Support for <code>/sys</code>
 - A user-space pthread library (Although threading can already be done using <code>fork()</code> and <code>clone()</code>)
 - TCP/IP-stack
 - Start testing the thing on real hardware again
 - WLAN support for "Atheros AR2427 Wireless" (That one's inside my test machine)

## Build Requirements ##
 - Havn't gotten around to <i>really</i> setting up a new toolchain. - You're on your own till then...




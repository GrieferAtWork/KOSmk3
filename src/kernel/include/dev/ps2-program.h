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
#ifndef GUARD_KERNEL_INCLUDE_DEV_PS2_PROGRAM_H
#define GUARD_KERNEL_INCLUDE_DEV_PS2_PROGRAM_H 1

#include <hybrid/compiler.h>
#include <dev/ps2.h>

#ifdef CONFIG_HAVE_DEV_PS2
DECL_BEGIN

/* PS/2 program rules:
 *   - Upon receiving `PS2_RESEND', the program is restarted a limited number of times.
 *   - The last received interrupt is stored in register `%irr'
 *   - Assertion failure (see use of `assert' in comments below) causes `ps2_runprogram()' to return `false'
 * Registers:
 *   - %irr  -- Interrupt result register (read-only; the last result received by an interrupt)
 *   - %res  -- Program result pointer (write-only; Incremented when written)
 *   - %arg  -- Program argument pointer (read-only; Incremented when read)
 *   - %pc   -- The current program counter.
 * Initial values:
 *   - %irr  -- Initialized to `PS2_RESEND'
 *   - %res  -- Empty
 *   - %arg  -- All input arguments passed to `ps2_runprogram()'
 *   - %pc   -- Set to the start of the `program' argument passed to `ps2_runprogram()'
 */
#define PS2_PROGOP_STOP   0x00 /* `ps2_stop'                       Stop program execution. */
#define PS2_PROGOP_FAIL   0x01 /* `ps2_fail'                       Fail program execution. */
#define PS2_PROGOP_SEND   0x02 /* `ps2_send %arg'                  Send the next argument. */
#define PS2_PROGOP_DISP   0x42 /* `ps2_send imm8'                  Send an immediate byte. */
#define PS2_PROGOP_WAIT   0x03 /* `ps2_wait'                       Wait for an interrupt, but discard the result. */
#define PS2_PROGOP_WACK   0x13 /* `ps2_wait PS2_ACK'               Wait for an interrupt and assert that it is `PS2_ACK'. */
#define PS2_PROGOP_READ   0x23 /* `ps2_wait %res'                  Wait for an interrupt and store it in the result vector. */
#define PS2_PROGOP_WIMM   0x43 /* `ps2_wait imm8'                  Wait for an interrupt and assert that it is `$imm8'. */
#define PS2_PROGOP_PRES   0x44 /* `ps2_mov imm8, %res'             Push `imm8' onto the result stack. */
#define PS2_PROGOP_PIRR   0x45 /* `ps2_mov %irr, %res'             Push `%irr' onto the result stack. */
#define PS2_PROGOP_JMP    0x7f /* `ps2_jmp Simm8_rel'              Jump relative by adding `Simm8_rel' to `%pc' (new) */
#define PS2_PROGOP_IJIFE  0x80 /* `ps2_je  %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr == imm8'. */
#define PS2_PROGOP_IJIFNE 0x81 /* `ps2_jne %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr != imm8'. */
#define PS2_PROGOP_IJIFB  0x82 /* `ps2_jb  %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr < imm8'. */
#define PS2_PROGOP_IJIFBE 0x83 /* `ps2_jbe %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr <= imm8'. */
#define PS2_PROGOP_IJIFA  0x84 /* `ps2_ja  %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr > imm8'. */
#define PS2_PROGOP_IJIFAE 0x85 /* `ps2_jae %irr, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%irr >= imm8'. */
#define PS2_PROGOP_AJIFE  0x88 /* `ps2_je  %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg == imm8'. */
#define PS2_PROGOP_AJIFNE 0x89 /* `ps2_jne %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg != imm8'. */
#define PS2_PROGOP_AJIFB  0x8a /* `ps2_jb  %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg < imm8'. */
#define PS2_PROGOP_AJIFBE 0x8b /* `ps2_jbe %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg <= imm8'. */
#define PS2_PROGOP_AJIFA  0x8c /* `ps2_ja  %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg > imm8'. */
#define PS2_PROGOP_AJIFAE 0x8d /* `ps2_jae %arg, imm8, Simm8_rel'  Jump relative by adding `Simm8_rel' to `%pc' (new) if `%arg >= imm8'. */
#define PS2_PROGOP_OPSIZE(x) (((x) & 0xc0) >> 6) /* Size of additional arguments */

#ifdef __CC__
/* Try to execute a PS/2 command `command_id' on `port'
 * HINT: If you have to count the number of output arguments,
 *       you may use `PS2_RESEND' to pre-fill all result items
 *       and may rest assured that the `%irr' will never take
 *       on that value after any wait-command.
 * @param: port:    One of `PS2_PORT*'
 * @param: program: The program that should be executed.
 * @param: argv:    Vector of additional arguments to be passed to the device. (command-specific)
 * @param: resv:    Vector of response bytes to receive from the device. */
FUNDEF bool KCALL ps2_runprogram(u8 port, u8 const *__restrict argv,
                                 u8 *__restrict resv, u8 const *__restrict program);
FUNDEF bool KCALL ps2_runprogram_ex(u8 port, u8 const *__restrict argv,
                                    u8 *__restrict resv, u8 const *__restrict program,
                                    jtime_t rel_timeout);

#endif /* __CC__ */






/* Define a new PS/2 program. */
#define PS2_DEFINE_INTERN_PROGRAM(name,...) \
 INTDEF u8 const name[]; \
 GLOBAL_ASM(L(.pushsection .rodata) \
            L(INTERN_ENTRY(name)) \
            L(__VA_ARGS__) \
            L(SYMEND(name)) \
            L(.popsection))
#define PS2_DEFINE_PRIVATE_PROGRAM(name,...) \
 INTDEF u8 const name[]; \
 GLOBAL_ASM(L(.pushsection .rodata) \
            L(PRIVATE_ENTRY(name)) \
            L(__VA_ARGS__) \
            L(SYMEND(name)) \
            L(.popsection))
#define PS2_DEFINE_PUBLIC_PROGRAM(name,...) \
 DATDEF u8 const name[]; \
 GLOBAL_ASM(L(.pushsection .rodata) \
            L(PUBLIC_ENTRY(name)) \
            L(__VA_ARGS__) \
            L(SYMEND(name)) \
            L(.popsection))

/* Create and return a PS/2 program. */
#define PS2_PROGRAM(...) \
 XBLOCK({ INTDEF u8 const __PS2_TEMP_NAME[]; \
          GLOBAL_ASM(L(.pushsection .rodata) \
                     L(INTERN_ENTRY(__PS2_TEMP_NAME)) \
                     L(__VA_ARGS__) \
                     L(SYMEND(__PS2_TEMP_NAME)) \
                     L(.popsection)); \
          XRETURN __PS2_TEMP_NAME; })
#define __PS2_TEMP_NAME  __PP_CAT2(__ps2_local_program,__LINE__)



#ifdef __x86_64__
#define __PS2_R_PC8  R_X86_64_PC8
#elif defined(__i386__)
#define __PS2_R_PC8  R_386_PC8
#else
#error "Unsupported architecture"
#endif




/* Define assembly macros for defining PS/2 programs. */
GLOBAL_ASM(
L(.macro ps2_stop)
L(   .byte PS2_PROGOP_STOP)
L(.endm)
L(.macro ps2_fail)
L(   .byte PS2_PROGOP_FAIL)
L(.endm)
L(.macro ps2_send arg)
L(   .ifc MACROARG(arg),%arg)
L(      .byte PS2_PROGOP_SEND)
L(   .else)
L(      .byte PS2_PROGOP_DISP)
L(      .byte MACROARG(arg))
L(   .endif)
L(.endm)
L(.macro ps2_wait arg)
L(   .ifc MACROARG(arg),%res)
L(      .byte PS2_PROGOP_READ)
L(   .else)
L(      .ifc MACROARG(arg),'')
L(         .byte PS2_PROGOP_WAIT)
L(      .else)
L(         .if MACROARG(arg) == PS2_ACK)
L(            .byte PS2_PROGOP_WACK)
L(         .else)
L(            .byte PS2_PROGOP_WIMM)
L(            .byte MACROARG(arg))
L(         .endif)
L(      .endif)
L(   .endif)
L(.endm)
L(.macro __ps2_jcc cond, reg, val, dst)
L(   .ifnc MACROARG(reg),%irr)
L(      .byte 0x8+MACROARG(cond))
L(   .else)
L(      .ifc MACROARG(reg),%arg)
L(         .byte MACROARG(cond))
L(      .else)
L(         .error "Invalid register \reg")
L(      .endif)
L(   .endif)
L(   .byte MACROARG(val))
L(   .reloc ., __PS2_R_PC8, MACROARG(dst))
L(   .byte 0xff)
L(.endm)
L(.macro ps2_jmp dst)
L(   .byte PS2_PROGOP_JMP)
L(   .reloc ., __PS2_R_PC8, MACROARG(dst))
L(   .byte 0xff)
L(.endm)
L(.macro ps2_je reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFE, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_jne reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFNE, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_jb reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFB, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_jbe reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFBE, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_ja reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFA, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_jae reg, val, dst)
L(   __ps2_jcc PS2_PROGOP_IJIFAE, MACROARG(reg), MACROARG(val), MACROARG(dst))
L(.endm)
L(.macro ps2_mov val, dst)
L(   .ifc MACROARG(dst),%res)
L(      .ifc MACROARG(val),%irr)
L(         .byte PS2_PROGOP_PIRR)
L(      .else)
L(         .byte PS2_PROGOP_PRES)
L(         .byte MACROARG(val))
L(      .endif)
L(   .else)
L(      .error "Invalid destination for ps2_mov: \dst")
L(   .endif)
L(.endm)
);


DECL_END

#endif /* CONFIG_HAVE_DEV_PS2 */

#endif /* !GUARD_KERNEL_INCLUDE_DEV_PS2_PROGRAM_H */

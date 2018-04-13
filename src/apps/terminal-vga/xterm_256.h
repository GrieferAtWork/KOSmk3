/* MIT License
 *
 * Copyright (c) 2017 GrieferAtWork
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef GUARD_LIBTERM_XTERM_256_H
#define GUARD_LIBTERM_XTERM_256_H 1

#include <hybrid/compiler.h>
#include "libterm.h"

DECL_BEGIN

/* Colors used for XTerm 256 color mode */
PRIVATE struct term_rgba const xterm_256[256] = {
#define RGB(r,g,b)  TERM_RGBA_INIT(r,g,b,0xff)
    RGB(0x00,0x00,0x00), /*< Black. */
    RGB(0x80,0x00,0x00), /*< Maroon. */
    RGB(0x00,0x80,0x00), /*< Green. */
    RGB(0x80,0x80,0x00), /*< Olive. */
    RGB(0x00,0x00,0x80), /*< Navy. */
    RGB(0x80,0x00,0x80), /*< Purple. */
    RGB(0x00,0x80,0x80), /*< Teal. */
    RGB(0xc0,0xc0,0xc0), /*< Silver. */
    RGB(0x80,0x80,0x80), /*< Grey. */
    RGB(0xff,0x00,0x00), /*< Red. */
    RGB(0x00,0xff,0x00), /*< Lime. */
    RGB(0xff,0xff,0x00), /*< Yellow. */
    RGB(0x00,0x00,0xff), /*< Blue. */
    RGB(0xff,0x00,0xff), /*< Fuchsia. */
    RGB(0x00,0xff,0xff), /*< Aqua. */
    RGB(0xff,0xff,0xff), /*< White. */
    RGB(0x00,0x00,0x00),
    RGB(0x00,0x00,0x5f),
    RGB(0x00,0x00,0x87),
    RGB(0x00,0x00,0xaf),
    RGB(0x00,0x00,0xd7),
    RGB(0x00,0x00,0xff),
    RGB(0x00,0x5f,0x00),
    RGB(0x00,0x5f,0x5f),
    RGB(0x00,0x5f,0x87),
    RGB(0x00,0x5f,0xaf),
    RGB(0x00,0x5f,0xd7),
    RGB(0x00,0x5f,0xff),
    RGB(0x00,0x87,0x00),
    RGB(0x00,0x87,0x5f),
    RGB(0x00,0x87,0x87),
    RGB(0x00,0x87,0xaf),
    RGB(0x00,0x87,0xd7),
    RGB(0x00,0x87,0xff),
    RGB(0x00,0xaf,0x00),
    RGB(0x00,0xaf,0x5f),
    RGB(0x00,0xaf,0x87),
    RGB(0x00,0xaf,0xaf),
    RGB(0x00,0xaf,0xd7),
    RGB(0x00,0xaf,0xff),
    RGB(0x00,0xd7,0x00),
    RGB(0x00,0xd7,0x5f),
    RGB(0x00,0xd7,0x87),
    RGB(0x00,0xd7,0xaf),
    RGB(0x00,0xd7,0xd7),
    RGB(0x00,0xd7,0xff),
    RGB(0x00,0xff,0x00),
    RGB(0x00,0xff,0x5f),
    RGB(0x00,0xff,0x87),
    RGB(0x00,0xff,0xaf),
    RGB(0x00,0xff,0xd7),
    RGB(0x00,0xff,0xff),
    RGB(0x5f,0x00,0x00),
    RGB(0x5f,0x00,0x5f),
    RGB(0x5f,0x00,0x87),
    RGB(0x5f,0x00,0xaf),
    RGB(0x5f,0x00,0xd7),
    RGB(0x5f,0x00,0xff),
    RGB(0x5f,0x5f,0x00),
    RGB(0x5f,0x5f,0x5f),
    RGB(0x5f,0x5f,0x87),
    RGB(0x5f,0x5f,0xaf),
    RGB(0x5f,0x5f,0xd7),
    RGB(0x5f,0x5f,0xff),
    RGB(0x5f,0x87,0x00),
    RGB(0x5f,0x87,0x5f),
    RGB(0x5f,0x87,0x87),
    RGB(0x5f,0x87,0xaf),
    RGB(0x5f,0x87,0xd7),
    RGB(0x5f,0x87,0xff),
    RGB(0x5f,0xaf,0x00),
    RGB(0x5f,0xaf,0x5f),
    RGB(0x5f,0xaf,0x87),
    RGB(0x5f,0xaf,0xaf),
    RGB(0x5f,0xaf,0xd7),
    RGB(0x5f,0xaf,0xff),
    RGB(0x5f,0xd7,0x00),
    RGB(0x5f,0xd7,0x5f),
    RGB(0x5f,0xd7,0x87),
    RGB(0x5f,0xd7,0xaf),
    RGB(0x5f,0xd7,0xd7),
    RGB(0x5f,0xd7,0xff),
    RGB(0x5f,0xff,0x00),
    RGB(0x5f,0xff,0x5f),
    RGB(0x5f,0xff,0x87),
    RGB(0x5f,0xff,0xaf),
    RGB(0x5f,0xff,0xd7),
    RGB(0x5f,0xff,0xff),
    RGB(0x87,0x00,0x00),
    RGB(0x87,0x00,0x5f),
    RGB(0x87,0x00,0x87),
    RGB(0x87,0x00,0xaf),
    RGB(0x87,0x00,0xd7),
    RGB(0x87,0x00,0xff),
    RGB(0x87,0x5f,0x00),
    RGB(0x87,0x5f,0x5f),
    RGB(0x87,0x5f,0x87),
    RGB(0x87,0x5f,0xaf),
    RGB(0x87,0x5f,0xd7),
    RGB(0x87,0x5f,0xff),
    RGB(0x87,0x87,0x00),
    RGB(0x87,0x87,0x5f),
    RGB(0x87,0x87,0x87),
    RGB(0x87,0x87,0xaf),
    RGB(0x87,0x87,0xd7),
    RGB(0x87,0x87,0xff),
    RGB(0x87,0xaf,0x00),
    RGB(0x87,0xaf,0x5f),
    RGB(0x87,0xaf,0x87),
    RGB(0x87,0xaf,0xaf),
    RGB(0x87,0xaf,0xd7),
    RGB(0x87,0xaf,0xff),
    RGB(0x87,0xd7,0x00),
    RGB(0x87,0xd7,0x5f),
    RGB(0x87,0xd7,0x87),
    RGB(0x87,0xd7,0xaf),
    RGB(0x87,0xd7,0xd7),
    RGB(0x87,0xd7,0xff),
    RGB(0x87,0xff,0x00),
    RGB(0x87,0xff,0x5f),
    RGB(0x87,0xff,0x87),
    RGB(0x87,0xff,0xaf),
    RGB(0x87,0xff,0xd7),
    RGB(0x87,0xff,0xff),
    RGB(0xaf,0x00,0x00),
    RGB(0xaf,0x00,0x5f),
    RGB(0xaf,0x00,0x87),
    RGB(0xaf,0x00,0xaf),
    RGB(0xaf,0x00,0xd7),
    RGB(0xaf,0x00,0xff),
    RGB(0xaf,0x5f,0x00),
    RGB(0xaf,0x5f,0x5f),
    RGB(0xaf,0x5f,0x87),
    RGB(0xaf,0x5f,0xaf),
    RGB(0xaf,0x5f,0xd7),
    RGB(0xaf,0x5f,0xff),
    RGB(0xaf,0x87,0x00),
    RGB(0xaf,0x87,0x5f),
    RGB(0xaf,0x87,0x87),
    RGB(0xaf,0x87,0xaf),
    RGB(0xaf,0x87,0xd7),
    RGB(0xaf,0x87,0xff),
    RGB(0xaf,0xaf,0x00),
    RGB(0xaf,0xaf,0x5f),
    RGB(0xaf,0xaf,0x87),
    RGB(0xaf,0xaf,0xaf),
    RGB(0xaf,0xaf,0xd7),
    RGB(0xaf,0xaf,0xff),
    RGB(0xaf,0xd7,0x00),
    RGB(0xaf,0xd7,0x5f),
    RGB(0xaf,0xd7,0x87),
    RGB(0xaf,0xd7,0xaf),
    RGB(0xaf,0xd7,0xd7),
    RGB(0xaf,0xd7,0xff),
    RGB(0xaf,0xff,0x00),
    RGB(0xaf,0xff,0x5f),
    RGB(0xaf,0xff,0x87),
    RGB(0xaf,0xff,0xaf),
    RGB(0xaf,0xff,0xd7),
    RGB(0xaf,0xff,0xff),
    RGB(0xd7,0x00,0x00),
    RGB(0xd7,0x00,0x5f),
    RGB(0xd7,0x00,0x87),
    RGB(0xd7,0x00,0xaf),
    RGB(0xd7,0x00,0xd7),
    RGB(0xd7,0x00,0xff),
    RGB(0xd7,0x5f,0x00),
    RGB(0xd7,0x5f,0x5f),
    RGB(0xd7,0x5f,0x87),
    RGB(0xd7,0x5f,0xaf),
    RGB(0xd7,0x5f,0xd7),
    RGB(0xd7,0x5f,0xff),
    RGB(0xd7,0x87,0x00),
    RGB(0xd7,0x87,0x5f),
    RGB(0xd7,0x87,0x87),
    RGB(0xd7,0x87,0xaf),
    RGB(0xd7,0x87,0xd7),
    RGB(0xd7,0x87,0xff),
    RGB(0xd7,0xaf,0x00),
    RGB(0xd7,0xaf,0x5f),
    RGB(0xd7,0xaf,0x87),
    RGB(0xd7,0xaf,0xaf),
    RGB(0xd7,0xaf,0xd7),
    RGB(0xd7,0xaf,0xff),
    RGB(0xd7,0xd7,0x00),
    RGB(0xd7,0xd7,0x5f),
    RGB(0xd7,0xd7,0x87),
    RGB(0xd7,0xd7,0xaf),
    RGB(0xd7,0xd7,0xd7),
    RGB(0xd7,0xd7,0xff),
    RGB(0xd7,0xff,0x00),
    RGB(0xd7,0xff,0x5f),
    RGB(0xd7,0xff,0x87),
    RGB(0xd7,0xff,0xaf),
    RGB(0xd7,0xff,0xd7),
    RGB(0xd7,0xff,0xff),
    RGB(0xff,0x00,0x00),
    RGB(0xff,0x00,0x5f),
    RGB(0xff,0x00,0x87),
    RGB(0xff,0x00,0xaf),
    RGB(0xff,0x00,0xd7),
    RGB(0xff,0x00,0xff),
    RGB(0xff,0x5f,0x00),
    RGB(0xff,0x5f,0x5f),
    RGB(0xff,0x5f,0x87),
    RGB(0xff,0x5f,0xaf),
    RGB(0xff,0x5f,0xd7),
    RGB(0xff,0x5f,0xff),
    RGB(0xff,0x87,0x00),
    RGB(0xff,0x87,0x5f),
    RGB(0xff,0x87,0x87),
    RGB(0xff,0x87,0xaf),
    RGB(0xff,0x87,0xd7),
    RGB(0xff,0x87,0xff),
    RGB(0xff,0xaf,0x00),
    RGB(0xff,0xaf,0x5f),
    RGB(0xff,0xaf,0x87),
    RGB(0xff,0xaf,0xaf),
    RGB(0xff,0xaf,0xd7),
    RGB(0xff,0xaf,0xff),
    RGB(0xff,0xd7,0x00),
    RGB(0xff,0xd7,0x5f),
    RGB(0xff,0xd7,0x87),
    RGB(0xff,0xd7,0xaf),
    RGB(0xff,0xd7,0xd7),
    RGB(0xff,0xd7,0xff),
    RGB(0xff,0xff,0x00),
    RGB(0xff,0xff,0x5f),
    RGB(0xff,0xff,0x87),
    RGB(0xff,0xff,0xaf),
    RGB(0xff,0xff,0xd7),
    RGB(0xff,0xff,0xff),
    RGB(0x08,0x08,0x08),
    RGB(0x12,0x12,0x12),
    RGB(0x1c,0x1c,0x1c),
    RGB(0x26,0x26,0x26),
    RGB(0x30,0x30,0x30),
    RGB(0x3a,0x3a,0x3a),
    RGB(0x44,0x44,0x44),
    RGB(0x4e,0x4e,0x4e),
    RGB(0x58,0x58,0x58),
    RGB(0x62,0x62,0x62),
    RGB(0x6c,0x6c,0x6c),
    RGB(0x76,0x76,0x76),
    RGB(0x80,0x80,0x80),
    RGB(0x8a,0x8a,0x8a),
    RGB(0x94,0x94,0x94),
    RGB(0x9e,0x9e,0x9e),
    RGB(0xa8,0xa8,0xa8),
    RGB(0xb2,0xb2,0xb2),
    RGB(0xbc,0xbc,0xbc),
    RGB(0xc6,0xc6,0xc6),
    RGB(0xd0,0xd0,0xd0),
    RGB(0xda,0xda,0xda),
    RGB(0xe4,0xe4,0xe4),
    RGB(0xee,0xee,0xee),
};
#undef RGB

DECL_END

#endif /* !GUARD_LIBTERM_XTERM_256_H */

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
#ifndef GUARD_APPS_WMS_RECT_H
#define GUARD_APPS_WMS_RECT_H 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>

DECL_BEGIN

struct rect {
    unsigned int r_xmin; /* Starting X coord. */
    unsigned int r_ymin; /* Starting Y coord. */
    unsigned int r_xsiz; /* Number of pixels in X. */
    unsigned int r_ysiz; /* Number of pixels in Y. */
};

/* Return the intersection of `a' and `b' */
INTDEF struct rect WMCALL rect_intersect(struct rect a, struct rect b);


struct rect_block {
    unsigned int rb_ymin;
    unsigned int rb_ysiz;
};
struct rect_strip {
    SLIST_NODE(struct rect_strip) rs_chain;   /* Chain of striped, sorted in ascending order. */
    unsigned int                  rs_xmin;    /* Width in X */
    unsigned int                  rs_xsiz;    /* Width in X */
    unsigned int                  rs_blkc;    /* [!0] Number of blocks. */
    struct rect_block             rs_blkv[1]; /* [rs_blkc] Vector of blocks, sorted in ascending order. */
};
#define RECT_STRIP_ALLOC(num_blocks) \
   ((struct rect_strip *)Xmalloc(offsetof(struct rect_strip,rs_blkv)+ \
                      (num_blocks)*sizeof(struct rect_block)))
#define RECT_STRIP_DUP(self) \
   ((struct rect_strip *)Xmemdup(self,offsetof(struct rect_strip,rs_blkv)+ \
                                (self)->rs_blkc*sizeof(struct rect_block)))


struct rects {
    SLIST_HEAD(struct rect_strip) r_strips; /* [owned] Chain of rectangle strips. */
};
#define RECTS_FOREACH(r,self) \
 for (struct rect_strip *_strip = (self).r_strips; \
      _strip; _strip = _strip->rs_chain.le_next) \
 if (((r).r_xmin = _strip->rs_xmin, \
      (r).r_xsiz = _strip->rs_xsiz),0); else \
 for (unsigned int _block = 0; _block < _strip->rs_blkc; ++_block) \
 if (((r).r_ymin = _strip->rs_blkv[_block].rb_ymin, \
      (r).r_ysiz = _strip->rs_blkv[_block].rb_ysiz),0); else


/* Modify the rects vector to cover, or no longer cover the given rect `r' */
INTDEF void WMCALL rects_insert(struct rects *__restrict self, struct rect r);
INTDEF void WMCALL rects_remove(struct rects *__restrict self, struct rect r);

/* Duplicate all rects from `src' into `dst' and adjust
 * all relative coords by `x_offset' and `y_offset' */
INTDEF void WMCALL
rects_duplicate_and_move(struct rects *__restrict dst,
                         struct rects const *__restrict src,
                         int x_offset, int y_offset);
#if defined(NDEBUG) || 0
#define rects_assert(self)  (void)0
#else
INTDEF void WMCALL
rects_assert(struct rects *__restrict self);
#endif

/* Apply an offset to all rects stored in `self' */
INTDEF void WMCALL
rects_move(struct rects *__restrict self,
           int x_offset, int y_offset);

/* Finalize the given rects vector. */
INTDEF void WMCALL rects_fini(struct rects *__restrict self);


DECL_END

#endif /* !GUARD_APPS_WMS_RECT_H */

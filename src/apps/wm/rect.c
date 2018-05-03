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
#ifndef GUARD_APPS_WM_RECT_C
#define GUARD_APPS_WM_RECT_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <hybrid/align.h>
#include <hybrid/minmax.h>
#include <kos/types.h>
#include <malloc.h>
#include <assert.h>
#include <string.h>
#include <stdbool.h>
#include <syslog.h>

#include "window.h"

DECL_BEGIN

INTERN struct rect KCALL
rect_intersect(struct rect a, struct rect b) {
 struct rect result;
 unsigned int a_xend = a.r_xmin+a.r_xsiz;
 unsigned int a_yend = a.r_ymin+a.r_ysiz;
 unsigned int b_xend = b.r_xmin+b.r_xsiz;
 unsigned int b_yend = b.r_ymin+b.r_ysiz;
 if (a_xend <= b.r_xmin) goto empty;
 if (a_yend <= b.r_ymin) goto empty;
 if (a.r_xmin >= b_xend) goto empty;
 if (a.r_ymin >= b_yend) goto empty;
 result.r_xmin = MAX(a.r_xmin,b.r_xmin);
 result.r_ymin = MAX(a.r_ymin,b.r_ymin);
 result.r_xsiz = MIN(a_xend,b_xend) - result.r_xmin;
 result.r_ysiz = MIN(a_yend,b_yend) - result.r_ymin;
 return result;
empty:
 result.r_xsiz = 0;
 result.r_ysiz = 0;
 return result;
}



/* Split `self' at offset `x_offset_from_self'
 * @return: * : The upper half after the split. */
PRIVATE struct rect_strip *KCALL
rects_strip_split(struct rect_strip *__restrict self,
                  unsigned int x_offset_from_self) {
 struct rect_strip *copy;
 assert(x_offset_from_self < self->rs_xsiz);
 copy = RECT_STRIP_DUP(self);
 copy->rs_xmin += x_offset_from_self;
 copy->rs_xsiz -= x_offset_from_self;
 self->rs_xsiz  = x_offset_from_self;
 self->rs_chain.le_next = copy;
 return copy;
}

/* Try try to merge `self' with its successor. */
PRIVATE bool KCALL
rects_strip_mergex(struct rect_strip *__restrict self) {
 struct rect_strip *next = self->rs_chain.le_next;
 if unlikely(!next) goto nope; /* No successor. */
 assertf(self->rs_xmin+self->rs_xsiz <= next->rs_xmin,
        "self->rs_xmin = %u(%x)\n"
        "self->rs_xsiz = %u(%x)\n"
        "next->rs_xmin = %u(%x)\n"
        ,self->rs_xmin,self->rs_xmin
        ,self->rs_xsiz,self->rs_xsiz
        ,next->rs_xmin,next->rs_xmin);
 if (self->rs_xmin+self->rs_xsiz != next->rs_xmin)
     goto nope; /* Blocks aren't consecutive. */
 if (self->rs_blkc != next->rs_blkc)
     goto nope; /* Different number of blocks. */
 if (memcmp(self->rs_blkv,next->rs_blkv,self->rs_blkc*sizeof(struct rect_block)) != 0)
     goto nope; /* Different block layout */
 /* Yes, we can merge them! */
 self->rs_xsiz += next->rs_xsiz;
 self->rs_chain.le_next = next->rs_chain.le_next;
 free(next);
 return true;
nope:
 return false;
}


/* Try try to merge block index `block_index' with its successor. */
PRIVATE bool KCALL
rects_strip_mergey(struct rect_strip *__restrict self,
                   unsigned int block_index) {
 assert(block_index < self->rs_blkc);
 if (block_index == self->rs_blkc-1)
     goto nope; /* No successor. */
 assert(self->rs_blkv[block_index].rb_ymin+
        self->rs_blkv[block_index].rb_ysiz <=
        self->rs_blkv[block_index+1].rb_ymin);
 if (self->rs_blkv[block_index].rb_ymin+
     self->rs_blkv[block_index].rb_ysiz !=
     self->rs_blkv[block_index+1].rb_ymin)
     goto nope; /* Non-consecutive blocks. */
 /* Actually merge the 2 blocks! */
 self->rs_blkv[block_index].rb_ysiz += self->rs_blkv[block_index+1].rb_ysiz;
 /* Move following blocks to fill the gap. */
 --self->rs_blkc;
 memmove(&self->rs_blkv[block_index+1],
         &self->rs_blkv[block_index+2],
         (self->rs_blkc-(block_index+1))*
          sizeof(struct rect_block));
 return true;
nope:
 return false;
}


/* Insert a new block under `block_index', shifting following blocks upwards.
 * @return: * : Always return `self', or the relocated `self' if realloc() was called. */
PRIVATE struct rect_strip *KCALL
rects_strip_inserty(struct rect_strip **__restrict pself,
                    struct rect_strip *__restrict self,
                    unsigned int block_index,
                    unsigned int ymin, unsigned int ysiz) {
 unsigned int size_avail;
 assert(block_index <= self->rs_blkc);
 /* Assert that the new block doesn't overlap
  * with anything and is properly ordered. */
 assert(!self->rs_blkc || block_index == 0 ||
         self->rs_blkv[block_index-1].rb_ymin+
         self->rs_blkv[block_index-1].rb_ysiz <= ymin);
 assert(!self->rs_blkc || block_index == self->rs_blkc-1 ||
         ymin+ysiz <= self->rs_blkv[block_index].rb_ymin);
 size_avail = (malloc_usable_size(self)-offsetof(struct rect_strip,rs_blkv))/
               sizeof(struct rect_block);
 assert(self->rs_blkc <= size_avail);
 if unlikely(self->rs_blkc == size_avail) {
  /* Must reallocate a larger strip. */
  self = (struct rect_strip *)Xrealloc(self,
                                       offsetof(struct rect_strip,rs_blkv)+
                                      (self->rs_blkc+1)*sizeof(struct rect_block));
  *pself = self;
 }
 /* Move following entries to make space. */
 memmove(&self->rs_blkv[block_index+1],
         &self->rs_blkv[block_index],
         (self->rs_blkc-block_index)*
          sizeof(struct rect_block));
 self->rs_blkv[block_index].rb_ymin = ymin;
 self->rs_blkv[block_index].rb_ysiz = ysiz;
 ++self->rs_blkc;
 return self;
}


PRIVATE struct rect_strip *KCALL
rects_strip_insert(struct rect_strip **__restrict pself,
                   struct rect_strip *__restrict self,
                   unsigned int ymin, unsigned int ysiz) {
 unsigned int block_index;
 unsigned int yend = ymin+ysiz;
 /* Figure out where to insert the block, while also
  * extending any blocks which which we are overlapping. */
 for (block_index = 0; block_index < self->rs_blkc;) {
  if (self->rs_blkv[block_index].rb_ymin+
      self->rs_blkv[block_index].rb_ysiz < ymin) {
   ++block_index;
   continue; /* Skip leading blocks. */
  }
  if (ymin < self->rs_blkv[block_index].rb_ymin) {
   unsigned int insert_size;
   insert_size = self->rs_blkv[block_index].rb_ymin-ymin;
   insert_size = MIN(insert_size,ysiz);
   if (block_index &&
      (assert(self->rs_blkv[block_index-1].rb_ymin+
              self->rs_blkv[block_index-1].rb_ysiz <= ymin),
       self->rs_blkv[block_index-1].rb_ymin+
       self->rs_blkv[block_index-1].rb_ysiz == ymin)) {
    /* Extend the previous block. */
    self->rs_blkv[block_index-1].rb_ysiz += insert_size;
    /* Merge the newly extended block with its successor at `block_index'. */
    if (rects_strip_mergey(self,block_index-1)) --block_index;
   } else {
    /* Must insert a new block. */
    self = rects_strip_inserty(pself,self,block_index,
                               ymin,insert_size);
    /* Try to merge the newly created block. */
    rects_strip_mergey(self,block_index);
   }
   ymin += insert_size;
   ysiz -= insert_size;
   if (!ysiz) goto done;
  }
  if (yend > self->rs_blkv[block_index].rb_ymin) {
   unsigned int part_end;
   /* Overlap with `next' */
   part_end = (self->rs_blkv[block_index].rb_ymin+
               self->rs_blkv[block_index].rb_ysiz);
   if (ysiz <= (part_end-ymin)) goto done;
   ysiz -= (part_end-ymin);
   ymin  = part_end;
   ++block_index;
  }
 }
 /* Append a new entry at the end. */
 self = rects_strip_inserty(pself,self,block_index,ymin,ysiz);
 if (block_index != 0)
     rects_strip_mergey(self,block_index-1);
done:
 return self;
}

/* Modify the rects vector to cover, or no longer cover the given rect `r' */
INTERN void KCALL
rects_insert(struct rects *__restrict self, struct rect r) {
 struct rect_strip *next,*new_strip,**pnext;
 unsigned int x_end = r.r_xmin+r.r_xsiz;
 if (!r.r_xsiz || !r.r_ysiz) return;
#if 1
 assert((int)r.r_xsiz >= 0);
 assert((int)r.r_ysiz >= 0);
#endif
 next = self->r_strips;
 if (!next || next->rs_xmin > x_end) {
  /* Special case: Insert an unrelated strip at the front. */
  new_strip = RECT_STRIP_ALLOC(1);
  new_strip->rs_blkc            = 1;
  new_strip->rs_xmin            = r.r_xmin;
  new_strip->rs_xsiz            = r.r_xsiz;
  new_strip->rs_blkv[0].rb_ymin = r.r_ymin;
  new_strip->rs_blkv[0].rb_ysiz = r.r_ysiz;
  new_strip->rs_chain.le_next   = next;
  self->r_strips                = new_strip;
  return;
 }
 /* Find the string before which to insert this new one. */
 pnext = &self->r_strips;
 for (;next;) {
  if (next->rs_xmin+next->rs_xsiz <= r.r_xmin) {
   pnext = &next->rs_chain.le_next;
   next = *pnext;
   continue;
  }
  if (r.r_xmin < next->rs_xmin) {
   /* Insert a new strip before `next' */
   new_strip = RECT_STRIP_ALLOC(1);
   new_strip->rs_blkc            = 1;
   new_strip->rs_xmin            = r.r_xmin;
   new_strip->rs_xsiz            = MIN(r.r_xsiz,next->rs_xmin-r.r_xmin);
   assert(new_strip->rs_xsiz <= r.r_xsiz);
   new_strip->rs_blkv[0].rb_ymin = r.r_ymin;
   new_strip->rs_blkv[0].rb_ysiz = r.r_ysiz;
   new_strip->rs_chain.le_next   = next;
   r.r_xmin += new_strip->rs_xsiz;
   r.r_xsiz -= new_strip->rs_xsiz;
   *pnext = new_strip;
   /* Try to merge the modified strip with its predecessor. */
   if (pnext != &self->r_strips &&
        rects_strip_mergex(COMPILER_CONTAINER_OF(pnext,struct rect_strip,rs_chain.le_next)));
   else pnext = &new_strip->rs_chain.le_next;
   assert(*pnext == next);
   if (!r.r_xsiz) return;
  }
  assert(r.r_xmin >= next->rs_xmin);
  if (x_end > next->rs_xmin) {
   unsigned int strip_part;
   unsigned int part_end;
   /* Overlap with `next' */
   strip_part = MIN(x_end-next->rs_xmin,next->rs_xsiz);
   if (strip_part < next->rs_xsiz) {
    /* Split the strip if this is a partial cover. */
    rects_strip_split(next,strip_part);
   }
   assert(next->rs_xsiz != 0);
   assert(x_end >= next->rs_xmin+next->rs_xsiz);
   /* Insert a new vertical entry into this strip. */
   next = rects_strip_insert(pnext,next,r.r_ymin,r.r_ysiz);
   part_end = next->rs_xmin+next->rs_xsiz;
   /* Try to merge the modified strip with its successor. */
   if (pnext != &self->r_strips &&
       rects_strip_mergex(COMPILER_CONTAINER_OF(pnext,struct rect_strip,rs_chain.le_next)))
       next = *pnext;
   if (!rects_strip_mergex(next)) {
    pnext = &next->rs_chain.le_next;
    next  = *pnext;
   }
   if (r.r_xsiz <= (part_end-r.r_xmin)) return;
   r.r_xsiz -= (part_end-r.r_xmin);
   r.r_xmin  = part_end;
  }
 }
 /* Append a new strip at the end (after `*pnext') */
 assert(!*pnext);
 new_strip = RECT_STRIP_ALLOC(1);
 new_strip->rs_blkc            = 1;
 new_strip->rs_xmin            = r.r_xmin;
 new_strip->rs_xsiz            = r.r_xsiz;
 new_strip->rs_blkv[0].rb_ymin = r.r_ymin;
 new_strip->rs_blkv[0].rb_ysiz = r.r_ysiz;
 new_strip->rs_chain.le_next   = NULL;
 *pnext = new_strip;
 if (pnext != &self->r_strips)
     rects_strip_mergex(COMPILER_CONTAINER_OF(pnext,struct rect_strip,rs_chain.le_next));
}



PRIVATE struct rect_strip *KCALL
rects_strip_removey(struct rect_strip **__restrict pself,
                    struct rect_strip *__restrict self,
                    unsigned int ymin, unsigned int ysiz) {
 unsigned int block_index;
 assert(self->rs_blkc);
 for (block_index = 0;
      block_index < self->rs_blkc;) {
  unsigned int block_end,remove_size;
  block_end = (self->rs_blkv[block_index].rb_ymin+
               self->rs_blkv[block_index].rb_ysiz);
  if (ymin >= block_end) {
   /* Skip leading blocks. */
   ++block_index;
   continue;
  }
  if (ymin < self->rs_blkv[block_index].rb_ymin) {
   unsigned int skip_size;
   skip_size = self->rs_blkv[block_index].rb_ymin - ymin;
   if (ysiz <= skip_size) break;
   ysiz -= skip_size;
   ymin  = self->rs_blkv[block_index].rb_ymin;
  }
  if (ymin == self->rs_blkv[block_index].rb_ymin) {
   /* Truncate, or remove this block. */
   remove_size = MIN(self->rs_blkv[block_index].rb_ysiz,ysiz);
   if (remove_size == self->rs_blkv[block_index].rb_ysiz) {
    /* Remove the block */
    --self->rs_blkc;
    memmove(&self->rs_blkv[block_index],
            &self->rs_blkv[block_index+1],
            (self->rs_blkc-block_index)*
             sizeof(struct rect_block));
   } else {
    /* Truncate this block. */
    self->rs_blkv[block_index].rb_ymin += remove_size;
    assert(self->rs_blkv[block_index].rb_ysiz > remove_size);
    self->rs_blkv[block_index].rb_ysiz -= remove_size;
    ++block_index;
   }
  } else if (ymin+ysiz >= block_end) {
   assert(ymin > self->rs_blkv[block_index].rb_ymin);
   remove_size = (self->rs_blkv[block_index].rb_ysiz-
                 (ymin - self->rs_blkv[block_index].rb_ymin));
   assert(remove_size <= ysiz);
   /* Truncate this block. */
   assertf(self->rs_blkv[block_index].rb_ysiz > remove_size,
           "self->rs_blkv[block_index].rb_ysiz = %u\n"
           "remove_size                        = %u\n",
           self->rs_blkv[block_index].rb_ysiz,remove_size);

   self->rs_blkv[block_index].rb_ysiz -= remove_size;
   ++block_index;
  } else {
   /* Complicated case: Split into 2 rects, so we can remove the center one. */
   unsigned int higher_half_size;
   assert(ymin      > self->rs_blkv[block_index].rb_ymin);
   assert(ymin+ysiz < self->rs_blkv[block_index].rb_ymin+
                      self->rs_blkv[block_index].rb_ysiz);
   higher_half_size  = (self->rs_blkv[block_index].rb_ymin+
                        self->rs_blkv[block_index].rb_ysiz);
   higher_half_size -= (ymin+ysiz);
   /* Truncate the lower block. */
   assert(ymin > self->rs_blkv[block_index].rb_ymin);
   self->rs_blkv[block_index].rb_ysiz = ymin-self->rs_blkv[block_index].rb_ymin;
   /* Insert a block for the higher half. */
   self = rects_strip_inserty(pself,self,block_index+1,ymin+ysiz,higher_half_size);
   block_index += 2;
   remove_size = ysiz;
  }
  ysiz -= remove_size;
  if (!ysiz) break;
  ymin += remove_size;
 }
 return self;
}



INTERN void KCALL
rects_remove(struct rects *__restrict self, struct rect r) {
 struct rect_strip *iter,**piter;
 unsigned int x_end = r.r_xmin+r.r_xsiz;
 unsigned int strip_end;
 if (!r.r_xsiz || !r.r_ysiz) return;
#if 1
 assert((int)r.r_xsiz >= 0);
 assert((int)r.r_ysiz >= 0);
#endif

 piter = &self->r_strips;
 while ((iter = *piter) != NULL) {
  strip_end = iter->rs_xmin+iter->rs_xsiz;
  if (strip_end <= r.r_xmin) {
   /* Skip entries below our range of interest. */
   piter = &iter->rs_chain.le_next;
   continue;
  }
  if (r.r_xmin < iter->rs_xmin) {
   unsigned int ignore_size;
   ignore_size = iter->rs_xmin-r.r_xmin;
   if (r.r_xsiz >= ignore_size)
       return;
   r.r_xsiz -= ignore_size;
   r.r_xmin  = iter->rs_xmin;
  }
  assert(iter->rs_xsiz != 0);
  assert(strip_end == iter->rs_xmin+iter->rs_xsiz);
  assert(r.r_xmin >= iter->rs_xmin);
  assertf(r.r_xmin <  strip_end,
          "r.r_xmin  = %u\n"
          "strip_end = %u\n",
          r.r_xmin,strip_end);
  if (r.r_xmin != iter->rs_xmin) {
   /* Split the strip if we're not at its starting location. */
   piter = &iter->rs_chain.le_next;
   iter = rects_strip_split(iter,r.r_xmin-iter->rs_xmin);
   assert(*piter == iter);
  }
  assert(r.r_xmin == iter->rs_xmin);
  assert(r.r_xmin <  strip_end);
  assert(strip_end == iter->rs_xmin+iter->rs_xsiz);
  if (x_end < strip_end) {
   /* Split the strip if we're not at its starting location. */
   rects_strip_split(iter,x_end-iter->rs_xmin);
   strip_end = x_end;
  }
  assert(r.r_xmin == iter->rs_xmin);
  assert(r.r_xsiz >= iter->rs_xsiz);

  /* Remove blocks from this strip. */
  iter = rects_strip_removey(piter,iter,r.r_ymin,r.r_ysiz);

  if (!iter->rs_blkc) {
   /* Special case: the strip is now gone.
    * Continue by removing parts from the next strip. */
   *piter = iter->rs_chain.le_next;
   free(iter);
   iter = *piter;
  } else {
   /* Try to merge this strip with its neighbors. */
   if (piter != &self->r_strips &&
       rects_strip_mergex(COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next)))
       iter = *piter;
   if (!rects_strip_mergex(iter))
        piter = &iter->rs_chain.le_next;
  }
  /* subtract what we removed from the strip. */
  assert(r.r_xsiz >= (strip_end-r.r_xmin));
  if (r.r_xsiz == (strip_end-r.r_xmin)) break;
  r.r_xsiz -= (strip_end-r.r_xmin);
  r.r_xmin  = strip_end;
 }
}


/* Finalize the given rects vector. */
INTERN void KCALL
rects_fini(struct rects *__restrict self) {
 struct rect_strip *iter,*next;
 iter = self->r_strips;
 while (iter) {
  next = iter->rs_chain.le_next;
  free(iter);
  iter = next;
 }
}



INTERN void KCALL
rects_duplicate_and_move(struct rects *__restrict dst,
                         struct rects const *__restrict src,
                         int x_offset, int y_offset) {
 struct rect_strip *iter,**pdst,*copy;
 pdst = &dst->r_strips;
 for (iter = src->r_strips; iter;
      iter = iter->rs_chain.le_next) {
  copy = RECT_STRIP_DUP(iter);
  copy->rs_xmin += x_offset;
  if (y_offset) {
   unsigned int i;
   for (i = 0; i < copy->rs_blkc; ++i)
       copy->rs_blkv[i].rb_ymin += y_offset;
  }
  *pdst = copy;
  pdst = &copy->rs_chain.le_next;
 }
 *pdst = NULL;
}

INTERN void KCALL
rects_move(struct rects *__restrict self,
           int x_offset, int y_offset) {
 struct rect_strip *iter;
 for (iter = self->r_strips; iter;
      iter = iter->rs_chain.le_next) {
  iter->rs_xmin += x_offset;
  if (y_offset) {
   unsigned int i;
   for (i = 0; i < iter->rs_blkc; ++i)
       iter->rs_blkv[i].rb_ymin += y_offset;
  }
 }
}



DECL_END

#endif /* !GUARD_APPS_WM_RECT_C */

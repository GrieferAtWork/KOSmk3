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
#ifndef GUARD_APPS_WMS_RECT_C
#define GUARD_APPS_WMS_RECT_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/minmax.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <string.h>
#include <malloc.h>
#include <assert.h>
#include <stdbool.h>

#include "rect.h"
#include <syslog.h>
#include <kos/heap.h>

DECL_BEGIN

INTERN struct rect WMCALL
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
 /*result.r_ysiz = 0;*/ /* One ZERO is enough. */
 return result;
}


#if 1

#if 0
#define rects_assert(self) (void)0
#else
PRIVATE void WMCALL
rects_assert(struct rects *__restrict self) {
 struct rect_strip *iter;
 unsigned int i;
 heap_validate_all();
 iter = self->r_strips;
 for (; iter; iter = iter->rs_chain.le_next) {
  assert(iter->rs_xsiz != 0);
  assertf(!iter->rs_chain.le_next ||
           iter->rs_chain.le_next->rs_xmin >=
           iter->rs_xmin+iter->rs_xsiz,
          "iter->rs_xmin                   = %u\n"
          "iter->rs_xsiz                   = %u\n"
          "iter->rs_xmin+iter->rs_xsiz     = %u\n"
          "iter->rs_chain.le_next->rs_xmin = %u\n"
          ,iter->rs_xmin
          ,iter->rs_xsiz
          ,iter->rs_xmin+iter->rs_xsiz
          ,iter->rs_chain.le_next->rs_xmin);
  assert(iter->rs_blkc != 0);
  for (i = 0; i < iter->rs_blkc-1; ++i) {
   assert(iter->rs_blkv[i].rb_ysiz != 0);
   assertf(iter->rs_blkv[i+1].rb_ymin >
           iter->rs_blkv[i].rb_ymin+
           iter->rs_blkv[i].rb_ysiz,
          "iter->rs_blkv[%u].rb_ymin                           = %u\n"
          "iter->rs_blkv[%u].rb_ymin                           = %u\n"
          "iter->rs_blkv[%u].rb_ysiz                           = %u\n"
          "iter->rs_blkv[%u].rb_ymin+iter->rs_blkv[%u].rb_ysiz = %u\n"
          ,i+1,iter->rs_blkv[i+1].rb_ymin
          ,i,iter->rs_blkv[i].rb_ymin
          ,i,iter->rs_blkv[i].rb_ysiz
          ,i,i,iter->rs_blkv[i].rb_ymin+iter->rs_blkv[i].rb_ysiz);
  }
  assert(iter->rs_blkv[iter->rs_blkc-1].rb_ysiz != 0);
 }
}
#endif

PRIVATE void WMCALL
rects_split_strip(struct rect_strip *__restrict self,
                  unsigned int offset_from_base) {
 struct rect_strip *copy;
 assertf(offset_from_base < self->rs_xsiz,
         "offset_from_base = %u\n"
         "self->rs_xsiz    = %u\n",
         offset_from_base,
         self->rs_xsiz);
 heap_validate_all();
 copy = RECT_STRIP_DUP(self);
 copy->rs_xmin += offset_from_base;
 copy->rs_xsiz -= offset_from_base;
 self->rs_xsiz  = offset_from_base;
 self->rs_chain.le_next = copy;
}

PRIVATE bool WMCALL
rects_split_merge_next(struct rect_strip *__restrict self) {
 struct rect_strip *next = self->rs_chain.le_next;
 if (!next) goto nope;
 if (next->rs_xmin != self->rs_xmin+self->rs_xsiz) goto nope;
 if (next->rs_blkc != self->rs_blkc) goto nope;
 if (memcmp(next->rs_blkv,self->rs_blkv,
            next->rs_blkc*sizeof(struct rect_block)))
     goto nope;
 /* Yes, we can merge them! */
 self->rs_xsiz += next->rs_xsiz;
 self->rs_chain.le_next = next->rs_chain.le_next;
 free(next);
 return true;
nope:
 return false;
}

PRIVATE struct rect_strip *WMCALL
rects_split_insert_block_at(struct rect_strip **__restrict pself,
                            struct rect_strip *__restrict self,
                            unsigned int index,
                            unsigned int ymin, unsigned int ysiz) {
 unsigned int usable;
 assert(*pself == self);
 assert(index <= self->rs_blkc);
 assertf(index == self->rs_blkc ||
         self->rs_blkv[index].rb_ymin > ymin+ysiz,
         "Instead of an insert, this should have been a merge with the successor\n"
         "index                        = %u\n"
         "self->rs_blkv[index].rb_ymin = %u\n"
         "ymin                         = %u\n"
         "ysiz                         = %u\n"
         "ymin+ysiz                    = %u\n",
         index,self->rs_blkv[index].rb_ymin,
         ymin,ysiz,ymin+ysiz);
 assertf(!index ||
          self->rs_blkv[index-1].rb_ymin+
          self->rs_blkv[index-1].rb_ysiz < ymin,
          "Instead of an insert, this should have been a merge with the predecessor\n"
          "index                                                         = %u\n"
          "self->rs_blkv[index-1].rb_ymin                                = %u\n"
          "self->rs_blkv[index-1].rb_ysiz                                = %u\n"
          "self->rs_blkv[index-1].rb_ymin+self->rs_blkv[index-1].rb_ysiz = %u\n"
          "ymin                                                          = %u\n"
          ,index
          ,self->rs_blkv[index-1].rb_ymin
          ,self->rs_blkv[index-1].rb_ysiz
          ,self->rs_blkv[index-1].rb_ymin+self->rs_blkv[index-1].rb_ysiz
          ,ymin);
 usable = (malloc_usable_size(self)-
           offsetof(struct rect_strip,rs_blkv))/
           sizeof(struct rect_block);
 assert(usable >= self->rs_blkc);
 if (usable == self->rs_blkc) {
  /* Must reallocate the block to gain more memory. */
  self = (struct rect_strip *)Xrealloc(self,
                                       offsetof(struct rect_strip,rs_blkv)+
                                      (usable+1)*sizeof(struct rect_block));
  *pself = self;
 }
 assert(index <= self->rs_blkc);
 heap_validate_all();
 /* Adjust to move around following blocks. */
 memmove(&self->rs_blkv[index+1],
         &self->rs_blkv[index],
         (self->rs_blkc-index)*
          sizeof(struct rect_block));
 heap_validate_all();
 /* Setup and track the new data block. */
 self->rs_blkv[index].rb_ymin = ymin;
 self->rs_blkv[index].rb_ysiz = ysiz;
 ++self->rs_blkc;
 heap_validate_all();
 return self;
}

PRIVATE struct rect_strip *WMCALL
rects_split_insert_block(struct rect_strip **__restrict pself,
                         struct rect_strip *__restrict self,
                         unsigned int ymin, unsigned int ysiz) {
 unsigned int i = 0,usable;
 assert(ysiz != 0);
continue_at_i:
 for (; i < self->rs_blkc; ++i) {
  unsigned int block_end;
  unsigned int insert_height;
  heap_validate_all();
  block_end = self->rs_blkv[i].rb_ymin+self->rs_blkv[i].rb_ysiz;
  assert(i == self->rs_blkc-1 ||
         block_end < self->rs_blkv[i+1].rb_ymin);
  assert(i == 0 ||
         self->rs_blkv[i].rb_ymin >
         self->rs_blkv[i-1].rb_ymin+
         self->rs_blkv[i-1].rb_ysiz);
  assert(i == 0 ||
         ymin >
         self->rs_blkv[i-1].rb_ymin+
         self->rs_blkv[i-1].rb_ysiz);
  if (block_end < ymin) {
   /* Skip strips before our rect. */
   continue;
  }
  if (ymin < self->rs_blkv[i].rb_ymin) {
   /* Either insert a new block, or extend the block at `i' downwards. */
   if (ymin+ysiz >= self->rs_blkv[i].rb_ymin) {
    /* Extend the block. */
    insert_height = self->rs_blkv[i].rb_ymin-ymin;
    heap_validate_all();
    self->rs_blkv[i].rb_ymin -= insert_height;
    self->rs_blkv[i].rb_ysiz += insert_height;
    heap_validate_all();
   } else {
    /* Insert a new block. */
    heap_validate_all();
    self = rects_split_insert_block_at(pself,self,i,ymin,ysiz);
    heap_validate_all();
    /* Because we couldn't extend an existing block,
     * we already know that the insert-height is smaller
     * than the range leading up to the next block.
     * Because of that, the insert must have been the full size. */
    goto done;
   }
  }
  assert(ymin >= self->rs_blkv[i].rb_ymin);
  /* Deal with overlap. */
  insert_height = block_end - ymin;
  if (insert_height >= ysiz)
      goto done; /* The remainder was an overlap. */
  ymin += insert_height;
  ysiz -= insert_height;
  assert(ymin == block_end);
  /* Now we can extend this block upwards. */
  if (i == self->rs_blkc-1) {
   /* Last block. -> We can extend it into the infinite. */
   heap_validate_all();
   self->rs_blkv[i].rb_ysiz += ysiz;
   heap_validate_all();
   goto done;
  } else {
   assert(self->rs_blkc >= 2);
   assert(i <= self->rs_blkc-2);
   assert(self->rs_blkv[i+1].rb_ymin > block_end);
   /* Check how far we can extend this block */
   insert_height = self->rs_blkv[i+1].rb_ymin - block_end;
   if (insert_height > ysiz)
       insert_height = ysiz;
   heap_validate_all();
   self->rs_blkv[i].rb_ysiz += insert_height;
   block_end                += insert_height;
   heap_validate_all();
   ymin += insert_height;
   ysiz -= insert_height;
   assert(block_end == self->rs_blkv[i].rb_ymin+self->rs_blkv[i].rb_ysiz);
   assert(block_end <= self->rs_blkv[i+1].rb_ymin);
   if (block_end == self->rs_blkv[i+1].rb_ymin) {
    /* Merge this block with its successor. */
    heap_validate_all();
    self->rs_blkv[i].rb_ysiz += self->rs_blkv[i+1].rb_ysiz;
    heap_validate_all();
    assert(self->rs_blkc >= 2);
    --self->rs_blkc;
    heap_validate_all();
    memmove(&self->rs_blkv[i+1],
            &self->rs_blkv[i+2],
            (self->rs_blkc-(i+1))*
             sizeof(struct rect_block));
    heap_validate_all();
    if (!ysiz) goto done;
    goto continue_at_i;
   }
   if (!ysiz) goto done;
  }
 }
 assert(i == self->rs_blkc);
 /* Append a new block for the remainder at the end. */
 usable = (malloc_usable_size(self)-
           offsetof(struct rect_strip,rs_blkv))/
           sizeof(struct rect_block);
 assert(usable >= self->rs_blkc);
 if (usable == self->rs_blkc) {
  /* Must reallocate the block to gain more memory. */
  self = (struct rect_strip *)Xrealloc(self,
                                       offsetof(struct rect_strip,rs_blkv)+
                                      (usable+1)*sizeof(struct rect_block));
  *pself = self;
 }
 /* Setup and track the new data block. */
 heap_validate_all();
 self->rs_blkv[i].rb_ymin = ymin;
 self->rs_blkv[i].rb_ysiz = ysiz;
 ++self->rs_blkc;
 heap_validate_all();
done:
 return self;
}

PRIVATE struct rect_strip *WMCALL
rects_split_remove_block(struct rect_strip **__restrict pself,
                         struct rect_strip *__restrict self,
                         unsigned int ymin, unsigned int ysiz) {
 unsigned int i = 0;
 assert(ysiz != 0);
continue_at_i:
 for (; i < self->rs_blkc; ++i) {
  unsigned int block_end;
  unsigned int remove_height;
  block_end = self->rs_blkv[i].rb_ymin+self->rs_blkv[i].rb_ysiz;
  assert(i == self->rs_blkc-1 ||
         block_end < self->rs_blkv[i+1].rb_ymin);
  if (block_end <= ymin) {
   /* Skip strips before our rect. */
   continue;
  }
  if (ymin < self->rs_blkv[i].rb_ymin) {
   /* Skip unused space prior to the block. */
   remove_height = self->rs_blkv[i].rb_ymin - ymin;
   if (remove_height >= ysiz)
       goto done; /* Block wasn't mapped. */
   ymin += remove_height;
   ysiz -= remove_height;
  } else if (ymin > self->rs_blkv[i].rb_ymin) {
   /* The unmap only starts partially into the block.
    * If the unmap extends to, or past the end of the block,
    * simply trim the block at the back, otherwise we must
    * split the block in 2 so we can have a gap between the
    * two of them! */
   remove_height = block_end - ymin;
   if (remove_height <= ysiz) {
    /* The block can be truncated. */
    self->rs_blkv[i].rb_ysiz -= remove_height;
    ysiz -= remove_height;
    if (ysiz == 0) goto done;
    ymin += remove_height;
    continue;
   }
   /* The block needs to be split. */
   assert(block_end > (ymin+ysiz));
   assert(i == self->rs_blkc-1 || block_end < self->rs_blkv[i+1].rb_ymin);
   self->rs_blkv[i].rb_ysiz = ymin - self->rs_blkv[i].rb_ymin;
   self = rects_split_insert_block_at(pself,self,i+1,ymin+ysiz,
                                      block_end - (ymin+ysiz));
   /* Since we know of an upper portion that shouldn't
    * be unmapped, we know we're done after this */
   goto done;
  }
  assert(self->rs_blkv[i].rb_ymin == ymin);
  /* Trim the block at the front, or remove it completely. */
  remove_height = self->rs_blkv[i].rb_ysiz;
  if (ysiz >= remove_height) {
   /* Remove the block completely. */
   assert(self->rs_blkc >= 1);
   --self->rs_blkc;
   memmove(&self->rs_blkv[i],
           &self->rs_blkv[i+1],
           (self->rs_blkc-i)*
            sizeof(struct rect_block));
   ysiz -= remove_height;
   if (!ysiz) goto done;
   ymin += remove_height;
   goto continue_at_i;
  } else {
   /* Only need to trim the block. */
   self->rs_blkv[i].rb_ymin += ysiz;
   self->rs_blkv[i].rb_ysiz -= ysiz;
   goto done;
  }
 }
done:
 return self;
}

INTERN void WMCALL
rects_insert(struct rects *__restrict self, struct rect r) {
 struct rect_strip **piter,*iter;
 struct rect_strip *insert;
 rects_assert(self);
 if (!r.r_xsiz || !r.r_ysiz) goto done;
again:
 piter = &self->r_strips;
 while ((iter = *piter) != NULL) {
  unsigned int strip_end;
  unsigned int insert_width;
  strip_end = iter->rs_xmin+iter->rs_xsiz;
  rects_assert(self);
  if (strip_end <= r.r_xmin) {
   /* Skip strips before our rect. */
   piter = &iter->rs_chain.le_next;
   continue;
  }
  if (r.r_xmin < iter->rs_xmin) {
   /* Must insert a new strip before `iter' */
   insert_width = iter->rs_xmin-r.r_xmin;
   if (insert_width > r.r_xsiz)
       insert_width = r.r_xsiz;
   insert = RECT_STRIP_ALLOC(1);
   insert->rs_blkc            = 1;
   insert->rs_xmin            = r.r_xmin;
   insert->rs_xsiz            = insert_width;
   insert->rs_blkv[0].rb_ymin = r.r_ymin;
   insert->rs_blkv[0].rb_ysiz = r.r_ysiz;
   rects_assert(self);
   /* Insert the new strip. */
   insert->rs_chain.le_next = iter;
   *piter = insert;
   r.r_xsiz -= insert_width;
   rects_assert(self);
   if (!r.r_xsiz) {
    rects_split_merge_next(insert);
    rects_assert(self);
    goto done;
   }
   r.r_xmin += insert_width;
   piter = &insert->rs_chain.le_next;
   iter = *piter;
   rects_assert(self);
  } else if (r.r_xmin > iter->rs_xmin) {
   /* Must split the strip so we start where _it_ starts. */
   rects_assert(self);
   rects_split_strip(iter,r.r_xmin-iter->rs_xmin);
   piter = &iter->rs_chain.le_next;
   iter = *piter;
   rects_assert(self);
  }
  assert(r.r_xmin == iter->rs_xmin);
  assert(iter->rs_xmin+iter->rs_xsiz == strip_end);
  insert_width = iter->rs_xsiz;
  if (insert_width > r.r_xsiz)
      insert_width = r.r_xsiz;
  if (insert_width < iter->rs_xsiz) {
   /* Split again so our insert-width matches the strip width. */
   rects_split_strip(iter,insert_width);
   rects_assert(self);
  }
  assert(r.r_xmin == iter->rs_xmin);
  assert(insert_width == iter->rs_xsiz);
  /* Insert the y-block */
  rects_assert(self);
  iter = rects_split_insert_block(piter,iter,r.r_ymin,r.r_ysiz);
  rects_assert(self);
  if (piter != &self->r_strips &&
      rects_split_merge_next(COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next))) {
   iter = COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next);
   rects_assert(self);
   if (rects_split_merge_next(iter)) {
    rects_assert(self);
    r.r_xsiz -= insert_width;
    if (!r.r_xsiz) goto done;
    r.r_xmin += insert_width;
    /* *piter got invalidated and we can't recover it...
     * However, our work didn't get lost, so the next iteration will
     * work on a smaller insert-rect, meaning we'll finish eventually! */
    goto again;
   }
   rects_assert(self);
   r.r_xsiz -= insert_width;
   if (!r.r_xsiz) goto done;
   r.r_xmin += insert_width;
  } else {
   if (!rects_split_merge_next(iter))
        piter = &iter->rs_chain.le_next;
   rects_assert(self);
   r.r_xsiz -= insert_width;
   if (!r.r_xsiz) goto done;
   r.r_xmin += insert_width;
  }
 }
 rects_assert(self);
 /* Append a new strip at the end. */
 insert = RECT_STRIP_ALLOC(1);
 insert->rs_blkc            = 1;
 insert->rs_xmin            = r.r_xmin;
 insert->rs_xsiz            = r.r_xsiz;
 insert->rs_blkv[0].rb_ymin = r.r_ymin;
 insert->rs_blkv[0].rb_ysiz = r.r_ysiz;
 insert->rs_chain.le_next   = NULL;
 *piter = insert;
 rects_assert(self);
 /* Try to merge the new strip in the very end with its predecessor. */
 if (piter != &self->r_strips)
     rects_split_merge_next(COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next));
 rects_assert(self);
done:
 rects_assert(self);
}

INTERN void WMCALL
rects_remove(struct rects *__restrict self, struct rect r) {
 struct rect_strip **piter,*iter;
 rects_assert(self);
 if (!r.r_xsiz || !r.r_ysiz) goto done;
again:
 piter = &self->r_strips;
 while ((iter = *piter) != NULL) {
  unsigned int strip_end;
  unsigned int remove_width;
  strip_end = iter->rs_xmin+iter->rs_xsiz;
  rects_assert(self);
  if (strip_end <= r.r_xmin) {
   /* Skip strips before our rect. */
   piter = &iter->rs_chain.le_next;
   continue;
  }
  if (r.r_xmin < iter->rs_xmin) {
   /* Skip unmapped portion. */
   remove_width = iter->rs_xmin - r.r_xmin;
   if (remove_width > r.r_xsiz)
       remove_width = r.r_xsiz;
   r.r_xsiz -= remove_width;
   if (!r.r_xsiz) goto done;
   r.r_xmin += remove_width;
   rects_assert(self);
  } else if (r.r_xmin > iter->rs_xmin) {
   /* Must split the strip so our rect starts where it starts. */
   rects_split_strip(iter,r.r_xmin-iter->rs_xmin);
   piter = &iter->rs_chain.le_next;
   iter = *piter;
   rects_assert(self);
  }
  assert(r.r_xmin == iter->rs_xmin);
  remove_width = iter->rs_xsiz;
  if (remove_width > r.r_xsiz)
      remove_width = r.r_xsiz;
  if (remove_width < iter->rs_xsiz) {
   /* Split again so our insert-width matches the strip width. */
   rects_split_strip(iter,remove_width);
   rects_assert(self);
  }
  /* Remove the y-block */
  rects_assert(self);
  iter = rects_split_remove_block(piter,iter,r.r_ymin,r.r_ysiz);
  assert(*piter == iter);
  if (!iter->rs_blkc) {
   /* Must remove this strip. */
   *piter = iter->rs_chain.le_next;
   free(iter);
  } else if ((rects_assert(self),
              piter != &self->r_strips &&
              rects_split_merge_next(COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next)))) {
   rects_assert(self);
   iter = COMPILER_CONTAINER_OF(piter,struct rect_strip,rs_chain.le_next);
   if (rects_split_merge_next(iter)) {
    r.r_xsiz -= remove_width;
    if (!r.r_xsiz) goto done;
    r.r_xmin += remove_width;
    /* *piter got invalidated and we can't recover it...
     * However, our work didn't get lost, so the next iteration will
     * work on a smaller remove-rect, meaning we'll finish eventually! */
    goto again;
   }
  } else {
   rects_assert(self);
   if (!rects_split_merge_next(iter))
        piter = &iter->rs_chain.le_next;
  }
  rects_assert(self);
  r.r_xsiz -= remove_width;
  if (!r.r_xsiz) goto done;
  r.r_xmin += remove_width;
 }
done:
 rects_assert(self);
}

#else

/* Split `self' at offset `x_offset_from_self'
 * @return: * : The upper half after the split. */
PRIVATE struct rect_strip *WMCALL
rects_strip_split(struct rect_strip *__restrict self,
                  unsigned int x_offset_from_self) {
 struct rect_strip *copy;
 assertf(x_offset_from_self < self->rs_xsiz,
         "x_offset_from_self = %u\n"
         "self->rs_xsiz      = %u\n",
         x_offset_from_self,
         self->rs_xsiz);
 heap_validate_all();
 copy = RECT_STRIP_DUP(self);
 copy->rs_xmin += x_offset_from_self;
 copy->rs_xsiz -= x_offset_from_self;
 self->rs_xsiz  = x_offset_from_self;
 self->rs_chain.le_next = copy;
 return copy;
}

/* Try try to merge `self' with its successor. */
PRIVATE bool WMCALL
rects_strip_mergex(struct rect_strip *__restrict self) {
 struct rect_strip *next = self->rs_chain.le_next;
 if unlikely(!next) goto nope; /* No successor. */
 heap_validate_all();
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
 heap_validate_all();
 free(next);
 return true;
nope:
 return false;
}


/* Try try to merge block index `block_index' with its successor. */
PRIVATE bool WMCALL
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
PRIVATE struct rect_strip *WMCALL
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
 assert(!block_index || ymin != 0);
 assert(ysiz != 0);
 assertf(!self->rs_blkc || block_index == self->rs_blkc-1 ||
          ymin+ysiz <= self->rs_blkv[block_index].rb_ymin,
          "self->rs_blkv[%u].rb_ymin = %u\n"
          "ymin                      = %u\n"
          "ysiz                      = %u\n"
          "ymin+ysiz                 = %u\n",
          block_index,self->rs_blkv[block_index].rb_ymin,
          ymin,ysiz,ymin+ysiz);
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


PRIVATE struct rect_strip *WMCALL
rects_strip_insert(struct rect_strip **__restrict pself,
                   struct rect_strip *__restrict self,
                   unsigned int ymin, unsigned int ysiz) {
 unsigned int block_index;
 unsigned int yend = ymin+ysiz;
 /* Figure out where to insert the block, while also
  * extending any blocks with which we are overlapping. */
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
   assert(insert_size != 0);
   if (ymin+insert_size == self->rs_blkv[block_index].rb_ymin) {
    /* Extend the next block. */
    self->rs_blkv[block_index].rb_ymin -= insert_size;
    self->rs_blkv[block_index].rb_ysiz += insert_size;
   } else {
    /* Must insert a new block. */
    self = rects_strip_inserty(pself,self,block_index,
                               ymin,insert_size);
    assert(ysiz == insert_size);
   }
   ymin += insert_size;
   ysiz -= insert_size;
   if (!ysiz) goto done;
  }
  assert(ymin >= self->rs_blkv[block_index].rb_ymin);
  /* Skip overlap. */
  if (yend <= (self->rs_blkv[block_index].rb_ymin+
               self->rs_blkv[block_index].rb_ysiz))
      goto done;
  ymin = (self->rs_blkv[block_index].rb_ymin+
          self->rs_blkv[block_index].rb_ysiz);
  ysiz = yend-ymin;
  ++block_index;
 }
 /* Append a new entry at the end. */
 self = rects_strip_inserty(pself,self,block_index,ymin,ysiz);
 if (block_index != 0)
     rects_strip_mergey(self,block_index-1);
done:
 return self;
}

/* Modify the rects vector to cover, or no longer cover the given rect `r' */
INTERN void WMCALL
rects_insert(struct rects *__restrict self, struct rect r) {
 struct rect_strip *next,*new_strip,**pnext;
 unsigned int x_end = r.r_xmin+r.r_xsiz;
 if (!r.r_xsiz || !r.r_ysiz) return;
 heap_validate_all();
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
  goto done;
 }
 /* Find the string before which to insert this new one. */
 pnext = &self->r_strips;
 while (next) {
  if (next->rs_xmin+next->rs_xsiz < r.r_xmin) {
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
        rects_strip_mergex(COMPILER_CONTAINER_OF(pnext,struct rect_strip,rs_chain.le_next)))
        next = *pnext;
   else pnext = &new_strip->rs_chain.le_next;
   assert(*pnext == next);
   if (!r.r_xsiz) goto done;
  }
  assert(r.r_xmin >= next->rs_xmin);
  if (r.r_xmin < next->rs_xmin+next->rs_xsiz) {
   unsigned int overlap;
   heap_validate_all();
   overlap = (next->rs_xmin+next->rs_xsiz) - r.r_xmin;
   if (overlap > r.r_xsiz)
       overlap = r.r_xsiz;
   assert(overlap != 0);
   if (r.r_xmin != next->rs_xmin) {
    rects_strip_split(next,r.r_xmin-next->rs_xmin);
    pnext = &next->rs_chain.le_next;
    next = *pnext;
   }
   heap_validate_all();
   assert(next->rs_xmin == r.r_xmin);
   assert(overlap <= next->rs_xsiz);
   if (overlap != next->rs_xsiz)
       rects_strip_split(next,overlap);
   assert(next->rs_xmin == r.r_xmin);
   assert(next->rs_xsiz == overlap);
   next = rects_strip_insert(pnext,next,r.r_ymin,r.r_ysiz);
   assert(r.r_xmin+overlap >= next->rs_xmin+next->rs_xsiz);
   heap_validate_all();
   if (pnext != &self->r_strips &&
       rects_strip_mergex(COMPILER_CONTAINER_OF(pnext,struct rect_strip,rs_chain.le_next)))
       next = *pnext;
   if (!rects_strip_mergex(next)) {
    pnext = &next->rs_chain.le_next;
    next  = *pnext;
   }
   if (r.r_xsiz == overlap)
       goto done;
   r.r_xmin += overlap;
   r.r_xsiz -= overlap;
   continue;
  }
  pnext = &next->rs_chain.le_next;
  next = *pnext;
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
done:
 heap_validate_all();
}



PRIVATE struct rect_strip *WMCALL
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
  assertf(block_index == self->rs_blkc-1 ||
          self->rs_blkv[block_index+1].rb_ymin > block_end,
          "Illegal overlap:\n"
          "self->rs_blkv[%u] = { %u + %u = %u }\n"
          "self->rs_blkv[%u] = { %u + %u = %u }\n",
          block_index,
          self->rs_blkv[block_index].rb_ymin,
          self->rs_blkv[block_index].rb_ysiz,
          self->rs_blkv[block_index].rb_ymin+
          self->rs_blkv[block_index].rb_ysiz,
          block_index+1,
          self->rs_blkv[block_index+1].rb_ymin,
          self->rs_blkv[block_index+1].rb_ysiz,
          self->rs_blkv[block_index+1].rb_ymin+
          self->rs_blkv[block_index+1].rb_ysiz);
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
   assert(higher_half_size != 0);
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



INTERN void WMCALL
rects_remove(struct rects *__restrict self, struct rect r) {
 struct rect_strip *iter,**piter;
 unsigned int x_end = r.r_xmin+r.r_xsiz;
 unsigned int strip_end;
 if (!r.r_xsiz || !r.r_ysiz) return;
#if 1
 assert((int)r.r_xsiz >= 0);
 assert((int)r.r_ysiz >= 0);
#endif
 heap_validate_all();
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
   if (ignore_size >= r.r_xsiz)
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
 heap_validate_all();
}
#endif


/* Finalize the given rects vector. */
INTERN void WMCALL
rects_fini(struct rects *__restrict self) {
 struct rect_strip *iter,*next;
 rects_assert(self);
 iter = self->r_strips;
 while (iter) {
  next = iter->rs_chain.le_next;
  free(iter);
  iter = next;
 }
}



INTERN void WMCALL
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

INTERN void WMCALL
rects_move(struct rects *__restrict self,
           int x_offset, int y_offset) {
 struct rect_strip *iter;
 rects_assert(self);
 for (iter = self->r_strips; iter;
      iter = iter->rs_chain.le_next) {
  iter->rs_xmin += x_offset;
  if (y_offset) {
   unsigned int i;
   for (i = 0; i < iter->rs_blkc; ++i) {
    iter->rs_blkv[i].rb_ymin += y_offset;
   }
  }
 }
 rects_assert(self);
}

DECL_END

#endif /* !GUARD_APPS_WMS_RECT_C */

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
#ifndef GUARD_APPS_WMS_WINDOWMAP_C
#define GUARD_APPS_WMS_WINDOWMAP_C 1
#define _KOS_SOURCE 1
#define _EXCEPT_SOURCE 1

#include <hybrid/compiler.h>
#include <hybrid/list/list.h>
#include <kos/types.h>
#include <wm/api.h>
#include <malloc.h>
#include <except.h>
#include <syslog.h>

#include "window.h"
#include "display.h"

DECL_BEGIN

INTERN void WMCALL
WindowMap_Init(WindowMap *__restrict self) {
 self->wm_size     = 0;
 self->wm_mask     = 3;
 self->wm_nextid   = 1;
 self->wm_map      = (Window **)Xcalloc(4,sizeof(Window *));
 self->wm_overflow = false;
}

INTERN ATTR_NOTHROW void WMCALL
WindowMap_Fini(WindowMap *__restrict self_) {
 WindowMap *EXCEPT_VAR self = self_;
 wms_window_id_t EXCEPT_VAR i;
 for (i = 0; i <= self->wm_mask; ++i) {
  Window *EXCEPT_VAR iter = self->wm_map[i];
  while (iter) {
   Window *EXCEPT_VAR next;
   Display *EXCEPT_VAR disp = iter->w_display;
   next = iter->w_idchain.le_next;
   atomic_rwlock_write(&disp->d_lock);
   TRY {
    /* Destroy the window. */
    Window_DestroyUnlocked(iter);
    atomic_rwlock_endwrite(&disp->d_lock);
   } EXCEPT (EXCEPT_EXECUTE_HANDLER) {
    atomic_rwlock_endwrite(&disp->d_lock);
    error_printf("Failed to destroy window %p\n",iter);
    error_handled();
   }
   iter = next;
  }
 }
 free(self->wm_map);
}

INTERN ATTR_NOTHROW wms_window_id_t WMCALL
WindowMap_MakeID(WindowMap *__restrict self) {
 wms_window_id_t i,result;
 if (!self->wm_overflow) {
  result = self->wm_nextid++;
  if likely(result != 0)
     return result;
  self->wm_overflow = true;
 }
next_id:
 result = self->wm_nextid++;
 /* After the first overflow, we must search for IDs already in use. */
 for (i = 0; i <= self->wm_mask; ++i) {
  Window *iter;
  iter = self->wm_map[i];
  for (; iter; iter = iter->w_idchain.le_next) {
   if (iter->w_id == result)
       goto next_id;
  }
 }
 /* Id isn't being used. */
 return result;
}

INTERN void WMCALL
WindowMap_Insert(WindowMap *__restrict self,
                 Window *__restrict win) {
 LIST_HEAD(Window) *bucket;
 if (self->wm_size/2 >= self->wm_mask) {
  /* Increase the hash mask. */
  size_t new_mask,min_mask,i;
  LIST_HEAD(Window) *new_map;
  new_mask = self->wm_mask;
  min_mask = (self->wm_size*3)/2;
  while (new_mask < min_mask)
      new_mask = (new_mask << 1) | 1;
  if (new_mask < 15) new_mask = 15;
  new_map = (LIST_HEAD(Window) *)Xcalloc(new_mask+1,
                                         sizeof(LIST_HEAD(Window)));
  if (self->wm_map) {
   LIST_HEAD(Window) iter;
   LIST_HEAD(Window) next;
   /* Rehash the existing map. */
   for (i = 0; i <= self->wm_mask; ++i) {
    iter = self->wm_map[i];
    while (iter) {
     next = iter->w_idchain.le_next;
     bucket = &new_map[iter->w_id & new_mask];
     LIST_INSERT(*bucket,iter,w_idchain);
     iter = next;
    }
   }
   free(self->wm_map);
  }
  /* Save the new map. */
  self->wm_map  = new_map;
  self->wm_mask = new_mask;
 }
 /* Insert the given window into its bucket. */
 bucket = &self->wm_map[win->w_id & self->wm_mask];
 LIST_INSERT(*bucket,win,w_idchain);
 ++self->wm_size;
}

INTERN ATTR_NOTHROW void WMCALL
WindowMap_Remove(WindowMap *__restrict self,
                 Window *__restrict win) {
 assert(self->wm_size != 0);
 LIST_REMOVE(win,w_idchain);
 --self->wm_size;
}

/* @return: NULL: Invalid window ID. */
INTERN ATTR_NOTHROW Window *WMCALL
WindowMap_Lookup(WindowMap *__restrict self,
                 wms_window_id_t id) {
 Window *result;
 result = self->wm_map[id & self->wm_mask];
 while (result && result->w_id != id)
        result = result->w_idchain.le_next;
 if (!result)
      syslog(LOG_DEBUG,"Unknown window id %Iu\n",id);
 return result;
}


DECL_END

#endif /* !GUARD_APPS_WMS_WINDOWMAP_C */
